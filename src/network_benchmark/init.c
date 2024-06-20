#include "init.h"
#include "muggle/c/net/socket.h"
#include "network_benchmark/log.h"
#include "network_benchmark/record.h"
#include <stdio.h>

bool nb_init_components(int argc, char **argv, const char *name,
						nb_sys_args_t *args)
{
	if (!nb_parse_args(argc, argv, args)) {
		fprintf(stderr, "failed parse input arguments\n");
		return false;
	}
	nb_args_fillup_empty(args, name);
	nb_args_output(args);

	// init log
	if (!nb_log_init(args->log_console_level, args->log_file_level,
					 args->log_path, args->log_bind_cpu)) {
		fprintf(stderr, "failed init log");
		return false;
	}
	nb_log_init_thread_ctx();

	// init record
	if (!nb_record_init(args->record_path, 2048, sizeof(nb_ts_record_t),
						args->record_bind_cpu, nb_ts_record_callback)) {
		NB_LOG_ERROR("failed init record");
		nb_log_cleanup_thread_ctx();
		return false;
	}

	// init socket
	if (muggle_socket_lib_init() != 0) {
		NB_LOG_ERROR("failed init socket library");
		nb_log_cleanup_thread_ctx();
		return false;
	}

	return true;
}
