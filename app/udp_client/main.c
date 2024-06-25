#include "network_benchmark/netbench.h"
#include "udp_client_handle.h"

void run(nb_sys_args_t *args)
{
	// check args
	if (args->bind_host[0] == '\0') {
		NB_LOG_ERROR("client run without bind host/port");
		return;
	}
	if (args->conn_host[0] == '\0' || args->conn_port[0] == '\0') {
		NB_LOG_ERROR("clinet run without connect host/port");
		return;
	}
	memcpy(args->serv_host, args->bind_host, sizeof(args->serv_host));
	memcpy(args->serv_port, args->bind_port, sizeof(args->serv_port));

	struct addrinfo addr_info;
	struct sockaddr_storage sa;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if (muggle_socket_getaddrinfo(args->conn_host, args->conn_port, &hints,
								  &addr_info, (struct sockaddr *)&sa) != 0) {
		NB_LOG_ERROR("failed get addrinfo: host=%s, port=%s", args->conn_host,
					 args->conn_port);
		return;
	}

	// init event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	ev_init_args.hints_max_fd = 32;
	ev_init_args.use_mem_pool = 0;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL) {
		NB_LOG_ERROR("failed new event loop");
		return;
	}
	NB_LOG_INFO("success new event loop");

	// init socket event loop handle
	muggle_socket_evloop_handle_t ev_handle;
	muggle_socket_evloop_handle_init(&ev_handle);
	muggle_socket_evloop_handle_set_cb_add_ctx(&ev_handle,
											   udp_client_on_add_ctx);
	muggle_socket_evloop_handle_set_cb_timer(&ev_handle, &udp_client_on_timer);
	if (args->cpu_freq == NETBENCH_CPU_FREQ_PERFORMANCE) {
		muggle_socket_evloop_handle_set_timer_interval(&ev_handle, 0);
	} else {
		muggle_socket_evloop_handle_set_timer_interval(&ev_handle,
													   args->round_interval_ms);
	}
	muggle_socket_evloop_handle_attach(&ev_handle, evloop);
	NB_LOG_INFO("socket handle attached event loop");

	// bind
	muggle_socket_context_t *ctx = nb_udp_bind(args);
	if (ctx == NULL) {
		NB_LOG_ERROR("failed udp_bind_connect");
		return;
	}
	muggle_socket_evloop_add_ctx(evloop, ctx);

	// evloop data
	udp_client_evloop_data_t evloop_data;
	memset(&evloop_data, 0, sizeof(evloop_data));
	evloop_data.args = args;
	evloop_data.dst_sa = (struct sockaddr *)&sa;
	evloop_data.dst_addrlen = addr_info.ai_addrlen;
	muggle_realtime_get(evloop_data.last_ts);
	evloop_data.round_index = 0;
	evloop_data.datas =
		malloc(args->msg_size * args->num_per_round * args->round_num);

	char *p = (char *)evloop_data.datas;
	for (uint32_t i = 0; i < args->num_per_round * args->round_num; ++i) {
		nb_msg_hdr_t *hdr = (nb_msg_hdr_t *)p;
		memset(hdr, 0, sizeof(nb_msg_hdr_t));
		hdr->msg_id = 1;
		hdr->payload_len = args->msg_size - sizeof(nb_msg_hdr_t);

		nb_msg_data_t *data = (nb_msg_data_t *)(hdr + 1);
		data->user_id = args->user_id;
		data->sequence = i;

		p += args->msg_size;
	}

	// run
	muggle_evloop_set_data(evloop, &evloop_data);
	muggle_evloop_run(evloop);

	// cleanup
	free(evloop_data.datas);
	muggle_socket_evloop_handle_destroy(&ev_handle);
	muggle_evloop_delete(evloop);
}

int main(int argc, char *argv[])
{
	// init components
	nb_sys_args_t args;
	if (!nb_init_components(argc, argv, "udp_client", &args)) {
		exit(EXIT_FAILURE);
	}

	if (args.msg_size == 0) {
		uint32_t msg_size_arr[] = { 16, 32, 64, 128, 256, 512, 1024 };
		for (uint32_t i = 0; i < sizeof(msg_size_arr) / sizeof(msg_size_arr[0]);
			 ++i) {
			args.msg_size = msg_size_arr[i];
			run(&args);
		}
	} else {
		run(&args);
	}

	// cleanup thread context
	nb_record_cleanup_thread_ctx();
	nb_log_cleanup_thread_ctx();

	return 0;
}
