#include "uvhttp_util.h"
#include <assert.h>
#include <string.h>

#ifndef UVBUF_REALLOC
#define UVBUF_REALLOC realloc
#endif

#ifndef UVBUF_FREE
#define UVBUF_FREE free
#endif

void uvhttp_buf_init(struct uvhttp_buf *buf, size_t initial_size) {
  buf->len = buf->size = 0;
  buf->base = NULL;
  uvhttp_buf_resize(buf, initial_size);
}

void uvhttp_buf_free(struct uvhttp_buf *buf) {
  if (buf->base != NULL) {
    UVBUF_FREE(buf->base);
    uvhttp_buf_init(buf, 0);
  }
}

void uvhttp_buf_resize(struct uvhttp_buf *a, size_t new_size) {
  if (new_size > a->size || (new_size < a->size && new_size >= a->len)) {
    char *buf = (char *) UVBUF_REALLOC(a->base, new_size);
    /*
     * In case realloc fails, there's not much we can do, except keep things as
     * they are. Note that NULL is a valid return value from realloc when
     * size == 0, but that is covered too.
     */
    if (buf == NULL && new_size != 0) return;
    a->base = buf;
    a->size = new_size;
  }
}

void uvhttp_buf_trim(struct uvhttp_buf *buf) {
  uvhttp_buf_resize(buf, buf->len);
}

size_t uvhttp_buf_insert(struct uvhttp_buf *a, size_t off, const void *buf, size_t len) {
  char *p = NULL;

  assert(a != NULL);
  assert(a->len <= a->size);
  assert(off <= a->len);

  /* check overflow */
  if (~(size_t) 0 - (size_t) a->base < len) return 0;

  if (a->len + len <= a->size) {
    memmove(a->base + off + len, a->base + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->base + off, buf, len);
    }
    a->len += len;
  } else if ((p = (char *) UVBUF_REALLOC(
                  a->base, (a->len + len) * UVBUF_SIZE_MULTIPLIER)) != NULL) {
    a->base = p;
    memmove(a->base + off + len, a->base + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->base + off, buf, len);
    }
    a->len += len;
    a->size = a->len * UVBUF_SIZE_MULTIPLIER;
  } else {
    len = 0;
  }

  return len;
}

size_t uvhttp_buf_append(struct uvhttp_buf *a, const void *buf, size_t len) {
  return uvhttp_buf_insert(a, a->len, buf, len);
}

void uvhttp_buf_remove(struct uvhttp_buf *mb, size_t n) {
  if (n > 0 && n <= mb->len) {
    memmove(mb->base, mb->base + n, mb->len - n);
    mb->len -= n;
  }
}

static struct uvhttp_slist *slist_get_last(struct uvhttp_slist *list)
{
  struct uvhttp_slist     *item;

  /* if caller passed us a NULL, return now */
  if(!list)
    return NULL;

  /* loop through to find the last item */
  item = list;
  while(item->next) {
    item = item->next;
  }
  return item;
}

struct uvhttp_slist *uvhttp_slist_append_nodup(struct uvhttp_slist *list, char *data)
{
  struct uvhttp_slist     *last;
  struct uvhttp_slist     *new_item;

  new_item = (struct uvhttp_slist*)malloc(sizeof(struct uvhttp_slist));
  if(!new_item)
    return NULL;

  new_item->next = NULL;
  new_item->data = data;

  /* if this is the first item, then new_item *is* the list */
  if(!list)
    return new_item;

  last = slist_get_last(list);
  last->next = new_item;
  return list;
}

/*
 * uvhttp_slist_append() appends a string to the linked list. It always returns
 * the address of the first record, so that you can use this function as an
 * initialization function as well as an append function. If you find this
 * bothersome, then simply create a separate _init function and call it
 * appropriately from within the program.
 */
struct uvhttp_slist *uvhttp_slist_append(struct uvhttp_slist *list,
                                     const char *data)
{
  char *dupdata = strdup(data);

  if(!dupdata)
    return NULL;

  list = uvhttp_slist_append_nodup(list, dupdata);
  if(!list)
    free(dupdata);

  return list;
}

/*
 * uvhttp_slist_duplicate() duplicates a linked list. It always returns the
 * address of the first record of the cloned list or NULL in case of an
 * error (or if the input list was NULL).
 */
struct uvhttp_slist *uvhttp_slist_duplicate(struct uvhttp_slist *inlist)
{
  struct uvhttp_slist *outlist = NULL;
  struct uvhttp_slist *tmp;

  while(inlist) {
    tmp = uvhttp_slist_append(outlist, inlist->data);

    if(!tmp) {
      uvhttp_slist_free_all(outlist);
      return NULL;
    }

    outlist = tmp;
    inlist = inlist->next;
  }
  return outlist;
}

/* be nice and clean up resources */
void uvhttp_slist_free_all(struct uvhttp_slist *list)
{
  struct uvhttp_slist     *next;
  struct uvhttp_slist     *item;

  if(!list)
    return;

  item = list;
  do {
    next = item->next;
    UVHTTP_SAFE_FREE(item->data);
    free(item);
    item = next;
  } while(next);
}