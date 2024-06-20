/******************************************************************************
 *  @file         message.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-19
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark message struct
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_MSG_STRUCT_H_
#define NETWORK_BENCHMARK_MSG_STRUCT_H_

#include "network_benchmark/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

	; // for avoid vim LSP complain
#pragma pack(push)
#pragma pack(1)

typedef struct {
	uint32_t msg_id; //!< message id
	uint32_t payload_len; //!< payload length (not include message head)
} nb_msg_hdr_t;
NET_BENCH_STRICT_CHECK_MSG_STRUCT(nb_msg_hdr_t);

typedef struct {
	uint32_t user_id;
	uint32_t sequence;
} nb_msg_data_t;
NET_BENCH_STRICT_CHECK_MSG_STRUCT(nb_msg_data_t);

#pragma pack(pop)

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_MSG_STRUCT_H_
