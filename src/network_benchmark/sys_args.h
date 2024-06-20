/******************************************************************************
 *  @file         sys_args.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-18
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark system arguments
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_SYSTEM_ARGUMENTS_H_
#define NETWORK_BENCHMARK_SYSTEM_ARGUMENTS_H_

#include "network_benchmark/macro.h"
#include "muggle/c/net/socket.h"
#include <stdint.h>
#include <stdbool.h>

EXTERN_C_BEGIN

enum {
	NETBENCH_CPU_FREQ_POWERSAVE = 0,
	NETBENCH_CPU_FREQ_PERFORMANCE,
};

typedef struct {
	int log_console_level;
	int log_file_level;
	char log_path[128];
	int log_bind_cpu;
	char record_path[128];
	int record_bind_cpu;
	char serv_host[MUGGLE_SOCKET_ADDR_STRLEN]; //!< server listen/bind host
	char serv_port[16]; //!< server list/bind port
	char conn_host[MUGGLE_SOCKET_ADDR_STRLEN]; //!< client connect to host
	char conn_port[16]; //!< client connect to port
	char bind_host[MUGGLE_SOCKET_ADDR_STRLEN]; //!< client bind host
	char bind_port[16]; //!< client bind port
	int cpu_freq;
	int cpu_bind;
	uint32_t user_id;
	uint32_t round_num;
	uint32_t num_per_round;
	uint32_t round_interval_ms;
	uint32_t msg_size; //!< network message size (include head and data)
} nb_sys_args_t;

/**
 * @brief parse arguments
 *
 * @param argc  input argc
 * @param argv  input argv
 * @param args  arguments
 *
 * @return 
 */
NET_BENCH_EXPORT
bool nb_parse_args(int argc, char **argv, nb_sys_args_t *args);

/**
 * @brief fillup empty fields
 *
 * @param args  arguments
 * @param name  app name
 */
NET_BENCH_EXPORT
void nb_args_fillup_empty(nb_sys_args_t *args, const char *name);

NET_BENCH_EXPORT
void nb_args_output(nb_sys_args_t *args);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_SYSTEM_ARGUMENTS_H_
