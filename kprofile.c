/*
 * =====================================================================================
 *
 *       Filename:  profile.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/28/2013 04:34:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Chen Yuheng (Chen Yuheng), chyh1990@163.com
 *   Organization:  Tsinghua Unv.
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config-host.h"
#include "elf.h"

#include "monitor.h"
#include "sysemu.h"
#include "gdbstub.h"
#include "dma.h"
#include "kvm.h"
#include "qmp-commands.h"

#include "qemu-thread.h"
#include "cpus.h"
#include "qtest.h"
#include "gdbstub.h"

#include "cpus.h"
#include "kprofile.h"

static FILE* elffd = NULL;
int kprofile_on = 0;
static Elf64_Shdr max_shdr;

static inline void kprofile_start(unsigned long lowpc, unsigned long highpc)
{
	__kmonstartup(lowpc, highpc);
}

void kprofile_init(const char* fn)
{
	static int called = 0;
	int i;
	if(called)
		return;
	called = 1;
	elffd = fopen(fn, "rb");
	if(!elffd){
		perror("kprofile");
		return;
	}
	Elf64_Ehdr hdr;
	Elf64_Shdr shdr;
	fread(&hdr, sizeof(hdr), 1, elffd);
	if(memcmp(hdr.e_ident, ELFMAG, 4)){
		fprintf(stderr, "kprofile: %s not an ELF\n", fn);
		goto bad;
	}
	if(hdr.e_ident[EI_CLASS] != ELFCLASS64){
		fprintf(stderr, "kprofile: %s not an ELF64\n", fn);
		goto bad;
	}
	fseek(elffd, hdr.e_shoff, SEEK_SET);
	max_shdr.sh_size = 0;
	for(i=0;i<hdr.e_shnum;i++){
		fread(&shdr, sizeof(shdr), 1, elffd);
		if(shdr.sh_type != SHT_PROGBITS)
			continue;
		if(!(shdr.sh_flags & SHF_EXECINSTR))
			continue;
		if(shdr.sh_size >= max_shdr.sh_size)
			max_shdr = shdr;
	}
	if(max_shdr.sh_size == 0){
		fprintf(stderr, "kprofile: %s .text not found\n", fn);
		goto bad;
	}
	fprintf(stderr, "kprofile: Text Section: %p %d\n", (void*)max_shdr.sh_addr, (int)max_shdr.sh_size);

	kprofile_start(max_shdr.sh_addr, max_shdr.sh_addr + max_shdr.sh_size);

	kprofile_on = 1;
	fprintf(stderr, "kprofile: ok\n");
bad:
	fclose(elffd);
}

void kprofile_stop(void)
{
	if(!kprofile_on)
		return;
	fprintf(stderr, "kprofile: saving gmon\n");
	_kmcleanup();
}

int kprofile_get_eips(unsigned long eips[])
{
    	CPUArchState *env;
	int cnt = 0;
    	for (env = first_cpu; cnt < MAX_PROFILE_CPUS && env != NULL; env = env->next_cpu) {
        cpu_synchronize_state(env);
	if(env->halted)
		continue;
#if defined(TARGET_I386)
	//XXX only for longmode
	eips[cnt++] = env->eip;// + env->segs[R_CS].base;
#endif
	}
	return cnt;
}


