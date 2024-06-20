#include "network_benchmark/netbench.h"
#include "tcp_server_handle.h"

void run(nb_sys_args_t *args)
{
	// check args
	if (args->serv_host[0] == '\0' || args->serv_port[0] == '\0') {
		NB_LOG_ERROR("server run without server host/port");
		return;
	}
	if (args->conn_host[0] == '\0' || args->conn_port[0] == '\0') {
		NB_LOG_ERROR("server run without conn host/port");
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
											   &tcp_server_on_add_ctx);
	muggle_socket_evloop_handle_set_cb_conn(&ev_handle, &tcp_server_on_connect);
	muggle_socket_evloop_handle_set_cb_close(&ev_handle, &tcp_server_on_close);
	muggle_socket_evloop_handle_set_cb_release(&ev_handle,
											   &tcp_server_on_release);
	muggle_socket_evloop_handle_set_cb_msg(&ev_handle, &tcp_server_on_message);
	if (args->cpu_freq == NETBENCH_CPU_FREQ_PERFORMANCE) {
		muggle_socket_evloop_handle_set_timer_interval(&ev_handle, 0);
	} else {
		muggle_socket_evloop_handle_set_timer_interval(&ev_handle, -1);
	}
	muggle_socket_evloop_handle_attach(&ev_handle, evloop);
	NB_LOG_INFO("socket handle attached event loop");

	// listen & connect
	muggle_socket_context_t *listen_ctx = nb_tcp_listen(args);
	muggle_socket_evloop_add_ctx(evloop, listen_ctx);

	muggle_socket_context_t *conn_ctx = nb_tcp_connect(args);
	muggle_socket_evloop_add_ctx(evloop, conn_ctx);

	// evloop data
	tcp_server_evloop_data_t evloop_data;
	memset(&evloop_data, 0, sizeof(evloop_data));
	evloop_data.args = args;

	// run
	muggle_evloop_set_data(evloop, &evloop_data);
	muggle_evloop_run(evloop);

	// cleanup
	muggle_socket_evloop_handle_destroy(&ev_handle);
	muggle_evloop_delete(evloop);
}

int main(int argc, char *argv[])
{
	// init components
	nb_sys_args_t args;
	if (!nb_init_components(argc, argv, "tcp_server", &args)) {
		exit(EXIT_FAILURE);
	}

	run(&args);

	// cleanup thread context
	nb_record_cleanup_thread_ctx();
	nb_log_cleanup_thread_ctx();

	return 0;
}
