I re-implement a more effective and powerful share hash_map based on dpdk. Please see following link:

https://github.com/Jiangwlee/more-effective-dpdk-share-hashmap

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



Description:

This is an implementation of share hash map in dpdk. I would like to offer
a hash map template as std::hash_map. For a user, he only needs to know
the interface of this hash map. He could use it easily even does not know
the implementation of dpdk.

Build:

1. Download the source codes of dpdk from dpdk.org

2. Read the intel-dpdk-getting-start-guide.pdf to learn how to set up a dpdk develop environment
   
3. Build and run the helloworld example in dpdk-1.6.0r2/examples/

4. Copy the files under hashmap_mp/mk/ to dpdk-1.6.0r2/mk/ to enable g++ for dpdk

5. Build this program by following command:
   $ make CC=g++

Run:

1. Make sure you have run dpdk-1.6.0r2/tools/setup.sh to set up your dpdk
   running environment < You would know what I mean if you have run the 
   helloworld program >

2. Start the primary process
   $ sudo ./build/hashmap -c 1 -n 4 --proc-type=primary

3. Start the secondary process
   $ sudo ./build/hashmap -c c -n 4 --proc-type=secondary

Have fun!
