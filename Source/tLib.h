#ifndef __TLIB_H
#define __TLIB_H

#include <stdint.h>

typedef struct
{
	uint32_t bitmap;
}tBitmap;

void tBitmapInit(tBitmap* bitmap);
uint32_t tBitmapLength(void);
void tBitmapSet(tBitmap* bitmap, uint32_t pos);
void tBitmapClear(tBitmap* bitmap, uint32_t pos);
uint32_t tBitmapGetFirstSet(tBitmap* bitmap);

typedef struct _tNode
{
	struct _tNode * preNode;
	struct _tNode * nextNode;
}tNode;

typedef struct _tList
{
	tNode headNode;
	uint32_t NodeCount;
}tList;

#define tNodeParent(node, parent, name) (parent*)((uint32_t)node - (uint32_t)&((parent*)0)->name)

void tNodeInit(tNode* node);
void tListInit(tList* list);
uint32_t tListCount(tList* list);
tNode* tListFirst(tList* list);
tNode* tListLast(tList* list);
tNode* tListPre(tNode* node);
tNode* tListNext(tNode* node);
tNode* tListRemoveAll(tList* list);
void tListAddFirst(tList* list, tNode* node);
void tListAddLast(tList* list, tNode* node);
tNode* tListRemoveFirst(tList* list);
tNode* tListRemoveLast(tList* list);
tNode* tListRemove(tList* list, tNode* node);
void tListInsertAfter(tList* list, tNode* node, tNode* nodeToInsert);

#endif
