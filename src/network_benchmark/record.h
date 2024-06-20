/******************************************************************************
 *  @file         record.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-19
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark record
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_RECORD_H_
#define NETWORK_BENCHMARK_RECORD_H_

#include "network_benchmark/macro.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

EXTERN_C_BEGIN

typedef void (*fn_nb_record_callback)(FILE *fp, unsigned long tid, void *data);

typedef struct {
	struct timespec ts;
	uint32_t user_id;
	uint32_t id;
	const char *action;
} nb_ts_record_t;

#define NETBENCH_RECORD(uid, seq, str_action)                      \
	{                                                              \
		nb_ts_record_t *r = (nb_ts_record_t *)nb_record_prepare(); \
		timespec_get(&r->ts, TIME_UTC);                            \
		r->user_id = uid;                                          \
		r->id = seq;                                               \
		r->action = str_action;                                    \
		nb_record_commit();                                        \
	}

/**
 * @brief timestamp record callback
 *
 * @param fp    FILE*
 * @param tid   thread id
 * @param data  data
 */
NET_BENCH_EXPORT
void nb_ts_record_callback(FILE *fp, unsigned long tid, void *data);

/**
 * @brief init record
 *
 * @param filepath   output filepath
 * @param capacity   capacity of record's ring
 * @param data_size  data size
 * @param bind_cpu   bind cpu
 * @param fn         record callback
 */
NET_BENCH_EXPORT
bool nb_record_init(const char *filepath, int capacity, int data_size,
					int bind_cpu, fn_nb_record_callback fn);

/**
 * @brief initialize record thread context
 */
NET_BENCH_EXPORT
void nb_record_init_thread_ctx();

/**
 * @brief lceanup record thread context
 */
NET_BENCH_EXPORT
void nb_record_cleanup_thread_ctx();

/**
 * @brief fetch data
 *
 * @return data
 */
NET_BENCH_EXPORT
void *nb_record_prepare();

/**
 * @brief commit data
 */
NET_BENCH_EXPORT
void nb_record_commit();

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_RECORD_H_
