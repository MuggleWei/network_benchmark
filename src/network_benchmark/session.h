/******************************************************************************
 *  @file         session.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-20
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark session
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_SESSION_H_
#define NETWORK_BENCHMARK_SESSION_H_

#include "network_benchmark/macro.h"
#include "muggle/c/event/event_loop.h"
#include "muggle/c/net/socket_context.h"
#include "muggle/c/memory/bytes_buffer.h"
#include "network_benchmark/msg_struct.h"
#include "network_benchmark/sys_args.h"

EXTERN_C_BEGIN

#define NET_BENCH_TCP_RECV_UNIT_SIZE (64 * 1024)
#define NET_BENCH_TCP_BUF_SIZE (2 * 1024 * 1024)

typedef void (*fn_nb_msg_callback)(muggle_event_loop_t *evloop,
								   muggle_socket_context_t *ctx,
								   nb_msg_hdr_t *hdr, void *data,
								   uint32_t datalen);

typedef struct {
	fn_nb_msg_callback cb;
	muggle_bytes_buffer_t bytes_buf;
	uint32_t msg_len_limit; //!< limit of message length, 0 represent no limit
	uint32_t role;
} nb_tcp_session_t;

/**
 * @brief tcp connect
 *
 * @param args  system arguments
 *
 * @return  socket context
 */
NET_BENCH_EXPORT
muggle_socket_context_t *nb_tcp_connect(nb_sys_args_t *args);

/**
 * @brief tcp async connect
 *
 * @param evloop  event loop
 * @param args    system arguments
 */
NET_BENCH_EXPORT
void nb_tcp_async_connect(muggle_event_loop_t *evloop, nb_sys_args_t *args);

/**
 * @brief tcp listen
 *
 * @param args  system arguments
 *
 * @return   socket context
 */
NET_BENCH_EXPORT
muggle_socket_context_t *nb_tcp_listen(nb_sys_args_t *args);

/**
 * @brief udp bind
 *
 * @param args  system arguments
 *
 * @return   socket context
 */
NET_BENCH_EXPORT
muggle_socket_context_t *nb_udp_bind(nb_sys_args_t *args);

/**
 * @brief udp connect
 *
 * @param args  system arguments
 *
 * @return   socket context
 */
NET_BENCH_EXPORT
muggle_socket_context_t *nb_udp_connect(nb_sys_args_t *args);

/**
 * @brief udp bind and connect
 *
 * @param args  system arguments
 *
 * @return   socket context
 */
NET_BENCH_EXPORT
muggle_socket_context_t *nb_udp_bind_connect(nb_sys_args_t *args);

/**
 * @brief detect is sockaddr equal
 *
 * @param sa1  sockaddr 1
 * @param sa2  sockaddr 2
 *
 * @return boolean
 */
NET_BENCH_EXPORT
bool nb_sockaddr_equal(const struct sockaddr *sa1, const struct sockaddr *sa2);

/**
 * @brief new tcp session
 *
 * @param cb  callback
 *
 * @return tcp session
 */
NET_BENCH_EXPORT
nb_tcp_session_t *nb_tcp_session_new(fn_nb_msg_callback cb);

/**
 * @brief delete tcp session
 *
 * @param session  tcp session
 */
NET_BENCH_EXPORT
void nb_tcp_session_delete(nb_tcp_session_t *session);

NET_BENCH_EXPORT
void nb_tcp_session_on_msg(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_SESSION_H_
