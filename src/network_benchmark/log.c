#include "log.h"
#include "muggle/c/os/cpu.h"
#include "muggle/c/os/sys.h"
#include <string.h>
#include <stdlib.h>

static bool nb_log_add_console_handler(int level)
{
	static haclog_console_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_console_handler_init(&handler, 1) != 0) {
		fprintf(stderr, "failed init console handler");
		return false;
	}

	haclog_handler_set_level((haclog_handler_t *)&handler, level);
	haclog_context_add_handler((haclog_handler_t *)&handler);

	return true;
}

static bool nb_log_add_file_handle(int level, const char *filepath)
{
	static haclog_file_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	if (haclog_file_handler_init(&handler, filepath, "a") != 0) {
		fprintf(stderr, "failed init file handler");
		return false;
	}
	haclog_handler_set_level((haclog_handler_t *)&handler, level);
	haclog_context_add_handler((haclog_handler_t *)&handler);
	return true;
}

static int s_log_bind_cpu = -1;
static void nb_log_bind_cpu()
{
	if (s_log_bind_cpu < 0) {
		return;
	}

	muggle_pid_handle_t pid = 0;
	int ret = 0;

	muggle_cpu_mask_t mask;
	muggle_cpu_mask_zero(&mask);
	muggle_cpu_mask_set(&mask, s_log_bind_cpu);
	ret = muggle_cpu_set_thread_affinity(pid, &mask);
	if (ret != 0) {
		char errmsg[256];
		muggle_sys_strerror(ret, errmsg, sizeof(errmsg));
		fprintf(stderr, "failed log thread bind CPU: err=%s", errmsg);
	}
}

bool nb_log_init(int console_level, int file_level, const char *filepath,
					   int log_bind_cpu)
{
	if (console_level >= 0) {
		if (!nb_log_add_console_handler(console_level)) {
			fprintf(stderr, "failed add console log handler");
			return false;
		}
	}

	if (file_level >= 0) {
		if (!nb_log_add_file_handle(file_level, filepath)) {
			fprintf(stderr, "failed add file log handler");
			return false;
		}
	}

	s_log_bind_cpu = log_bind_cpu;
	haclog_context_set_before_run_cb(nb_log_bind_cpu);

	haclog_backend_run();

	return true;
}

void nb_log_init_thread_ctx()
{
	haclog_thread_context_init();
}

void nb_log_cleanup_thread_ctx()
{
	haclog_thread_context_cleanup();
}

int nb_log_str_to_level(const char *s)
{
	if (strcmp(s, "TRACE") == 0 || strcmp(s, "trace") == 0) {
		return NB_LOG_LEVEL_TRACE;
	}
	if (strcmp(s, "DEBUG") == 0 || strcmp(s, "debug") == 0) {
		return NB_LOG_LEVEL_DEBUG;
	}
	if (strcmp(s, "INFO") == 0 || strcmp(s, "info") == 0) {
		return NB_LOG_LEVEL_INFO;
	}
	if (strcmp(s, "WARNING") == 0 || strcmp(s, "warning") == 0 ||
		strcmp(s, "WARN") == 0 || strcmp(s, "warn") == 0) {
		return NB_LOG_LEVEL_WARNING;
	}
	if (strcmp(s, "ERROR") == 0 || strcmp(s, "error") == 0) {
		return NB_LOG_LEVEL_ERROR;
	}
	if (strcmp(s, "FATAL") == 0 || strcmp(s, "fatal") == 0) {
		return NB_LOG_LEVEL_FATAL;
	}
	return -1;
}

const char *nb_log_level_to_str(int level)
{
	switch (level) {
	case NB_LOG_LEVEL_TRACE: {
		return "TRACE";
	} break;
	case NB_LOG_LEVEL_DEBUG: {
		return "DEBUG";
	} break;
	case NB_LOG_LEVEL_INFO: {
		return "INFO";
	} break;
	case NB_LOG_LEVEL_WARNING: {
		return "WARNING";
	} break;
	case NB_LOG_LEVEL_ERROR: {
		return "ERROR";
	} break;
	case NB_LOG_LEVEL_FATAL: {
		return "FATAL";
	} break;
	default: {
		return "(unknown)";
	} break;
	}
}
