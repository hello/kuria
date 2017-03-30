#ifndef __HLO_QUEUE_H__
#define __HLO_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

#define HLO_QUEUE_ID_INVALID -1
#define HLO_QUEUE_MAX 50

typedef struct struct_hlo_queue_t hlo_queue_t;
typedef int32_t hlo_queue_id_t;

hlo_queue_id_t hlo_queue_create (uint32_t num_of_items, uint32_t size_per_item);
int32_t hlo_queue_delete (hlo_queue_id_t id);
int32_t hlo_queue_send (hlo_queue_id_t id, void* data, uint32_t timeout);
int32_t hlo_queue_recv (hlo_queue_id_t id, void* data, uint32_t timeout);
bool hlo_queue_is_full (hlo_queue_id_t id);
bool hlo_queue_is_empty (hlo_queue_id_t id);

#endif
