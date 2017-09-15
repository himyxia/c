#ifndef _list_h
#define _list_h

struct ListNode;

typedef struct ListNode {
	struct ListNode *prev;
	struct ListNode *next;
	void *val;
}ListNode;

typedef struct List {
	ListNode *first;
	ListNode *last;
	int count;
}List;

List *List_create();
void List_push(List *list, void *val);
ListNode *list_pop();
void list_destory(List *list);
void list_clear_destory(List *list);
void list_shift(List *list);
void list_unshift(List *list);

#endif
