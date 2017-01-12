#define ERR_CHK(e) { if(int \
	err = e) return err; }

#include "color.cpp"
#include "image.cpp"
#include "windows.cpp"
#include "bitmap.cpp"
#include "imghlp.cc"
#include "gif.cpp"
#include "png.cpp"
#include "drawing.cpp"
#include "resample/resample.cpp"

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
}
