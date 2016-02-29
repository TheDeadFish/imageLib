// DeadFish image library
// Png File functions
// with parts taken from loadPng

#include <malloc.h>
#include <zlib.h>
#include "imageLib.h"

namespace ImageLib{



// Taken from lodePng
static const unsigned ADAM7_IX[7] = { 0, 4, 0, 2, 0, 1, 0 };
static const unsigned ADAM7_IY[7] = { 0, 0, 4, 0, 2, 0, 1 };
static const unsigned ADAM7_DX[7] = { 8, 8, 4, 4, 2, 2, 1 };
static const unsigned ADAM7_DY[7] = { 8, 8, 8, 4, 4, 2, 2 };

SHITSTATIC byte paethPredictor(int a, int b, int c) {
  int pa = abs(b - c); int pb = abs(a - c); int pc = abs(a + b - c - c);
  if(pc < pa && pc < pb) return c; else if(pb < pa) return b; else return a; 
}
SHITSTATIC int unfilterScanline(byte* scanline,
	const byte* precon, uint bytewidth, uint length)
{
  size_t i; switch(RDI(scanline)) { case 0: for(i = 0; i != length; ++i) scanline[i] = scanline[i]; 
  break; case 1: for(i = 0; i != bytewidth; ++i) scanline[i] = scanline[i]; for(i = bytewidth; i < 
  length; ++i) scanline[i] = scanline[i] + scanline[i - bytewidth]; break; case 2: if(precon) { for(i 
  = 0; i != length; ++i) scanline[i] = scanline[i] + precon[i]; } else { for(i = 0; i != length; ++i)
  scanline[i] = scanline[i]; } break; case 3: if(precon) { for(i = 0; i != bytewidth; ++i) scanline[i] = 
  scanline[i] + precon[i] / 2; for(i = bytewidth; i < length; ++i) scanline[i] = scanline[i] + 
  ((scanline[i - bytewidth] + precon[i]) / 2); } else { for(i = 0; i != bytewidth; ++i) scanline[i] =
  scanline[i]; for(i = bytewidth; i < length; ++i) scanline[i] = scanline[i] + scanline[i - bytewidth] 
  / 2; } break; case 4: if(precon) { for(i = 0; i != bytewidth; ++i) { scanline[i] = (scanline[i] +
  precon[i]); } for(i = bytewidth; i < length; ++i) { scanline[i] = (scanline[i] + paethPredictor(
  scanline[i - bytewidth], precon[i], precon[i - bytewidth])); }} else { for(i = 0; i != bytewidth;
  ++i) { scanline[i] = scanline[i]; } for(i = bytewidth; i < length; ++i) { scanline[i] = (scanline[i] 
  + scanline[i - bytewidth]); }} break; default: return ImageObj::BAD_IMAGE; } return 0;
}


enum { TYPE_GREYSCALE = 0,
	TYPE_TRUECOLOR = 2,
	TYPE_INDEXED = 3,
	TYPE_GREYALPHA = 4,
	TYPE_TRUEALPHA = 6 };

enum { TYPE_IHDR = 0x52444849,
	TYPE_PLTE = 0x45544C50,
	TYPE_tRNS = 0x534E5274,
	TYPE_IDAT = 0x54414449,
	TYPE_IEND = 0x444E4549 };
	
struct CHNK {
	uint len; uint type; byte data[];
	uint Len() { return bswap32(len); }
	int check(byte* endPos) { if((data+4) > endPos) return -1;
		uint len = Len(); return ((data+4+len) > endPos) ? -1 : len; }
	CHNK* next(void) { return Void(this, Len()+12); }
};


struct PLTE : CHNK { RGB8 pal[];
	int check2() { uint len = Len();
		if((len > 768)||(len % 3))
			return -1; return len / 3; }};		
struct tRNS : CHNK { union { u16 tg;
	RGB16B trgb; u8 ridx[]; }; };

struct IHDR : CHNK {
	uint width, height;
	byte bpc, ctype;
	word cpfl; byte intl;
	
	bool check2(byte* endPos) { return ((CHNK::check(endPos) == 13)
		&& (type == TYPE_IHDR) && (cpfl == 0) && (intl <= 1)); }
	uint Width() { return bswap32(width); }
	uint Height() { return bswap32(height); }
};

struct ImageObj::LoadPng
{
	uint w, h, bpp;
	uint imgSize_out;
	uint pixSize_raw;
	uint lineSize_raw;
	uint imgSize_raw;
	
	
	byte u8colType;
	
	
	
	

	byte* buffer;
	byte* decodeBuff;
	z_stream strm;
	uint predict;
	
	
	struct Adam7 {
		uint offset; SIZE size; };
	Adam7 adam7[7];
	
	uint adam7_init(void);
	int adam7_deint(ImageObj& img);
	int getData(CHNK* chnk);
	int unFilter(byte* inout, uint w, uint h);
	int load(ImageObj& img, byte* data, int size);
};

uint ImageObj::LoadPng::adam7_init(void)
{
	uint curPos = 0;
	for(int i = 0; i != 7; ++i)	{
		SIZE size = { (w + ADAM7_DX[i] - ADAM7_IX[i] - 1) / ADAM7_DX[i],
			(h + ADAM7_DY[i] - ADAM7_IY[i] - 1) / ADAM7_DY[i] };
		if(!size.cx) size.cy = 0; ei(!size.cy) size.cx = 0;
		adam7[i].offset = curPos; adam7[i].size = size;
		curPos += size.cy * (1 + (size.cx * bpp + 7) / 8);
	} return curPos;
}

int ImageObj::LoadPng::adam7_deint(ImageObj& img)
{
	for(int i = 0; i != 7; ++i) { IFRET(unFilter(buffer + 
		adam7[i].offset, adam7[i].size.cx, adam7[i].size.cy)); }
	if(!(img.bColors = malloc(imgSize_out)))
		return ERR_ALLOC;

	auto fn = lineDeintFunc(u8colType, bpp); byte* src = buffer;
	int pitch = img.pitch; int pixSize = img.pixSize();
	for(int i = 0; i != 7; ++i)	{ byte* dst = img.bColors +
		(pitch * ADAM7_IY[i]) + (ADAM7_IX[i] * pixSize);
		int deltaX = ADAM7_DX[i]; int deltaY = ADAM7_DY[i] * pitch;
		for(int count = adam7[i].size.cy; --count >= 0;) { src = 
			fn(src+1, dst, deltaX, adam7[i].size.cx); dst += deltaY; }
	} return 0;
}
	

int ImageObj::LoadPng::getData(CHNK* chnk)
{
	if(strm.next_out == 0)
		return ImageObj::BAD_IMAGE;
	strm.avail_in = chnk->Len();
	if(!release(strm.next_in, chnk->data)) {
		if(int ret = inflateInit(&strm)) return (ret == Z_MEM_ERROR)
			? ImageObj::ERR_ALLOC : ImageObj::BAD_IMAGE; 
	} int ret = inflate(&strm, Z_NO_FLUSH);
	if(ret == Z_MEM_ERROR) return ImageObj::ERR_ALLOC;
	if(is_one_of(ret, Z_OK, Z_BUF_ERROR)) {
		return strm.avail_out ? 0 : ImageObj::BAD_IMAGE; }	
	if((ret != Z_STREAM_END) || strm.avail_in)
		return ImageObj::BAD_IMAGE; 
	strm.next_out = 0; return 0;	
}




int ImageObj::LoadPng::unFilter(
	byte* inout, uint w, uint h)
{
	uint pixbytes = (bpp + 7) / 8;
	uint linebytes = (w * bpp + 7) / 8;
	byte* prevline = 0;
	
	while(!isNeg(--h)) {
		IFRET(unfilterScanline(inout, prevline, pixbytes, linebytes));
		prevline = inout+1; inout += linebytes + 1; 
	} return 0;
}

ALWAYS_INLINE int ImageObj::LoadPng::
	load(ImageObj& img, byte* data, int size)
{
	// check image header
	IHDR* head = Void(data, 8);
	byte* endPos = data + size;
	if((head->check(endPos) < 0) ||
	!(w = head->Width()) || !(h = head->Height()))
		return BAD_IMAGE;
		
	// check color format
	switch(head->ctype) {
	case TYPE_GREYSCALE: u8colType = 0;
		if(head->bpc == 16) { u8colType |= HAS_16BIT; } if(0) {
	case TYPE_INDEXED: u8colType = HAS_PALETTE;	} if(!is_one_of(
		head->bpc, 1, 2, 4, 8)) return BAD_IMAGE; break;
	case TYPE_TRUECOLOR: u8colType = HAS_COLOR;	if(0) { 
	case TYPE_GREYALPHA: u8colType = HAS_ALPHA; } if(0) {
	case TYPE_TRUEALPHA: u8colType = HAS_RGBA; }
		if(head->bpc == 16) u8colType |= HAS_16BIT;
		ei(head->bpc != 8) { default: return BAD_IMAGE; }
	}
	
	// initialize state
	bpp = calcBpp(u8colType, head->bpc);
	imgSize_out = img.initSize(w, h, u8colType, bpp);
	if(imgSize_out < 0) return USP_IMAGE;
	lineSize_raw = (w * bpp + 7) / 8;		// line size, excluding filter byte
	imgSize_raw = h * lineSize_raw;			// image size, excluding filter byte
	
	// allocate buffer
	{ uint allocSize; if(head->intl == 0) {
		allocSize = predict = imgSize_raw + h;
		max_ref(allocSize, imgSize_out);
	} else { allocSize = predict = adam7_init(); }
	if(!(buffer = malloc(allocSize))) return ERR_ALLOC; 
	strm.next_out = decodeBuff = buffer +
		(allocSize - (strm.avail_out = predict)); }
	
	// parse the file
	PLTE* plte = NULL; tRNS* trns = NULL;
	for(CHNK* curPos = head;;) { curPos=curPos->next();
		if(curPos->check(endPos)<0) return BAD_IMAGE;
		switch(curPos->type) {
		case TYPE_PLTE:	plte = (PLTE*)curPos; break;
		case TYPE_tRNS: trns = (tRNS*)curPos; break;
		case TYPE_IDAT: IFRET(getData(curPos)); break;
		case TYPE_IEND: if(predict != strm.total_out)
			return BAD_IMAGE; goto END_OF_FILE; 
		}
	}
	
	// read palette
END_OF_FILE:;
	if(u8colType & HAS_PALETTE) {
		int nCols = plte->check2();
		img.palSize = nCols;
		if(nCols < 0) return BAD_IMAGE;
		if(!(img.palette = malloc(1024)))
			return ERR_ALLOC;
		convLine24To32_(plte->data,
			(byte*)img.palette, 1, nCols);	
	}
	
	// unFilter image
	if(head->intl == 0) {
		IFRET(unFilter(decodeBuff, w, h));
		auto fn = lineDeintFunc(u8colType, bpp);
		byte* dst = img.bColors = release(buffer);
		byte* src = decodeBuff+1; int count = h;
		int tmp = w;
		
		while(--count >= 0) {
			GET_RETPAIR(src, dst, fn(src, dst, 1, tmp));
			src += 1; dst = (byte*)ALIGN4(size_t(dst)); }
	} else {
		IFRET(adam7_deint(img));
	}
	
	// apply transparency
	if(trns != NULL) {
		uint len = trns->Len();
		if(u8colType & HAS_ALPHA) return BAD_IMAGE;
		if(u8colType & HAS_PALETTE) {
			if(len > img.palSize) return BAD_IMAGE;
			convLine8To8_(trns->data, &img.palette->a, 4, len);
		} else {
			Color16 trns16;
			if(u8colType & HAS_COLOR) {
				if(len != 6) return BAD_IMAGE;
				trns16.setRGB16B(trns->data);
				trns->trgb.get((byte*)&trns16);
			} else {
				if(len != 2) return BAD_IMAGE;
				u16 val = bswap16(*(u16*)trns->data);
				switch(bpp) { case 1: val *= 255;
				case 2: val *= 85; case 4: val *= 17; };
				trns16.SetValue(-1, val, val, val);
			}
			uint count = w*h;
			if(u8colType & HAS_16BIT) {
				Color16* curPos = img.wColors;
				while(--count) { if(curPos->Value == trns16)
					curPos->SetA(-1); curPos++; }
			} else {
				Color trns8 = Color::conv16to8Lo(trns16);
				Color* curPos = img.cColors; 
				while(--count) { if(curPos->Value == trns8)
					curPos->SetA(0); curPos++; }
			}
		}
	}
	
	return 0;
}

int ImageObj::loadPng(void* data, int size)
{
	LoadPng state = {0}; SCOPE_EXIT(free(state.buffer));
	int ret = state.load(*this, (byte*)data, size);
	if(ret != NULL) { this->Delete(); } return ret;
}
}
