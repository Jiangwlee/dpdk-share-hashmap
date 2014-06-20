/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Bruce.Li <jiangwlee@163.com>, 2014
 */

/*
 * @ file
 * @ This is an variant of rte_hash.c 
 * @ The major change is that the memory zone of signatre and key are
 * @ independant from rte_hash structure. It supports dynamic increment.
 * @ Another change is that, now the key is a combination of key and data.
 */

#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/queue.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_memcpy.h>
#include <rte_prefetch.h>
#include <rte_branch_prediction.h>
#include <rte_memzone.h>
#include <rte_malloc.h>
#include <rte_tailq.h>
#include <rte_eal.h>
#include <rte_eal_memconfig.h>
#include <rte_per_lcore.h>
#include <rte_errno.h>
#include <rte_string_fns.h>
#include <rte_cpuflags.h>
#include <rte_log.h>
#include <rte_rwlock.h>
#include <rte_spinlock.h>

#include "share_rte_hash.h"


TAILQ_HEAD(rte_hash_list, rte_hash);

/* Hash function used if none is specified */
#ifdef RTE_MACHINE_CPUFLAG_SSE4_2
#include <rte_hash_crc.h>
#define DEFAULT_HASH_FUNC       rte_hash_crc
#else
#include <rte_jhash.h>
#define DEFAULT_HASH_FUNC       rte_jhash
#endif


rte_hash *
ShareRteHash::attach_hash_table(const char *name)
{
	struct rte_hash *h;
	struct rte_hash_list *hash_list;

	/* check that we have an initialised tail queue */
	if ((hash_list = RTE_TAILQ_LOOKUP_BY_IDX(RTE_TAILQ_HASH, rte_hash_list)) == NULL) {
		rte_errno = E_RTE_NO_TAILQ;
		return NULL;
	}

	rte_rwlock_read_lock(RTE_EAL_TAILQ_RWLOCK);
	TAILQ_FOREACH(h, hash_list, next) {
		if (strncmp(name, h->name, RTE_HASH_NAMESIZE) == 0)
			break;
	}
	rte_rwlock_read_unlock(RTE_EAL_TAILQ_RWLOCK);

	if (h == NULL)
		rte_errno = ENOENT;
	return h;
}

/*
 * @ Description:
 *   This function is based on rte_hash_create. It made following changes to
 *   support storing <key, value> in rte_hash :
 *     . The key_value_size = length of key + length of value
 *     . The memory zone of rte_hash, signature table and key_value table are
 *       independent now. So that it could support resize of hash table.
 *     . The maximum bulket entires is extended to 1024 now
 *
 * @ The overview of this rte_hash looks like fowlloing graphic:
 *                       +-----------+ 
 *                       |  rte_hash |
 *                       |-----------|        +-------------------+
 *                       |  sig_tbl  |------> |  signature table  | 
 *                       |-----------|        |-------------------|
 *                       |  key_tbl  |        | bucket locks array|
 *                       +-----------+        +-------------------+
 *                             |
 *                             |              <* The bucket locks array just follows sig_tbl *>
 *                             v
 *                             +---------------+
 *                             |   key table   |
 *                             +---------------+
 *
 */
rte_hash *
ShareRteHash::create_hash_table(const rte_hash_parameters *params)
{
	struct rte_hash *h = NULL;
    uint8_t *p_sig_tbl = NULL;
    uint8_t *p_key_value_tbl = NULL;
	uint32_t num_buckets, sig_bucket_size, key_value_size,
		hash_tbl_size, sig_tbl_size, key_value_tbl_size,
        bucket_locks_array_size;
	char hash_name[RTE_HASH_NAMESIZE];
	char sig_name[RTE_HASH_NAMESIZE];
	char key_value_name[RTE_HASH_NAMESIZE];
	struct rte_hash_list *hash_list;

	/* check that we have an initialised tail queue */
	if ((hash_list = 
	     RTE_TAILQ_LOOKUP_BY_IDX(RTE_TAILQ_HASH, rte_hash_list)) == NULL) {
		rte_errno = E_RTE_NO_TAILQ;
		return NULL;	
	}

	/* Check for valid parameters */
	if ((params == NULL) ||
			(params->entries > k_RTE_HASH_ENTRIES_MAX) ||
			(params->bucket_entries > k_RTE_HASH_BUCKET_ENTRIES_MAX) ||
			(params->entries < params->bucket_entries) ||
			!rte_is_power_of_2(params->entries) ||
			!rte_is_power_of_2(params->bucket_entries) ||
			(params->key_len == 0) || 
            (params->key_len > k_RTE_HASH_KEY_VALUE_LENGTH_MAX)) {
		rte_errno = EINVAL;
		RTE_LOG(ERR, HASH, "ShareRteHash::create_hash_table has invalid parameters\n");
		return NULL;
	}

	rte_snprintf(hash_name, sizeof(hash_name), "HT_%s", params->name);
	rte_snprintf(sig_name, sizeof(sig_name), "SIG_%s", params->name);
	rte_snprintf(key_value_name, sizeof(key_value_name), "KV_%s", params->name);

	/* Calculate hash dimensions */
	num_buckets = params->entries / params->bucket_entries;
	sig_bucket_size = align_size(params->bucket_entries *
				     sizeof(hash_sig_t), k_SIG_BUCKET_ALIGNMENT);
	key_value_size =  align_size(params->key_len, k_KEY_ALIGNMENT);
	hash_tbl_size = align_size(sizeof(struct rte_hash), CACHE_LINE_SIZE);
	sig_tbl_size = align_size(num_buckets * sig_bucket_size,
				  CACHE_LINE_SIZE);
	key_value_tbl_size = align_size(num_buckets * key_value_size *
				  params->bucket_entries, CACHE_LINE_SIZE);
    bucket_locks_array_size = align_size(num_buckets * sizeof(rte_rwlock_t),
                                         CACHE_LINE_SIZE);
	
	/* Total memory required for hash context */
	// mem_size = hash_tbl_size + sig_tbl_size + key_value_tbl_size;

	rte_rwlock_write_lock(RTE_EAL_TAILQ_RWLOCK);

	/* guarantee there's no existing */
	TAILQ_FOREACH(h, hash_list, next) {
		if (strncmp(params->name, h->name, RTE_HASH_NAMESIZE) == 0)
			break;
	}
	if (h != NULL)
		goto exit;

	h = (struct rte_hash *)rte_zmalloc_socket(hash_name, hash_tbl_size,
					   CACHE_LINE_SIZE, params->socket_id);
	if (h == NULL) {
		RTE_LOG(ERR, HASH, "memory allocation failed - hash table\n");
		goto exit;
	}

    /* put the bucket locks array just after sig_tbl */
    p_sig_tbl = (uint8_t *)rte_zmalloc_socket(sig_name, sig_tbl_size + bucket_locks_array_size,
            CACHE_LINE_SIZE, params->socket_id);

	if (p_sig_tbl == NULL) {
		RTE_LOG(ERR, HASH, "memory allocation failed - sig table\n");
		goto error;
	}

    p_key_value_tbl = (uint8_t *)rte_zmalloc_socket(key_value_name, key_value_tbl_size,
            CACHE_LINE_SIZE, params->socket_id);

	if (p_key_value_tbl == NULL) {
		RTE_LOG(ERR, HASH, "memory allocation failed - key value table\n");
		goto error;
	}

	/* Setup hash context */
	rte_snprintf(h->name, sizeof(h->name), "%s", params->name);
	h->entries = params->entries;
	h->bucket_entries = params->bucket_entries;
	h->key_len = params->key_len;
	h->hash_func_init_val = params->hash_func_init_val;
	h->num_buckets = num_buckets;
	h->bucket_bitmask = h->num_buckets - 1;
	h->sig_msb = 1 << (sizeof(hash_sig_t) * 8 - 1);
	h->sig_tbl = p_sig_tbl;
	h->sig_tbl_bucket_size = sig_bucket_size;
	h->key_tbl = p_key_value_tbl;
	h->key_tbl_key_size = key_value_size;
	h->hash_func = (params->hash_func == NULL) ?
		DEFAULT_HASH_FUNC : params->hash_func;

	TAILQ_INSERT_TAIL(hash_list, h, next);
    goto exit;

error:
    if (h) {
        rte_free(h);
        h = NULL;
    }

    if (p_sig_tbl) {
        rte_free(p_sig_tbl);
        h = NULL;
    }

exit:
	rte_rwlock_write_unlock(RTE_EAL_TAILQ_RWLOCK);

	return h;
}

void
ShareRteHash::free_hash_table(rte_hash *& h)
{
	if (h == NULL)
		return;

	RTE_EAL_TAILQ_REMOVE(RTE_TAILQ_HASH, rte_hash_list, h);
    
    if (h->sig_tbl)
        rte_free(h->sig_tbl);

    if (h->key_tbl)
        rte_free(h->key_tbl);

	rte_free(h);
    h = NULL;
}

#if 0
int32_t
ShareRteHash::add_key_with_hash(const rte_hash *h, 
				const void *key, hash_sig_t sig)
{
	RETURN_IF_TRUE(((h == NULL) || (key == NULL)), -EINVAL);

	hash_sig_t *sig_bucket;
	uint8_t *key_bucket;
	uint32_t bucket_index, i;
	int32_t pos;

	/* Get the hash signature and bucket index */
	sig |= h->sig_msb;
	bucket_index = sig & h->bucket_bitmask;
	sig_bucket = get_sig_tbl_bucket(h, bucket_index);
	key_bucket = get_key_tbl_bucket(h, bucket_index);

	/* Check if key is already present in the hash */
	for (i = 0; i < h->bucket_entries; i++) {
		if ((sig == sig_bucket[i]) &&
		    likely(memcmp(key, get_key_from_bucket(h, key_bucket, i),
				  h->key_len) == 0)) {
			return bucket_index * h->bucket_entries + i;
		}
	}

	/* Check if any free slot within the bucket to add the new key */
	pos = find_first(k_NULL_SIGNATURE, sig_bucket, h->bucket_entries);

	if (unlikely(pos < 0))
		return -ENOSPC;

	/* Add the new key to the bucket */
	sig_bucket[pos] = sig;
	rte_memcpy(get_key_from_bucket(h, key_bucket, pos), key, h->key_len);
	return bucket_index * h->bucket_entries + pos;
}


int32_t
ShareRteHash::del_key_with_hash(const rte_hash *h, 
				const void *key, hash_sig_t sig)
{
	RETURN_IF_TRUE(((h == NULL) || (key == NULL)), -EINVAL);

	hash_sig_t *sig_bucket;
	uint8_t *key_bucket;
	uint32_t bucket_index, i;

	/* Get the hash signature and bucket index */
	sig = sig | h->sig_msb;
	bucket_index = sig & h->bucket_bitmask;
	sig_bucket = get_sig_tbl_bucket(h, bucket_index);
	key_bucket = get_key_tbl_bucket(h, bucket_index);

	/* Check if key is already present in the hash */
	for (i = 0; i < h->bucket_entries; i++) {
		if ((sig == sig_bucket[i]) &&
		    likely(memcmp(key, get_key_from_bucket(h, key_bucket, i),
				  h->key_len) == 0)) {
			sig_bucket[i] = k_NULL_SIGNATURE;
			return bucket_index * h->bucket_entries + i;
		}
	}

	return -ENOENT;
}
#endif

