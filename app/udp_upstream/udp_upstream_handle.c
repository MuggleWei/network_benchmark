#include "udp_upstream_handle.h"
#include "network_benchmark/log.h"

void udp_upstream_on_message(muggle_event_loop_t *evloop,
							 muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	char buf[1024];
	int num_bytes = muggle_socket_ctx_read(ctx, buf, sizeof(buf));
	if (num_bytes <= 0) {
		NB_LOG_ERROR("failed read from socket context");
		return;
	}

	nb_msg_hdr_t *hdr = (nb_msg_hdr_t *)buf;
	NB_ASSERT(hdr->msg_id == 1);
	nb_msg_data_t *msg = (nb_msg_data_t *)(hdr + 1);
	NB_ASSERT((int)(sizeof(nb_msg_hdr_t) + hdr->payload_len) == num_bytes);
	NETBENCH_RECORD(msg->user_id, msg->sequence, num_bytes, "upstream.rcv");
}
