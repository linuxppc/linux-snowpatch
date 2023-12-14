/* SPDX-License-Identifier: GPL-2.0 */
/*
 * powerpc KMSAN support.
 *
 */

#ifndef _ASM_POWERPC_KMSAN_H
#define _ASM_POWERPC_KMSAN_H

#ifdef CONFIG_KMSAN
#define EXPORT_SYMBOL_KMSAN(fn) SYM_FUNC_ALIAS(__##fn, fn) \
				EXPORT_SYMBOL(__##fn)
#else
#define EXPORT_SYMBOL_KMSAN(fn)
#endif

#ifndef __ASSEMBLY__
#ifndef MODULE

#include <linux/mmzone.h>
#include <asm/page.h>
#include <asm/book3s/64/pgtable.h>

/*
 * Functions below are declared in the header to make sure they are inlined.
 * They all are called from kmsan_get_metadata() for every memory access in
 * the kernel, so speed is important here.
 */

/*
 * No powerpc specific metadata locations
 */
static inline void *arch_kmsan_get_meta_or_null(void *addr, bool is_origin)
{
	unsigned long addr64 = (unsigned long)addr, off;
	if (KERN_IO_START <= addr64 && addr64 < KERN_IO_END) {
		off = addr64 - KERN_IO_START;
		return (void *)off + (is_origin ? KERN_IO_ORIGIN_START : KERN_IO_SHADOW_START);
	} else {
		return 0;
	}
}

static inline bool kmsan_virt_addr_valid(void *addr)
{
	return (unsigned long)addr >= PAGE_OFFSET && pfn_valid(virt_to_pfn(addr));
}

#endif /* !MODULE */
#endif /* !__ASSEMBLY__ */
#endif /* _ASM_POWERPC_KMSAN_H */
