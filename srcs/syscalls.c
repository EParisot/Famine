#include "../includes/famine.h"

#define SYSCALL_WRITE 1
#define SYSCALL_OPEN 2
#define SYSCALL_CLOSE 3
#define SYSCALL_FSTAT 5
#define SYSCALL_MMAP 9
#define SYSCALL_MUNMAP 11
#define SYSCALL_GETUID 102
#define SYSCALL_GETGID 104
#define SYSCALL_OPENAT 257
#define SYSCALL_GETDENTS SYS_getdents

int syscall_write(unsigned fd, const char *buf, unsigned count) {
	unsigned ret;

	asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYSCALL_WRITE), "D"(fd), "S"(buf), "d"(count)
		: "rcx", "r11", "memory"
	);

	return ret;
}

int syscall_open(const char *pathname, int flags) {
	unsigned ret;

	asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYSCALL_OPEN), "D"(pathname), "S"(flags)
		: "r11", "memory"
	);

	return ret;
}

int syscall_openat(int dirfd, const char *pathname, int flags) {
	unsigned ret;

	asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYSCALL_OPENAT), "D"(dirfd), "S"(pathname), "d"(flags)
		: "rcx", "r11", "memory"
	);

	return ret;
}

int syscall_getdents(unsigned fd, const struct dirent *buf, unsigned count) {
	unsigned ret;

	asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYSCALL_GETDENTS), "D"(fd), "S"(buf), "d"(count)
		: "rcx", "r11", "memory"
	);

	return ret;
}

int syscall_fstat(int fd, struct stat *buf) {
	int ret;

	asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYSCALL_FSTAT), "D"(fd), "S"(buf)
		: "r11", "memory"
	);

	return ret;
}

__uid_t syscall_getuid() {
	__uid_t ret;

	asm volatile(
		"syscall"
		:"=a"(ret)
		: "a"(SYSCALL_GETUID)
	);

	return ret;
}

__gid_t syscall_getgid() {
	__gid_t ret;

	asm volatile(
		"syscall"
		:"=a"(ret)
		: "a"(SYSCALL_GETGID)
	);

	return ret;
}

void *syscall_mmap(unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long off) {
	void *ret;
	register long r10 asm("r10") = flags;
	register long r8 asm("r8") = fd;
	register long r9 asm("r9") = off;
	asm volatile(
		"syscall"
		:"=a"(ret)
		: "a"(SYSCALL_MMAP), "D"(addr), "S"(len), "d"(prot), "r"(r10), "r"(r8), "r"(r9)
		: "rcx", "r11", "memory"
	);

	return ret;
}


int syscall_munmap(void * addr, size_t len) {
	unsigned ret;

	asm volatile(
		"syscall"
		: "=a"(ret)
		: "a"(SYSCALL_MUNMAP), "D"(addr), "S"(len)
		: "rcx", "r11", "memory"
	);

	return ret;
}

void syscall_close(unsigned int fd) {
	unsigned ret;

	asm volatile(
		"syscall"
		:"=a"(ret)
		: "a"(SYSCALL_CLOSE), "D"(fd)
		: "memory"
	);
}