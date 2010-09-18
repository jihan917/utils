#ifndef GETOPT_H_INCLUDED
#define GETOPT_H_INCLUDED

/**
 * getopt_r - command option parsing
 * free to distribute under the GPL license.
 * (C) Copyright 2009, 2010, Ji Han (jihan917<at>yahoo<dot>com).
 * see http://www.opengroup.org/onlinepubs/000095399/functions/getopt.html
 * for the specification of getopt, the non-reentrant counterpart.
 */

#if defined(__cplusplus)
extern "C" {
#endif

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct opt_st;
typedef struct opt_st opt_st;

const char *const optarg(const opt_st *const opt);
const int optind(const opt_st *const opt);
const int optopt(const opt_st *const opt);

void opt_init(opt_st **popt, void* (*alloc)(size_t));

int getopt_r(int argc, char *const *argv, const char *optstring, opt_st *opt);

#if defined(__cplusplus)
}   /* extern "C" */
#endif

#endif  /* GETOPT_H_INCLUDED */

