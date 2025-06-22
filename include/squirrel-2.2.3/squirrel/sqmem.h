#ifndef SQMEM_H
#define SQMEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> // pour size_t

typedef unsigned int SQUnsignedInteger;

void *sq_vm_malloc(SQUnsignedInteger size);
void *sq_vm_realloc(void *p, SQUnsignedInteger oldsize, SQUnsignedInteger size);
void sq_vm_free(void *p, SQUnsignedInteger size);

#ifdef __cplusplus
}
#endif

#endif // SQMEM_H