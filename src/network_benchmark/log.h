/******************************************************************************
 *  @file         log.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-18
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark log
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_LOG_H_
#define NETWORK_BENCHMARK_LOG_H_

#include "network_benchmark/macro.h"
#include <stdbool.h>

EXTERN_C_BEGIN

#include "haclog/haclog.h"

// use haclog
#define LOG_LEVEL_TRACE HACLOG_LEVEL_TRACE
#define LOG_LEVEL_DEBUG HACLOG_LEVEL_DEBUG
#define LOG_LEVEL_INFO HACLOG_LEVEL_INFO
#define LOG_LEVEL_WARNING HACLOG_LEVEL_WARNING
#define LOG_LEVEL_ERROR HACLOG_LEVEL_ERROR
#define LOG_LEVEL_FATAL HACLOG_LEVEL_FATAL

#define LOG_TRACE(format, ...) HACLOG_TRACE(format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) HACLOG_DEBUG(format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) HACLOG_INFO(format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) HACLOG_WARNING(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) HACLOG_ERROR(format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) HACLOG_FATAL(format, ##__VA_ARGS__)

#define LOG_ASSERT(x) HACLOG_ASSERT(x)
#define LOG_ASSERT_MSG(x, format, ...) \
	HACLOG_ASSERT_MSG(x, format, ##__VA_ARGS__)

/**
 * @brief init log
 *
 * @param console_level  console output level; if it's -1, disable console log
 * @param file_level     file output level; if it's -1, disable file log
 * @param filepath       output filepath
 * @param log_bind_cpu   log output thread bind cpu
 *
 * @return boolean
 */
NET_BENCH_EXPORT
bool netbench_log_init(int console_level, int file_level, const char *filepath,
					   int log_bind_cpu);

/**
 * @brief initialize log thread context
 */
NET_BENCH_EXPORT
void netbench_log_init_thread_ctx();

/**
 * @brief cleanup log thread context
 */
NET_BENCH_EXPORT
void netbench_log_cleanup_thread_ctx();

/**
 * @brief convert log string to level
 *
 * @param s  log string
 *
 * @return log level, when error, return -1
 */
NET_BENCH_EXPORT
int netbench_log_str_to_level(const char *s);

/**
 * @brief convert log level to string
 *
 * @param level  log level
 *
 * @return string log level
 */
NET_BENCH_EXPORT
const char *netbench_log_level_to_str(int level);

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_LOG_H_
