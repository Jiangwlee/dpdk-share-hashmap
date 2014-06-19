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


#ifndef _SHARE_RTE_HASH_H_
#define _ShARE_RTE_HASH_H_

#include <rte_hash.h>
#include <rte_memcpy.h>         /* for definition of CACHE_LINE_SIZE */

/* Macro to enable/disable run-time checking of function parameters */
#if defined(RTE_LIBRTE_HASH_DEBUG)
#define RETURN_IF_TRUE(cond, retval) do { \
	if (cond) return (retval); \
} while (0)
#else
#define RETURN_IF_TRUE(cond, retval)
#endif

class ShareRteHash {
    public:
        typedef uint32_t hash_sig_t;

        static const uint32_t k_RTE_HASH_ENTRIES_MAX          = (1 << 26);
        static const uint32_t k_RTE_HASH_BUCKET_ENTRIES_MAX   = 1024;
        static const uint32_t k_RTE_HASH_KEY_VALUE_LENGTH_MAX = 128;
        static const uint32_t k_RTE_HASH_NAMESIZE             = 32; 

        /* Signature bucket size is a multiple of this value */
        static const uint32_t k_SIG_BUCKET_ALIGNMENT = 16;

        /* Stoered key size is a multiple of this value */
        static const uint32_t k_KEY_ALIGNMENT = 16;

        /* The high bit is always set in real signatures */
        static const uint32_t k_NULL_SIGNATURE = 0;

    public:
        template<typename _KeyValue>
        int32_t add_key_value_with_hash(const rte_hash *h, const _KeyValue *key_value, hash_sig_t sig)
        {
        	RETURN_IF_TRUE(((h == NULL) || (key_value == NULL)), -EINVAL);
        
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
        		if (sig == sig_bucket[i]) {// &&
        		    //likely(memcmp(key, get_key_from_bucket(h, key_bucket, i),
        		    //		  h->key_len) == 0)) {
                    _KeyValue * tmp = static_cast<_KeyValue*>(get_key_from_bucket(h, key_bucket, i));
                    if (tmp && (key_value->k == tmp->k))
        			    return bucket_index * h->bucket_entries + i;
        		}
        	}
        
        	/* Check if any free slot within the bucket to add the new key */
        	pos = find_first(k_NULL_SIGNATURE, sig_bucket, h->bucket_entries);
        
        	if (pos < 0)
        		return -ENOSPC;
        
        	/* Add the new key to the bucket */
        	sig_bucket[pos] = sig;
        	rte_memcpy(get_key_from_bucket(h, key_bucket, pos), key_value, h->key_len);
        	return bucket_index * h->bucket_entries + pos;
        }

        template<typename _KeyValue>
        int32_t del_key_value_with_hash(const rte_hash *h, const _KeyValue *key_value, hash_sig_t sig)
        {
        	RETURN_IF_TRUE(((h == NULL) || (key_value == NULL)), -EINVAL);
        
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
        		if (sig == sig_bucket[i]) {// &&
        		    //likely(memcmp(key, get_key_from_bucket(h, key_bucket, i),
        		    //		  h->key_len) == 0)) {
                    _KeyValue *tmp = static_cast<_KeyValue*>(get_key_from_bucket(h, key_bucket, i));
                    if (tmp && (key_value->k == tmp->k)) {
        			    sig_bucket[i] = k_NULL_SIGNATURE;
        			    return bucket_index * h->bucket_entries + i;
                    }
        		}
        	}
        
        	return -ENOENT;
        }

        template<typename _KeyValue>
        int32_t lookup_with_hash(const rte_hash *h, const _KeyValue *key_value, hash_sig_t sig)
        {
        	RETURN_IF_TRUE(((h == NULL) || (key_value == NULL)), -EINVAL);
        
        	hash_sig_t *sig_bucket;
        	uint8_t *key_bucket;
        	uint32_t bucket_index, i;
        
        	/* Get the hash signature and bucket index */
        	sig |= h->sig_msb;
        	bucket_index = sig & h->bucket_bitmask;
        	sig_bucket = get_sig_tbl_bucket(h, bucket_index);
        	key_bucket = get_key_tbl_bucket(h, bucket_index);
        
        	/* Check if key is already present in the hash */
        	for (i = 0; i < h->bucket_entries; i++) {
        		if (sig == sig_bucket[i]) {// &&
        		    //likely(memcmp(key, get_key_from_bucket(h, key_bucket, i),
        			//	  h->key_len) == 0)) {
                    _KeyValue *tmp = static_cast<_KeyValue*>(get_key_from_bucket(h, key_bucket, i));
                    if (tmp && (key_value->k == tmp->k)) {
        			    return bucket_index * h->bucket_entries + i;
                    }
        		}
        	}
        
        	return -ENOENT;
        }

        template<typename _KeyValue>
        void * find_or_insert_with_hash(const rte_hash *h, const _KeyValue *key_value, hash_sig_t sig)
        {
        	RETURN_IF_TRUE(((h == NULL) || (key_value == NULL)), NULL);
        
        	hash_sig_t *sig_bucket;
        	uint8_t *key_bucket;
        	uint32_t bucket_index, i;
        
        	/* Get the hash signature and bucket index */
        	sig |= h->sig_msb;
        	bucket_index = sig & h->bucket_bitmask;
        	sig_bucket = get_sig_tbl_bucket(h, bucket_index);
        	key_bucket = get_key_tbl_bucket(h, bucket_index);
        
        	/* Check if key is already present in the hash */
        	for (i = 0; i < h->bucket_entries; i++) {
        		if (sig == sig_bucket[i]) {
                    _KeyValue * tmp = static_cast<_KeyValue*>(get_key_from_bucket(h, key_bucket, i));
                    if (tmp && (key_value->k == tmp->k))
        			    return static_cast<void *>(&(tmp->v));
        		}
        	}
        
        	/* Check if any free slot within the bucket to add the new key */
        	int32_t pos = find_first(k_NULL_SIGNATURE, sig_bucket, h->bucket_entries);
        
        	if (pos < 0)
        		return NULL;
        
        	/* Add the new key to the bucket */
        	sig_bucket[pos] = sig;
            _KeyValue * tmp = static_cast<_KeyValue *>(get_key_from_bucket(h, key_bucket, pos));
        	rte_memcpy(static_cast<void *>(tmp), key_value, h->key_len);
        	return static_cast<void *>(&(tmp->v));
        }

    public:
        ~ShareRteHash() {}

        static ShareRteHash & instance(void) {
            static ShareRteHash share_rte_hash;
            return share_rte_hash;
        }

        rte_hash * create_hash_table(const rte_hash_parameters *params);
        rte_hash * attach_hash_table(const char * name);
        void       free_hash_table(rte_hash *& hash_tbl); 
        int32_t    lookup_with_hash(const rte_hash *h, const void *key, hash_sig_t sig);


    private:
        ShareRteHash(void) {}

        /* Returns a pointer to the first signature in specified bucket. */
        inline hash_sig_t *
        get_sig_tbl_bucket(const rte_hash *h, uint32_t bucket_index)
        {
        	return (hash_sig_t *)
        			&(h->sig_tbl[bucket_index * h->sig_tbl_bucket_size]);
        }
        
        /* Returns a pointer to the first key in specified bucket. */
        inline uint8_t *
        get_key_tbl_bucket(const rte_hash *h, uint32_t bucket_index)
        {
        	return (uint8_t *) &(h->key_tbl[bucket_index * h->bucket_entries *
        				     h->key_tbl_key_size]);
        }
        
        /* Returns a pointer to a key at a specific position in a specified bucket. */
        inline void *
        get_key_from_bucket(const rte_hash *h, uint8_t *bkt, uint32_t pos)
        {
        	return (void *) &bkt[pos * h->key_tbl_key_size];
        }
        
        /* Does integer division with rounding-up of result. */
        inline uint32_t
        div_roundup(uint32_t numerator, uint32_t denominator)
        {
        	return (numerator + denominator - 1) / denominator;
        }
        
        /* Increases a size (if needed) to a multiple of alignment. */
        inline uint32_t
        align_size(uint32_t val, uint32_t alignment)
        {
        	return alignment * div_roundup(val, alignment);
        }
        
        /* Returns the index into the bucket of the first occurrence of a signature. */
        inline int
        find_first(uint32_t sig, const uint32_t *sig_bucket, uint32_t num_sigs)
        {
        	uint32_t i;
        	for (i = 0; i < num_sigs; i++) {
        		if (sig == sig_bucket[i])
        			return i;
        	}
        	return -1;
        }
};

#endif
