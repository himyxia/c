#include "list.h" 
#include "dbg.h" 

List *List_create() {
	return calloc(1, sizeof(List));
}

void List_push(List *list, void *val) {
	check_mem(list);
	ListNode *node = calloc(1, sizeof(ListNode));
	node->val = val;

	if (list->last == NULL) {
		list->first = node;
		list->last = node;
	}else {
		node->prev = list->last;
		list->last->next = node;
		list->last = node;
	}
	list->count++;
error:
	return;
}

ListNode *List_pop(List *list) {
	check_mem(list);

	ListNode *node = list->last;
	List_remove(list, list->last);
	return node;

error:
	return NULL;
}

void List_destory(List *list) {
  if(!list) {
	  return;
  }
  ListNode *cur = NULL;
  ListNode *_node = NULL;
  for(cur = _node = list->first; _node != NULL; cur = _node = _node->next) {
	  if (cur->prev) {
			free(cur->prev->val);
			free(cur->prev);
	  }
  }
  free(list->last->val);
  free(list->last);
  free(list);
}

void List_remove(List *list, ListNode *node) {
	if(!list || !node) {
		return;
	}
	if(!list->first && !list->last) {
		// list is empty
		return;
	}
	if(node == list->first && node == list->last) {
		list->first = NULL;
		list->last = NULL;
	}else if (list->first == node) {
		list->first = list->first->next;
		list->first->prev = NULL;
	}else if(list->last == node){
		list->last = list->last->prev;
		list->last->next = NULL;
	}else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}
	free(node);
	list->count--;
	return;
}

void List_shift(List *list) {
	if(!list) {
		return;
	}
	return list->first == NULL?NULL:List_remove(list, list->first);
}

void List_unshift(List *list, void *val) {
	if(!list) {
		return;
	}
	ListNode *node = calloc(1, sizeof(ListNode));
	node->val = val;
	if (list->last == NULL) {
		list->first = node;
		list->last = node;
	}else {
		list->first->prev = node;
		node->next = list->first;
		list->first = node;
	}
	list->count++;
}
