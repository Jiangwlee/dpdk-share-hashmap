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

#include "keys.h"

ostream& operator<< (ostream &os, const struct_key& key) {
    ostringstream str;
    str << "src_ip is " << hex << key.src_ip << " dst_ip is " << key.dst_ip << endl;
    os << str.str();
    return os;
}

