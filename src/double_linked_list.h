/*
double_linked_list.h - a lazy implementation of the linked list
2023/07/29, peshqa
*/
#pragma once

typedef struct DoubleLinkedNode
{
	void *data;
	DoubleLinkedNode* next;
	DoubleLinkedNode* prev;
} DoubleLinkedNode;

typedef struct DoubleLinkedList
{
	DoubleLinkedNode* first;
	DoubleLinkedNode* last;
} DoubleLinkedList;

void InitDoubleLinkedList(DoubleLinkedList *list)
{
	list->first = 0;
	list->last = 0;
}

int AddNodeAtFront(DoubleLinkedList *list, void *data)
{
	DoubleLinkedNode *node = new DoubleLinkedNode;
	node->data = data;
	node->prev = 0;
	node->next = list->first;
	
	if (list->first == 0)
	{
		list->first = list->last = node;
		return 0;
	}
	
	DoubleLinkedNode *first_node = list->first;
	first_node->prev = node;
	
	list->first = node;
	
	return 0;
}

int AddNodeAtBack(DoubleLinkedList *list, void *data)
{
	DoubleLinkedNode *node = new DoubleLinkedNode;
	node->data = data;
	node->prev = list->last;
	node->next = 0;
	
	if (list->first == 0)
	{
		list->first = list->last = node;
		return 0;
	}
	
	DoubleLinkedNode *last_node = list->last;
	last_node->next = node;
	
	list->last = node;
	
	return 0;
}

void *RemoveNodeAtFront(DoubleLinkedList *list)
{
	if (list->first == 0)
	{
		return 0;
	}
	
	DoubleLinkedNode *node = list->first;
	void *data = node->data;
	
	DoubleLinkedNode *first_node = node->next;
	
	list->first = first_node;
	
	if (first_node != 0)
	{
		first_node->prev = 0;
	} else {
		list->last = 0;
	}
	
	delete node;
	return data;
}

void *RemoveNodeAtBack(DoubleLinkedList *list)
{
	if (list->first == 0)
	{
		return 0;
	}
	
	DoubleLinkedNode *node = list->last;
	void *data = node->data;
	
	DoubleLinkedNode *last_node = node->prev;
	
	list->last = last_node;
	
	if (last_node != 0)
	{
		last_node->next = 0;
	} else {
		list->first = 0;
	}
	
	delete node;
	return data;
}