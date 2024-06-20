/******************************************************************************
 *  @file         tcp_upstream_handle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-20
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark tcp upstream handle
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_APP_TCP_UPSTREAM_HANDLE_H_
#define NETWORK_BENCHMARK_APP_TCP_UPSTREAM_HANDLE_H_

#include "network_benchmark/netbench.h"

EXTERN_C_BEGIN

void tcp_upstream_on_connect(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx);
void tcp_upstream_on_close(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);
void tcp_upstream_on_release(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx);
void tcp_upstream_on_message(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx);

void tcp_upstream_handle_message(muggle_event_loop_t *evloop,
								 muggle_socket_context_t *ctx,
								 nb_msg_hdr_t *hdr, void *data,
								 uint32_t datalen);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_APP_TCP_UPSTREAM_HANDLE_H_
