/******************************************************************************
 *  @file         init.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-19
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark init
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_INIT_H_
#define NETWORK_BENCHMARK_INIT_H_

#include "network_benchmark/macro.h"
#include "network_benchmark/sys_args.h"
#include <stdbool.h>

EXTERN_C_BEGIN

/**
 * @brief network benchmark init components
 *
 * @param argc  input argc
 * @param argv  input argv
 * @param name  application name
 * @param args  arguments
 *
 * @return boolean
 */
NET_BENCH_EXPORT
bool nb_init_components(int argc, char **argv, const char *name,
						nb_sys_args_t *args);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_INIT_H_
