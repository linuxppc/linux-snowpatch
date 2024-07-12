/* Copyright 2017 NXP Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *	 notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *	 notice, this list of conditions and the following disclaimer in the
 *	 documentation and/or other materials provided with the distribution.
 *     * Neither the name of NXP Semiconductor nor the
 *	 names of its contributors may be used to endorse or promote products
 *	 derived from this software without specific prior written permission.
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP Semiconductor ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NXP Semiconductor BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/dma-mapping.h>
#include "dpaa_sys.h"

static int qbman_reserved_mem_lookup(struct device_node *mem_node,
				     dma_addr_t *addr, size_t *size)
{
	struct reserved_mem *rmem;

	rmem = of_reserved_mem_lookup(mem_node);
	if (!rmem) {
		pr_err("of_reserved_mem_lookup(%pOF) returned NULL\n", mem_node);
		return -ENODEV;
	}
	*addr = rmem->base;
	*size = rmem->size;

	return 0;
}

/**
 * qbman_find_reserved_mem_by_idx() - Find QBMan reserved-memory node
 * @dev: Pointer to QMan or BMan device structure
 * @idx: for BMan, pass 0 for the FBPR region.
 *	 for QMan, pass 0 for the FQD region and 1 for the PFDR region.
 * @addr: Pointer to storage for the region's base address.
 * @size: Pointer to storage for the region's size.
 */
int qbman_find_reserved_mem_by_idx(struct device *dev, int idx,
				   dma_addr_t *addr, size_t *size)
{
	struct device_node *mem_node;

	mem_node = of_parse_phandle(dev->of_node, "memory-region", idx);
	if (!mem_node)
		return -ENODEV;

	return qbman_reserved_mem_lookup(mem_node, addr, size);
}

/**
 * qbman_find_reserved_mem_by_compatible() - Find QBMan reserved-memory node (PowerPC)
 * @dev: Pointer to QMan or BMan device structure
 * @compat: one of "fsl,bman-fbpr", "fsl,qman-fqd" or "fsl,qman-pfdr"
 * @addr: Pointer to storage for the region's base address.
 * @size: Pointer to storage for the region's size.
 *
 * This is a legacy variant of qbman_find_reserved_mem_by_idx(), which should
 * only be used for backwards compatibility with certain PowerPC device trees.
 */
int qbman_find_reserved_mem_by_compatible(struct device *dev, const char *compat,
					  dma_addr_t *addr, size_t *size)
{
	struct device_node *mem_node;

	mem_node = of_find_compatible_node(NULL, NULL, compat);
	if (!mem_node)
		return -ENODEV;

	return qbman_reserved_mem_lookup(mem_node, addr, size);
}
