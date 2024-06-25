/******************************************************************************
 *  @file         udp_upstream_handle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-24
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark udp upstream handle
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_APP_UDP_UPSTREAM_HANDLE_H_
#define NETWORK_BENCHMARK_APP_UDP_UPSTREAM_HANDLE_H_

#include "network_benchmark/netbench.h"

EXTERN_C_BEGIN

void udp_upstream_on_message(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_APP_UDP_UPSTREAM_HANDLE_H_
