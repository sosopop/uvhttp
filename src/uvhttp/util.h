#ifndef util_h__
#define util_h__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>

#ifndef UVBUF_SIZE_MULTIPLIER
#define UVBUF_SIZE_MULTIPLIER 2
#endif

/* Memory buffer descriptor */
struct uv_buf_ex {
  char *base;   /* Buffer pointer */
  size_t len;  /* Data length. Data is located between offset 0 and len. */
  size_t size; /* Buffer size allocated by realloc(1). Must be >= len */
};
void uv_buf_ex_init(struct uv_buf_ex *, size_t initial_capacity);
void uv_buf_ex_free(struct uv_buf_ex *);
size_t uv_buf_ex_append(struct uv_buf_ex *, const void *data, size_t data_size);
size_t uv_buf_ex_insert(struct uv_buf_ex *, size_t, const void *, size_t);
void uv_buf_ex_remove(struct uv_buf_ex *, size_t data_size);
void uv_buf_ex_resize(struct uv_buf_ex *, size_t new_size);
void uv_buf_ex_trim(struct uv_buf_ex *);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif // util_h__
