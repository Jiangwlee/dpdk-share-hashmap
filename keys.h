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
 * Copyright (C) Bruce <jiangwlee@163.com>, 2014
 */

/*
 * @file : keys.h
 * @description : define different key types and hashers
 *
 */

#ifndef __KYES_H__
#define __KEYS_H__

#include <iostream>
#include <sstream>

#include <rte_jhash.h>

using namespace std;

// declarations
struct struct_key;
ostream& operator<< (ostream &os, const struct_key& key); 

struct struct_key {
    int src_ip;
    int dst_ip; 

    struct_key(int key) : src_ip(key), dst_ip(key << 2) {}
};

template <typename _Key>
struct jhasher{
    size_t operator() (const _Key& key) const {
        return rte_jhash((const void *)&key, sizeof(key), 0);
    }
};

#endif
