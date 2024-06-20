/******************************************************************************
 *  @file         tcp_server_handle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-20
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark tcp server handle
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_APP_TCP_SERVER_HANDLE_H_
#define NETWORK_BENCHMARK_APP_TCP_SERVER_HANDLE_H_

#include "network_benchmark/netbench.h"

EXTERN_C_BEGIN

typedef struct {
	nb_sys_args_t *args;
	muggle_socket_context_t *listen_ctx; //!< connect socket context
	muggle_socket_context_t *conn_ctx; //!< connect socket context
} tcp_server_evloop_data_t;

void tcp_server_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void tcp_server_on_connect(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void tcp_server_on_close(muggle_event_loop_t *evloop,
						 muggle_socket_context_t *ctx);
void tcp_server_on_release(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void tcp_server_on_message(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);

void tcp_server_handle_message(muggle_event_loop_t *evloop,
							   muggle_socket_context_t *ctx, nb_msg_hdr_t *hdr,
							   void *data, uint32_t datalen);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_APP_TCP_SERVER_HANDLE_H_
