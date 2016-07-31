#include "uvhttp_util.h"
#include <assert.h>
#include <string.h>

void uvhttp_buffer_init(
    struct uvhttp_buffer *buf,
    unsigned int initial_size
    )
{
    buf->len = buf->size = 0;
    buf->base = NULL;
    uvhttp_buf_resize(buf, initial_size);
}

void uvhttp_buffer_free(
    struct uvhttp_buffer *buf
    )
{
    if (buf->base != NULL) {
        free(buf->base);
        uvhttp_buffer_init(buf, 0);
    }
}

void uvhttp_buf_resize(
    struct uvhttp_buffer *a,
    unsigned int new_size)
{
    if (new_size > a->size || (new_size < a->size && new_size >= a->len)) {
        char *buf = (char *) realloc(a->base, new_size);
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

void uvhttp_buf_trim(
    struct uvhttp_buffer *buf
    )
{
    uvhttp_buf_resize(buf, buf->len);
}

unsigned int uvhttp_buf_insert(
    struct uvhttp_buffer *a, 
    unsigned int off, 
    const void *buf, 
    unsigned int len)
{
    char *p = NULL;

    assert(a != NULL);
    assert(a->len <= a->size);
    assert(off <= a->len);

    /* check overflow */
    if (~(unsigned int) 0 - (unsigned int) a->base < len) return 0;

    if (a->len + len <= a->size) {
        memmove(a->base + off + len, a->base + off, a->len - off);
        if (buf != NULL) {
            memcpy(a->base + off, buf, len);
        }
        a->len += len;
    } else if ((p = (char *) realloc(
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

unsigned int uvhttp_buf_append(
    struct uvhttp_buffer *a, 
    const void *buf, 
    unsigned int len) 
{
    return uvhttp_buf_insert(a, a->len, buf, len);
}

void uvhttp_buf_remove(struct uvhttp_buffer *mb, unsigned int n) {
    if (n > 0 && n <= mb->len) {
        memmove(mb->base, mb->base + n, mb->len - n);
        mb->len -= n;
    }
}

struct uvhttp_list* uvhttp_list_append( 
    struct uvhttp_list* list,
    void* data
    )
{
    int len = 0;
    struct uvhttp_list* first = 0;
    if ( list == 0) {
        list = (struct uvhttp_list*)malloc( sizeof(struct uvhttp_list));
        list->data = data;
        list->next = (struct uvhttp_list*)malloc( sizeof(struct uvhttp_list));
        list->next->data = (void*)1;
        list->next->next = list;
        return list->next;
    }
    first = list->next;
    list->next =  (struct uvhttp_list*)malloc( sizeof(struct uvhttp_list));
    len = (int)list->data;
    list->next->data = (void*)(++len);
    list->next->next = first;
    list->data = data;

    return list->next;
}

struct uvhttp_list* uvhttp_list_begin( 
    struct uvhttp_list* list
    )
{
    return list->next;
}

struct uvhttp_list* uvhttp_list_end( 
    struct uvhttp_list* list
    )
{
    return list;
}

void uvhttp_list_free(
    struct uvhttp_list* list
    )
{
    struct uvhttp_list* begin = list;
    struct uvhttp_list* cur = begin;
    struct uvhttp_list* next = begin;
    do 
    {
        next = next->next;
        free( cur);
        cur = next;
    } while ( begin != cur);
}

int uvhttp_list_size(
    struct uvhttp_list* list
    )
{
    return (int)list->data;
}