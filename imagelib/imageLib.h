// DeadFish Image Library
// Core components
#ifndef _IMAGELIB_H_
#define _IMAGELIB_H_
#include "stdshit.h"

#define ImageLib_assert assert

NEVER_INLINE
static __stdcall byte* fBuf(FILE* fp, int size) { fp->_cnt -= size;
	return (byte*)release(fp->_ptr, fp->_ptr+size); }
NEVER_INLINE
static __stdcall int wBuf(FILE* fp) { int result = _flsbuf(0, fp); 
	fp->_cnt++; fp->_ptr--; return result; }
static int snapToBits(int val)
	{	return ((__builtin_clz(val-1)^31)+1); }
#define byte_reg(x) asm("" : "+q"(x))
#define byte_move(d,s) asm volatile("hello" : "=r"(d) : "r"(s))






#include "rect.h"
#include "color.h"
#include "image.h"
#include "imghlp.h"
#include "resample/resample.h"
#endif
