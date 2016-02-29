/* Implements a simple median cut */
/* Results probably far from optimal */
#define NUM_DIMENSIONS 4
#include "quantize.h"
#include <algorithm>

namespace ImageLib{

class Block
{
public:
	union{
		Color finalColor_;
		int longestLength; };
	int longestIndex;
	Color* colors;
	int colorsLength;
	
	void init(Color* colors, int colorsLength)
	{
		// find corner points
		this->colors = colors;
		this->colorsLength = colorsLength;
				
		// get longest edge
		auto side = colors->longestSide(colorsLength, sizeof(Color));
		longestLength = side.length; longestIndex = side.index;
	}
	
	void calcColor(void)
	{
		int sum[NUM_DIMENSIONS] = {0};
		for(int i=0; i < colorsLength; i++)
		  for(int j=0; j < NUM_DIMENSIONS; j++)
		    sum[j] += colors[i].getRef(j);
		for(int j=0; j < NUM_DIMENSIONS; j++) { finalColor_.
			getRef(j) = lrintf(float(sum[j]) /colorsLength); }
	}
	
	Color* getColors() const 		{	return colors; 				}
	int numColors() const 			{	return colorsLength; 		}
	Color finalColor(void) const	{	return finalColor_;			}
	int longestSideIndex() const 	{	return longestIndex;		}
	int longestSideLength() const 	{	return longestLength; 		}
};

class BlockQue
{
	Block* blocks;
	int curPos;
	
public:
	bool Init(int size)
	{
		blocks = (Block*)malloc(size*sizeof(Block));
		curPos = -1;
		return (blocks != 0);
	}
	
	void findTop()
	{
		if(curPos > 0)
		{
			int bestLength = top().longestSideLength();
			int bestPos = curPos;
			for(int i = 0; i < curPos; i++)
			{
				int curLength = blocks[i].longestSideLength();
				if(curLength > bestLength)
				{
					bestLength = curLength;
					bestPos = i;
				}
			}
			if(bestPos != curPos)
				std::swap(blocks[curPos], blocks[bestPos]);
		}
	}
	
	void calcColors(void)
	{
		for(int i = 0; i <= curPos; i++)
			blocks[i].calcColor();
	}
	
	void Free(void)		{ 	free(blocks);				}
	int size()			{  	return curPos+1;			}
	Block& top()		{	return blocks[curPos];		}
	void push(Color* colors, int colorsLength)	{ curPos += 1;
		top().init(colors, colorsLength);	}
		
		
		
		
		
		
		
		
	void pop(void)		{	curPos -= 1;				}
};


	
SHITSTATIC
Color* medianSplit_median(Block& longestBlock,
	Color* median, Color* end)
{
	Color* begin = longestBlock.getColors();
	int sideIdx = longestBlock.longestSideIndex();
	
	// locate boundaries nearest median
	int medianSide = median[0].getRef(sideIdx);
	Color* leftMedian = median;
	while(leftMedian > begin) {
		if( leftMedian[-1].getRef(sideIdx) != medianSide )
			break;
		leftMedian--; }
	Color* rightMedian = median;
	while(rightMedian < end) {
		if( rightMedian[-1].getRef(sideIdx) != medianSide )
			break;
		rightMedian++;}
		
	// select nearest boundary
	median = ((median-leftMedian) < (rightMedian-median))
		? leftMedian : rightMedian;
	if((median == begin)||(median == end))
		return NULL;
	return median;
}

SHITSTATIC
Color* medianSplit_mode(Block& longestBlock,
	Color* median, Color* end)
{
	Color* begin = longestBlock.getColors();
	int numCols = longestBlock.numColors();
	int sideIdx = longestBlock.longestSideIndex();
	
	// get average
	int total = 0;
	for(Color* i = begin; i < end; i++)
		total += i->getRef(sideIdx);
	total /= numCols;
	
	// split on average
	for(Color* i = begin; i < end; i++)
	if(total >= i->getRef(sideIdx))
	{
		if(i == begin)
			return NULL;
		return i;
	}
	return NULL;
}

int medianCut_core(Color* image, int numColors, Color* palette,
	int reqColors, int split_mode)
{
	// setup the first block
	typeof(&medianSplit_median) medianSplit = split_mode
		? medianSplit_mode : medianSplit_median;
	BlockQue blockQue;
	if(!blockQue.Init(reqColors))
		return -1;
	blockQue.push(image, numColors);

	// divide the blocks
	while((blockQue.size() < reqColors)
	&& (blockQue.top().longestSideLength() > 0))
	{
		Block& longestBlock = blockQue.top();
		Color* begin  = longestBlock.getColors();
		Color* median = longestBlock.getColors() + (longestBlock.numColors()+1)/2;
		Color* end    = longestBlock.getColors() + longestBlock.numColors();
		int numCols = longestBlock.numColors();
		int sideIdx = longestBlock.longestSideIndex();

		begin->sortr(sideIdx, numCols, 4);
		median = medianSplit(longestBlock, median, end);
		if( median == NULL ) {
			longestBlock.longestLength = 0;
			goto FIND_TOP; }
			
		blockQue.pop();
		blockQue.push(begin, median-begin);
		blockQue.push(median, end-median);
	FIND_TOP:
		blockQue.findTop();
	}
	
	// generate the new palette	
	blockQue.calcColors();
	int actualColors = blockQue.size();
	for(int i = 0; i < actualColors; i++) {
		palette[i] = blockQue.top().finalColor();
		blockQue.pop();	}
	blockQue.Free();
	return actualColors;
}

int medianCut(Image& img, Color* palette, int reqColors,
	int split_mode, LineConvFunc2 fn, size_t arg)
{
	auto buff = img.alloc(0);
	if(!buff) return -1; SCOPE_EXIT(free(buff));
	convImgTo32(img, buff.data, buff.pitch, fn, arg);
	return medianCut_core((Color*)buff.data, 
		img.nPixels(), palette, reqColors, split_mode);
}



}
