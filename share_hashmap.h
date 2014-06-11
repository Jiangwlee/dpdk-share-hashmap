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

#include <rte_hash.h>

using namespace std;

class ShareHashMap {
    public:
        static const int DEFAULT_BUCKET_ENTRIES = 16;
        static const int DEFAULT_TOTAL_ENTRIES  = 16 << 3;
        static const int DEFAULT_KEY_LENGTH     = 4;
        
    public:
        // The first version, the key is always 4 byte
        ShareHashMap(const char * name); 
        
        bool    create(void); // create a hashmap, used by primary process
        bool    attach(void); // attach to an existing hashmap, used by secondary process
        int32_t insert(int key);
        int32_t find(int key);
        int32_t erase(int key);

        void    print(void);  // show the basic information of this rte_hash
        void    str(ostream &);  // show the basic information of this rte_hash
        int32_t free_entry_count(void);
        int32_t used_entry_count(void);

    private:
        rte_hash *m_rte_hash;
        rte_hash_function m_hash_func;  // we can't use the hash_fun in rte_hash, because it would be in share memory.
        uint32_t m_hash_func_init_val;
        rte_hash_parameters m_hash_params; 
};

#endif
