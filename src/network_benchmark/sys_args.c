#include "sys_args.h"
#include "muggle/c/base/str.h"
#include "network_benchmark/log.h"
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

enum {
	OPT_VAL_LOG_CONSOLE = 1000,
	OPT_VAL_LOG_FILE,
	OPT_VAL_LOG_PATH,
	OPT_VAL_LOG_BIND_CPU,
};

bool netbench_parse_args(int argc, char **argv, netbench_sys_args_t *args)
{
	int c;

	static const char *str_usage =
		"Usage: %s <options>\n"
		"    -h, --help    show help information\n"
		"      , --log.console     log console level\n"
		"      , --log.file        log file level\n"
		"      , --log.path        log output path\n"
		"      , --log.bind_cpu    log output thread bind cpu\n"
		"";

	memset(args, 0, sizeof(*args));
	args->log_console_level = LOG_LEVEL_WARNING;
	args->log_file_level = LOG_LEVEL_DEBUG;
	strncpy(args->log_path, "", sizeof(args->log_path) - 1);
	args->log_bind_cpu = -1;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "log.console", required_argument, NULL, OPT_VAL_LOG_CONSOLE },
			{ "log.file", required_argument, NULL, OPT_VAL_LOG_FILE },
			{ "log.path", required_argument, NULL, OPT_VAL_LOG_PATH },
			{ "log.bind_cpu", required_argument, NULL, OPT_VAL_LOG_BIND_CPU },
		};

		c = getopt_long(argc, argv, "h", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
		case 'h': {
			fprintf(stdout, str_usage, argv[0]);
			exit(EXIT_SUCCESS);
		} break;
		case OPT_VAL_LOG_CONSOLE: {
			int level = netbench_log_str_to_level(optarg);
			if (level == -1) {
				fprintf(stderr, "failed convert '%s' to log level\n", optarg);
				return false;
			}
			args->log_file_level = level;
		} break;
		case OPT_VAL_LOG_FILE: {
			int level = netbench_log_str_to_level(optarg);
			if (level == -1) {
				fprintf(stderr, "failed convert '%s' to log level\n", optarg);
				return false;
			}
			args->log_file_level = level;
		} break;
		case OPT_VAL_LOG_PATH: {
			strncpy(args->log_path, optarg, sizeof(args->log_path) - 1);
		} break;
		case OPT_VAL_LOG_BIND_CPU: {
			muggle_str_toi(optarg, &args->log_bind_cpu, 10);
		} break;
		}
	}

	return true;
}

void netbench_args_output(netbench_sys_args_t *args)
{
	fprintf(stdout,
			"-------- input args --------\n"
			"log.console_level: %s\n"
			"log.file_level: %s\n"
			"log.filepath: %s\n"
			"log.bind_cpu: %d\n"
			"----------------------------\n",
			netbench_log_level_to_str(args->log_console_level),
			netbench_log_level_to_str(args->log_file_level), args->log_path,
			args->log_bind_cpu);
}
