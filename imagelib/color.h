// DeadFish Image Library
// Color class
// Based on Gdiplus::Color
#ifndef _COLOR_H_
#define _COLOR_H_
#define MAKEDWORD64(ldw, hdw) ((DWORD64(hdw) << 32) | ((ldw) & 0xFFFFFFFF))

// pixel format conversion
inline DWORD64 RGB16B_BGRA16(void* data) {
	DWORD hi = bswap16((*(u16*)data)) | 0xFFFF0000;
	return MAKEDWORD64(bswap32(*(u32*)(data+2)), hi); }
inline DWORD64 RGBA16B_BGRA16(void* data) {
	DWORD hi = bswap32((*(u16*)data+6) | (*(u16*)data << 16));
	return MAKEDWORD64(bswap32(*(u32*)(data+2)), hi); }
inline void BGRA16_BGRA(void* src, void* dst) { clobber(eax); clobber(ecx); 
	byte a = RB(src,1); byte c = RB(src,3); RB(dst,0) = a; RB(dst,1) = c;
	a = RB(src,5); c = RB(src,7); RB(dst,2) = a; RB(dst,3) = c;	}

#define ColorX_TMPL template <class mem_t_, class val_t_>
#define ColorX_TMPO ColorX_<mem_t_,val_t_>
#define ColorX_TMPX(...) ColorX_TMPL __VA_ARGS__ ColorX_TMPO

ColorX_TMPL struct ColorX_
{
	// raw access
	enum { bpc = sizeof(mem_t_)*8 };
	typedef mem_t_ mem_t; typedef val_t_ val_t; 
	union{ struct { mem_t b, g, r, a; };
	  val_t Value; mem_t data[4]; };
	mem_t& getRef(int i) { return data[i]; }
	mem_t& operator[](int i) { return data[i]; }
	mem_t* begin() { return data; }
	mem_t* end() { return data+4; }
	
	// constructors
	ColorX_() = default;
	constexpr ColorX_(val_t Value_) : Value(Value_) {}
	constexpr ColorX_(mem_t R, mem_t G, mem_t B) : ColorX_(-1, R, G, B) {}
	constexpr ColorX_(mem_t A, mem_t R, mem_t G, mem_t B) : a(A), r(R), g(G), b(B) {}
	void SetValue(val_t argb) { Value = argb; }
	void SetValue(mem_t a, mem_t r, mem_t g, mem_t b) {
		this->a = a; this->r = r; this->g = g; this->b = b; }
	static val_t MakeARGB(mem_t a, mem_t r, mem_t g, mem_t b) {
		return ColorX_(a,r,g,b).Value; }
	
	// member get/set
	mem_t GetA() const 	{ return a; } 	mem_t GetB() const 	{ return b; } 
	mem_t GetG() const 	{ return g; } 	mem_t GetR() const 	{ return r; } 
	void SetA(mem_t a) 	{ this->a = a; } void SetB(mem_t b)  { this->b = b; } 
	void SetG(mem_t g)  { this->g = g; } void SetR(mem_t r)  { this->r = r; } 
	void OrA(WORD a) { Value |= val_t(a) << bpc*3; }

	// operator overloads
	operator val_t() { return Value; }
	bool operator==(const ColorX_& that) const  { return Value == that.Value; }
	bool operator!=(const ColorX_& that) const  { return Value != that.Value; }
	bool operator<(const ColorX_& that) const  { return Value < that.Value; }

	// Pixel operation functions
	void toolBlend(mem_t alpha, ColorX_ color);
	void bkgdBlend(ColorX_ bkgdColor);
	static mem_t alphaBlend(mem_t alpha, mem_t a, mem_t b) { const mem_t
		max = -1; return (DWORD(a*alpha + b*(max-alpha)) + max/2) / max; }
	
	// Algorithms
	struct SideMinMax; struct SideLength;
	SideMinMax* sideMinMax(SideMinMax* result, int length, int pitch);
	SideLength* sideLength(SideLength* result, int length, int pitch);
	DEF_RETPAIR(side_t, int, index, int, length);
	side_t longestSide(int length, int pitch);
	
	int makeUnique(int length);
	bool isUnique(int count, ColorX_ color);
	int diff3(const ColorX_& that);
	int alphaType(int length);
	int alphaType(int w, int h, int pitch);
};

ColorX_TMPX(struct)::SideMinMax { ColorX_ minCorner; ColorX_ maxCorner; };
ColorX_TMPX(struct)::SideLength : SideMinMax { ColorX_ sideLen; };

struct Color16 : ColorX_<WORD, DWORD64>
{ 	
	using ColorX_::ColorX_; 

	static DWORD64 swapOrder(DWORD64 val) { DWORD x = val; DWORD y = val>>32;
		asm("xchg %w0, %w1" : "+r"(x), "+r"(y)); return x | DWORD64(y)<<32; }
		
	void setRGB(DWORD64 rgb) { setRGBA(rgb | 0xFF000000); }
	void setRGBA(DWORD64 rgb) { Value = swapOrder(rgb); }
	
	
	// big endian setters
	void setGRY16B(u16 v) { v = bswap16(v); SetValue(-1, v, v, v); }
	void setGRYA16B(u16 a, u16 v) { v = bswap16(v); SetValue(bswap16(a), v, v, v); }
	void setRGB16B(void* p) { Value = RGB16B_BGRA16(p); }
	void setRGBA16B(void* p) { Value = RGBA16B_BGRA16(p); }
};

struct Color : ColorX_<BYTE, DWORD>
{ 
	using ColorX_::ColorX_; 

	operator RGBQUAD() const { return *(RGBQUAD*)this; }
	COLORREF colorRef() { return bswap32(Value)>>8; }
	void setRGB(void* p) { Value = swapOrder(*(u32*)p | 0xFF000000); }
	void setRGBA(void* p) { Value = swapOrder(*(u32*)p); }
	Color() = default;
	constexpr Color(BYTE R, BYTE G, BYTE B) : Color(-1, R, G, B) {}
	constexpr Color (BYTE A, BYTE R, BYTE G, BYTE B) : 
		ColorX_((A<<24) | (R<<16) | (G << 8) | B) {}
		
	static DWORD MakeARGB(BYTE a, BYTE r, BYTE g, BYTE b) {
		return Color(a,r,g,b).Value; }
	static DWORD swapOrder(DWORD val) {	val = bswap32(val);
		asm("ror $8, %0" : "+r"(val)); return val; }		
	static DWORD conv16to8Lo(DWORD64 val) {	DWORD x = val; DWORD y = val>>32;
		asm("shl $8, %w0; shl $8, %w1; shr $8, %0; shl $8, %1;"
		"orl %1, %0" : "+r"(x) :  "r"(y)); return x; }
	void sort(int index, int length, int pitch);
	void sortr(int index, int length, int pitch);
};

#endif
