#include "tcp_server_handle.h"
#include "network_benchmark/session.h"

enum {
	SERVER_SESSION_ROLE_NULL,
	SERVER_SESSION_ROLE_CLIENT,
	SERVER_SESSION_ROLE_UPSTREAM,
};

void tcp_server_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	tcp_server_evloop_data_t *evloop_data =
		(tcp_server_evloop_data_t *)muggle_evloop_get_data(evloop);

	int ctx_type = muggle_socket_ctx_type(ctx);
	switch (ctx_type) {
	case MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN: {
		evloop_data->listen_ctx = ctx;
	} break;
	case MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT: {
		evloop_data->conn_ctx = ctx;

		muggle_socket_t fd = muggle_socket_ctx_get_fd(ctx);

		int enable = 1;
		if (muggle_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable,
							  sizeof(enable)) != 0) {
			NB_LOG_ERROR("failed set TCP NODELAY");
			muggle_socket_ctx_shutdown(ctx);
		}

		nb_tcp_session_t *session =
			nb_tcp_session_new(tcp_server_handle_message);
		session->role = SERVER_SESSION_ROLE_UPSTREAM;
		muggle_socket_ctx_set_data(ctx, session);
	} break;
	}
}
void tcp_server_on_connect(muggle_event_loop_t *evloop,
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

	nb_tcp_session_t *session = nb_tcp_session_new(tcp_server_handle_message);
	session->role = SERVER_SESSION_ROLE_CLIENT;
	muggle_socket_ctx_set_data(ctx, session);
}
void tcp_server_on_close(muggle_event_loop_t *evloop,
						 muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	muggle_socket_t fd = muggle_socket_ctx_get_fd(ctx);

	char remote_addr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_socket_remote_addr(fd, remote_addr, sizeof(remote_addr), 0);

	tcp_server_evloop_data_t *evloop_data =
		(tcp_server_evloop_data_t *)muggle_evloop_get_data(evloop);

	nb_tcp_session_t *session = muggle_socket_ctx_get_data(ctx);
	if (session) {
		switch (session->role) {
		case SERVER_SESSION_ROLE_UPSTREAM: {
			NB_LOG_WARNING("upstream session close: remote_addr=%s",
						   remote_addr);
			nb_tcp_async_connect(evloop, evloop_data->args);

			evloop_data->conn_ctx = NULL;
		} break;
		case SERVER_SESSION_ROLE_CLIENT: {
			NB_LOG_WARNING("client session close: remote_addr=%s", remote_addr);
		} break;
		}
	}
}
void tcp_server_on_release(muggle_event_loop_t *evloop,
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
void tcp_server_on_message(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	nb_tcp_session_t *session = muggle_socket_ctx_get_data(ctx);
	if (session) {
		nb_tcp_session_on_msg(evloop, ctx);
	}
}

void tcp_server_handle_message(muggle_event_loop_t *evloop,
							   muggle_socket_context_t *ctx, nb_msg_hdr_t *hdr,
							   void *data, uint32_t datalen)
{
	MUGGLE_UNUSED(evloop);
	MUGGLE_UNUSED(ctx);
	MUGGLE_UNUSED(datalen);

	NB_ASSERT(hdr->msg_id == 1);

	nb_tcp_session_t *session = muggle_socket_ctx_get_data(ctx);
	if (session == NULL) {
		NB_LOG_ERROR("failed get session");
		return;
	}

	switch (session->role) {
	case SERVER_SESSION_ROLE_CLIENT: {
		nb_msg_data_t *msg = (nb_msg_data_t *)data;
		NETBENCH_RECORD(msg->user_id, msg->sequence, "server.rcv");

		tcp_server_evloop_data_t *evloop_data =
			(tcp_server_evloop_data_t *)muggle_evloop_get_data(evloop);
		muggle_socket_context_t *conn_ctx = evloop_data->conn_ctx;
		if (conn_ctx == NULL) {
			NB_LOG_WARNING("upstream connection not ready yet");
			return;
		}

		NETBENCH_RECORD(msg->user_id, msg->sequence, "server.snd_begin");
		int num_bytes = muggle_socket_ctx_write(conn_ctx, hdr,
												datalen + sizeof(nb_msg_hdr_t));
		if (num_bytes != (int)(datalen + sizeof(nb_msg_hdr_t))) {
			NB_LOG_ERROR("failed write message: seq=%u", msg->sequence);
			muggle_socket_ctx_shutdown(conn_ctx);
			return;
		}
	} break;
	case SERVER_SESSION_ROLE_UPSTREAM: {
		// TODO:
	} break;
	}
}
