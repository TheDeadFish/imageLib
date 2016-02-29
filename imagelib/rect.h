// Image handling library
// Rectangle struct
#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_
namespace ImageLib {

struct Rect : RECT
{
	// Construction / Assignment
	Rect() = default; Rect(int x);
	Rect(const RECT& that) { PCST(RECT, this) = that; }
	Rect(LONG l, LONG t, LONG r, LONG b);
	static Rect RectXYWH(LONG x, LONG y, LONG w, LONG h);
	Rect& Set(LONG l, LONG t, LONG r, LONG b);
	Rect& SetXW(LONG x, LONG w); 
	Rect& SetYH(LONG y, LONG h);
	
	// Mutators
	Rect& offset(int xDelta, int yDelta);
	Rect& inflate(int xDelta, int yDelta);
	
	LONG Width() const; 
	LONG Height() const;
	Rect clipRect(LONG width, LONG height) const;
	Rect clipRect(const RECT& that) const;
	int negExtX() const; int negExtY() const;
	
	// Its sad that its come to this
	template <bool w, bool x, bool y, bool z>
	Rect& Set2(LONG l, LONG t, LONG r, LONG b);
};

// IMPLEMENTATION
inline Rect::Rect(int x) {
	this->Set(0,0,0,0); }
inline Rect::Rect(LONG l, LONG t, LONG r, LONG b) {
	this->Set(l,t,r,b); }
inline Rect& Rect::Set(LONG l, LONG t, LONG r, LONG b) {
	left = l; top = t; right = r; bottom = b; return *this;}
inline Rect& Rect::SetXW(LONG x, LONG w) {
	left = x; right = x+w; return *this;}
inline Rect& Rect::SetYH(LONG y, LONG h) {
	top = y; bottom = y+h; return *this;}
inline Rect Rect::RectXYWH(LONG x, LONG y, LONG w, LONG h) {
	return Rect(x, y, x+w, y+h); }
	
// mutators	
inline Rect& Rect::offset(int xDelta, int yDelta) {
	left += xDelta; right += xDelta; top += yDelta;
	bottom += yDelta; return *this;}
inline Rect& Rect::inflate(int xDelta, int yDelta) {
	left -= xDelta; right += xDelta; top -= yDelta;
	bottom += yDelta; return *this;}
	
	
inline LONG Rect::Width() const {
	return right-left; }
inline LONG Rect::Height() const {
	return bottom-top; }
	
inline Rect Rect::clipRect(LONG width, LONG height) const{
	return clipRect(Rect(0, 0, width, height)); }
inline Rect Rect::clipRect(const RECT& that) const {
	Rect result;
	result.left = std::max(left, that.left);
	result.right = std::min(right, that.right);
	if(result.left > result.right)
		result.left = result.right;
	result.top = std::max(top, that.top);
	result.bottom = std::min(bottom, that.bottom);
	if(result.top > result.bottom)
		result.top = result.bottom;
	return result; }
inline int Rect::negExtX() const {
	return -std::min(left, (LONG)0); }
inline int Rect::negExtY() const {
	return -std::min(top, (LONG)0); }
	
// its sad that its come to this
template <bool w, bool x, bool y, bool z>
Rect& Rect::Set2(LONG l, LONG t, LONG r, LONG b) {
	if(w) left = l; if(x) top = t;
	if(y) right = r; if(z) bottom = b; return *this; }
}

#endif
