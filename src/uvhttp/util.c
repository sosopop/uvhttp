#include "util.h"
#include <assert.h>
#include <string.h>

#ifndef UVBUF_REALLOC
#define UVBUF_REALLOC realloc
#endif

#ifndef UVBUF_FREE
#define UVBUF_FREE free
#endif

void uv_buf_ex_init(struct uv_buf_ex *buf, size_t initial_size) {
  buf->len = buf->size = 0;
  buf->base = NULL;
  uv_buf_ex_resize(buf, initial_size);
}

void uv_buf_ex_free(struct uv_buf_ex *buf) {
  if (buf->base != NULL) {
    UVBUF_FREE(buf->base);
    uv_buf_ex_init(buf, 0);
  }
}

void uv_buf_ex_resize(struct uv_buf_ex *a, size_t new_size) {
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

void uv_buf_ex_trim(struct uv_buf_ex *buf) {
  uv_buf_ex_resize(buf, buf->len);
}

size_t uv_buf_ex_insert(struct uv_buf_ex *a, size_t off, const void *buf, size_t len) {
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

size_t uv_buf_ex_append(struct uv_buf_ex *a, const void *buf, size_t len) {
  return uv_buf_ex_insert(a, a->len, buf, len);
}

void uv_buf_ex_remove(struct uv_buf_ex *mb, size_t n) {
  if (n > 0 && n <= mb->len) {
    memmove(mb->base, mb->base + n, mb->len - n);
    mb->len -= n;
  }
}