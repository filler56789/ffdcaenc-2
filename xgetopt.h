// XGetopt.h  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich2@hotmail.com
//
// This software is released into the public domain.
// You are free to use it in any way you like.
//
// This software is provided "as is" with no expressed
// or implied warranty.  I accept no liability for any
// damage or loss of business that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XGETOPT_H
#define XGETOPT_H

typedef struct
{
	int optind;
	int opterr;
	char *optarg;
	char *next;
}
xgetopt_t;

int xgetopt(int argc, char *argv[], char *optstring, xgetopt_t *ctx);

#endif //XGETOPT_H
