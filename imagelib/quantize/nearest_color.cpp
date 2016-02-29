#include "nearest_color.h"

namespace ImageLib{
namespace {

struct KdNode
{
	KdNode* parent;
	KdNode* left;
	KdNode* right;
	Color color;
	short splitIndex;
	short palIndex;
};


}

void nearest_color::create(Color* palette, int palSize)
{
	free_ref(nodes); 
	new (this) nearest_color(palette, palSize); nothing();
}

nearest_color::nearest_color(Color* palette, int palSize)
{
	// allocate memory
	struct ColorData { Color color; int palIndex; };
	struct NodeInfo	{ ColorData* begin; int length; };
	KdNode* nodes = xMalloc(palSize*2);
	NodeInfo* nodeInfo = (NodeInfo*)xmalloc(palSize
		*(2*sizeof(NodeInfo) + sizeof(ColorData)));
	SCOPE_EXIT(::free(nodeInfo));
	
	// copy colors
	ColorData* colorBase = (ColorData*)&nodeInfo[palSize*2];
	for(int i = 0; i < palSize; i++)
		colorBase[i] = {palette[i], i}; 
		
	// setup root node
	nodes[0].parent = 0;
	nodes[0].left = 0;
	nodeInfo[0] = {colorBase, palSize};
	int curPos = 0; int maxPos = 1;
	
	// generate nodes
	for(;curPos != maxPos; curPos++)
	{
		// get the current node
		KdNode& curNode = nodes[curPos];
		ColorData* begin = nodeInfo[curPos].begin;
		int length = nodeInfo[curPos].length;
		if(length <= 1) continue;
		
		// find split index
		Color::SideLength sideLen;
		begin->color.sideLength(&sideLen, 
			length, sizeof(ColorData));
		curNode.splitIndex = 0;
		int longestLength = 0;
		for(int i = 0; i < 4; i++) {
			int diff = sideLen.sideLen.getRef(i);
			if(diff > longestLength) {
				longestLength = diff;
				curNode.splitIndex = i; 
			}
		}	

		// split the node along median
		begin->color.sort(curNode.splitIndex, length, 8);
		int leftLength = (length+1)/2;
		int rightLength = length-leftLength;
		ColorData* leftBegin = begin;
		ColorData* rightBegin = begin+leftLength;

		// setup parent
		nodes[curPos].left = &nodes[maxPos+0];
		nodes[curPos].right = &nodes[maxPos+1];
		nodes[curPos].color = rightBegin[0].color;
		nodes[curPos].palIndex = rightBegin[0].palIndex;
		
		// setup left
		nodes[maxPos+0].parent = &nodes[curPos];
		nodeInfo[maxPos+0] = {leftBegin, leftLength};
		if(leftLength == 1) {
			nodes[maxPos+0].left = 0;
			nodes[maxPos+0].color = leftBegin[0].color;
			nodes[maxPos+0].palIndex = leftBegin[0].palIndex;
		}
		
		// setup right
		nodes[maxPos+1].parent = &nodes[curPos];
		nodeInfo[maxPos+1] = {rightBegin, rightLength};
		if(rightLength == 1) {
			nodes[maxPos+1].left = 0;
			nodes[maxPos+1].color = rightBegin[0].color;
			nodes[maxPos+1].palIndex = rightBegin[0].palIndex;
		}
		maxPos += 2;						
	}
	
	this->nodes = nodes; 
}

nearest_color::pair_t 
	nearest_color::nearest(Color color)
{
	// initialize
	//int mulo = int(color.GetA())+1;
	//int divo = (255 + mulo)/mulo;
	KdNode* kdNodes = (KdNode*)nodes;
	KdNode* bestNode = (KdNode*)nodes;
	int bestDistance = 1e6;
	KdNode* stack_[256];
	KdNode** stack = stack_;
	stack[0] = 0;

FOWARD_SEARCH:
	// forward search
	while(kdNodes->left != 0)
	{
		if( color.getRef(kdNodes->splitIndex) >= 
		kdNodes->color.getRef(kdNodes->splitIndex))
			kdNodes = kdNodes->right;
		else
			kdNodes = kdNodes->left;
	}
	
	// check distance, take node as current best
	int rDiff = color.GetR() - kdNodes->color.GetR();
	int gDiff = color.GetG() - kdNodes->color.GetG();
	int bDiff = color.GetB() - kdNodes->color.GetB();
	//int aDiff = color.GetA() - kdNodes->color.GetA();
	int curDistance = rDiff*rDiff + gDiff*gDiff + bDiff*bDiff;
	//curDistance = (curDistance * mulo) >> 8;
	//curDistance += aDiff*aDiff;
	if(curDistance < bestDistance)
	{
		bestDistance = curDistance;
		bestNode = kdNodes;
	}
	
	
	// backward search
	while(kdNodes->parent != 0)
	{
		// have we done this node?
		kdNodes = kdNodes->parent;
		if(kdNodes == stack[0])
		{
			stack--;
			continue;
		}
		
		// check hypersphere
		int splitTmp = color.getRef(kdNodes->splitIndex) -
			kdNodes->color.getRef(kdNodes->splitIndex);
		int splitDist = splitTmp*splitTmp;
		int radius = bestDistance;
		//if(kdNodes->splitIndex != 3)
		//	radius *= (divo+1);
		//radius *= 2;
		if(radius > splitDist)
		{
			// search other side of plane
			*++stack = kdNodes;
			if( color.getRef(kdNodes->splitIndex) >= 
			kdNodes->color.getRef(kdNodes->splitIndex))
				kdNodes = kdNodes->left;
			else
				kdNodes = kdNodes->right;
			goto FOWARD_SEARCH;
		}
	}
	
	return { bestNode->palIndex,
		bestNode->color};
}

void nearest_color::convLine32ToIdx(byte* src, byte* dst, int width)
{
	while(--width >= 0) { WRI(dst, nearest(RDI(PI(src)))); }
}

void nearest_conv(Image& src, Image& dst, 
	LineConvFunc2 fn, size_t arg)
{
	assert((dst.palSize != 0)
	&&(src.width == dst.width)
	&&(src.height == dst.height));
	byte* lineBuff = xmalloc(dst.width*4);
	SCOPE_EXIT(free(lineBuff));
	nearest_color nc(dst.palette, dst.palSize);
	
	int dst_pitch = dst.pitch; 
	byte* dstPos = dst.bColors;
	byte* srcPos = src.bColors;
	int height = src.height; VARFIX(height);
	while(--height >= 0) {
		byte* line = convLineTo32(src, srcPos, lineBuff);
		line = fn(line, lineBuff, arg, src.width);
		nc.convLine32ToIdx(line, dstPos, src.width); 
		srcPos += src.pitch; dstPos += dst_pitch; }
}
}
