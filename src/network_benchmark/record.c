#include "record.h"
#include "muggle/c/os/cpu.h"
#include "muggle/c/os/os.h"
#include "muggle/c/os/path.h"
#include "muggle/c/os/sys.h"
#include "muggle/c/sync/ma_ring.h"
#include "network_benchmark/log.h"

typedef struct {
	FILE *fp;
	fn_nb_record_callback cb;
} nb_record_context_t;

static void nb_record_default_callback(FILE *fp, unsigned long tid, void *data)
{
	MUGGLE_UNUSED(fp);
	MUGGLE_UNUSED(tid);
	MUGGLE_UNUSED(data);
}

static nb_record_context_t *nb_record_get_instance()
{
	static nb_record_context_t ctx = {
		.fp = NULL,
		.cb = nb_record_default_callback,
	};
	return &ctx;
}

static void nb_record_inner_callback(muggle_ma_ring_t *ring, void *data)
{
	nb_record_context_t *ctx = nb_record_get_instance();
	ctx->cb(ctx->fp, ring->tid, data);
}

static FILE *nb_record_open_file(const char *filepath)
{
	int ret = 0;
	char dirpath[MUGGLE_MAX_PATH];
	ret = muggle_path_dirname(filepath, dirpath, sizeof(dirpath));
	if (ret != 0) {
		NB_LOG_ERROR("failed get dirname: filepath=%s", filepath);
		return NULL;
	}

	if (!muggle_path_exists(dirpath)) {
		ret = muggle_os_mkdir(dirpath);
		if (ret != 0) {
			NB_LOG_ERROR("failed mkdir: dirpath=%s", dirpath);
			return NULL;
		}
	}

	FILE *fp = fopen(filepath, "a");
	if (fp == NULL) {
		NB_LOG_ERROR("failed open filepath: filepath=%s", filepath);
		return NULL;
	}

	return fp;
}

void nb_ts_record_callback(FILE *fp, unsigned long tid, void *data)
{
	nb_ts_record_t *r = (nb_ts_record_t *)data;
	fprintf(fp, "%lu,%llu,%lu,%u,%u,%u,%s\n", tid,
			(unsigned long long)r->ts.tv_sec, (unsigned long)r->ts.tv_nsec,
			r->user_id, r->id, r->msg_size, r->action);
	fflush(fp);
}

static int s_record_bind_cpu = -1;
static void nb_record_bind_cpu()
{
	if (s_record_bind_cpu < 0) {
		return;
	}

	muggle_pid_handle_t pid = 0;
	int ret = 0;

	muggle_cpu_mask_t mask;
	muggle_cpu_mask_zero(&mask);
	muggle_cpu_mask_set(&mask, s_record_bind_cpu);
	ret = muggle_cpu_set_thread_affinity(pid, &mask);
	if (ret != 0) {
		char errmsg[256];
		muggle_sys_strerror(ret, errmsg, sizeof(errmsg));
		fprintf(stderr, "failed record thread bind CPU: err=%s", errmsg);
	}
}

bool nb_record_init(const char *filepath, int capacity, int data_size,
					int bind_cpu, fn_nb_record_callback fn)
{
	nb_record_context_t *ctx = nb_record_get_instance();

	bool need_write_head = false;
	if (!muggle_path_exists(filepath)) {
		need_write_head = true;
	}

	ctx->fp = nb_record_open_file(filepath);
	if (ctx->fp == NULL) {
		return false;
	}

	if (need_write_head) {
		fprintf(ctx->fp, "tid,sec,nsec,uid,id,size,action\n");
		fflush(ctx->fp);
	}

	ctx->cb = fn;

	muggle_ma_ring_ctx_set_capacity(capacity);
	muggle_ma_ring_ctx_set_data_size(data_size);
	muggle_ma_ring_ctx_set_callback(nb_record_inner_callback);

	s_record_bind_cpu = bind_cpu;
	muggle_ma_ring_ctx_set_before_run_callback(nb_record_bind_cpu);

	muggle_ma_ring_backend_run();

	return true;
}

void nb_record_init_thread_ctx()
{
	muggle_ma_ring_thread_ctx_init();
}

void nb_record_cleanup_thread_ctx()
{
	muggle_ma_ring_thread_ctx_cleanup();
}

void *nb_record_prepare()
{
	muggle_ma_ring_t *ring = muggle_ma_ring_thread_ctx_get();
	return muggle_ma_ring_alloc(ring);
}

void nb_record_commit()
{
	muggle_ma_ring_t *ring = muggle_ma_ring_thread_ctx_get();
	muggle_ma_ring_move(ring);
}
