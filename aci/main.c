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

#define _DEFAULT_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <poll.h>
#include "drum/drum.h"
#include "aci/state.h"

#define IPC_BACKLOG 32
#define POLL_FD_COUNT 16
#define IPC_PATH "/tmp/odb.d"

static char *drum_dir = NULL;
static struct pollfd fds[POLL_FD_COUNT];
static struct aci_state state;

/*
 * Enumerate each available drum
 */
static void
drum_enumerate(void)
{
    char pathbuf[128];
    struct drum *drum = NULL;
    struct dirent *dirent;
    DIR *dir;

    if ((dir = opendir(drum_dir)) == NULL) {
        perror("opendir");
        return;
    }

    while ((dirent = readdir(dir)) != NULL) {
        if (dirent->d_name[0] == '.') {
            continue;
        }

        if (dirent->d_type != DT_DIR) {
            continue;
        }

        drum = malloc(sizeof(*drum));
        if (drum == NULL) {
            printf("fatal: drum allocation failure; out of memory\n");
            exit(1);
        }

        memset(drum, 0, sizeof(*drum));
        ++state.drum_count;
        snprintf(pathbuf, sizeof(pathbuf), "%s/%s", drum_dir, dirent->d_name);

        drum->path = strdup(pathbuf);
        printf("[ drum %zu ] @ %s\n", state.drum_count, pathbuf);
        TAILQ_INSERT_TAIL(&state.drum_list, drum, link);
    }

    closedir(dir);
}

/*
 * Allocate a file descriptor from the pollfd
 * list.
 */
static struct pollfd *
poll_fd_alloc(int fd)
{
    for (int i = 0; i < POLL_FD_COUNT; ++i) {
        if (fds[i].fd == -1) {
            fds[i].fd = fd;
            return &fds[i];
        }
    }

    return NULL;
}

/*
 * Accept an IPC connection
 */
static void
ipc_accept(int ssockfd)
{
    struct sockaddr_un client;
    struct pollfd *fd;
    socklen_t client_len;
    int client_fd;

    client_len = sizeof(client);
    client_fd = accept(
        ssockfd,
        (struct sockaddr *)&client,
        &client_len
    );

    if (client_fd < 0) {
        perror("accept");
        return;
    }

    if ((fd = poll_fd_alloc(client_fd)) == NULL) {
        close(client_fd);
        return;
    }

    fd->events = POLLIN;
}

static void
ipc_read(int client_fd, uint16_t poll_idx)
{
    char buf[256];
    ssize_t len;

    len = recv(client_fd, buf, sizeof(buf), 0);
    if (len <= 0) {
        printf("client closed connection\n");
        close(client_fd);
        fds[poll_idx].fd = -1;
        return;
    }

    printf("got %zd bytes\n", len);
}

static void
run(void)
{
    struct sockaddr_un un;
    int ssockfd, error;
    int pollret;

    memset(&un, 0, sizeof(un));
    memcpy(un.sun_path, IPC_PATH, sizeof(IPC_PATH));
    un.sun_family = AF_UNIX;

    /* Open a server side socket */
    ssockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ssockfd < 0) {
        perror("ssockfd");
        return;
    }

    error = bind(
        ssockfd,
        (struct sockaddr *)&un,
        sizeof(un)
    );

    if (error < 0) {
        perror("bind");
        return;
    }

    error = listen(ssockfd, IPC_BACKLOG);
    if (error < 0) {
        perror("listen");
        return;
    }

    fds[0].fd = ssockfd;
    fds[0].events = POLLIN;

    /* Read through events */
    for (;;) {
        pollret = poll(fds, POLL_FD_COUNT, 500);
        if (pollret < 0) {
            perror("poll");
            continue;
        }

        if (fds[0].revents & POLLIN) {
            ipc_accept(ssockfd);
        }

        for (int i = 1; i < POLL_FD_COUNT; ++i) {
            if (fds[i].fd < 0) {
                continue;
            }

            ipc_read(fds[i].fd, i);
        }
    }
}

int
main(int argc, char **argv)
{
    pid_t child;

    if (argc < 2) {
        printf("fatal: expected drum directory as argument\n");
        return -1;
    }

    drum_dir = argv[1];
    if (access(drum_dir, F_OK) != 0) {
        printf("fatal: could not access \"%s\"\n", drum_dir);
        return -1;
    }

    TAILQ_INIT(&state.drum_list);
    drum_enumerate();

    memset(fds, -1, sizeof(fds));
    child = fork();
    if (child == 0) {
        run();
    }

    return 0;
}
