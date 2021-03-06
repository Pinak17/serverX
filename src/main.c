/* Copyright 2017 The serverX Authors. All Rights Reserved.
 * Maintainer: Bytes Club (https://bytesclub.github.io)<bytes-club@googlegroups.com>

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
===============================================================================*/

// Main Source File
// serverX.c

// include header
#include <serverX.h>

// server's root
char* root = NULL;

// file descriptor for sockets
int cfd = -1, sfd = -1;

// flag indicating whether control-c has been heard
bool signaled = false;

int main(int argc, char* argv[])
{
    // a global variable defined in errno.h that's "set by system
    // calls and some library functions [to a nonzero value]
    // in the event of an error to indicate what went wrong"
    errno = 0;

    // Thread ID and Attributes
    pthread_t tid;
    pthread_attr_t tattr;

    // default to port 8080
    int port = 8080;

    // usage
    const char* usage = "Usage: serverX [-p port] /path/to/root";

    // parse command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "hp:")) != -1) {
        switch (opt) {
            // -h
            case 'h':
                printf("%s\n", usage);
                return 0;

            // -p port
            case 'p':
                port = atoi(optarg);
                break;
        }
    }

    // ensure port is a non-negative short and path to server's root is specified
    if (port < 0 || port > SHRT_MAX || argv[optind] == NULL ||
        strlen(argv[optind]) == 0) {
        // announce usage
        printf("%s\n", usage);

        // return 2 just like bash's builtins
        return 2;
    }

    // start server
    start(port, argv[optind]);

    // listen for SIGINT (aka control-c)
    SIGNAL(SIGINT, handler);

    // accept connections one at a time
    while (true) {
        // close last client's socket, if any
        if (cfd != -1) {
            close(cfd);
            cfd = -1;
        }

        // check for control-c
        if (signaled)    stop();

        // check whether client has connected
        if (connected()) {
            pthread_attr_init(&tattr);
            if (pthread_create(&tid, &tattr, process, NULL)) {
                error(500);
                continue;
            }
            printf("On Thread: %u\n", (unsigned int)tid);
            pthread_join(tid, NULL);
        }
    }
}
