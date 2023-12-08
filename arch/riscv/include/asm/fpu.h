/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2023 SiFive
 */

#ifndef _ASM_RISCV_FPU_H
#define _ASM_RISCV_FPU_H

#include <asm/switch_to.h>

#define kernel_fpu_available()	has_fpu()

#ifdef __riscv_f

#define kernel_fpu_begin() \
	static_assert(false, "floating-point code must use a separate translation unit")
#define kernel_fpu_end() kernel_fpu_begin()

#else

void kernel_fpu_begin(void);
void kernel_fpu_end(void);

#endif

#endif /* ! _ASM_RISCV_FPU_H */
