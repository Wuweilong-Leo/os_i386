#ifndef LIST_H
#define LIST_H
#include "global.h"
#include "stdint.h"
#ifndef NULL
#define NULL (void *)0
#endif
#define OFFSET(struct_type, member) ((uint32_t)(&(((struct_type *)0)->member)))
#define ELEM2ENTRY(struct_type, struct_mem_name, elem_ptr)                     \
  ((struct_type *)((uint32_t)elem_ptr - OFFSET(struct_type, struct_mem_name)))

struct list_elem {
  struct list_elem *prev;
  struct list_elem *next;
};
struct list {
  struct list_elem head;
  struct list_elem tail;
};
void list_init(struct list *list);
void list_push_front(struct list *list, struct list_elem *e);
void list_push_back(struct list *list, struct list_elem *e);
void list_remove(struct list_elem *e);
struct list_elem *list_pop_front(struct list *list);
bool list_elem_find(struct list *list, struct list_elem *e);
uint32_t list_len(struct list *list);
bool list_empty(struct list *list);
struct list_elem *list_first_elem(struct list *list);
#endif