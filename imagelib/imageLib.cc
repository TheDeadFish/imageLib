
#include "image.cpp"
#include "windows.cpp"
#include "bitmap.cpp"
#include "imghlp.cc"
#include "gif.cpp"
#include "png.cpp"

namespace ImageLib {
int FileOut::vbuf(int len)
{
	if((!memMode())&&(setvbuf((FILE*)
	this, NULL, _IOFBF, len)))
		return Image::ERR_SYSTEM; return Image::ERR_NONE;
}

int FileOut::write(void* data, int len)
{
	if(!memMode()) { if(fwrite(data, len, 1, 
		(FILE*)this) != 1) return Image::ERR_SYSTEM;
	} else { memcpy(curPos, data, len);
		curPos += len; } return Image::ERR_NONE;
}

int FileOut::write(int len)
{
	curPos += len; if(!memMode()) { 
		if(_flsbuf(0, (FILE*)this) < 0) 
		return Image::ERR_SYSTEM; curPos--;
	} return Image::ERR_NONE;
}

int FileOut::reserve(int len) {
	if(memMode()) {	data = malloc(len);
		if(!data) return Image::ERR_ALLOC;
		curPos = data; maxSize = len;
	} return Image::ERR_NONE;
}

BOOL REGCALL(2) clipRect(RECT& dst, const RECT& src, int x, int y) {
	RECT rc = {0,0,x,y}; return IntersectRect(&dst, &src, &rc); }
BOOL REGCALL(1) clipRect(RECT& dst, int x, int y) {
	return clipRect(dst, dst, x, y); }

}
