// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012 Vista Silicon S.L.
 * Copyright (C) 2019 Randy Li <ayaka@soulik.info>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <asm-generic/errno-base.h>
#include <linux/kernel.h>
#include <linux/types.h>

struct rbsp {
	char *buf;
	int size;
	int pos;
};

int rbsp_init(struct rbsp *rbsp, char *buf, int size, int bit_pos)
{
	if (!buf)
		return -EINVAL;
	if (DIV_ROUND_UP(bit_pos, 8) >= size)
		return -EINVAL;

	rbsp->buf = buf;
	rbsp->size = size;
	rbsp->pos = bit_pos;

	return 0;
}

static inline int rbsp_read_bit(struct rbsp *rbsp)
{
	int shift = 7 - (rbsp->pos % 8);
	int ofs = rbsp->pos++ / 8;

	if (ofs >= rbsp->size)
		return -EINVAL;

	return (rbsp->buf[ofs] >> shift) & 1;
}

static inline int rbsp_write_bit(struct rbsp *rbsp, int bit)
{
	int shift = 7 - (rbsp->pos % 8);
	int ofs = rbsp->pos++ / 8;

	if (ofs >= rbsp->size)
		return -EINVAL;

	rbsp->buf[ofs] &= ~(1 << shift);
	rbsp->buf[ofs] |= bit << shift;

	return 0;
}

static inline int rbsp_read_bits(struct rbsp *rbsp, int num, int *val)
{
	int i, ret;
	int tmp = 0;

	if (num > 32)
		return -EINVAL;

	for (i = 0; i < num; i++) {
		ret = rbsp_read_bit(rbsp);
		if (ret < 0)
			return ret;
		tmp |= ret << (num - i - 1);
	}

	if (val)
		*val = tmp;

	return 0;
}

int rbsp_write_bits(struct rbsp *rbsp, int num, int value)
{
	int ret;

	while (num--) {
		ret = rbsp_write_bit(rbsp, (value >> num) & 1);
		if (ret)
			return ret;
	}

	return 0;
}

int rbsp_write_flag(struct rbsp *rbsp, int value)
{
	if (value)
		return rbsp_write_bit(rbsp, BIT(0));
	else
		return rbsp_write_bit(rbsp, 0);
}

static int rbsp_read_uev(struct rbsp *rbsp, unsigned int *val)
{
	int leading_zero_bits = 0;
	unsigned int tmp = 0;
	int ret;

	while ((ret = rbsp_read_bit(rbsp)) == 0)
		leading_zero_bits++;
	if (ret < 0)
		return ret;

	if (leading_zero_bits > 0) {
		ret = rbsp_read_bits(rbsp, leading_zero_bits, &tmp);
		if (ret)
			return ret;
	}

	if (val)
		*val = (1 << leading_zero_bits) - 1 + tmp;

	return 0;
}

static int rbsp_write_uev(struct rbsp *rbsp, unsigned int value)
{
	int i;
	int ret;
	int tmp = value + 1;
	int leading_zero_bits = fls(tmp) - 1;

	for (i = 0; i < leading_zero_bits; i++) {
		ret = rbsp_write_bit(rbsp, 0);
		if (ret)
			return ret;
	}

	return rbsp_write_bits(rbsp, leading_zero_bits + 1, tmp);
}

static int rbsp_read_sev(struct rbsp *rbsp, int *val)
{
	unsigned int tmp;
	int ret;

	ret = rbsp_read_uev(rbsp, &tmp);
	if (ret)
		return ret;

	if (val) {
		if (tmp & 1)
			*val = (tmp + 1) / 2;
		else
			*val = -(tmp / 2);
	}

	return 0;
}
