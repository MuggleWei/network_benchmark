#include "session.h"
#include "muggle/c/base/sleep.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/net/socket_evloop_handle.h"
#include "muggle/c/net/socket_utils.h"
#include "network_benchmark/log.h"
#include <stdlib.h>

muggle_socket_context_t *nb_tcp_connect(nb_sys_args_t *args)
{
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		if (args->bind_host[0] != '\0') {
			fd = muggle_tcp_bind_connect(args->bind_host, args->bind_port,
										 args->conn_host, args->conn_port, 3);
		} else {
			fd = muggle_tcp_connect(args->conn_host, args->conn_port, 3);
		}

		if (fd == MUGGLE_INVALID_SOCKET) {
			NB_LOG_WARNING(
				"failed connect: "
				"bind_host=%s, bind_port=%s, conn_host=%s, conn_port=%s",
				args->bind_host, args->bind_port, args->conn_host,
				args->conn_port);
			muggle_msleep(3000);
		}
	} while (fd == MUGGLE_INVALID_SOCKET);

	NB_LOG_INFO("success connect: "
				"bind_host=%s, bind_port=%s, conn_host=%s, conn_port=%s",
				args->bind_host, args->bind_port, args->conn_host,
				args->conn_port);

	muggle_socket_context_t *ctx =
		(muggle_socket_context_t *)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);

	return ctx;
}

typedef struct {
	muggle_event_loop_t *evloop;
	nb_sys_args_t *args;
} tcp_async_args_t;

static muggle_thread_ret_t tcp_async_connect(void *p_args)
{
	nb_log_init_thread_ctx();

	tcp_async_args_t *p = (tcp_async_args_t *)p_args;
	muggle_socket_context_t *ctx = nb_tcp_connect(p->args);
	muggle_socket_evloop_add_ctx(p->evloop, ctx);

	nb_log_cleanup_thread_ctx();

	free(p_args);

	return 0;
}

void nb_tcp_async_connect(muggle_event_loop_t *evloop, nb_sys_args_t *args)
{
	tcp_async_args_t *p = (tcp_async_args_t *)malloc(sizeof(tcp_async_args_t));
	p->evloop = evloop;
	p->args = args;

	muggle_thread_t th;
	muggle_thread_create(&th, tcp_async_connect, p);
	muggle_thread_detach(&th);
}

muggle_socket_context_t *nb_tcp_listen(nb_sys_args_t *args)
{
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = muggle_tcp_listen(args->serv_host, args->serv_port, 512);
		if (fd == MUGGLE_INVALID_SOCKET) {
			NB_LOG_ERROR("failed listen: host=%s, port=%s", args->serv_host,
						 args->serv_port);
			muggle_msleep(3000);
		}
	} while (fd == MUGGLE_INVALID_SOCKET);

	NB_LOG_INFO("success TCP listen: host=%s, port=%s", args->serv_host,
				args->serv_port);

	muggle_socket_context_t *ctx =
		(muggle_socket_context_t *)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN);

	return ctx;
}

nb_tcp_session_t *nb_tcp_session_new(fn_nb_msg_callback cb)
{
	nb_tcp_session_t *session =
		(nb_tcp_session_t *)malloc(sizeof(nb_tcp_session_t));
	session->cb = cb;
	muggle_bytes_buffer_init(&session->bytes_buf, NET_BENCH_TCP_BUF_SIZE);
	session->msg_len_limit = 0;

	return session;
}

void nb_tcp_session_delete(nb_tcp_session_t *session)
{
	muggle_bytes_buffer_destroy(&session->bytes_buf);
	free(session);
}

void nb_tcp_session_on_msg(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	nb_tcp_session_t *session =
		(nb_tcp_session_t *)muggle_socket_ctx_get_data(ctx);
	NB_ASSERT(session != NULL);

	muggle_bytes_buffer_t *bytes_buf =
		(muggle_bytes_buffer_t *)&session->bytes_buf;
	NB_ASSERT(bytes_buf != NULL);

	// read bytes
	while (1) {
		//  find contiguous memory
		char *p = (char *)muggle_bytes_buffer_writer_fc(
			bytes_buf, NET_BENCH_TCP_RECV_UNIT_SIZE);
		if (p == NULL) {
			NB_LOG_ERROR("bytes buffer full");
			break;
		}

		int n = muggle_socket_ctx_read(ctx, p, NET_BENCH_TCP_RECV_UNIT_SIZE);
		if (n > 0) {
			if (!muggle_bytes_buffer_writer_move(bytes_buf, n)) {
				NB_ASSERT(0);
				NB_LOG_FATAL("bytes buffer inner error!");
				muggle_socket_ctx_shutdown(ctx);
				return;
			}
		}

		if (n < NET_BENCH_TCP_RECV_UNIT_SIZE) {
			break;
		}
	}

	// parse message
	nb_msg_hdr_t hdr;
	while (1) {
		if (!muggle_bytes_buffer_fetch(bytes_buf, sizeof(nb_msg_hdr_t), &hdr)) {
			// readable bytes less than sizeof(nb_msg_hdr_t)
			break;
		}

		// check message length
		uint32_t n = (uint32_t)sizeof(nb_msg_hdr_t) + hdr.payload_len;
		if (session->msg_len_limit > 0 && n > session->msg_len_limit) {
			NB_LOG_ERROR(
				"invalid message length: msg_id=%u, len=%u, msg_len_limit=%u",
				hdr.msg_id, n, session->msg_len_limit);
			muggle_socket_ctx_shutdown(ctx);
			break;
		}

		// check readable
		if ((uint32_t)muggle_bytes_buffer_readable(bytes_buf) < n) {
			break;
		}

		// handle message
		nb_msg_hdr_t *p_hdr =
			(nb_msg_hdr_t *)muggle_bytes_buffer_reader_fc(bytes_buf, (int)n);
		if (p_hdr) {
			session->cb(evloop, ctx, p_hdr, (void *)(p_hdr + 1),
						p_hdr->payload_len);

			if (!muggle_bytes_buffer_reader_move(bytes_buf, (int)n)) {
				NB_ASSERT(0);
				NB_LOG_FATAL("failed buffer reader move");
				muggle_socket_ctx_shutdown(ctx);
				break;
			}
		} else {
			void *buf = malloc(n);
			if (!muggle_bytes_buffer_read(bytes_buf, (int)n, buf)) {
				NB_ASSERT(0);
				NB_LOG_FATAL("failed buffer read");
				muggle_socket_ctx_shutdown(ctx);
				break;
			}

			p_hdr = (nb_msg_hdr_t *)buf;
			session->cb(evloop, ctx, p_hdr, (void *)(p_hdr + 1),
						p_hdr->payload_len);

			free(buf);
		}
	}
}
