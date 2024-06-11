/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Author: Kevin Wells <kevin.wells@nxp.com>
 *
 * Copyright (C) 2008 NXP Semiconductors
 * Copyright 2023 Timesys Corporation <piotr.wojtaszczyk@timesys.com>
 */

#ifndef __SOUND_SOC_LPC3XXX_I2S_H
#define __SOUND_SOC_LPC3XXX_I2S_H

#include <linux/types.h>
#include <linux/regmap.h>

struct lpc3xxx_i2s_info {
	struct device *dev;
	struct clk *clk;
	struct mutex lock; /* To serialize user-space access */
	struct regmap *regs;
	u32 streams_in_use;
	u32 clkrate;
	int freq;
	struct snd_dmaengine_dai_dma_data playback_dma_config;
	struct snd_dmaengine_dai_dma_data capture_dma_config;
};

int lpc3xxx_pcm_register(struct platform_device *pdev);

#define _SBF(f, v) ((v) << (f))
#define _BIT(n) _SBF(n, 1)

/* I2S controller register offsets */
#define I2S_DAO		0x00
#define I2S_DAI		0x04
#define I2S_TX_FIFO	0x08
#define I2S_RX_FIFO	0x0C
#define I2S_STAT	0x10
#define I2S_DMA0	0x14
#define I2S_DMA1	0x18
#define I2S_IRQ		0x1C
#define I2S_TX_RATE	0x20
#define I2S_RX_RATE	0x24

/* i2s_daO i2s_dai register definitions */
#define I2S_WW8      _SBF(0, 0) /* Word width is 8bit*/
#define I2S_WW16     _SBF(0, 1) /* Word width is 16bit*/
#define I2S_WW32     _SBF(0, 3) /* Word width is 32bit*/
#define I2S_MONO     _BIT(2)   /* Mono */
#define I2S_STOP     _BIT(3)   /* Stop, diables the access to FIFO, mutes the channel */
#define I2S_RESET    _BIT(4)   /* Reset the channel */
#define I2S_WS_SEL   _BIT(5)   /* Channel Master(0) or slave(1) mode select*/
#define I2S_WS_HP(s) _SBF(6, s) /* Word select half period - 1 */

#define I2S_MUTE     _BIT(15)  /* Mute the channel, Transmit channel only */

#define I2S_WW32_HP  0x1f /* Word select half period for 32bit word width */
#define I2S_WW16_HP  0x0f /* Word select half period for 16bit word width */
#define I2S_WW8_HP   0x7  /* Word select half period for 8bit word width */

#define WSMASK_HP	  0X7FC /* Mask for WS half period bits */

/* i2s_tx_fifo register definitions */
#define I2S_FIFO_TX_WRITE(d)              (d)

/* i2s_rx_fifo register definitions */
#define I2S_FIFO_RX_WRITE(d)              (d)

/* i2s_stat register definitions */
#define I2S_IRQ_STAT     _BIT(0)
#define I2S_DMA0_REQ     _BIT(1)
#define I2S_DMA1_REQ     _BIT(2)

#define I2S_RX_STATE_MASK	0x0000ff00
#define I2S_TX_STATE_MASK	0x00ff0000

/* i2s_dma0 Configuration register definitions */
#define I2S_DMA0_RX_EN     _BIT(0)       /* Enable RX DMA1*/
#define I2S_DMA0_TX_EN     _BIT(1)       /* Enable TX DMA1*/
#define I2S_DMA0_RX_DEPTH(s)  _SBF(8, s)  /* Set the level for DMA1 RX Request */
#define I2S_DMA0_TX_DEPTH(s)  _SBF(16, s) /* Set the level for DMA1 TX Request */

/* i2s_dma1 Configuration register definitions */
#define I2S_DMA1_RX_EN     _BIT(0)       /* Enable RX DMA1*/
#define I2S_DMA1_TX_EN     _BIT(1)       /* Enable TX DMA1*/
#define I2S_DMA1_RX_DEPTH(s)  _SBF(8, s) /* Set the level for DMA1 RX Request */
#define I2S_DMA1_TX_DEPTH(s)  _SBF(16, s) /* Set the level for DMA1 TX Request */

/* i2s_irq register definitions */
#define I2S_RX_IRQ_EN     _BIT(0)       /* Enable RX IRQ*/
#define I2S_TX_IRQ_EN     _BIT(1)       /* Enable TX IRQ*/
#define I2S_IRQ_RX_DEPTH(s)  _SBF(8, s)  /* valid values ar 0 to 7 */
#define I2S_IRQ_TX_DEPTH(s)  _SBF(16, s) /* valid values ar 0 to 7 */

#endif
