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
 * @file : test.h
 * @desription : define a test function template
 *
 */

#ifndef __TEST_H__
#define __TEST_H__

#include "modifier.h"

#if 0
template <typename _Key, typename _Value, typename _Hashfunc>
void test_share_hashmap(void)
{
    char name[] = "test01";
    ShareHashMap<_Key, _Value, _Hashfunc> shm(name);

	if (rte_eal_process_type() == RTE_PROC_PRIMARY){ 
        if (!shm.create())                           
            return;
	} else {
        if (!shm.attach())
            return;
	}

    cout << "Please input your choice : a[dd], d[elete], f[ind], s[how], q[uit]" << endl;

    while (1) {
        int input = getchar();
        int quit = false;
        int key_value = 0;

        switch (input) {
            case 'a':
                cout << " ... Please input the key [1 ~ 65536] : ";
                cin  >> key_value;
                cout << " ... You just input " << key_value << endl;
                shm.insert(key_value);
                break;
            case 'd':
                cout << " ... Please input the key [1 ~ 65536] : ";
                cin  >> key_value;
                cout << " ... You just input " << key_value << endl;
                shm.erase(key_value);
                break;
            case 'f':
                cout << " ... Please input the key [1 ~ 65536] : ";
                cin  >> key_value;
                cout << " ... You just input " << key_value << endl;
                shm.find(key_value);
                break;
            case 's':
                shm.print();
                break;
            case 'q':
                quit = true;
                break;
            default:
                continue;
        }

        cout << "Please input your choice : a[dd], d[elete], f[ind], s[how], q[uit]" << endl;

        if (quit)
            break;
    }
}
#endif

static int prompt_key(void) {
    int key;

    cout << " ... Please input the key [1 ~ 65536] : ";
    cin  >> key;
    cout << " ... You just input " << key << endl;

    return key;
}

static int prompt_value(void) {
    int value;

    cout << " ... Please input the value[1 ~ 65536] : ";
    cin  >> value;
    cout << " ... You just input " << value << endl;

    return value;
}

template <typename _Key, typename _Value>
void test_share_hashmap(void)
{
    char name[] = "test01";
    typedef ShareHashMap<_Key, _Value> hashmap;
    hashmap shm(name);

	if (rte_eal_process_type() == RTE_PROC_PRIMARY){ 
        if (!shm.create())                           
            return;
	} else {
        if (!shm.attach())
            return;
	}

    cout << "Please input your choice : a[dd], d[elete], f[ind], m[odify], s[how], q[uit]" << endl;

    try {
        while (1) {
            int input = getchar();
            int quit = false;
            int key = 0;
            int value = 0;
            int index = 0;
    
            switch (input) {
                case 'a':
                    key = prompt_key();
                    value = prompt_value();
                    shm.insert(key, value);
                    break;
                case 'd':
                    key = prompt_key();
                    shm.erase(key);
                    break;
                case 'f':
                    key = prompt_key();
                    index = shm.find(key);
                    if (index >= 0) {
                        typename hashmap::key_value_pair_type *key_value = NULL;
                        shm.get_entry_with_index(key_value, index);
                        if (key_value)
                            cout << "Key : " << key_value->k << " Value : " << key_value->v << endl;
                    }
                    break;
                case 'm':
                    key = prompt_key();
                    value = prompt_value();
                    add<_Value> add_op;
                    shm.update_value(key, value, add_op);
                    break;
                case 's':
                    shm.print();
                    break;
                case 'q':
                    quit = true;
                    break;
                default:
                    continue;
            }
    
            cout << "Please input your choice : a[dd], d[elete], f[ind], m[odify], s[how], q[uit]" << endl;
    
            if (quit)
                break;
        }
    } catch (Exception &ex) {
        cout << ex << endl;
    }
}

#endif
