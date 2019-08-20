#include "imageLib.h"

namespace ImageLib {


Rect NotRect::Get(int step, const Rect& rc, int width, int height)
{	Rect This; return GetX<1>(This,	step, rc, width, height); }


SHITCALL int snapToBits(int val) { return (val > 16) ? 8 :
	(val > 4) ? 4: (val > 2) ? 2 : 1; }

// pack indexed lines to 1/2/4 bits
#define CL8FB(s) byte* srcEnd = src+width; \
	while(src < srcEnd) { byte tmp = *src << s; src++;
#define CL8FN(s) if(src >= srcEnd) goto end; tmp |= (byte)(*src << s); src++; 
#define CL8FE CL8FN(0) end: *dst = tmp; dst++; };
void REGCALL(2) convLine8To1(byte* src, byte* dst, int width) {
	CL8FB(7) CL8FN(6) CL8FN(5) CL8FN(4)
	CL8FN(3) CL8FN(2) CL8FN(1) CL8FE; } 
void REGCALL(2) convLine8To2(byte* src, byte* dst, int width) {
	CL8FB(6) CL8FN(4) CL8FN(2) CL8FE; }
void REGCALL(2) convLine8To4(byte* src, byte* dst, int width) {
	CL8FB(4) CL8FE; }
	
// unpack indexed lines to 8bit
#define CL8DW(s, m) { VARFIX(width); byte tmp2 = tmp; if(s > 0) asm("shr %1,%0" : \
	"+b"(tmp2) : "i"(s)); if(m > 0) asm("and %1,%0" :"+b"(tmp2) : "i"(m)); \
	asm("movb %2,(%0); add %1,%0":"+d"(dst):"r"(delta),"b"(tmp2)); }
#define CL8TB(s) while(--width >= 0) { byte tmp = RDI(src); CL8DW(s,-1)
#define CL8TN(s,m) if(--width < 0) break; CL8DW(s, m);
#define CL8TE(m) CL8TN(0,m) }


#define convLineWW_(nm, ...) convLine_t REGCALL(3) MCAT(nm,_)(byte* src, \
	byte* dst, int delta, int width) { __VA_ARGS__; return {src, dst}; } \
	void REGCALL(2) nm(byte* src, byte* dst, int width) { \
		MCAT(nm,_)(src, dst, 1, width); }
convLineWW_(convLine1To8, CL8TB(7) CL8TN(6,1) CL8TN(5,1) CL8TN(4,1)
	CL8TN(3,1) CL8TN(2,1) CL8TN(1,1) CL8TE(1));
convLineWW_(convLine2To8, CL8TB(6) CL8TN(4,3) CL8TN(2,3) CL8TE(3));
convLineWW_(convLine4To8, CL8TB(4) CL8TE(15));

// pack/unpack RGB <-> ARGB
void REGCALL(2) convLine32To24(byte* src, byte* dst, int width) {
	for(auto& pix : Range((DWORD*)src, width)) {
		*(RGB8*)dst = *(RGB8*)&pix; PTRADD(dst, 3); } }
void REGCALL(2) convLine64To48(byte* src, byte* dst, int width) {
	for(auto& pix : Range((INT64*)src, width)) {
		*(INT64*)dst = pix; PTRADD(dst, 6); } }
void REGCALL(2) convLine24To32(byte* src, byte* dst, int width) {
	for(auto& pix : Range((DWORD*)dst, width)) {
		pix = 0xFF000000 | *(DWORD*)src; PTRADD(src, 3); } }
void REGCALL(2) convLine48To64(byte* src, byte* dst, int width) {
	for(auto& pix : Range((INT64*)dst, width)) {
		((DWORD*)&pix)[0] = *(DWORD*)src; PTRADD(src, 3);
		((DWORD*)&pix)[1] = 0xFF000000 | *(DWORD*)src; PTRADD(src, 3); } }
		
// simple copy
void REGCALL(2) convLine8To8(byte* src, byte* dst, int width) {
	memcpy(dst, src, width); }	
void REGCALL(2) convLine32To32(byte* src, byte* dst, int width) {
	memcpy(dst, src, width*4); }
void REGCALL(2) convLine64To64(byte* src, byte* dst, int width) {
	memcpy(dst, src, width*8); }
	
linePackFunc_t __stdcall linePackFunc(int nBits) {
	switch(nBits) { case 1: return convLine8To1; case 2: return convLine8To2;
		case 4: return convLine8To4; case 8: return convLine8To8;
		case 24: return convLine32To24; case 32: return convLine32To32;
		case 48: return convLine64To48; case 64: return convLine64To64;
		default: assert(false); 	}
}
linePackFunc_t __stdcall lineUnpackFunc(int nBits) {
	switch(nBits) { case 1: return convLine1To8; case 2: return convLine2To8;
		case 4: return convLine4To8; case 8: return convLine8To8;
		case 24: return convLine24To32; case 32: return convLine32To32;
		case 48: return convLine48To64; case 64: return convLine64To64;
		default: assert(false); 	}
}

// deinterlace line-unpacking
#define convLineXX(n, T) convLine_t REGCALL(3) MCAT(convLine,n)(byte* src, \
  byte* dst, int delta, int width) { delta *= T::osz; while(--width >= 0) { \
  ((T*)src)->get(dst); src += sizeof(T); dst += delta; }  return {src,dst}; }
convLineXX(8To8_, IDX8); convLineXX(8To32_, GRY8); convLineXX(16To64_, GRY16B);
convLineXX(16To32_, GRYA8); convLineXX(32To64_, GRYA16B); convLineXX(24To32_, RGB8);
convLineXX(48To64_, RGB16B); convLineXX(32To32_, RGBA8); convLineXX(64To64_, RGBA16B);

#define convLineYY(nm, fn, sc) convLine_t REGCALL(3) MCAT(convLine,nm)( \
	byte* src, byte* dst, int delta, int width) { delta *= 4; src = MCAT( \
	convLine,fn)(src, dst, delta, width).src; while(--width >= 0) { GRY8 \
	v = {*dst * sc}; v.get(dst); dst += delta;  } return {src, dst}; }

convLineYY(1To32_, 1To8_, 255);
convLineYY(2To32_, 2To8_, 85); 
convLineYY(4To32_, 4To8_, 17);

linePackFunc2_t __stdcall lineDeintFunc(u8 u8colType, u8 bpp) 
{
	if(u8colType & Image::HAS_PALETTE) { switch(bpp) {
		case 1: return convLine1To8_; case 2: return convLine2To8_;
		case 4: return convLine4To8_; case 8: return convLine8To8_; }
	} ei(!(u8colType & Image::HAS_16BIT)) { switch(bpp) {
		case 1: return convLine1To32_; case 2: return convLine2To32_;
		case 4: return convLine4To32_; case 8: return convLine8To32_;
		case 16: return convLine16To32_; case 24: return convLine24To32_;
		case 32: return convLine32To32_; }
	} else  { switch(bpp) {
		case 16: return convLine16To64_; case 32: return convLine32To64_;
		case 48: return convLine48To64_; case 64: return convLine64To64_; }		
	}
	
	__builtin_unreachable();
}

byte* REGCALL(2) convLineTo32(
	Image& img, byte* src, byte* dst)
{
	int mode = img.colorMode();
	if(mode == Image::MODE_ARGB8) return src;
	int width = img.width; VARFIX(width); Color* dstPos = (Color*)dst;
	if(mode == Image::MODE_INDEX) {	Color* pal = img.palette;
	while(--width >= 0) { WRI( dstPos, pal[*src]); src += 1; } return dst; }
	else { 	while(--width >= 0) { BGRA16_BGRA(src, dstPos);
		VARFIX(src); VARFIX(dstPos); src += 8; dstPos += 1; } return dst; }
}

byte* REGCALL(3) copyLine32(byte* src,
	byte* dst, size_t arg, int width)
{
	if(src != dst) { byte* dstPos = dst; while(
		--width >= 0) { WRI(PI(dstPos), RDI(PI(src))); } }
	return dst;
}

void SHITCALL convImgTo32(Image& img, byte* dst, int pitch,
	LineConvFunc2 fn, size_t arg)
{
	byte* srcPos = img.bColors;
	int height = img.height; VARFIX(height);
	while(--height >= 0) {
	
		byte* tmp = convLineTo32(img, srcPos, dst);
		dst = fn(tmp, dst, arg, img.width);
		srcPos += img.pitch; dst += pitch;
	}
}

void SHITCALL convImgTo32(Image& src, Image& dst,
	LineConvFunc2 fn, size_t arg)
{
	convImgTo32(src, dst.bColors, dst.pitch, fn, arg);
}

uint maxIndex(Image& img) {
	uint result = 0; 
	byte* linePos = img.bColors;
	for(uint y : Range(0U, (uint)img.height)) {
	  for(uint x : Range(0U, (uint)img.width))
		max_ref(result, linePos[x]);
	  linePos += img.pitch; }
	return result;
}
}
