// DeadFish Image Library
// Core components
#ifndef _IMAGELIB_H_
#define _IMAGELIB_H_
#include "stdshit.h"

#define ImageLib_assert assert

namespace ImageLib {

// file/memory data writer
struct FileOut {
	byte* curPos; int mode; int maxSize; byte* data;
	FileOut() : mode(INT_MIN),	maxSize(0), data(0) {}
	xarray<byte> getData() { return {data, curPos-data}; }
	bool memMode(void) { return mode == INT_MIN; }
	byte* buff(void) { return curPos; }	
	int vbuf(int len); int reserve(int len);
	int write(int len); int write(void* data, int len);
};

#define byte_reg(x) asm("" : "+q"(x))
#define byte_move(d,s) asm volatile("hello" : "=r"(d) : "r"(s))

class Image;
class ImageObj;

BOOL REGCALL(2) clipRect(RECT& dst, const RECT& src, int x, int y);
BOOL REGCALL(1) clipRect(RECT& dst, int x, int y);


// rect helpers
static LONG RECT_W(const RECT& rc) { return rc.right-rc.left; }
static LONG RECT_H(const RECT& rc) { return rc.bottom-rc.top; }

TMPL(T) const T* pCnst(T* val) { return val; }

}

#include "rect.h"
#include "color.h"
#include "imghlp.h"
#include "image.h"
#endif
