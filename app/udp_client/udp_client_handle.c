#include "udp_client_handle.h"

void udp_client_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	muggle_socket_t fd = muggle_socket_ctx_get_fd(ctx);

	char local_addr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_socket_local_addr(fd, local_addr, sizeof(local_addr), 0);
	NB_LOG_INFO("session add into evloop: local_addr=%s", local_addr);

	udp_client_evloop_data_t *evloop_data =
		(udp_client_evloop_data_t *)muggle_evloop_get_data(evloop);
	evloop_data->ctx = ctx;
}
void udp_client_on_timer(muggle_event_loop_t *evloop)
{
	udp_client_evloop_data_t *evloop_data =
		(udp_client_evloop_data_t *)muggle_evloop_get_data(evloop);

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
		int num_bytes = muggle_socket_ctx_sendto(evloop_data->ctx, p, msg_size,
												 0, evloop_data->dst_sa,
												 evloop_data->dst_addrlen);
		if (num_bytes != (int)msg_size) {
			int errnum = muggle_event_lasterror();
			char errmsg[256];
			muggle_socket_strerror(errnum, errmsg, sizeof(errmsg));

			NB_LOG_ERROR(
				"failed write message: seq=%u, num_bytes=%d, err=%d[%s]",
				data->sequence, num_bytes, errnum, errmsg);
			muggle_socket_ctx_shutdown(evloop_data->ctx);
			return;
		}

		NETBENCH_RECORD(user_id, data->sequence, msg_size, "client.snd_end");
		p += msg_size;
	}

	NB_LOG_DEBUG("round[%u] completed", evloop_data->round_index);

	++evloop_data->round_index;
}
