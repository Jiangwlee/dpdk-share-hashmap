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

template <typename _Key, typename _Hashfunc>
void test_share_hashmap(void)
{
    char name[] = "test01";
    ShareHashMap<_Key, _Hashfunc> shm(name);

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


template <typename _Key>
void test_share_hashmap(void)
{
    char name[] = "test01";
    ShareHashMap<_Key> shm(name);

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