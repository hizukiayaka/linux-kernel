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

#ifndef _RBSP_H_
#define _RBSP_H_

struct rbsp;

int rbsp_init(struct rbsp *rbsp, char *buf, int size, int bit_pos);
int rbsp_write_flag(struct rbsp *rbsp, int bit);
int rbsp_write_bits(struct rbsp *rbsp, int num, int value);
int rbsp_write_uev(struct rbsp *rbsp, unsigned int value);

#endif
