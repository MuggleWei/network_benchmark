import logging
import os
import sys

from latency_handle import LatencyHandle

if __name__ == "__main__":
    # init log
    LatencyHandle.init_log()

    # parse input arguments
    input, output = LatencyHandle.parse_args()
    if len(input) == 0:
        logging.error("without input directory")
        sys.exit(1)

    # load record datas
    records = os.listdir(input)
    record_list_dict = {}  # k: msg_size, v: list[record]
    for record_file in records:
        if record_file.startswith("report_"):
            continue
        LatencyHandle.load_record_list_dict(record_list_dict, os.path.join(input, record_file))

    # prepare output dir
    if os.path.exists(output):
        os.makedirs(output, exist_ok=True)

    # calculate latency and dump to file
    target = {
        "report_send.csv": {
            "file": None,
            "range": ["client.snd_begin", "client.snd_end"],
        },
        "report_throughout.csv": {
            "file": None,
            "range": ["client.snd_begin", "upstream.rcv"],
        },
    }

    for msg_size, records in record_list_dict.items():
        logging.info("handle records: size={}".format(msg_size))

        record_data_dict = {}  # k: user_id+sequence, v: dict<action, [int(sec), int(nsec)]>
        for row in records:
            k = "{}_{}".format(row["uid"], row["id"])
            if k not in record_data_dict:
                record_data_dict[k] = {}
            record_data_dict[k][row["action"]] = [int(row["sec"]), int(row["nsec"])]

        for filename, v in target.items():
            if v["file"] is None:
                v["file"] = LatencyHandle.open_file(os.path.join(output, filename))

            latency_list = LatencyHandle.cal_latency(record_data_dict, v["range"][0], v["range"][1])
            LatencyHandle.dump_statistics_result(v["file"], msg_size, latency_list)

    for v in target.values():
        if v["file"] is not None:
            v["file"].close()
