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
	OPT_VAL_RECORD_PATH,
	OPT_VAL_RECORD_BIND_CPU,
	OPT_VAL_SERVER_HOST,
	OPT_VAL_SERVER_PORT,
	OPT_VAL_CONN_HOST,
	OPT_VAL_CONN_PORT,
	OPT_VAL_BIND_HOST,
	OPT_VAL_BIND_PORT,
	OPT_VAL_CPU_FREQ,
	OPT_VAL_CPU_BIND,
	OPT_VAL_USER_ID,
	OPT_VAL_ROUND,
	OPT_VAL_NUM_PER_ROUND,
	OPT_VAL_ROUND_INTERVAL,
	OPT_VAL_MSG_SIZE,
};

bool nb_parse_args(int argc, char **argv, nb_sys_args_t *args)
{
	int c;

	static const char *str_usage =
		"Usage: %s <options>\n"
		"    -h, --help            show help information\n"
		"      , --log.console     log console level\n"
		"      , --log.file        log file level\n"
		"      , --log.path        log output path\n"
		"      , --log.bind_cpu    log output thread bind cpu\n"
		"      , --record.path     record filepath\n"
		"      , --record.bind_cpu record output thread bind cpu\n"
		"      , --server.host     server host\n"
		"      , --server.port     server port\n"
		"      , --conn.host       conn host\n"
		"      , --conn.port       conn port\n"
		"      , --bind.host       bind host\n"
		"      , --bind.port       bind port\n"
		"      , --cpu.freq        cpu freq: powersave | performance\n"
		"      , --cpu.bind        cpu bind\n"
		"      , --user.id         user id\n"
		"      , --round.num       total round number\n"
		"      , --round.num_per   number order per round\n"
		"      , --round.interval  interval between round (ms)\n"
		"      , --msg.size        network message size (include custom head & data)\n"
		"";

	memset(args, 0, sizeof(*args));
	args->log_console_level = NB_LOG_LEVEL_INFO;
	args->log_file_level = NB_LOG_LEVEL_DEBUG;
	strncpy(args->log_path, "", sizeof(args->log_path) - 1);
	args->log_bind_cpu = -1;
	strncpy(args->record_path, "", sizeof(args->record_path) - 1);
	args->record_bind_cpu = -1;
	strncpy(args->serv_host, "", sizeof(args->serv_host) - 1);
	strncpy(args->serv_port, "", sizeof(args->serv_port) - 1);
	strncpy(args->conn_host, "", sizeof(args->conn_host) - 1);
	strncpy(args->conn_port, "", sizeof(args->conn_port) - 1);
	strncpy(args->bind_host, "", sizeof(args->bind_host) - 1);
	strncpy(args->bind_port, "", sizeof(args->bind_port) - 1);
	args->cpu_freq = NETBENCH_CPU_FREQ_POWERSAVE;
	args->cpu_bind = -1;
	args->user_id = 1;
	args->round_num = 1000;
	args->num_per_round = 1;
	args->round_interval_ms = 1;
	args->msg_size = 16;

	static struct option long_options[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "log.console", required_argument, NULL, OPT_VAL_LOG_CONSOLE },
		{ "log.file", required_argument, NULL, OPT_VAL_LOG_FILE },
		{ "log.path", required_argument, NULL, OPT_VAL_LOG_PATH },
		{ "log.bind_cpu", required_argument, NULL, OPT_VAL_LOG_BIND_CPU },
		{ "record.path", required_argument, NULL, OPT_VAL_RECORD_PATH },
		{ "record.bind_cpu", required_argument, NULL, OPT_VAL_RECORD_BIND_CPU },
		{ "server.host", required_argument, NULL, OPT_VAL_SERVER_HOST },
		{ "server.port", required_argument, NULL, OPT_VAL_SERVER_PORT },
		{ "conn.host", required_argument, NULL, OPT_VAL_CONN_HOST },
		{ "conn.port", required_argument, NULL, OPT_VAL_CONN_PORT },
		{ "bind.host", required_argument, NULL, OPT_VAL_BIND_HOST },
		{ "bind.port", required_argument, NULL, OPT_VAL_BIND_PORT },
		{ "cpu.freq", required_argument, NULL, OPT_VAL_CPU_FREQ },
		{ "cpu.bind", required_argument, NULL, OPT_VAL_CPU_BIND },
		{ "user.id", required_argument, NULL, OPT_VAL_USER_ID },
		{ "round.num", required_argument, NULL, OPT_VAL_ROUND },
		{ "round.num_per", required_argument, NULL, OPT_VAL_NUM_PER_ROUND },
		{ "round.interval", required_argument, NULL, OPT_VAL_ROUND_INTERVAL },
		{ "msg.size", required_argument, NULL, OPT_VAL_MSG_SIZE },
	};

	while (1) {
		int option_index = 0;

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
			int level = nb_log_str_to_level(optarg);
			if (level == -1) {
				fprintf(stderr, "failed convert '%s' to log level\n", optarg);
				return false;
			}
			args->log_console_level = level;
		} break;
		case OPT_VAL_LOG_FILE: {
			int level = nb_log_str_to_level(optarg);
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
		case OPT_VAL_RECORD_PATH: {
			strncpy(args->record_path, optarg, sizeof(args->record_path) - 1);
		} break;
		case OPT_VAL_RECORD_BIND_CPU: {
			muggle_str_toi(optarg, &args->record_bind_cpu, 10);
		} break;
		case OPT_VAL_SERVER_HOST: {
			strncpy(args->serv_host, optarg, sizeof(args->serv_host) - 1);
		} break;
		case OPT_VAL_SERVER_PORT: {
			strncpy(args->serv_port, optarg, sizeof(args->serv_port) - 1);
		} break;
		case OPT_VAL_CONN_HOST: {
			strncpy(args->conn_host, optarg, sizeof(args->conn_host) - 1);
		} break;
		case OPT_VAL_CONN_PORT: {
			strncpy(args->conn_port, optarg, sizeof(args->conn_port) - 1);
		} break;
		case OPT_VAL_BIND_HOST: {
			strncpy(args->bind_host, optarg, sizeof(args->bind_host) - 1);
		} break;
		case OPT_VAL_BIND_PORT: {
			strncpy(args->bind_port, optarg, sizeof(args->bind_port) - 1);
		} break;
		case OPT_VAL_CPU_FREQ: {
			if (strcmp(optarg, "powersave") == 0) {
				args->cpu_freq = NETBENCH_CPU_FREQ_POWERSAVE;
			} else if (strcmp(optarg, "performance") == 0) {
				args->cpu_freq = NETBENCH_CPU_FREQ_PERFORMANCE;
			} else {
				fprintf(stderr, "invalid cpu freq: cpu_freq=%s", optarg);
				return false;
			}
		} break;
		case OPT_VAL_CPU_BIND: {
			muggle_str_toi(optarg, &args->cpu_bind, 10);
		} break;
		case OPT_VAL_USER_ID: {
			muggle_str_tou(optarg, &args->user_id, 10);
		} break;
		case OPT_VAL_ROUND: {
			muggle_str_tou(optarg, &args->round_num, 10);
		} break;
		case OPT_VAL_NUM_PER_ROUND: {
			muggle_str_tou(optarg, &args->num_per_round, 10);
		} break;
		case OPT_VAL_ROUND_INTERVAL: {
			muggle_str_tou(optarg, &args->round_interval_ms, 10);
		} break;
		case OPT_VAL_MSG_SIZE: {
			muggle_str_tou(optarg, &args->msg_size, 10);
		} break;
		}
	}

	return true;
}

void nb_args_fillup_empty(nb_sys_args_t *args, const char *name)
{
	time_t ts = time(NULL);
	struct tm t;
	localtime_r(&ts, &t);

	if (args->log_path[0] == '\0') {
		snprintf(args->log_path, sizeof(args->log_path),
				 "logs/%s.%d%02d%02dT%02d%02d%02d.log", name, t.tm_year + 1900,
				 t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	}
	if (args->record_path[0] == '\0') {
		snprintf(args->record_path, sizeof(args->record_path),
				 "records/%s.%d%02d%02dT%02d%02d%02d.csv", name,
				 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
				 t.tm_sec);
	}
}

void nb_args_output(nb_sys_args_t *args)
{
	fprintf(stdout,
			"-------- input args --------\n"
			"log.console_level: %s\n"
			"log.file_level: %s\n"
			"log.path: %s\n"
			"log.bind_cpu: %d\n"
			"record.path: %s\n"
			"record.bind_cpu: %d\n"
			"server.host: %s\n"
			"server.port: %s\n"
			"conn.host: %s\n"
			"conn.port: %s\n"
			"bind.host: %s\n"
			"bind.port: %s\n"
			"cpu.freq: %s\n"
			"cpu.bind: %d\n"
			"user.id: %u\n"
			"round.num: %u\n"
			"round.num_per: %u\n"
			"round.interval: %u (ms)\n"
			"msg.size: %u\n"
			"----------------------------\n",
			nb_log_level_to_str(args->log_console_level),
			nb_log_level_to_str(args->log_file_level), args->log_path,
			args->log_bind_cpu, args->record_path, args->record_bind_cpu,
			args->serv_host, args->serv_port, args->conn_host, args->conn_port,
			args->bind_host, args->bind_port,
			args->cpu_freq == NETBENCH_CPU_FREQ_POWERSAVE ? "powersave" :
															"performance",
			args->cpu_bind, args->user_id, args->round_num, args->num_per_round,
			args->round_interval_ms, args->msg_size);
}
