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

#include "leveldbd.h"

#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <iostream>
#include "asyncd/asyncd.h"
#include "macro.h"

/*---------------------------------------------------------------------------*
 *                             GLOBAL VARIABLES                              *
 *---------------------------------------------------------------------------*/
int g_log_level = LOG_WARN;

/*---------------------------------------------------------------------------*
 *                          INTERNAL STATIC FUNCTIONS                        *
 *---------------------------------------------------------------------------*/
static void print_usage(void);

static void signal_init(void (func)(int));
static void signal_set(int signo);
static void signal_handler(void);

/*---------------------------------------------------------------------------*
 *                          INTERNAL STATIC VARIABLES                        *
 *---------------------------------------------------------------------------*/
static sigset_t _sigflags;
static ad_server_t *server = NULL;

int http_get_handler(short event, ad_conn_t *conn, void *userdata) {
    if (ad_http_get_status(conn) == AD_HTTP_REQ_DONE) {
        ad_http_response(conn, 200, "text/html", "Hello World", 11);
        return AD_DONE;  // Keep connection alive.
    }
    return AD_OK;
}

int http_default_handler(short event, ad_conn_t *conn, void *userdata) {
    if (ad_http_get_status(conn) == AD_HTTP_REQ_DONE) {
        ad_http_response(conn, 501, "text/html", "Not implemented", 15);
        return AD_CLOSE;  // Close connection.
    }
    return AD_OK;
}

int main(int argc, char **argv) {
    bool daemonize = true;
    int port = DEFAULT_PORT;

    // Parse command line arguments.
    int opt;
    while ((opt = getopt(argc, argv, "p:dDhc:")) != -1) {
        switch (opt) {
            case 'd': {
                fprintf(stdout, "Entering debugging mode.\n");
                g_log_level = LOG_DEBUG;
                ad_log_level(AD_LOG_DEBUG);
                daemonize = false;
                break;
            }
            case 'D': {
                fprintf(stdout, "Entering console mode.\n");
                daemonize = false;
                break;
            }
            case 'h': {
                print_usage();
                return EXIT_SUCCESS;
            }
            case 'p': {
                port = atoi(optarg);
                break;
            }
            default: {
                print_usage();
                return EXIT_FAILURE;
            }
        }
    }

    // Initialize signal.
    signal_init(signal_set);

    // Daemonize
    if (daemonize) {
        g_log_level = 0;
        daemon(true, true);
    }

    // start daemon services
    char *port_str = qstrdupf("%d", port);
    server = ad_server_new();
    ad_server_set_option(server, "server.port", port_str);
    ad_server_register_hook(server, ad_http_handler, NULL);  // HTTP Parser is also a hook.
    ad_server_register_hook_on_method(server, "GET", http_get_handler, NULL);
    ad_server_register_hook(server, http_default_handler, NULL);
    free(port_str);
    return ad_server_start(server);
}

static void print_usage(void) {
    FILE *out = stderr;
    fprintf(out, "Usage: %s [options]\n", PRGNAME);
    fprintf(out, "    Options:\n");
    fprintf(out, "\t-p : port number (default: %d)\n", DEFAULT_PORT);
    fprintf(out, "\t-d : debug mode\n");
    fprintf(out, "\t-D : do not demonize\n");
    fprintf(out, "\t-h : print out help message\n");
}

/**
 * Initialize signal.
 */
static void signal_init(void (func)(int)) {
    struct sigaction sa;
    sa.sa_handler = func;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    // Hook up signals
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    // reset signal flags;
    sigemptyset(&_sigflags);
}

/**
 * Call back function.
 */
static void signal_set(int signo) {
    sigaddset(&_sigflags, signo);
    signal_handler();
}

/**
 * Actual signal handler.
 */
static void signal_handler() {
    if (sigismember(&_sigflags, SIGTERM)) {
        sigdelset(&_sigflags, SIGTERM);
        DEBUG("Caught SIGTERM");
        ad_server_stop(server);
    } else if (sigismember(&_sigflags, SIGINT)) {
        sigdelset(&_sigflags, SIGINT);
        DEBUG("Caught SIGINT");
        ad_server_stop(server);
    } else if (sigismember(&_sigflags, SIGUSR1)) {
        sigdelset(&_sigflags, SIGUSR1);
        DEBUG("Caught SIGUSR1");
    }
}
