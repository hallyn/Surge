/*
 * Surge init
 *
 * Copyright (C) Serge Hallyn <serge@hallyn.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

int setresuid(uid_t ruid, uid_t euid, uid_t suid);
int setresgid(gid_t rgid, gid_t egid, gid_t sgid);

enum objtype {
	// fs mount
	MOUNT = 1,
	// configured network interface
	NIC,
	// getty console
	CONSOLE,
	// a service to be restarted
	DAEMON,
	// a one time job
	TASK,
	// an event to broadcast
	EVENT,
	// a value to track
	VALUE,
};

struct config {
	enum objtype objtype;
	union {
		struct {
			char *src;
			char *dest;
			char *options;
			char *fstype;
		} mount;
	} u;
};

int run_parser(int dirfd, int sockfd)
{
	int i, pid;
	char dirstr[30], sockstr[30];

	pid = fork();
	if (pid < 0)
		return -errno;
	if (pid > 0)
		return pid;

	if (setgroups(0, NULL) < 0)
		fprintf(stderr, "Failed clearing aux groups: %m\n");
	if (setresgid(65534, 65534, 65534) < 0)
		fprintf(stderr, "Failed setting group nogroup: %m\n");
	if (setresuid(65534, 65534, 65534) < 0)
		fprintf(stderr, "Failed setting user nobody: %m\n");

	// TODO - nicely close all files opened acoording to /proc/self
	for (i = 0; i < 1024; i++) {
		if (i == dirfd || i == sockfd)
			continue;
		close(i);
	}
	snprintf(dirstr, 30, "%d", dirfd);
	snprintf(sockstr, 30, "%d", sockfd);

	execlp("./svc.parse", "parse", dirstr, sockstr, NULL);
	perror("parser exec failed");
	exit(1);
}

int read_parse_config(int sock, struct config **configp)
{
	return 0;
}

int read_config(struct config **configp)
{
	int fd = -1, s[2] = {-1, -1}, ret = -1;

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, s) < 0) {
		perror("socketpair failed");
		return -1;
	}

	fd = open("./confs", O_RDONLY | O_DIRECTORY);
	if (fd < 0)
		goto out;

	if (run_parser(fd, s[1]) < 0)
		goto out;

	close(fd);
	fd = -1;

	close(s[1]);
	s[1] = -1;

	if (read_parse_config(s[0], configp) < 0)
		goto out;

	ret = 0;

out:
	if (s[0] != -1)
		close(s[0]);
	if (s[1] != -1)
		close(s[1]);
	if (fd != -1)
		close(fd);
	return ret;
}

int main(int argc, char *argv)
{
	int ret, status;
	struct config *config = NULL;

	ret = read_config(&config);
	if (ret < 0) {
		fprintf(stderr, "Failed reading configuration\n");
		exit(1);
	}

	// TODO -
	//   use signalfd here, and poll over signalfd and api sockets
	while (1) {
		ret = waitpid(-1, &status, __WALL);
		if (ret == -1) {
			if (errno == -EINTR)
				continue;
			break;
		}
		printf("child %d died\n", ret);
	}
	printf("done\n");
}
