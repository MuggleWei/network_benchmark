#include "tcp_client_handle.h"
#include "muggle/c/time/realtime_get.h"

void tcp_client_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	muggle_socket_t fd = muggle_socket_ctx_get_fd(ctx);

	char remote_addr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_socket_remote_addr(fd, remote_addr, sizeof(remote_addr), 0);
	NB_LOG_INFO("session add into evloop: remote_addr=%s", remote_addr);

	int enable = 1;
	if (muggle_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable,
						  sizeof(enable)) != 0) {
		NB_LOG_ERROR("failed set TCP NODELAY");
		muggle_socket_ctx_shutdown(ctx);
		return;
	}

	nb_tcp_session_t *session = nb_tcp_session_new(tcp_client_handle_message);
	muggle_socket_ctx_set_data(ctx, session);

	tcp_client_evloop_data_t *evloop_data =
		(tcp_client_evloop_data_t *)muggle_evloop_get_data(evloop);
	evloop_data->ctx = ctx;
}
void tcp_client_on_close(muggle_event_loop_t *evloop,
						 muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	muggle_socket_t fd = muggle_socket_ctx_get_fd(ctx);

	char remote_addr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_socket_remote_addr(fd, remote_addr, sizeof(remote_addr), 0);
	NB_LOG_WARNING("session close: remote_addr=%s", remote_addr);

	tcp_client_evloop_data_t *evloop_data =
		(tcp_client_evloop_data_t *)muggle_evloop_get_data(evloop);
	NB_ASSERT(evloop_data->ctx == ctx);
	evloop_data->ctx = NULL;

	nb_tcp_async_connect(evloop, evloop_data->args);
}
void tcp_client_on_release(muggle_event_loop_t *evloop,
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
void tcp_client_on_message(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	nb_tcp_session_t *session = muggle_socket_ctx_get_data(ctx);
	if (session) {
		nb_tcp_session_on_msg(evloop, ctx);
	}
}
void tcp_client_on_timer(muggle_event_loop_t *evloop)
{
	tcp_client_evloop_data_t *evloop_data =
		(tcp_client_evloop_data_t *)muggle_evloop_get_data(evloop);

	if (evloop_data->round_index >= evloop_data->args->round_num) {
		NB_LOG_INFO("task[msg_size=%u] completed, exit evloop",
					evloop_data->args->msg_size);
		muggle_evloop_exit(evloop);
		return;
	}

	if (evloop_data->ctx == NULL) {
		return;
	}

	struct timespec ts;
	muggle_realtime_get(ts);
	struct timespec *last_ts = &evloop_data->last_ts;
	uint32_t elapsed_ms = (ts.tv_sec - last_ts->tv_sec) * 1000 +
						  ts.tv_nsec / 1000000 - last_ts->tv_nsec / 1000000;
	if (elapsed_ms < evloop_data->args->round_interval_ms) {
		return;
	}
	evloop_data->last_ts.tv_sec = ts.tv_sec;
	evloop_data->last_ts.tv_nsec = ts.tv_nsec;

	NB_LOG_DEBUG("round[%u] start", evloop_data->round_index);

	uint32_t n = evloop_data->args->num_per_round;
	uint32_t msg_size = evloop_data->args->msg_size;
	uint32_t seq = evloop_data->round_index * n;
	uint32_t user_id = evloop_data->args->user_id;
	char *p =
		(char *)evloop_data->datas + evloop_data->round_index * n * msg_size;

	for (uint32_t i = 0; i < n; ++i) {
		nb_msg_hdr_t *hdr = (nb_msg_hdr_t *)p;
		nb_msg_data_t *data = (nb_msg_data_t *)(hdr + 1);
		NB_ASSERT(hdr->msg_id == 1);
		NB_ASSERT(data->sequence == seq);
		data->sequence = seq++;

		NETBENCH_RECORD(user_id, data->sequence, msg_size, "client.snd_begin");
		int num_bytes = muggle_socket_ctx_write(evloop_data->ctx, p, msg_size);
		NETBENCH_RECORD(user_id, data->sequence, msg_size, "client.snd_end");

		if (num_bytes != (int)msg_size) {
			NB_LOG_ERROR("failed write message: seq=%u", data->sequence);
			muggle_socket_ctx_shutdown(evloop_data->ctx);
			return;
		}
		p += msg_size;
	}

	NB_LOG_DEBUG("round[%u] completed", evloop_data->round_index);

	++evloop_data->round_index;
}

void tcp_client_handle_message(muggle_event_loop_t *evloop,
							   muggle_socket_context_t *ctx, nb_msg_hdr_t *hdr,
							   void *data, uint32_t datalen)
{
	MUGGLE_UNUSED(evloop);
	MUGGLE_UNUSED(ctx);
	MUGGLE_UNUSED(hdr);
	MUGGLE_UNUSED(data);
	MUGGLE_UNUSED(datalen);
}
