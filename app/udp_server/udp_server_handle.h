/******************************************************************************
 *  @file         udp_server_handle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-24
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark udp server handle
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_APP_UDP_SERVER_HANDLE_H_
#define NETWORK_BENCHMARK_APP_UDP_SERVER_HANDLE_H_

#include "network_benchmark/netbench.h"

EXTERN_C_BEGIN

typedef struct {
	nb_sys_args_t *args;
	muggle_socket_context_t *ctx; //!< socket context
	struct sockaddr *upstream_sa;
	muggle_socklen_t upstream_addrlen;
} udp_server_evloop_data_t;

void udp_server_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void udp_upstream_on_message(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_APP_UDP_SERVER_HANDLE_H_
