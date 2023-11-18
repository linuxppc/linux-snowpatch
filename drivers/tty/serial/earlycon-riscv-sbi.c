// SPDX-License-Identifier: GPL-2.0
/*
 * RISC-V SBI based earlycon
 *
 * Copyright (C) 2018 Anup Patel <anup@brainfault.org>
 */
#include <linux/kernel.h>
#include <linux/console.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <asm/sbi.h>

static void sbi_putc(struct uart_port *port, unsigned char c)
{
	sbi_console_putchar(c);
}

static void sbi_0_1_console_write(struct console *con,
				  const char *s, unsigned int n)
{
	struct earlycon_device *dev = con->data;
	uart_console_write(&dev->port, s, n, sbi_putc);
}

static void sbi_dbcn_console_write(struct console *con,
				   const char *s, unsigned int n)
{
	sbi_debug_console_write(n, __pa(s));
}

static int __init early_sbi_setup(struct earlycon_device *device,
				  const char *opt)
{
	int ret = 0;

	if (sbi_debug_console_available) {
		device->con->write = sbi_dbcn_console_write;
	} else {
		if (IS_ENABLED(CONFIG_RISCV_SBI_V01))
			device->con->write = sbi_0_1_console_write;
		else
			ret = -ENODEV;
	}

	return ret;
}
EARLYCON_DECLARE(sbi, early_sbi_setup);
