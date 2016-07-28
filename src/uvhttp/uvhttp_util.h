#ifndef UVHTTP_UTIL_H__
#define UVHTTP_UTIL_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>

#define UVHTTP_CONTAINER_PTR(  s, m, p) (s*)((unsigned char*)p - (int)(&((s*)0)->m))
#define UVHTTP_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define UVHTTP_SAFE_FREE(p) if(p){free(p); p = 0;}


#ifndef UVBUF_SIZE_MULTIPLIER
#define UVBUF_SIZE_MULTIPLIER 2
#endif

struct uvhttp_str {
    unsigned int len;
    char* base;
};

/* Memory buffer descriptor */
struct uvhttp_buf {
  char *base;   /* Buffer pointer */
  size_t len;  /* Data length. Data is located between offset 0 and len. */
  size_t size; /* Buffer size allocated by realloc(1). Must be >= len */
};

void uvhttp_buf_init(struct uvhttp_buf *, size_t initial_capacity);
void uvhttp_buf_free(struct uvhttp_buf *);
size_t uvhttp_buf_append(struct uvhttp_buf *, const void *data, size_t data_size);
size_t uvhttp_buf_insert(struct uvhttp_buf *, size_t, const void *, size_t);
void uvhttp_buf_remove(struct uvhttp_buf *, size_t data_size);
void uvhttp_buf_resize(struct uvhttp_buf *, size_t new_size);
void uvhttp_buf_trim(struct uvhttp_buf *);

struct uvhttp_slist {
    char *data;
    struct uvhttp_slist *next;
};

struct uvhttp_slist *uvhttp_slist_append(
    struct uvhttp_slist *,
    const char *
    );

void uvhttp_slist_free_all(
    struct uvhttp_slist*
    );

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif // UVHTTP_UTIL_H__