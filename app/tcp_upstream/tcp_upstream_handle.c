#include "tcp_upstream_handle.h"

void tcp_upstream_on_connect(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	muggle_socket_t fd = muggle_socket_ctx_get_fd(ctx);

	char remote_addr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_socket_remote_addr(fd, remote_addr, sizeof(remote_addr), 0);
	NB_LOG_INFO("session connect: remote_addr=%s", remote_addr);

	int enable = 1;
	if (muggle_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable,
						  sizeof(enable)) != 0) {
		NB_LOG_ERROR("failed set TCP NODELAY");
		muggle_socket_ctx_shutdown(ctx);
		return;
	}

	nb_tcp_session_t *session = nb_tcp_session_new(tcp_upstream_handle_message);
	muggle_socket_ctx_set_data(ctx, session);
}
void tcp_upstream_on_close(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	muggle_socket_t fd = muggle_socket_ctx_get_fd(ctx);

	char remote_addr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_socket_remote_addr(fd, remote_addr, sizeof(remote_addr), 0);
	NB_LOG_WARNING("session close: remote_addr=%s", remote_addr);
}
void tcp_upstream_on_release(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	muggle_socket_t fd = muggle_socket_ctx_get_fd(ctx);

	char remote_addr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_socket_remote_addr(fd, remote_addr, sizeof(remote_addr), 0);
	NB_LOG_WARNING("session release: remote_addr=%s", remote_addr);

	nb_tcp_session_t *session = muggle_socket_ctx_get_data(ctx);
	if (session) {
		nb_tcp_session_delete(session);
	}
}
void tcp_upstream_on_message(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx)
{
	nb_tcp_session_t *session = muggle_socket_ctx_get_data(ctx);
	if (session) {
		nb_tcp_session_on_msg(evloop, ctx);
	}
}

void tcp_upstream_handle_message(muggle_event_loop_t *evloop,
								 muggle_socket_context_t *ctx,
								 nb_msg_hdr_t *hdr, void *data,
								 uint32_t datalen)
{
	MUGGLE_UNUSED(evloop);
	MUGGLE_UNUSED(ctx);

	NB_ASSERT(hdr->msg_id == 1);
	uint32_t msg_size = sizeof(nb_msg_hdr_t) + datalen;
	nb_msg_data_t *msg = (nb_msg_data_t *)data;
	NETBENCH_RECORD(msg->user_id, msg->sequence, msg_size, "upstream.rcv");
}
