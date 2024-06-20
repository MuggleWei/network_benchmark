/******************************************************************************
 *  @file         tcp_client_handle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-20
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark tcp client handle
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_APP_TCP_CLIENT_HANDLE_H_
#define NETWORK_BENCHMARK_APP_TCP_CLIENT_HANDLE_H_

#include "network_benchmark/netbench.h"

EXTERN_C_BEGIN

typedef struct {
	nb_sys_args_t *args;
	muggle_socket_context_t *ctx; //!< connect socket context
	struct timespec last_ts; //!< last timer trigger timestamp
	uint32_t round_index;
	void *datas;
} tcp_client_evloop_data_t;

void tcp_client_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void tcp_client_on_close(muggle_event_loop_t *evloop,
						 muggle_socket_context_t *ctx);
void tcp_client_on_release(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void tcp_client_on_message(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void tcp_client_on_timer(muggle_event_loop_t *evloop);

void tcp_client_handle_message(muggle_event_loop_t *evloop,
							   muggle_socket_context_t *ctx, nb_msg_hdr_t *hdr,
							   void *data, uint32_t datalen);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_APP_TCP_CLIENT_HANDLE_H_
