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
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/queue.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_tailq.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>

#include "main.h"
#include "share_hashmap.h"


static const struct rte_memzone *
get_share_memory(void)
{
	if (rte_eal_process_type() == RTE_PROC_PRIMARY){
        int socket_id = rte_socket_id();
        printf("This socket_id is %d\n", socket_id);
        printf("This is primary process!\n");
        
        const struct rte_memzone * zone = rte_memzone_reserve(SHM_MEMZONE_NAME,
                                                              SHM_MEMZONE_SIZE,
                                                              socket_id,
                                                              0);
        if (zone != NULL)
            *(int *)zone->addr = 0; 

        return zone;
	} else {
        printf("This is secondary process!\n");
        return rte_memzone_lookup(SHM_MEMZONE_NAME);
	}
}

static void test_share_hashmap(void)
{
    char name[] = "test";
    ShareHashMap shm(name);

	if (rte_eal_process_type() == RTE_PROC_PRIMARY){
        // create a new hashmap, return if it fails
        if (!shm.create())
            return;
	} else {
        // attach to an existing hashmap, return if it fails
        if (!shm.attach())
            return;
	}

    cout << "Please input your choice : a[dd], d[elete], f[ind], s[how], q[uit]" << endl;

    while (1) {
        int input = getchar();
        int quit = FALSE;
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
                quit = TRUE;
                break;
            default:
                continue;
        }

        cout << "Please input your choice : a[dd], d[elete], f[ind], s[how], q[uit]" << endl;

        if (quit)
            break;
    }
}

int
MAIN(int argc, char **argv)
{
	int ret;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");

    test_share_hashmap();

	return 0;
}
