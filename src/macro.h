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

#ifndef _MACRO_H
#define _MACRO_H

/*
 * Macro Functions
 */
#define ASSERT(c)           assert(c)
#define CONST_STRLEN(s)     (sizeof(s) - 1)
#define IS_EMPTY_STR(s)     ((*s == '\0') ? true : false)
#define IS_EQUAL_STR(s1,s2) (!strcmp(s1,s2))
#define ENDING_CHAR(s)      (*(s + strlen(s) - 1))
#define NEW_OBJECT(t)       ((t *)calloc(1, sizeof(t)))

#define DYNAMIC_VSPRINTF(s, f) do {                                     \
        size_t _strsize;                                                \
        for(_strsize = 1024; ; _strsize *= 2) {                         \
            s = (char*)malloc(_strsize);                                \
            if(s == NULL) {                                             \
                DEBUG("DYNAMIC_VSPRINTF(): can't allocate memory.");    \
                break;                                                  \
            }                                                           \
            va_list _arglist;                                           \
            va_start(_arglist, f);                                      \
            int _n = vsnprintf(s, _strsize, f, _arglist);               \
            va_end(_arglist);                                           \
            if(_n >= 0 && _n < _strsize) break;                         \
            free(s);                                                    \
        }                                                               \
    } while(0)

/*
 * Error message
 */
extern int g_log_level;

#define LOG_ERROR     (1)
#define LOG_WARN      (2)
#define LOG_INFO      (3)
#define LOG_DEBUG     (4)

#define ERROROUT    (stderr)
#define ERROR(fmt, args...) if (g_log_level >= LOG_ERROR) {             \
        fprintf(ERROROUT, "[ERROR] " fmt "\n", ##args);                 \
    }
#define WARN(fmt, args...) if (g_log_level >= LOG_WARN) {               \
        fprintf(ERROROUT, "[WARN] " fmt "\n", ##args);                  \
    }
#define INFO(fmt, args...) if (g_log_level >= LOG_INFO) {               \
        fprintf(ERROROUT, "[INFO] " fmt "\n", ##args);                  \
    }

/*
 * Debug Macros
 */
#ifdef BUILD_DEBUG
#define DEBUGOUT    (stdout)
#define DEBUG(fmt, args...) if (g_log_level >= LOG_DEBUG) {             \
        fprintf(DEBUGOUT, "[DEBUG %d] " fmt " [%s(),%s:%d]\n",          \
                getpid(), ##args, __func__, __FILE__, __LINE__);        \
    }
#else
#define DEBUG(fms, args...)
#endif  /* BUILD_DEBUG */

// debug timer
#include <sys/time.h>
#define TIMER_START()                                                   \
    int _swno = 0;                                                      \
    struct timeval _tv1, _tv2;                                          \
    gettimeofday(&_tv1, NULL)

#define TIMER_STOP(prefix)  {                                           \
        gettimeofday(&_tv2, NULL);                                      \
        _swno++;                                                        \
        struct timeval _diff;                                           \
        _diff.tv_sec = _tv2.tv_sec - _tv1.tv_sec;                       \
        if(_tv2.tv_usec >= _tv1.tv_usec) {                              \
            _diff.tv_usec = _tv2.tv_usec - _tv1.tv_usec;                \
        } else {                                                        \
            _diff.tv_sec -= 1;                                          \
            _diff.tv_usec = 1000000 - _tv1.tv_usec + _tv2.tv_usec;      \
        }                                                               \
        printf("TIMER(%d,%s,%d): %zus %dus (%s:%d)\n",                  \
               getpid(), prefix, _swno, _diff.tv_sec, (int)(_diff.tv_usec), \
               __FILE__, __LINE__);                                     \
        gettimeofday(&_tv1, NULL);                                      \
    }

#endif  /* _MACRO_H */
