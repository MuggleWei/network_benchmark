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
#include <stdbool.h>

EXTERN_C_BEGIN

typedef struct {
	int log_console_level;
	int log_file_level;
	char log_path[128];
	int log_bind_cpu;
} netbench_sys_args_t;

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
bool netbench_parse_args(int argc, char **argv, netbench_sys_args_t *args);

NET_BENCH_EXPORT
void netbench_args_output(netbench_sys_args_t *args);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_SYSTEM_ARGUMENTS_H_
