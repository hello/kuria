/**
 * @file
 *
 *
 */

#ifndef TASK_HOSTCOM_H
#define TASK_HOSTCOM_H

#include "xep_dispatch.h"

typedef struct
{
	void* hostcom_task_handle;
	XepDispatch_t* dispatch;
	uint8_t* messagebuild_buffer;
	uint32_t messagebuild_index;
	uint32_t messagebuild_length;

} XepHostComMCPUserReference_t;


#ifdef __cplusplus
extern "C" {
#endif

uint32_t task_hostcom_init(XepDispatch_t* dispatch);

#ifdef __cplusplus
}
#endif

#endif // TASK_HOSTCOM_H