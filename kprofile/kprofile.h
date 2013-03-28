/*
 * =====================================================================================
 *
 *       Filename:  kprofile.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/28/2013 04:40:23 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Chen Yuheng (Chen Yuheng), chyh1990@163.com
 *   Organization:  Tsinghua Unv.
 *
 * =====================================================================================
 */

#ifndef __KPROFILE_H
#define __KPROFILE_H

void kprofile_init(const char* fn);
void kprofile_stop(void);
void _kmcount_internal(unsigned long frompc, unsigned long selfpc); /* _mcount; may be static, inline, etc */
void _kmcleanup (void);
void __kwrite_profiling (void);
void __kmonstartup (unsigned long lowpc, unsigned long highpc);
int
__kprofil (unsigned short *sample_buffer, size_t size, size_t offset, unsigned int scale);
int __kprofile_frequency (void);

#endif

