#ifndef UVHTTP_INTERNAL_H__
#define UVHTTP_INTERNAL_H__

#if defined(__cplusplus)
extern "C" {
#endif

#define  UVHTTP_NET_BUFFER_SIZE 4*1024

struct uvhttp_header* uvhttp_headers_append( 
    struct uvhttp_header* headers,
    char* field,
    char* value
    );

void uvhttp_headers_free(
    struct uvhttp_header* headers
    );

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // UVHTTP_INTERNAL_H__