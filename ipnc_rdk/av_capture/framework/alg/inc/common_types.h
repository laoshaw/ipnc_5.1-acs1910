#ifndef __COMMON_TYPES_H_
#define __COMMON_TYPES_H_

#define IPNC 

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#undef IPNC
#endif

#ifdef IPNC
 #include <osa_cmem.h>
 #include <osa_file.h>
 #include <sys/time.h>
#endif

typedef char   					S08;
typedef unsigned char   		U08;
typedef signed short    		S16;
typedef unsigned short  		U16;
typedef signed int      		S32;
typedef unsigned int    		U32;
typedef float           		F32; 
typedef long long				S64;
typedef unsigned long long		U64;
typedef void *					PTR;

#endif
