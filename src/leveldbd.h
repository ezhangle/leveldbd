/******************************************************************************
 * leveldbd
 *
 * Copyright (c) 2014 Seungyoung Kim.
 * All rights reserved.
 *
 * leveldbd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * leveldbd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with leveldbd. If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef _LEVELDBD_H
#define _LEVELDBD_H

#ifdef __cplusplus
extern "C" {
#endif

#define PRGNAME         "leveldbd"
#define PIDFILE         "/tmp/" PRGNAME ".pid"
#define STATFILE        "/tmp/" PRGNAME ".stat"

#define DEFAULT_PORT    (8888)

#ifdef __cplusplus
}
#endif

#endif /*_LEVELDBD_H */
