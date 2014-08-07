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
 *
 * Portions of this file changed by Alexei Andropov for ffdcaenc
*/

#ifndef WAVFILE_H
#define WAVFILE_H

#include <stdio.h>
#include <stdint.h>

#define UNKNOWN_SIZE 0xFFFFFFFF

typedef struct {
	FILE * file;
	uint32_t channels;
	uint32_t bits_per_sample;
	uint32_t sample_rate;
	uint32_t samples_left;
} wavfile;

wavfile * wavfile_open(const char * filename, const char ** error_msg, const int32_t ignore_len);
size_t wavfile_read_s32(wavfile * f, int32_t *samples, size_t sample_count);
size_t multi_mono_wavfile_read_s32(wavfile * f0, wavfile * f1, wavfile * f2, wavfile * f3, wavfile * f4, wavfile * f5, int32_t *samples, size_t sample_count, int32_t channel_config, int32_t has_lfe);
void wavfile_close(wavfile * f);

#endif
