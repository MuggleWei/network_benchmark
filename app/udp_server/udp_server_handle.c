#include "udp_server_handle.h"

void udp_server_on_add_ctx(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	muggle_socket_t fd = muggle_socket_ctx_get_fd(ctx);

	char local_addr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_socket_local_addr(fd, local_addr, sizeof(local_addr), 0);
	NB_LOG_INFO("session add into evloop: local_addr=%s", local_addr);

	udp_server_evloop_data_t *evloop_data =
		(udp_server_evloop_data_t *)muggle_evloop_get_data(evloop);
	evloop_data->ctx = ctx;
}
void udp_upstream_on_message(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	// recv message
	char buf[1024];
	struct sockaddr_storage sa;
	muggle_socklen_t addrlen = sizeof(sa);
	int num_bytes = muggle_socket_ctx_recvfrom(
		ctx, buf, sizeof(buf), 0, (struct sockaddr *)&sa, &addrlen);
	if (num_bytes <= 0) {
		NB_LOG_ERROR("failed read from socket context");
		return;
	}

	nb_msg_hdr_t *hdr = (nb_msg_hdr_t *)buf;
	NB_ASSERT(hdr->msg_id == 1);
	nb_msg_data_t *msg = (nb_msg_data_t *)(hdr + 1);
	NB_ASSERT((int)(sizeof(nb_msg_hdr_t) + hdr->payload_len) == num_bytes);
	NETBENCH_RECORD(msg->user_id, msg->sequence, num_bytes, "server.rcv");

	// send
	udp_server_evloop_data_t *evloop_data =
		(udp_server_evloop_data_t *)muggle_evloop_get_data(evloop);

	if (!nb_sockaddr_equal(evloop_data->upstream_sa, (struct sockaddr *)&sa)) {
		NETBENCH_RECORD(msg->user_id, msg->sequence, num_bytes,
						"server.snd_begin");
		int send_bytes = muggle_socket_ctx_sendto(
			evloop_data->ctx, hdr, num_bytes, 0, evloop_data->upstream_sa,
			evloop_data->upstream_addrlen);
		if (send_bytes != num_bytes) {
			NB_LOG_ERROR("failed send message: seq=%u", msg->sequence);
			return;
		}
	} else {
		// TODO:
	}
}
