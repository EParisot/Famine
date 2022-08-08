#include "../includes/famine.h"

#define SYSCALL_WRITE 1
#define SYSCALL_CLOSE 3
#define SYSCALL_OPENAT 257
#define SYSCALL_GETDENTS SYS_getdents

int syscall_write(unsigned fd, const char *buf, unsigned count)
{
	unsigned ret;

	asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYSCALL_WRITE), "D"(fd), "S"(buf), "d"(count)
		: "rcx", "r11", "memory"
	);

	return ret;
}

int syscall_openat(int dirfd, const char *pathname, int flags)
{
	unsigned ret;

	asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYSCALL_OPENAT), "D"(dirfd), "S"(pathname), "d"(flags)
		: "rcx", "r11", "memory"
	);

	return ret;
}

int syscall_getdents(unsigned fd, const struct dirent *buf, unsigned count)
{
	unsigned ret;

	asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYSCALL_GETDENTS), "D"(fd), "S"(buf), "d"(count)
		: "rcx", "r11", "memory"
	);

	return ret;
}

void syscall_close(unsigned int fd)
{
	unsigned ret;

	asm volatile(
		"syscall"
		:"=a"(ret)
		: "a"(SYSCALL_CLOSE), "D"(fd)
		: "memory"
	);
}