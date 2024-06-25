#include "network_benchmark/netbench.h"

muggle_thread_ret_t consumer(void *args)
{
	muggle_channel_t *chan = (muggle_channel_t *)args;
	while (true) {
		void *p = muggle_channel_read(chan);
		if (p == NULL) {
			break;
		}

		nb_msg_hdr_t *hdr = (nb_msg_hdr_t *)p;
		nb_msg_data_t *data = (nb_msg_data_t *)(hdr + 1);
		uint32_t msg_size = sizeof(nb_msg_hdr_t) + hdr->payload_len;
		NETBENCH_RECORD(data->user_id, data->sequence, msg_size,
						"consumer.rcv");

		muggle_ts_memory_pool_free(p);
	}

	nb_record_cleanup_thread_ctx();

	return 0;
}

void producer(nb_sys_args_t *args, void *datas, muggle_channel_t *chan,
			  muggle_ts_memory_pool_t *pool)
{
	for (uint32_t round = 0; round < args->round_num; ++round) {
		for (uint32_t i = 0; i < args->num_per_round; ++i) {
			uint32_t seq = round * args->num_per_round + i;
			uint32_t offset = args->msg_size * seq;
			nb_msg_hdr_t *hdr = (nb_msg_hdr_t *)((char *)datas + offset);
			nb_msg_data_t *data = (nb_msg_data_t *)(hdr + 1);

			NB_ASSERT(data->user_id == args->user_id);
			NB_ASSERT(data->sequence == seq);

			NETBENCH_RECORD(data->user_id, data->sequence, args->msg_size,
							"producer.prepare");

			void *p = muggle_ts_memory_pool_alloc(pool);
			memcpy(p, hdr, args->msg_size);

			NETBENCH_RECORD(data->user_id, data->sequence, args->msg_size,
							"producer.push_begin");

			muggle_channel_write(chan, p);

			NETBENCH_RECORD(data->user_id, data->sequence, args->msg_size,
							"producer.push_end");
		}

		muggle_msleep(args->round_interval_ms);
	}
}

void run(nb_sys_args_t *args)
{
	// check args
	if (args->msg_size < sizeof(nb_msg_hdr_t) + sizeof(nb_msg_data_t)) {
		NB_LOG_ERROR("message size is too small (< %lu)",
					 sizeof(nb_msg_hdr_t) + sizeof(nb_msg_data_t));
		return;
	}

	// prepare datas
	void *datas =
		malloc(args->msg_size * args->num_per_round * args->round_num);
	char *p = (char *)datas;
	for (uint32_t i = 0; i < args->num_per_round * args->round_num; ++i) {
		nb_msg_hdr_t *hdr = (nb_msg_hdr_t *)p;
		memset(hdr, 0, sizeof(nb_msg_hdr_t));
		hdr->msg_id = 1;
		hdr->payload_len = args->msg_size - sizeof(nb_msg_hdr_t);

		nb_msg_data_t *data = (nb_msg_data_t *)(hdr + 1);
		data->user_id = args->user_id;
		data->sequence = i;

		p += args->msg_size;
	}

	// prepare pool and channel
	muggle_ts_memory_pool_t pool;
	muggle_ts_memory_pool_init(&pool, args->num_per_round * 16, args->msg_size);

	muggle_channel_t chan;
	int chan_flags = MUGGLE_CHANNEL_FLAG_WRITE_SPIN |
					 MUGGLE_CHANNEL_FLAG_READ_BUSY;
	muggle_channel_init(&chan, args->num_per_round * 8, chan_flags);

	// run consumer
	muggle_thread_t th;
	muggle_thread_create(&th, consumer, &chan);
	muggle_msleep(500);

	// run producer
	producer(args, datas, &chan, &pool);

	// wait consumer exit
	muggle_channel_write(&chan, NULL);
	muggle_thread_join(&th);

	// cleanup
	muggle_channel_destroy(&chan);
	muggle_ts_memory_pool_destroy(&pool);
	free(datas);
}

int main(int argc, char *argv[])
{
	// init components
	nb_sys_args_t args;
	if (!nb_init_components(argc, argv, "thread", &args)) {
		exit(EXIT_FAILURE);
	}

	run(&args);

	// cleanup thread context
	nb_record_cleanup_thread_ctx();
	nb_log_cleanup_thread_ctx();

	return 0;
}
