// Image handling library
// Manages format capabilities
// DeadFish Shitware 2012
#ifndef _IMAGEOBJ_H_
#define _IMAGEOBJ_H_

#define CONST_FUNC(f) f; const f const;
#define CONST_FUNC2(t, f) t f; t const f const;
#define CONST_FUNC3(t, f, b) t f b; t const f const b;

namespace ImageLib{
	
	class Image
	{
	public:
		// basic members
		union{
			Color* cColors;		// 32bit Color array
			Color16* wColors;	// 64bit Color array
			byte* bColors;		// indexed color array
		};
		Color* palette;			// Palette colors
		u32 pitch;				// Pitch of image
		u32 width;				// Width of image
		u32 height;				// Height of image
		u16 palSize;			// Size of pallete
		u8 colType;				// color type
		u8 nBits;				// original bit depth
		
		// color type
		enum { HAS_ALPHA = 1, HAS_COLOR = 2, HAS_16BIT = 4,
			HAS_PALETTE = 8, HAS_RGBA = 3 };
		enum { MODE_ARGB8 = 0, MODE_ARGB16 = 4, MODE_INDEX = 8 };
		int colorMode() { return colType & (HAS_16BIT | HAS_PALETTE); }
		int alphaType() const;
		
		// high level types
		enum { TYPE_INDEX = HAS_PALETTE, TYPE_GRY8 = 0, TYPE_AGRY8 = HAS_ALPHA,
			TYPE_GRY16 = HAS_16BIT, TYPE_AGRY16 = HAS_16BIT | HAS_ALPHA,
			TYPE_RGB8 = HAS_COLOR, TYPE_RGB16 = HAS_16BIT | HAS_COLOR,
			TYPE_ARGB8 = HAS_COLOR | HAS_ALPHA,
			TYPE_ARGB16 = HAS_16BIT | HAS_COLOR | HAS_ALPHA 
		};
			
		int calcNBits(int colType_) const; 
		int calcNBits() const { return calcNBits(colType); }
		bool hasPalette(void) const { return colType & HAS_PALETTE; }
		bool hasAlpha() const { return !hasPalette() && (colType & HAS_ALPHA); }
		bool IsCompatible(const Image& that) const;
		SHITSTATIC uint calcBpp(uint colType, uint nBpc);
		
		// geometry / pixel access
		int nPixels() { return width * height; }
		int pixSize() const { int sz; asm("call "
			"Image_pixSize" : "=a"(sz) : "a"(this)); return sz;}
		int sizeExtra8(int) const;	int sizeExtra32(int) const;
		int sizeExtra8() const;	int sizeExtra32() const;
		BOOL clipRect(RECT& dst, const RECT& src) const; 
		
		Rect getRect() const;
		SIZE clipRect2(RECT& out, const Rect& rc) const;
		CONST_FUNC2( TMPL(F), void for_each8(const Rect& rc, F fn));
		CONST_FUNC2( TMPL(F), void for_each32(const Rect& rc, F fn));
		bool chkPos(uint x, uint y) const;
		
		// pixel access
		CONST_FUNC( byte* EndPtr() );
		CONST_FUNC( byte* getLn(int y) );
		CONST_FUNC( byte* getPtr(int x, int y) );
		CONST_FUNC( byte* get8(int x, int y) );
		CONST_FUNC( Color* get32(int x, int y) );
		
		
		
		/*CONST_FUNC( byte* GetPtr8(int x, int y) );
		CONST_FUNC( byte* GetPtr32(int x, int y) );
		
		
		
		
		CONST_FUNC( byte* GetPtr(int x, int y) );
		CONST_FUNC( Color& GetRef(int x, int y) );*/





		
		
		
		
		
		
		
		// basic drawing
		void fillImage(DWORD color);
		void fillRect(const RECT& dst, DWORD color);
		void fillNotRect(const RECT& dst, DWORD color);
		void drawRect(const RECT& dst, DWORD color);
		void drawRect(const RECT& dst, DWORD line, DWORD fill);
		void drawLineH(int y, int x1, int x2, DWORD color);
		void drawLineV(int x, int y1, int y2, DWORD color);

		// display / clipboard 
		enum{ ERR_NONE, ERR_PARAM,	ERR_ALLOC, ERR_SYSTEM,
			NOT_FOUND,	BAD_IMAGE, USP_IMAGE, CLIP_NODATA };
		int GetBitmapInfo(struct BmInfo256& bi, bool bltMode) const;
		int ToClip(HWND hwnd);
		void bitBlt(HDC hdc, int x, int y) const;
		void bitBlt(HDC hdc, int x, int y, int w, int h) const;
		void bitBlt(HDC hdc, int x, int y, const RECT& src) const;
		void strBlt(HDC hdc, const RECT& dst) const;
		void strBlt(HDC hdc, const RECT& dst, const RECT& src) const;	

		// construction
		Image() = default; Image(const Image& that) { initRef(that); }
		Image(const Image& that, const Rect& rc) { initRef(that, rc); }
		Image& initRef(const Image& that);
		Image& initRef(const Image& that, const Rect& rc);
		int initSize(uint w, uint h, uint colType, uint nBits);
		DEF_RETPAIR(alloc_t, byte*, data, int, pitch);
		SHITSTATIC alloc_t alloc(int width, int height, uint colType);
		alloc_t alloc(uint colType);
		
		
		SHITSTATIC uint calcPitch(uint width, uint colType) {
			return (colType & HAS_PALETTE) ? ALIGN4(width)
			: ((colType & HAS_16BIT) ? width*8 : width*4); }
			
		// saving interface
		enum { FORMAT_BITMAP, FORMAT_PNG };
		int Save(LPCSTR fName, int format, void* opts = 0);
		int Save(xarray<byte>& data, int format, void* opts = 0);
		int Save(FileOut* fo, int format, void* opts = 0);

	protected:
		int SaveBitmap(FileOut*, void*);
		enum{ CLIP_WAIT = 100,
			CLIP_TRYS = 12 };
	};

	
	class ImageObj : public Image
	{
	public:
		// Creation
		ImageObj();	~ImageObj(); void Delete(void);
		int Create(uint w, int h, int colType,
			int nBits, BOOL dibMode = 0);
		int Create(uint w, int h, int colType);
		int CreateDib(uint w, int h, bool alpha = 0);
		
		// Indexed Creation
		int Create(uint w, int h, Color* p, 
			int psz, BOOL dibMode = 0);
		int Create(const Image& that, Color* p, int psz);
		
		int Load(void* data, int size);
		int Load(LPCSTR file); int FromClip(HWND hwnd);
		int FromBmInfo(LPBITMAPV5HEADER bi, BYTE** imgData);
		
		// Soul transferance
		int Create(const Image& that, bool dibMode = 0);
		int Create(const Image& that, int w, int h, bool dibMode = 0);
		int Copy(const ImageObj& that, bool dibMode = 0);
		int Copy(const ImageObj& that, 
			const Rect& rc, bool dibMode = 0);
			
			
			
		void Swap(ImageObj& that);
		void setPalette(Color* palette, uint palSize);
		
		// HBITMAP version
		void BitBlt(HDC hdc, int x, int y) const;
		void BitBlt(HDC hdc, int x, int y, 
			const RECT& dst) const;
		void StrBlt(HDC hdc, const RECT& dst) const;
		void StrBlt(HDC hdc, const RECT& dst,
			const RECT& src) const;
		
		
		operator HDC() { return hdc; }

	private:
		HDC hdc;
		void resetObj();
		void CopyBmBuffer(byte* imgData, int biHeight);
		void Copy(const Image& that);
		int LoadBmp(void* data, int len);
		int LoadGif(void* data, int len);
		int loadPng(void* data, int len);
		
		struct LoadPng;
		
		
	};
	
	// IMPLEMENTATION: construction
	inline ImageObj::ImageObj() { this->resetObj(); }
	inline ImageObj::~ImageObj() { this->Delete(); }

	// IMPLEMENTATION: geometry access
	/*(inline int Image::sizeLine() const 			{ return palSize ? width : width*4; }
	inline int Image::sizeLine8() const 		{ return width; }
	inline int Image::sizeLine32() const 		{ return width*4; }*
	inline int Image::sizeExtra() const			{ return pitch - sizeLine(); } */
	
	
	
	
	inline int Image::sizeExtra8(int w) const 		{ return pitch - w; }
	inline int Image::sizeExtra32(int w) const 		{ return pitch - w*4; }
	inline int Image::sizeExtra8() const 		{ return sizeExtra8(width); }
	inline int Image::sizeExtra32() const 		{ return sizeExtra32(width); }
	
	
	
	
	inline Rect Image::getRect() const			{ return Rect(0,0,width,height); }
	inline bool Image::chkPos(uint x, uint y) const { return ((x < width)&&(y < height)); }
	
	// IMPLEMENTATION:  pixel access
	CONST_FUNC3(inline, byte* Image::EndPtr(), { return getLn(height); })
	CONST_FUNC3(inline, byte* Image::getLn(int y), { return bColors+y*pitch; });
	//CONST_FUNC3(inline, byte* Image::getPtr(int x, int y), { return getLn(y)+x*pixSize(); });
	CONST_FUNC3(inline, byte* Image::get8(int x, int y), { return getLn(y)+x; });
	CONST_FUNC3(inline, Color* Image::get32(int x, int y), { return ((Color*)getLn(y))+x; });
	
	
	inline byte* Image::getPtr(int x, int y) { 
		return (byte*)pCnst(this)->getPtr(x, y); }
	
	/*
	CONST_FUNC3(inline, byte* Image::GetPtr(int x, int y), {
		return &bColors[x + y*pitch]; })
	CONST_FUNC3(inline, Color& Image::GetRef(int x, int y),  {
		return *(Color*)GetPtr(x*4, y); })
	CONST_FUNC3(TMPL(F) inline, void Image::for_each8(const Rect& rc, F fn), {
		auto linePos = GetPtr(rc.left, rc.top); int width = rc.Width();	int height =
		rc.Height(); for(int y = 0; y < height; y++) {	for(int x = 0;x < width; x++) 
		fn(x, y, linePos[x], palette[linePos[x]]);	PTRADD(linePos, pitch); }})
	CONST_FUNC3(TMPL(F) inline, void Image::for_each32(const Rect& rc, F fn), {
		auto linePos = &GetRef(rc.left, rc.top); int width = rc.Width();
		int height = rc.Height(); for(int y = 0; y < height; y++) { for(int x = 0;
			x < width; x++) fn(x, y, linePos[x]); PTRADD(linePos, pitch); }})*/
}

#endif
