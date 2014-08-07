/* 
 * This file is part of dcaenc.
 *
 * Copyright (c) 2008-2012 Alexander E. Patrakov <patrakov@gmail.com>
 *
 * dcaenc is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * dcaenc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dcaenc; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef DCAENC_H
#define DCAENC_H

#include <stdint.h>

typedef struct dcaenc_context_s *dcaenc_context;

#define DCAENC_FLAG_28BIT 1
#define DCAENC_FLAG_BIGENDIAN 2
#define DCAENC_FLAG_LFE 4
#define DCAENC_FLAG_PERFECT_QMF 8
#define DCAENC_FLAG_IEC_WRAP 16

#define DCAENC_CHANNELS_MONO 0
#define DCAENC_CHANNELS_DUAL_MONO 1
#define DCAENC_CHANNELS_STEREO 2
#define DCAENC_CHANNELS_STEREO_SUMDIFF 3
#define DCAENC_CHANNELS_STEREO_TOTAL 4
#define DCAENC_CHANNELS_3FRONT 5
#define DCAENC_CHANNELS_2FRONT_1REAR 6
#define DCAENC_CHANNELS_3FRONT_1REAR 7
#define DCAENC_CHANNELS_2FRONT_2REAR 8
#define DCAENC_CHANNELS_3FRONT_2REAR 9
#define DCAENC_CHANNELS_4FRONT_2REAR 10
#define DCAENC_CHANNELS_3FRONT_2REAR_1OV 11
#define DCAENC_CHANNELS_3FRONT_3REAR 12
#define DCAENC_CHANNELS_5FRONT_2REAR 13
#define DCAENC_CHANNELS_4FRONT_4REAR 14
#define DCAENC_CHANNELS_5FRONT_3REAR 15

dcaenc_context dcaenc_create(int32_t sample_rate, int32_t channel_config, int32_t approx_bitrate, int32_t flags);
int32_t dcaenc_bitrate(dcaenc_context c);
int32_t dcaenc_input_size(dcaenc_context c);
int32_t dcaenc_output_size(dcaenc_context c);
int32_t dcaenc_convert_s32(dcaenc_context c, const int32_t *input, uint8_t *output);
int32_t dcaenc_destroy(dcaenc_context c, uint8_t *output);

#endif
