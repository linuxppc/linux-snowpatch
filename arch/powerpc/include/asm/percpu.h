/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_POWERPC_PERCPU_H_
#define _ASM_POWERPC_PERCPU_H_
#ifdef __powerpc64__

/*
 * Same as asm-generic/percpu.h, except that we store the per cpu offset
 * in the paca. Based on the x86-64 implementation.
 */

#ifdef CONFIG_SMP

#define __my_cpu_offset local_paca->data_offset

#endif /* CONFIG_SMP */
#endif /* __powerpc64__ */

#include <asm-generic/percpu.h>

#include <asm/paca.h>
#include <asm/cmpxchg.h>
#ifdef this_cpu_cmpxchg_1
#undef this_cpu_cmpxchg_1
#define this_cpu_cmpxchg_1(pcp, oval, nval)	__cmpxchg_local(raw_cpu_ptr(&(pcp)), oval, nval, 1)
#endif 
#ifdef this_cpu_cmpxchg_2
#undef this_cpu_cmpxchg_2
#define this_cpu_cmpxchg_2(pcp, oval, nval)	__cmpxchg_local(raw_cpu_ptr(&(pcp)), oval, nval, 2)
#endif
#ifdef this_cpu_cmpxchg_4
#undef this_cpu_cmpxchg_4
#define this_cpu_cmpxchg_4(pcp, oval, nval)	__cmpxchg_local(raw_cpu_ptr(&(pcp)), oval, nval, 4)
#endif
#ifdef this_cpu_cmpxchg_8
#undef this_cpu_cmpxchg_8
#define this_cpu_cmpxchg_8(pcp, oval, nval)	__cmpxchg_local(raw_cpu_ptr(&(pcp)), oval, nval, 8)
#endif

#endif /* _ASM_POWERPC_PERCPU_H_ */
