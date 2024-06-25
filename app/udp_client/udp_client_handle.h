/******************************************************************************
 *  @file         udp_client_handle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-24
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark udp client handle
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_APP_UDP_CLIENT_HANDLE_H_
#define NETWORK_BENCHMARK_APP_UDP_CLIENT_HANDLE_H_

#include "network_benchmark/netbench.h"

EXTERN_C_BEGIN

typedef struct {
	nb_sys_args_t *args;
	muggle_socket_context_t *ctx; //!< socket context
	struct sockaddr *dst_sa;
	muggle_socklen_t dst_addrlen;
	struct timespec last_ts; //!< last timer trigger timestamp
	uint32_t round_index;
	void *datas;
} udp_client_evloop_data_t;

void udp_client_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void udp_client_on_timer(muggle_event_loop_t *evloop);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_APP_UDP_CLIENT_HANDLE_H_
