import csv
import getopt
import logging
import statistics
import sys


class LatencyHandle(object):
    @classmethod
    def parse_args(cls):
        """
        parse input arguments
        :return: input record files list
        """
        input = ""
        output = ""
        opts, _ = getopt.getopt(sys.argv[1:], "hi:o:", ["help", "input", "output"])
        for opt, arg in opts:
            if opt in ("-h", "--help"):
                print("{} <options>".format(sys.argv[0]))
                print("  -i, --input    input record directory")
                print("  -o, --output   output directory")
                sys.exit(0)
            elif opt in ("-i", "--input"):
                input = arg
            elif opt in ("-o", "--output"):
                output = arg

        if len(output) == 0:
            output = input

        return input, output

    @classmethod
    def init_log(cls):
        """
        init log
        :return:
        """
        handler = logging.StreamHandler()
        handler.setLevel(logging.DEBUG)
        handler.setFormatter(
            logging.Formatter("%(asctime)s|%(name)s|%(levelname)s|%(filename)s:%(lineno)s - %(message)s"))
        logger = logging.getLogger()
        logger.setLevel(logging.DEBUG)
        logger.addHandler(handler)

    @classmethod
    def load_record_list_dict(cls, record_list_dict, record_file):
        """
        load record into list dict
        :param record_list_dict: dict[msg_size, list[record]]
        :param record_file:
        :return:
        """
        with open(record_file, "r") as f:
            reader = csv.DictReader(f)
            for row in reader:
                k = int(row["size"])
                if k not in record_list_dict:
                    record_list_dict[k] = []
                record_list_dict[k].append(row)
        return record_list_dict

    @classmethod
    def cal_latency(cls, record_data_dict, begin_action, end_action):
        """
        calculate throughout latency
        :param record_data_dict:
        :param begin_action:
        :param end_action:
        :return:
        """
        latency_list = []
        for k, v in record_data_dict.items():
            if begin_action not in v:
                logging.warning("failed find '{}' in {}".format(begin_action, k))
                continue
            if end_action not in v:
                logging.warning("failed find '{}' in {}".format(end_action, k))
                continue
            end_sec, end_nsec = v[end_action]
            begin_sec, begin_nsec = v[begin_action]
            elapsed = (end_sec - begin_sec) * 1000000000 + end_nsec - begin_nsec
            latency_list.append(elapsed)
        return latency_list

    @classmethod
    def open_file(cls, filepath):
        """
        open output file path
        :param filepath: 
        :return: 
        """
        f = open(filepath, "w")
        head = []
        head.extend(["size", "iter", "mean", "stdev"])
        for p in range(10):
            head.append("quantile_{}".format(int(p * 10)))
        head.append("quantile_{}".format(100))
        f.write(",".join(head))
        f.write("\n")
        return f

    @classmethod
    def dump_statistics_result(cls, f, msg_size, latency_list):
        """
        calculate statistics result and dump to file
        :param f:
        :param msg_size:
        :param latency_list:
        :return:
        """
        quantiles = []

        latency_list.sort()
        n = len(latency_list)
        for p in range(10):
            quantile_k = p * 10
            idx = quantile_k * (n / 100)
            i = int(round(idx))
            quantiles.append(latency_list[i])
        quantiles.append(latency_list[-1])

        result = [msg_size, n, int(statistics.mean(latency_list)), int(statistics.pstdev(latency_list))]
        result.extend(quantiles)
        f.write(",".join(["{}".format(r) for r in result]))
        f.write("\n")
