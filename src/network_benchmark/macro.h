/******************************************************************************
 *  @file         macro.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-06-18
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        network benchmark macro
 *****************************************************************************/

#ifndef NETWORK_BENCHMARK_MACRO_H_
#define NETWORK_BENCHMARK_MACRO_H_

#include "muggle/c/base/macro.h"
#include "network_benchmark/config.h"
#include <assert.h>

EXTERN_C_BEGIN

// dll export
#if MUGGLE_PLATFORM_WINDOWS && defined(NET_BENCH_USE_DLL)
	#ifdef NET_BENCH_EXPORTS
		#define NET_BENCH_EXPORT __declspec(dllexport)
	#else
		#define NET_BENCH_EXPORT __declspec(dllimport)
	#endif
#else
	#define NET_BENCH_EXPORT
#endif

// check message struct
#define NET_BENCH_STRICT_CHECK_MSG_STRUCT(msg)       \
	static_assert(sizeof(msg) % sizeof(void *) == 0, \
				  "message not align sizeof(ptr)");

EXTERN_C_END

#endif // !NETWORK_BENCHMARK_MACRO_H_
