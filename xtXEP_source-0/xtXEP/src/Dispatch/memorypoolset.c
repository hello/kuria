/**
 * @file
 *
 * 
 */

#include "memorypoolset.h"
#include <stddef.h>



uint32_t memorypoolset_get_instance_size(void)
{
    uint32_t size = sizeof(MemoryPoolSet_t);
    return size;
}

uint32_t memorypoolset_create(MemoryPoolSet_t** memorypoolset, void* instance_memory)
{
    MemoryPoolSet_t* mps = (MemoryPoolSet_t*)instance_memory;
    mps->set_count = 0;
	*memorypoolset = mps;
    return XEP_MEM_ERROR_OK;
}

uint32_t memorypoolset_add_memorypool(MemoryPool_t** memorypool, MemoryPoolSet_t* memorypoolset, void* instance_memory, uint32_t block_size, uint32_t block_count)
{
    if (memorypoolset->set_count >= MEMORYPOOLSET_MAX_POOLS)
        return XEP_MEM_ERROR_FULL;
		
    uint32_t status = XEP_MEM_ERROR_OK;
	MemoryPool_t* mp;
    status = memorypool_create(&mp, instance_memory, block_size, block_count);
    if (status != XEP_MEM_ERROR_OK)
        return status;
    memorypoolset->set[memorypoolset->set_count] = mp;
    memorypoolset->set_count++;
	
	if (memorypool)
		*memorypool = mp;
		
    return XEP_MEM_ERROR_OK;
}

uint32_t memorypoolset_select_memorypool(MemoryPool_t** memorypool, MemoryPoolSet_t* memorypoolset, uint32_t size)
{
    if (memorypoolset->set_count == 0)
        return XEP_MEM_ERROR_FAILED;

    MemoryPool_t* mp_candidate = memorypoolset->set[0];
    for (uint32_t i=1; i<memorypoolset->set_count; i++)
    {
        if ((memorypoolset->set[i]->block_size >= size) && (memorypoolset->set[i]->block_size < mp_candidate->block_size))
        {
            mp_candidate = memorypoolset->set[i];
        }
    }
	if (mp_candidate->block_size < size)
	{
		return XEP_MEM_ERROR_FAILED;
	}
    *memorypool = mp_candidate;
    return XEP_MEM_ERROR_OK;
}

uint32_t memorypoolset_take_auto(MemoryBlock_t** memoryblock, MemoryPoolSet_t* memorypoolset, uint32_t size)
{
    MemoryPool_t* memorypool = NULL;
    uint32_t status = memorypoolset_select_memorypool(&memorypool, memorypoolset, size);
    if (status != XEP_MEM_ERROR_OK)
        return status;

    status = memorypool_take(memoryblock, memorypool);

    return status;
}
