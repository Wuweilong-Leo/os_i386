#include "list.h"
#include "interrupt.h"

void list_init(struct list *list) {
  list->head.prev = NULL;
  list->head.next = &list->tail;
  list->tail.prev = &list->head;
  list->tail.next = NULL;
}

static void list_insert_before(struct list_elem *e1, struct list_elem *e2) {
  enum intr_status old_status = intr_disable();
  e1->prev->next = e2;
  e2->prev = e1->prev;
  e2->next = e1;
  e1->prev = e2;
  intr_set_status(old_status);
};

void list_push_front(struct list *list, struct list_elem *e) {
  list_insert_before(list->head.next, e);
}

void list_push_back(struct list *list, struct list_elem *e) {
  list_insert_before(&(list->tail), e);
}

void list_remove(struct list_elem *e) {
  enum intr_status old_status = intr_disable();
  e->prev->next = e->next;
  e->next->prev = e->prev;
  intr_set_status(old_status);
}

struct list_elem *list_pop_front(struct list *list) {
  struct list_elem *ret = list->head.next;
  list_remove(ret);
  return ret;
}

bool list_elem_find(struct list *list, struct list_elem *e) {
  struct list_elem *tmp = list->head.next;
  while (tmp != (&list->tail)) {
    if (tmp == e) {
      return true;
    }
    tmp = tmp->next;
  }
  return false;
}

uint32_t list_len(struct list *list) {
  struct list_elem *tmp = list->head.next;
  uint32_t cnt = 0;
  while (tmp != &list->tail) {
    cnt++;
    tmp = tmp->next;
  }
  return cnt;
}

bool list_empty(struct list *list) { return list->head.next == &list->tail; }
