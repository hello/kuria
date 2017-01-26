/**
 * @file
 *
 *
 */

#include "xtmemory.h"
#include <FreeRTOS.h>
#include "heap_4_default.h"
#include "heap_4_slow.h"
#include "heap_4_fast.h"


void * xtmemory_malloc_default(uint32_t size)
{
    return pvPortMalloc(size);
}

void xtmemory_free_default(void* ptr)
{
    vPortFree(ptr);
}

uint32_t xtmemory_heap_available_default(void)
{
    return xPortGetFreeHeapSize();
}

void * xtmemory_malloc_slow(uint32_t size)
{
	return pvPortMalloc_slow(size);
}

void xtmemory_free_slow(void* ptr)
{
	vPortFree_slow(ptr);
}

uint32_t xtmemory_heap_available_slow(void)
{
	return xPortGetFreeHeapSize_slow();
}

void* xtmemory_malloc_fast(uint32_t size)
{
	return pvPortMalloc_fast(size);
}

void xtmemory_free_fast(void* ptr)
{
	vPortFree_fast(ptr);
}

uint32_t xtmemory_heap_available_fast(void)
{
	return xPortGetFreeHeapSize_fast();
}
