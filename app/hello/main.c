#include "network_benchmark/netbench.h"

int main(int argc, char *argv[])
{
	// init components
	nb_sys_args_t args;
	if (!nb_init_components(argc, argv, "hello", &args)) {
		exit(EXIT_FAILURE);
	}

	NB_LOG_INFO("hello world");

	for (uint32_t i = 0; i < 16; ++i) {
		nb_ts_record_t *r =
			(nb_ts_record_t *)nb_record_prepare();
		timespec_get(&r->ts, TIME_UTC);
		r->user_id = 0;
		r->id = i;
		r->msg_size = 0;
		r->action = "test";
		nb_record_commit();
	}

	// cleanup thread context
	nb_record_cleanup_thread_ctx();
	nb_log_cleanup_thread_ctx();

	return 0;
}
