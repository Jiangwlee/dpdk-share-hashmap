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

#include <string>
#include <iostream>

class Exception {
    friend std::ostream & operator<< (std::ostream &os, const Exception &ex) {
        os << "Exception happend in " << ex.m_who << " *** Message : " << ex.m_why; 
        return os;
    }

    public:
        Exception(const std::string & who, const std::string & why) : m_who(who), m_why(why) {}

    private:
        std::string m_who;
        std::string m_why;
};


