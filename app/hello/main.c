#include "network_benchmark/netbench.h"

int main(int argc, char *argv[])
{
	// parse input arguments
	netbench_sys_args_t args;
	if (!netbench_parse_args(argc, argv, &args)) {
		fprintf(stderr, "failed parse input arguments\n");
		exit(EXIT_FAILURE);
	}

	if (args.log_path[0] == '\0') {
		time_t ts = time(NULL);
		struct tm t;
		localtime_r(&ts, &t);
		snprintf(args.log_path, sizeof(args.log_path),
				 "logs/hello.%d%02d%02dT%02d%02d%02d.log", t.tm_year + 1900,
				 t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	}

	netbench_args_output(&args);

	// init log
	if (!netbench_log_init(args.log_console_level, args.log_file_level,
						   args.log_path, args.log_bind_cpu)) {
		fprintf(stderr, "failed init log");
		exit(EXIT_FAILURE);
	}

	// run
	netbench_log_init_thread_ctx();

	LOG_INFO("hello world");

	netbench_log_cleanup_thread_ctx();

	return 0;
}
