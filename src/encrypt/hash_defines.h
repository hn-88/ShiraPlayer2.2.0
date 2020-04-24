/*
 * ShiraPlayer(TM)
 * Copyright (C) 2011 Asaf Yurdakul
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * ShiraPlayer is a trademark of Sureyyasoft.
 */

#ifndef HASH_DEFINES_H
#define HASH_DEFINES_H

#include <vector>
#include <string>

#ifdef linux
#define MAX_PATH    256
#endif

typedef std::vector<std::string>    StringList;


//error values
#define SUCCESS 0
#define FAIL -1
#define FILE_OPEN_ERR -2
#define MEM_EXH_ERR -3
#define INX_NOT_FOUND -4
#define INVALID_FILE_NAME -5
#define IO_ERR -6
#define PASS_FAIL -7


#endif // HASH_DEFINES_H
