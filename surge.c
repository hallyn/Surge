#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

enum objtype {
	MOUNT = 1,
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

int read_config(struct config **configp)
{
	return 0;
}

int main(int argc, char *argv)
{
	int ret, status;
	struct config *config;

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
