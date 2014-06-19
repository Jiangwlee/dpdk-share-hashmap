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
 * Description:
 *
 * This is an implementation of share hash map in dpdk. I would like to offer
 * a hash map template as std::hash_map. For a user, he only needs to know
 * the interface of this hash map. He could use it easily even does not know
 * the implementation of dpdk.
 */

#ifndef  _SHARE_HASHMAP_H_
#define  _SHARE_HASHMAP_H_

#include <iostream>
#include <stdio.h>
#include <iostream>
#include <sstream>

#include <errno.h>
#include <rte_errno.h>
/* Hash function used if none is specified */
#ifdef RTE_MACHINE_CPUFLAG_SSE4_2
#include <rte_hash_crc.h>
#define DEFAULT_HASH_FUNC       rte_hash_crc
#else
#include <rte_jhash.h>
#define DEFAULT_HASH_FUNC       rte_jhash
#endif

#include "hash_func.h"
#include "share_rte_hash.h"

using namespace std;

// Forward declaration
template <class _Key, class _Value, class _HashFunc = sharehash::hash<_Key> >
class ShareHashMap;

template <class _Key, class _Value, class _HashFunc>
class ShareHashMap {
    public:
        static const int DEFAULT_BUCKET_ENTRIES = 128;
        static const int DEFAULT_TOTAL_ENTRIES  = DEFAULT_BUCKET_ENTRIES * 16;

    public:
        typedef _Key key_type;
        typedef _Value value_type;
        typedef _HashFunc hasher;
        
        typedef struct KeyValuePair {
            key_type   k;
            value_type v;
        } key_value_pair_type; 

    public:
        ShareHashMap(const char * __name) {
            m_hash_params.name = __name;
            m_hash_params.entries = DEFAULT_TOTAL_ENTRIES;
            m_hash_params.bucket_entries = DEFAULT_BUCKET_ENTRIES;
            m_hash_params.key_len = sizeof(key_value_pair_type);
            m_hash_params.hash_func = NULL;
            m_hash_params.hash_func_init_val = 0;
            m_hash_params.socket_id = 0; 
        
            m_rte_hash = NULL;
        }

        ~ShareHashMap(void) {
	        if (rte_eal_process_type() == RTE_PROC_PRIMARY)
                ShareRteHash::instance().free_hash_table(m_rte_hash);
        }

        // create a hashmap, used by primary process
        bool create(void) {
            m_rte_hash = ShareRteHash::instance().create_hash_table(&m_hash_params); 
            
            if (m_rte_hash)
                return true;
            else
                return false;
        }

        // attach to an existing hashmap, used by secondary process
        bool attach(void) {
            m_rte_hash = ShareRteHash::instance().attach_hash_table(m_hash_params.name); 
            
            if (m_rte_hash)
                return true;
            else
                return false;
        }

        int32_t insert(const key_type& __key, const value_type& __value) {
            key_value_pair_type key_value_pair = {__key, __value};
            hash_sig_t signature = m_hash_func(__key);
            int32_t position = ShareRteHash::instance().add_key_value_with_hash(m_rte_hash, &key_value_pair, signature);
        
#ifdef DEBUG
            cout << " ... ... insert key : " << __key
                 << " signature : " << signature
                 << " to position " << position << endl;
#endif
        
            return position;
        }

        int32_t find(const key_type& __key) {
            key_value_pair_type key_value_pair;
            key_value_pair.k = __key;
            hash_sig_t signature = m_hash_func(__key);
            int32_t position = ShareRteHash::instance().lookup_with_hash(m_rte_hash, &key_value_pair, signature);
        
            if (position == EINVAL) {
                cout << " ... ... Invalid parameters!" << endl;
            } else if (position == ENOENT) {
                cout << " ... ... Can't find this key!" << endl;
            } else {
#ifdef DEBUG
                cout << " ... ... Lookup key : " << __key
                     << " signature : " << signature
                     << " to position " << position << endl;
#endif
            }
    
            return position;
        }

        int32_t erase(const key_type & __key) {
            key_value_pair_type key_value_pair;
            key_value_pair.k = __key;
            hash_sig_t signature = m_hash_func(__key);
            int32_t position = ShareRteHash::instance().del_key_value_with_hash(m_rte_hash, &key_value_pair, signature);
        
#ifdef DEBUG
            cout << " ... ... Erase key : " << __key
                 << " signature : " << signature
                 << " to position " << position << endl;
#endif
        
            return position;
        }
        
        
#define NULL_SIGNATURE 0
        int32_t free_entry_count(void)
        {
            int32_t count = 0;
            const uint32_t * sig_tbl = (const uint32_t *)m_rte_hash->sig_tbl;
        
            for (uint32_t i = 0; i < m_rte_hash->entries; ++i)
                if (sig_tbl[i] == NULL_SIGNATURE)
                    ++count;
        
            return count;
        }
        
        int32_t used_entry_count(void)
        {
            int32_t count = 0;
            const uint32_t * sig_tbl = (const uint32_t *)m_rte_hash->sig_tbl;
        
            for (uint32_t i = 0; i < m_rte_hash->entries; ++i)
                if (sig_tbl[i] != NULL_SIGNATURE)
                    ++count;
        
            return count;
        }
#undef NULL_SIGNATURE
        
        void str(ostream & __log) {
            if (!m_rte_hash) {
                __log << "m_rte_hash is NULL" << endl;
                return;
            }
        
            __log << "rte_hash information : " << endl;
            __log << "total entries : " << m_rte_hash->entries << endl;
            __log << "total buckets : " << m_rte_hash->num_buckets << endl;
            __log << "entries/buk   : " << m_rte_hash->bucket_entries << endl;
            __log << "key length    : " << m_rte_hash->key_len << endl;
            __log << "used entries  : " << used_entry_count() << endl;
            __log << "free entries  : " << free_entry_count() << endl;

            // for debug
            __log << endl;
            __log << "---------- Debug Information -----------" << endl;
            __log << " Address of rte_hash : " << hex << (void*)m_rte_hash << endl;
            __log << " Address of sig_tbl  : " << hex << (void*)m_rte_hash->sig_tbl
                  << " Bucket Size : " << dec << m_rte_hash->sig_tbl_bucket_size << endl;
            __log << " Address of key_tbl  : " << hex << (void*)m_rte_hash->key_tbl
                  << " Key Size : " << dec << m_rte_hash->key_tbl_key_size << endl << endl;
        }
        
        void print(void) {
            ostringstream __log;
            str(__log);
            cout << __log.str();
        }

    private:
        rte_hash *m_rte_hash;
        hasher    m_hash_func;  // we can't use the hash_fun in rte_hash, because it would be in share memory.
        rte_hash_parameters m_hash_params; 
};

#endif
