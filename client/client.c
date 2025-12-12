/*
 * Copyright (c) 2025 Ian Marco Moffett and the Osmora Team.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Hyra nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "aci/proto.h"

/* Various prefixes used for operations */
#define HELP_PREFIX    'h'
#define VERSION_PREFIX 'v'
#define LINK_PREFIX    'l'
#define QUIT_PREFIX    'q'
#define COMMAND_PREFIX 'c'
#define PREFIX_LEN 2

/* Commands used with command prefix */
#define CMD_NOP "NOP"

/* Environment defines */
#define IPC_PATH "/tmp/odb.d"
#define CLIENT_VERSION "v0.0.1"

static int ssockfd = -1;

static void
exit_hook(void)
{
    if (ssockfd > 0) {
        close(ssockfd);
    }

    exit(0);
}

static void
sig_hook(int signo)
{
    printf("got signal %d\n", signo);
    exit_hook();
}

static void
help(void)
{
    printf(
        "-- OSMORA database daemon --\n"
        "[h.]   Show this help menu\n"
        "[c.]   Perform a command\n"
        "[l.]   Get link path\n"
        "[v.]   Get version\n"
        "[q.]   Quit client\n"
    );
}

static void
unknown_command(void)
{
    printf(
        "* Unknown command\n"
        "[?]: Use the 'h.' prefix for help\n"
    );
}

/*
 * Send a no-operation packet to the ACI
 * daemon
 */
static void
db_nop(void)
{
    char pad[8];
    struct aci_pkt *pkt;
    int error;

    memset(pad, 0, sizeof(pad));
    error = aci_pkt_init(
        ACI_CMD_NOP,
        ACI_TYPE_NONE,
        sizeof(pad),
        pad,
        &pkt
    );

    if (error != 0) {
        perror("aci_pkt_init");
    }

    send(ssockfd, pkt, sizeof(*pkt) + pkt->length, 0);
    aci_pkt_free(pkt);
}

static void
db_command(const char *input)
{
    size_t input_len;

    if (input == NULL) {
        return;
    }

    input_len = strlen(input);
    switch (*input) {
    case 'N':
        if (strncmp(input, CMD_NOP, input_len - 1) == 0) {
            printf("[*] sending nop\n");
            db_nop();
            break;
        }
        break;
    default:
        unknown_command();
        break;
    }
}

static void
parse_input(const char *buffer)
{
    size_t buflen;
    char prefix;

    if (buffer == NULL) {
        return;
    }

    buflen = strlen(buffer);
    if (buflen < PREFIX_LEN) {
        unknown_command();
        return;
    }

    /* Prefix format: <prefix>.<operation> */
    if (buffer[1] != '.') {
        unknown_command();
        return;
    }

    prefix = buffer[0];
    if (buffer[2] == '\0') {
        printf("* Expected input\n");
        return;
    }

    switch (prefix) {
    case VERSION_PREFIX:
        printf("%s\n", CLIENT_VERSION);
        break;
    case LINK_PREFIX:
        printf("ipc link @ %s\n", IPC_PATH);
        break;
    case QUIT_PREFIX:
        exit(0);
        break;
    case HELP_PREFIX:
        help();
        break;
    case COMMAND_PREFIX:
        db_command(buffer + 2);
        break;
    default:
        unknown_command();
    }
}

int
main(void)
{
    struct sockaddr_un un;
    char buf[256];
    int error;

    if (access(IPC_PATH, F_OK) != 0) {
        printf("fatal: did not find IPC channel\n");
        printf("[?]: Is the daemon running?\n");
        return -1;
    }

    atexit(exit_hook);
    signal(SIGINT, sig_hook);

    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    memcpy(un.sun_path, IPC_PATH, sizeof(IPC_PATH));

    /* Create the server-side socket */
    ssockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ssockfd < 0) {
        perror("sockfd");
        return -1;
    }

    /* Connect to the IPC channel */
    error = connect(
        ssockfd,
        (struct sockaddr *)&un,
        sizeof(un)
    );

    if (error < 0) {
        perror("connect");
        return -1;
    }

    printf("-- odb client %s --\n", CLIENT_VERSION);
    for (;;) {
        printf("odb~> ");
        fgets(buf, sizeof(buf), stdin);
        if (buf[0] == '\n') {
            continue;
        }

        parse_input(buf);
    }

    return 0;
}
