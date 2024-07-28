#include "tLib.h"

#define firstNode headNode.nextNode
#define lastNode headNode.preNode


void tNodeInit(tNode* node)
{
	node->preNode = node;
	node->nextNode = node;
}

void tListInit(tList* list)
{
	list->firstNode = &(list->headNode);
	list->lastNode = &(list->headNode);
	list->NodeCount = 0;
}

uint32_t tListCount(tList* list)
{
	return list->NodeCount;
}

tNode* tListFirst(tList* list)
{
	tNode* node = (tNode*) 0;
	if(list->NodeCount != 0)
	{
		node = list->firstNode;
	}
	return node;
}

tNode* tListLast(tList* list)
{
	tNode* node = (tNode*) 0;
	if(list->NodeCount != 0)
	{
		node = list->lastNode;
	}
	return node;
}


tNode* tListPre(tNode* node)
{
	if(node->preNode == node) return (tNode*) 0;
	return node->preNode;
}

tNode* tListNext(tNode* node)
{
	if(node->nextNode == node) return (tNode*) 0;
	return node->nextNode;
}

tNode* tListRemoveAll(tList* list)
{
	uint32_t count;
	tNode* nextNode;
	nextNode = list->firstNode;
	for(count = list->NodeCount; count != 0; count--)
	{
		tNode* currentNode = nextNode;
		nextNode = nextNode->nextNode;
		
		currentNode->preNode = currentNode;
		currentNode->nextNode = currentNode;
	}
	
	list->firstNode = &(list->headNode);
	list->lastNode = &(list->headNode);
	list->NodeCount = 0;
}

// 头插
void tListAddFirst(tList* list, tNode* node)
{
	// 对新节点进行连接
	node->preNode = list->firstNode->preNode;
	node->nextNode = list->firstNode;
	// 对链表节点进行连接
	list->firstNode->preNode = node;
	list->firstNode = node;
	// 增加节点数
	list->NodeCount++;
}

// 尾插
void tListAddLast(tList* list, tNode* node)
{
	// 对新节点进行连接
	node->preNode = list->lastNode;
	node->nextNode = list->lastNode->nextNode;
	// 对链表节点进行连接
	list->lastNode->nextNode = node;
	list->lastNode = node;
	// 增加节点数
	list->NodeCount++;
}

// 头删
tNode* tListRemoveFirst(tList* list)
{
	tNode* node = (tNode*) 0;
	if(list->NodeCount != 0)
	{
		node = list->firstNode;
		node->nextNode->preNode = &(list->headNode);
		list->firstNode = node->nextNode;
		list->NodeCount--;
	}
	return node;
}

// 尾删
tNode* tListRemoveLast(tList* list)
{
	tNode* node = (tNode*) 0;
	if(list->NodeCount != 0)
	{
		node = list->lastNode;
		node->preNode->nextNode = &(list->headNode);
		list->lastNode = node;
		list->NodeCount--;
	}
	return node;
}

// 删除指定节点
tNode* tListRemove(tList* list, tNode* node)
{
	tNode* toRm = (tNode*) 0;
	if(list->NodeCount != 0)
	{
		toRm = node;
		node->nextNode->preNode = node->preNode;
		node->preNode->nextNode = node->nextNode;
		list->NodeCount--;
	}
	return toRm;
}

// 将节点插入到指定节点后面
void tListInsertAfter(tList* list, tNode* node, tNode* nodeToInsert)
{
	nodeToInsert->preNode = node;
	nodeToInsert->nextNode = node->nextNode;
	
	node->nextNode->preNode = nodeToInsert;
	node->nextNode = nodeToInsert;
	
	list->NodeCount++;
}