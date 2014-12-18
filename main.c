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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "config.h"

#include "ffdcaenc.h"
#include "wavfile.h"
#include "xgetopt.h"
#include "compiler_info.h"

//extern const int32_t prototype_filter[512];

static char status[4] = {'|','/','-','\\'};
static const int32_t AUTO_SELECT = -1;

#define BUFFSIZE_SPL 512
#define BUFFSIZE_CHN 6
#define BUFFSIZE_OUT 16384

static int32_t dcaenc_main(int32_t argc, char *argv[])
{
	dcaenc_context c;
	int32_t data[BUFFSIZE_SPL * BUFFSIZE_CHN];
	uint8_t output[BUFFSIZE_OUT];
	wavfile * f;
	wavfile * mono_f[6];
	FILE * outfile;
	const char *error_msg;
	uint32_t samples_total;
	uint32_t mono_samples_total[6];
	size_t samples_read;
	size_t samples_read_total, current_pos;
	double percent_done;
	int32_t i;
	int32_t bitrate;
	int32_t wrote;
	int32_t counter;
	int32_t status_idx;
	int32_t show_ver;
	int32_t show_help;
	int32_t ignore_len;
	int32_t enc_flags;
	int32_t channel_config;
	int32_t has_lfe;
	int32_t multi_mono;
	int32_t any_input;
	int32_t mono_channels;
	int32_t an_input;
	xgetopt_t opt;
	char t;
	char *file_input;
	char *file_output;
	char *mono_file_input[6];

	static const int32_t channel_map[6] = {DCAENC_CHANNELS_MONO, DCAENC_CHANNELS_STEREO, DCAENC_CHANNELS_3FRONT,
		DCAENC_CHANNELS_2FRONT_2REAR, DCAENC_CHANNELS_3FRONT_2REAR, DCAENC_CHANNELS_3FRONT_2REAR};

	fprintf(stderr, " \n");

	// ----------------------------

	file_input = NULL;
	for (i=0; i<6;i++)
		mono_file_input[i] = NULL;

	bitrate = 0;
	enc_flags = DCAENC_FLAG_BIGENDIAN;
	channel_config = AUTO_SELECT;
	has_lfe = 0;
	multi_mono = 0;
	show_ver = 0;
	ignore_len = 0;
	show_help = 0;

	memset(&opt, 0, sizeof(xgetopt_t));
	while((t = xgetopt(argc, argv, "i:o:b:c:0:1:2:3:4:5:fhmlrev", &opt)) != EOF)
	{
		switch(t)
		{
		case 'i':
			file_input = opt.optarg;
			break;
		case '0':
			mono_file_input[0] = opt.optarg;
			break;
		case '1':
			mono_file_input[1] = opt.optarg;
			break;
		case '2':
			mono_file_input[2] = opt.optarg;
			break;
		case '3':
			mono_file_input[3] = opt.optarg;
			break;
		case '4':
			mono_file_input[4] = opt.optarg;
			break;
		case '5':
			mono_file_input[5] = opt.optarg;
			break;
		case 'o':
			file_output = opt.optarg;
			break;
		case 'b':
			bitrate = (int)(atof(opt.optarg) * 1000.0f);
			if(bitrate > 6144000 || bitrate < 32000)
			{
				fprintf(stderr, "Bitrate must be between 32 and 6144 kbps!\n");
				return 1;
			}
			break;
		case 'c':
			channel_config = atoi(opt.optarg) - 1;
			if((channel_config < 0) || (channel_config > 15))
			{
				fprintf(stderr, "Bad channel configuration. Must be between 1 and 16!\n");
				return 1;
			}
			break;
		case 'f':
			has_lfe = 1;
			break;
		case 'm':
			multi_mono = 1;
			break;
		case 'h':
			show_help = 1;
			break;
		case 'l':
			ignore_len = 1;
			break;
		case 'e':
			enc_flags = enc_flags & (~DCAENC_FLAG_BIGENDIAN);
			break;
		case 'r':
			enc_flags = enc_flags | DCAENC_FLAG_28BIT;
			break;
		case 'v':
			show_ver = 1;
			break;
		case '?':
			fprintf(stderr, "Unknown commandline option or missing argument: %s\n", argv[opt.optind-1]);
			return 1;
		}
	}
	
	// ----------------------------

	if (multi_mono) {
		any_input = 0;
		for (i=0; i<6; i++){
			if(mono_file_input[i])
			any_input = 1;
		}
	}
	else {
		if (file_input)
			any_input = 1;
		else
			any_input = 0;
	}
	if(!any_input || !file_output || bitrate < 1 || show_ver || show_help)
	{
		if(show_ver)
		{
			printf(PACKAGE_NAME "-" PACKAGE_VERSION "\n");
			printf("compiled on " __DATE__ " at " __TIME__ " using " __COMPILER__ ";\n\n");
			printf(PACKAGE_URL "\n");
			printf("http://gitorious.org/~mulder/dtsenc/mulders-dtsenc \n\n");
			printf("Copyright (c) 2008-2012 Alexander E. Patrakov <patrakov@gmail.com>\n");
			printf("This program is free software: you can redistribute it and/or modify it\n");
			printf("under the terms of the GNU Lesser General Public License.\n");
			printf("Note that this program is distributed with ABSOLUTELY NO WARRANTY.\n\n");
			printf("Portions changed by Alexei Andropov for ffdcaenc.\n");
			printf("Linux-compatibility restored by filler56789.\n");
			printf("Windows stdin support fixed by Kurtnoise.\n\n");
			return 0;
		}
		else if(show_help)
		{
			printf("FFDCAENC --- experimental 'Coherent Acoustics' compressor.\n\n");
			printf("Usage:\n  ffdcaenc -i <input.wav> -o <output.dts> -b <bitrate_kbps>\n\n");
			printf("Optional:\n");
			printf("  -l  Ignore input length, can be useful when reading from stdin\n");
			printf("  -e  Switch output endianess to Little Endian (default is: Big-Endian)\n");
			printf("  -r  Reduced Bit Depth for DTS CD format (default is: Full Bit-Depth)\n");
			printf("  -h  Print this help screen\n");
			printf("  -c  Overwrite the channel configuration (default is: auto-selection)\n");
			printf("  -f  Add an additional LFE channel (default: used for 6-channel input)\n");
			printf("  -m  Multiple Mono input files (default: -i for multi-channel input file)\n");
			printf("       Use -0 <input.wav> -1 <input.wav> etc. (up to -5) Channels are in ITU order:\n");
			printf("       0,1,2,3,4,5 --> LF, RF, C, LFE, LS, RS\n");
			printf("       The following mono input file combinations are supported:\n");
			printf("		1.0		-2 center.wav\n");
			printf("		1.1		-2 center.wav -3 lfe-wav\n");
			printf("		2.0		-0 left.wav -1 right.wav\n");
			printf("		2.1		-0 left.wav -1 right.wav -3 lfe.wav\n");
			printf("		3.0		-0 left.wav -1 right.wav -2 center.wav\n");
			printf("		3.1		-0 left.wav -1 right.wav -2 center.wav -3 lfe.wav\n");
			printf("		4.0		-0 left.wav -1 right.wav -4 ls.wav -5 rs.wav\n");
			printf("		4.1		-0 left.wav -1 right.wav -4 ls.wav -5 rs.wav -3 lfe.wav\n");
			printf("		5.0		-0 left.wav -1 right.wav -2 center.wav -4 ls.wav -5 rs.wav\n");
			printf("		5.1		-0 left.wav -1 right.wav -2 center.wav -4 ls.wav -5 rs.wav -3 lfe.wav\n\n");
			printf("  -v  Show version info\n\n");
			printf("REMARKS:\n");
			printf("The input or output filename can be \"-\" for stdin/stdout.\n");
			printf("The bitrate is specified in kilobits per second and may be rounded up\n");
			printf("-- use floating-point values for bitrates that are not a multiple of 1 kbps.\n");
			printf("Because the encoder uses a 4-byte granularity, i.e., 32 bits per audio frame\n");
			printf("(with 512 samples/frame), the ACTUAL bitrate will always be a multiple of:\n\n");
			printf("3         kbps for     48 kHz\n");
			printf("2.75625   kbps for   44.1 kHz\n");
			printf("2         kbps for     32 kHz\n");
			printf("1.5       kbps for     24 kHz\n");
			printf("1.378125  kbps for  22.05 kHz\n");
			printf("1         kbps for     16 kHz\n");
			printf("0.75      kbps for     12 kHz\n");
			printf("0.6890625 kbps for 11.025 kHz\n");
			printf("0.5       kbps for      8 kHz\n\n");
			printf("-- NOTICE: the values 754.5 and 1509.75 AT _48kHz_ are an exception.\n\n");
			printf("* Available channel-layouts:\n\n");
			printf("  -  1: A\n");
			printf("  -  2: A, B\n");
			printf("  -  3: L, R\n");
			printf("  -  4: (L+R), (L-R)\n");
			printf("  -  5: Lt, Rt\n");
			printf("  -  6: FC, FL, FR\n");
			printf("  -  7: FL, FR, BC\n");
			printf("  -  8: FC, FL, FR, BC\n");
			printf("  -  9: FL, FR, BL, BR\n");
			printf("  - 10: FC, FL, FR, BL, BR\n");
			printf("  - 11: CL, CR, FL, FR, BL, BR (not supported)\n");
			printf("  - 12: FC, FL, FR, BL, BR, OV (not supported)\n");
			printf("  - 13: FC, BC, FL, FR, BL, BR (not supported)\n");
			printf("  - 14: CL, FC, CR, FL, FR, BL, BR (not supported)\n");
			printf("  - 15: CL, CR, FL, FR, SL1, SL2, SR1, SR2 (not supported)\n");
			printf("  - 16: CL, FC, CR, FL, FR, BL, BC, BR (not supported)\n\n");
			printf("* Valid sample rates (in kHz):\n\n  8 11.025 12 16 22.05 24 32 44.1 48\n\n");
			printf("* Transmission bitrates (in kbps):\n\n  32     56     64     96     112    128    192     224   \n  256    320    384    448    512    576    640     768   \n  960    1024   1152   1280   1344   1408   1411.2  1472  \n  1536   1920   2048   3072   3840   open   VBR     LOSSLESS\n");
			return 0;
		}
		else
		{
			printf("FFDCAENC --- experimental 'Coherent Acoustics' compressor.\n\n");
			printf("ERROR: Required arguments are missing.\nCall 'ffdcaenc -h' for more information.\n");
			return 1;
		}
	}

	if (multi_mono) {
		if ( !mono_file_input[0] && !mono_file_input[1] && !mono_file_input[2] && !mono_file_input[3] && !mono_file_input[4] && !mono_file_input[5]) {
			fprintf(stderr, "Missing Input File. -m option requires at least one input file\n");
			return 1;
		}


		if ( mono_file_input[0] )
			fprintf(stderr, "LF Source: %s\n", mono_file_input[0]);
		if ( mono_file_input[1] )
			fprintf(stderr, "RF Source: %s\n", mono_file_input[1]);
		if ( mono_file_input[2] )
			fprintf(stderr, "C Source: %s\n", mono_file_input[2]);
		if ( mono_file_input[3] )
			fprintf(stderr, "LFE Source: %s\n", mono_file_input[3]);
		if ( mono_file_input[4] )
			fprintf(stderr, "LS Source: %s\n", mono_file_input[4]);
		if ( mono_file_input[5] )
			fprintf(stderr, "RS Source: %s\n", mono_file_input[5]);
	}
	else
	fprintf(stderr, "FFDCAENC --- experimental 'Coherent Acoustics' compressor.\n\n");
	fprintf(stderr, "Input:   %s\n", file_input);
	fprintf(stderr, "Output:  %s\n", file_output);
	fprintf(stderr, "Bitrate: %u kbps\n\n", bitrate / 1000, bitrate % 1000);

	// ----------------------------

	if (!multi_mono) {

		f = wavfile_open(file_input, &error_msg, ignore_len);
		if (!f) {
			fprintf(stderr, "Could not open or parse \"%s\".\n", file_input);
			fprintf(stderr, "Error: %s!\n", error_msg);
			return 1;
		}
	
		samples_total = f->samples_left;

		if(channel_config == AUTO_SELECT)
		{
			channel_config = channel_map[f->channels - 1];
			if(f->channels == 6) has_lfe = 1;
		}

		if(has_lfe)
		{
			enc_flags = enc_flags | DCAENC_FLAG_LFE;
		}

		switch(f->channels - (has_lfe ? 1 : 0))
		{
		case 1:
			if(!(channel_config == DCAENC_CHANNELS_MONO))
			{
				fprintf(stderr, "Invalid channel configuration for input audio!\n");
				return 1;
			}
			break;
		case 2:
			if(!(channel_config == DCAENC_CHANNELS_DUAL_MONO || channel_config == DCAENC_CHANNELS_STEREO ||
				channel_config == DCAENC_CHANNELS_STEREO_SUMDIFF || channel_config == DCAENC_CHANNELS_STEREO_TOTAL))
			{
				fprintf(stderr, "Invalid channel configuration for input audio!\n");
				return 1;
			}
			break;
		case 4:
			if(!(channel_config == DCAENC_CHANNELS_3FRONT_1REAR || channel_config == DCAENC_CHANNELS_2FRONT_2REAR))
			{
				fprintf(stderr, "Invalid channel configuration for input audio!\n");
				return 1;
			}
			break;
		case 5:
			if(!(channel_config == DCAENC_CHANNELS_3FRONT_2REAR))
			{
				fprintf(stderr, "Invalid channel configuration for input audio!\n");
				return 1;
			}
			break;
		case 6:
			if(!(channel_config == DCAENC_CHANNELS_3FRONT_3REAR || channel_config == DCAENC_CHANNELS_4FRONT_2REAR ||
				 channel_config == DCAENC_CHANNELS_3FRONT_2REAR_1OV))
			{
				fprintf(stderr, "Invalid channel configuration for input audio!\n");
				return 1;
			}
			break;
		}
	}
	else { // multi_mono

		mono_channels = 0;
		has_lfe = 0;
		for (i=0; i<6; i++){
			if ( mono_file_input[i]) {
				mono_f[i] = wavfile_open(mono_file_input[i], &error_msg, ignore_len);
				if (!mono_f[i]) {
					fprintf(stderr, "Could not open or parse \"%s\".\n", mono_file_input[i]);
					fprintf(stderr, "Error: %s!\n", error_msg);
					return 1;
				}
				mono_samples_total[i] = mono_f[i]->samples_left;
				++ mono_channels;
			}
		}
		switch(mono_channels) {
		case 1:
			channel_config = DCAENC_CHANNELS_MONO;
			if (!mono_f[2]) {
				fprintf(stderr, "Mono requires -2\n");
				return 1;
			}
			an_input = 2;
			break;
		case 2:
			if ( !mono_file_input[3] ) {
				channel_config = DCAENC_CHANNELS_STEREO;
				if (!mono_f[0] ||!mono_f[1] ) {
						fprintf(stderr, "Stereo requires -0 and -1\n");
						return 1;
				}
				if ( mono_samples_total[0] != mono_samples_total[1]) {
					fprintf(stderr, "Multi mono inputs have different sample lengths\n");
					return 1;
				}
				if ( mono_f[0]->channels != 1 || mono_f[1]->channels != 1 ) {
					fprintf(stderr, "All input files must be mono (for multi-mono)\n");
					return 1;
				}
				an_input = 0;
			}
			else {
				channel_config = DCAENC_CHANNELS_MONO;
				if (!mono_f[2]) {
					fprintf(stderr, "1.1 requires -2 and -3\n");
					return 1;
				}
				if ( mono_samples_total[2] != mono_samples_total[3]) {
					fprintf(stderr, "Multi mono inputs have different sample lengths\n");
					return 1;
				}
				if ( mono_f[2]->channels != 1 || mono_f[3]->channels != 1 ) {
					fprintf(stderr, "All input files must be mono (for multi-mono)\n");
					return 1;
				}
				an_input = 2;
				has_lfe = 1;
			}
			break;
		case 3:
			if ( !mono_file_input[3] ) {
				channel_config = DCAENC_CHANNELS_3FRONT;
				if (!mono_f[0] || !mono_f[1] || !mono_f[2] ) {
					fprintf(stderr, "3.0 requires -0, -1, -2\n");
					return 1;
				}
				if ( mono_samples_total[0] != mono_samples_total[1] || mono_samples_total[0] != mono_samples_total[2]) {
					fprintf(stderr, "Multi mono inputs have different sample lengths\n");
					return 1;
				}
				if ( mono_f[0]->channels != 1 || mono_f[1]->channels != 1 || mono_f[2]->channels != 1) {
					fprintf(stderr, "All input files must be mono (for multi-mono)\n");
					return 1;
				}
				an_input = 0;
			}
			else {
				channel_config = DCAENC_CHANNELS_STEREO;
				if (!mono_f[0] ||!mono_f[1] ) {
						fprintf(stderr, "2.1 requires -0, -1 and -2\n");
						return 1;
				}
				if ( mono_samples_total[0] != mono_samples_total[1] || mono_samples_total[0] != mono_samples_total[3]) {
					fprintf(stderr, "Multi mono inputs have different sample lengths\n");
					return 1;
				}
				if ( mono_f[0]->channels != 1 || mono_f[1]->channels != 1 || mono_f[3]->channels != 1) {
					fprintf(stderr, "All input files must be mono (for multi-mono)\n");
					return 1;
				}
				an_input = 0;
				has_lfe = 1;
			}
			break;
		case 4:
			if ( !mono_file_input[3] ) {
				channel_config = DCAENC_CHANNELS_2FRONT_2REAR;
				if (!mono_f[0] || !mono_f[1] || !mono_f[4] || !mono_f[5]) {
						fprintf(stderr, "4.0 requires -0, -1, -4, -5\n");
						return 1;
					}
				if ( mono_samples_total[0] != mono_samples_total[1] || mono_samples_total[0] != mono_samples_total[4] || mono_samples_total[0] != mono_samples_total[5]) {
					fprintf(stderr, "Multi mono inputs have different sample lengths\n");
					return 1;
				}
				if ( mono_f[0]->channels != 1 || mono_f[1]->channels != 1 || mono_f[4]->channels != 1 || mono_f[5]->channels != 1 ) {
					fprintf(stderr, "All input files must be mono (for multi-mono)\n");
					return 1;
				}
				an_input = 0;
			}
			else {
				channel_config = DCAENC_CHANNELS_3FRONT;
				if (!mono_f[0] || !mono_f[1] || !mono_f[2] ) {
					fprintf(stderr, "3.1 requires -0, -1, -2, and -3\n");
					return 1;
				}
				if ( mono_samples_total[0] != mono_samples_total[1] || mono_samples_total[0] != mono_samples_total[2] || mono_samples_total[0] != mono_samples_total[3]) {
					fprintf(stderr, "Multi mono inputs have different sample lengths\n");
					return 1;
				}
				if ( mono_f[0]->channels != 1 || mono_f[1]->channels != 1 || mono_f[2]->channels != 1 || mono_f[3]->channels != 1) {
					fprintf(stderr, "All input files must be mono (for multi-mono)\n");
					return 1;
				}
				an_input = 0;
				has_lfe = 1;
			}
			break;
		case 5:
			if ( !mono_file_input[3] ) {
				channel_config = DCAENC_CHANNELS_3FRONT_2REAR;
				if (!mono_f[0] || !mono_f[1] || !mono_f[2] || !mono_f[4] || !mono_f[5]) {
					fprintf(stderr, "5.0 requires -0, -1, -2, -4, -5\n");
					return 1;
				}
				if ( mono_samples_total[0] != mono_samples_total[1] || mono_samples_total[0] != mono_samples_total[2] || mono_samples_total[0] != mono_samples_total[4] || mono_samples_total[0] != mono_samples_total[5]) {
					fprintf(stderr, "Multi mono inputs have different sample lengths\n");
					return 1;
				}
				if ( mono_f[0]->channels != 1 || mono_f[1]->channels != 1 || mono_f[2]->channels != 1 || mono_f[4]->channels != 1 || mono_f[5]->channels != 1 ) {
					fprintf(stderr, "All input files must be mono (for multi-mono)\n");
					return 1;
				}
			}
			else {
				channel_config = DCAENC_CHANNELS_2FRONT_2REAR;
				if (!mono_f[0] || !mono_f[1] || !mono_f[3] || !mono_f[4] || !mono_f[5]) {
						fprintf(stderr, "4.1 requires -0, -1, -3, -4, -5\n");
						return 1;
				}
				if ( mono_samples_total[0] != mono_samples_total[1] || mono_samples_total[0] != mono_samples_total[3] || mono_samples_total[0] != mono_samples_total[4] || mono_samples_total[0] != mono_samples_total[5]) {
					fprintf(stderr, "Multi mono inputs have different sample lengths\n");
					return 1;
				}
				if ( mono_f[0]->channels != 1 || mono_f[1]->channels != 1 || mono_f[3]->channels != 1|| mono_f[4]->channels != 1 || mono_f[5]->channels != 1 ) {
					fprintf(stderr, "All input files must be mono (for multi-mono)\n");
					return 1;
				}
				has_lfe = 1;
			}
			an_input = 0;
			break;
		case 6:
			channel_config = DCAENC_CHANNELS_3FRONT_2REAR;
			if ( mono_samples_total[0] != mono_samples_total[1] || mono_samples_total[0] != mono_samples_total[2]  || mono_samples_total[0] != mono_samples_total[3] || mono_samples_total[0] != mono_samples_total[4] || mono_samples_total[0] != mono_samples_total[5]) {
				fprintf(stderr, "Multi mono inputs have different sample lengths\n");
				return 1;
			}
			if ( mono_f[0]->channels != 1 || mono_f[1]->channels != 1 || mono_f[2]->channels != 1 || mono_f[3]->channels != 1 || mono_f[4]->channels != 1 || mono_f[5]->channels != 1 ) {
				fprintf(stderr, "All input files must be mono (for multi-mono)\n");
				return 1;
			}
			has_lfe = 1;
			break;
			an_input = 0;
		}	

		if(has_lfe)
		{
			enc_flags = enc_flags | DCAENC_FLAG_LFE;
		}


	}

	// ----------------------------

	c = (multi_mono ? dcaenc_create(mono_f[an_input]->sample_rate, channel_config, bitrate, enc_flags) : dcaenc_create(f->sample_rate, channel_config, bitrate, enc_flags));
	
	if (!c) {
		fprintf(stderr, "ERROR: Invalid bitrate or sample rate.\n");
		return 1;
	}


	if((((size_t)(dcaenc_output_size(c))) > BUFFSIZE_OUT) || (((size_t)(dcaenc_input_size(c))) > BUFFSIZE_SPL))
	{
		fprintf(stderr, "Internal error, buffers are too small!\n", file_output);
		return 1;
	}
	
	outfile = strcmp(file_output, "-") ? fopen(file_output, "wb") : stdout;
	if(!outfile) {
		fprintf(stderr, "Could not open \"%s\" for writing!\n", file_output);
		return 1;
	}
	
	fflush(stdout);
	fflush(stderr);
	
	// ----------------------------

	counter = 0;
	samples_read_total = 0;
	status_idx = 0;
	
	while(samples_read = ( multi_mono ? multi_mono_wavfile_read_s32(mono_f[an_input], mono_f[1], mono_f[2], mono_f[3], mono_f[4], mono_f[5], data, BUFFSIZE_SPL, channel_config, has_lfe) : wavfile_read_s32(f, data, BUFFSIZE_SPL)))
	{
		samples_read_total += samples_read;
		wrote = dcaenc_convert_s32(c, data, output);
		fwrite(output, 1, wrote, outfile);
		
		if(counter == 0)
		{
			current_pos = samples_read_total / (multi_mono ? mono_f[an_input]->sample_rate :f->sample_rate) ;
			
			if((samples_total > 0) && (samples_total < UNKNOWN_SIZE))
			{
				percent_done = ((double)(samples_total - (multi_mono ? mono_f[an_input]->samples_left :f->samples_left))) / ((double)(samples_total));
				fprintf(stderr, "Encoding... %u:%02u [%3.1f%%]\r", current_pos / 60, current_pos % 60, percent_done * 100.0);
				fflush(stderr);
			}
			else
			{
				fprintf(stderr, "Encoding... %u:%02u [%c]\r", current_pos / 60, current_pos % 60, status[(status_idx = (status_idx+1) % 4)]);
				fflush(stderr);
			}
		}
		
		counter = (counter+1) % 125;
	}
	
	fprintf(stderr, "Encoding... %u:%02u [%3.1f%%]\n", (samples_read_total / (multi_mono ? mono_f[an_input]->sample_rate :f->sample_rate)) / 60, (samples_read_total / (multi_mono ? mono_f[an_input]->sample_rate :f->sample_rate)) % 60, 100.0);
	fflush(stderr);

	wrote = dcaenc_destroy(c, output);
	fwrite(output, 1, wrote, outfile);
	if(outfile != stdout)
	{
		fclose(outfile);
	}
	if (!multi_mono)
		wavfile_close(f);
	else {
		if ( channel_config != DCAENC_CHANNELS_MONO ) {
			wavfile_close(mono_f[0]);
			wavfile_close(mono_f[1]);
		}
		if ( channel_config == DCAENC_CHANNELS_MONO || channel_config == DCAENC_CHANNELS_3FRONT_2REAR || channel_config == DCAENC_CHANNELS_3FRONT)
			wavfile_close(mono_f[2]);
		if(has_lfe)
			wavfile_close(mono_f[3]);
		if ( channel_config == DCAENC_CHANNELS_2FRONT_2REAR || channel_config == DCAENC_CHANNELS_3FRONT_2REAR )  {
			wavfile_close(mono_f[4]);
			wavfile_close(mono_f[5]);
		}
	}

	fprintf(stderr, "Done.\n");
	return 0;
}

// courtesy of <Kurtnoise>

#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#endif

int32_t main( int32_t argc, char **argv )
{
   #if defined(_WIN32) || defined(_WIN64)
      _setmode(_fileno(stdin),  _O_BINARY);
      _setmode(_fileno(stdout), _O_BINARY);
   #endif		

return dcaenc_main(argc, argv);
}

// </Kurtnoise>
