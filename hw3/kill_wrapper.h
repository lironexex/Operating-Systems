#include <linux/module.h>
#include <linux/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <cstdlib>


int kill(pid_t pid, int sig) {
	printf("welcome to our kill wrapper!\n");

	int __res;
	__asm__(
		"syscall;"
		: "=a" (__res)
		: "a" (62), "D" (pid), "S" (sig)
		:
		);
	if (__res < 0) {
		errno = -(__res);
		return -1;
	}
	return __res;
}
