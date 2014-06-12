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



#include <stdio.h>
#include <iostream>
#include <sstream>
#include "share_hashmap.h"

/* Hash function used if none is specified */
#ifdef RTE_MACHINE_CPUFLAG_SSE4_2
#include <rte_hash_crc.h>
#define DEFAULT_HASH_FUNC       rte_hash_crc
#else
#include <rte_jhash.h>
#define DEFAULT_HASH_FUNC       rte_jhash
#endif

#include <errno.h>
#include <rte_errno.h>


ShareHashMap::ShareHashMap(const char * name) {
    m_hash_params.name = name;
    m_hash_params.entries = DEFAULT_TOTAL_ENTRIES;
    m_hash_params.bucket_entries = DEFAULT_BUCKET_ENTRIES;
    m_hash_params.key_len = DEFAULT_KEY_LENGTH;
    m_hash_params.hash_func = NULL;
    m_hash_params.hash_func_init_val = 0;
    m_hash_params.socket_id = 0; 

    m_rte_hash = NULL;
    m_hash_func = DEFAULT_HASH_FUNC;
    m_hash_func_init_val = 0;
}

bool ShareHashMap::create(void) {
    m_rte_hash = rte_hash_create(&m_hash_params); 
    
    if (m_rte_hash)
        return true;
    else
        return false;
}

bool ShareHashMap::attach(void) {
    m_rte_hash = rte_hash_find_existing(m_hash_params.name); 
    
    if (m_rte_hash)
        return true;
    else
        return false;
}

int32_t ShareHashMap::insert(int key) {
    hash_sig_t signature = m_hash_func((const void *)&key, sizeof(key), m_hash_func_init_val);  
    int32_t position = rte_hash_add_key_with_hash(m_rte_hash, (const void *)&key, signature);

#ifdef DEBUG
    printf(" ... ... Insert key : %d Signature : %d to Position %d\n", key, signature, position);
#endif

    return position;
}

int32_t ShareHashMap::erase(int key) {
    hash_sig_t signature = m_hash_func((const void *)&key, sizeof(key), m_hash_func_init_val);
    int32_t position = rte_hash_del_key_with_hash(m_rte_hash, (const void *)&key, signature);

#ifdef DEBUG
    printf(" ... ... Delete key : %d Signature : %d from Position %d\n", key, signature, position);
#endif

    return position;
}

int32_t ShareHashMap::find(int key) {
    hash_sig_t signature = m_hash_func((const void *)&key, sizeof(key), m_hash_func_init_val);
    int32_t position = rte_hash_lookup_with_hash(m_rte_hash, (const void *)&key, signature);

    if (position == EINVAL) {
        cout << " ... ... Invalid parameters!" << endl;
    } else if (position == ENOENT) {
        cout << " ... ... Can't find this key!" << endl;
    } else {
#ifdef DEBUG
        printf(" ... ... Lookup key : %d Signature : %d on Position %d\n", key, signature, position);
#endif
    }
    
    return position;
}

#define NULL_SIGNATURE 0
int32_t ShareHashMap::free_entry_count(void)
{
    int32_t count = 0;
    const uint32_t * sig_tbl = (const uint32_t *)m_rte_hash->sig_tbl;

    for (uint32_t i = 0; i < m_rte_hash->entries; ++i)
        if (sig_tbl[i] == NULL_SIGNATURE)
            ++count;

    return count;
}

int32_t ShareHashMap::used_entry_count(void)
{
    int32_t count = 0;
    const uint32_t * sig_tbl = (const uint32_t *)m_rte_hash->sig_tbl;

    for (uint32_t i = 0; i < m_rte_hash->entries; ++i)
        if (sig_tbl[i] != NULL_SIGNATURE)
            ++count;

    return count;
}
#undef NULL_SIGNATURE

void ShareHashMap::str(ostream & log) {
    if (!m_rte_hash) {
        log << "m_rte_hash is NULL" << endl;
        return;
    }

    log << "rte_hash information : " << endl;
    log << "total entries : " << m_rte_hash->entries << endl;
    log << "total buckets : " << m_rte_hash->num_buckets << endl;
    log << "entries/buk   : " << m_rte_hash->bucket_entries << endl;
    log << "key length    : " << m_rte_hash->key_len << endl;
    log << "used entries  : " << used_entry_count() << endl;
    log << "free entries  : " << free_entry_count() << endl;
}

void ShareHashMap::print(void) {
    ostringstream log;
    str(log);
    cout << log.str();
}

