#include "tLib.h"

void tBitmapInit(tBitmap* bitmap)
{
	bitmap->bitmap = 0U;
}
uint32_t tBitmapLength(void)
{
	return 32;
}
void tBitmapSet(tBitmap* bitmap, uint32_t pos)
{
	bitmap->bitmap |= (1 << pos);
}
void tBitmapClear(tBitmap* bitmap, uint32_t pos)
{
	bitmap->bitmap &= ~(1 << pos);
}
uint32_t tBitmapGetFirstSet(tBitmap* bitmap)
{
	static const uint8_t quickFindTable[] = 
	{
		0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		6   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		7   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		6   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4   , 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
	};
	
	for(int i = 0; i < 4; i++)
	{
		uint8_t tmp = (bitmap->bitmap) >> (8 * i) & 0xff;
		if(tmp)
		{
			return quickFindTable[tmp] + (8 * i);
		}
	}
	
	return quickFindTable[0];
}