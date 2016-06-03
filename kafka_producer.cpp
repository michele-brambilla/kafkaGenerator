/*
 * librdkafka - Apache Kafka C library
 *
 * Copyright (c) 2014, Magnus Edenhill
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Apache Kafka consumer & producer example programs
 * using the Kafka driver from librdkafka
 * (https://github.com/edenhill/librdkafka)
 */

#include <iostream>
#include <random>

#include <string>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <csignal>
#include <cstring>

#include <getopt.h>
#include <librdkafka/rdkafkacpp.h>


static bool run = true;
static bool exit_eof = false;

static void sigterm (int sig) {
  run = false;
}


class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
 public:
  void dr_cb (RdKafka::Message &message) {
    std::cout << "Message delivery for (" << message.len() << " bytes): " <<
        message.errstr() << std::endl;
    if (message.key())
      std::cout << "Key: " << *(message.key()) << ";" << std::endl;
  }
};


class ExampleEventCb : public RdKafka::EventCb {
 public:
  void event_cb (RdKafka::Event &event) {
    switch (event.type())
    {
      case RdKafka::Event::EVENT_ERROR:
        std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
            event.str() << std::endl;
        if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
          run = false;
        break;

      case RdKafka::Event::EVENT_STATS:
        std::cerr << "\"STATS\": " << event.str() << std::endl;
        break;

      case RdKafka::Event::EVENT_LOG:
        fprintf(stderr, "LOG-%i-%s: %s\n",
                event.severity(), event.fac().c_str(), event.str().c_str());
        break;

      default:
        std::cerr << "EVENT " << event.type() <<
            " (" << RdKafka::err2str(event.err()) << "): " <<
            event.str() << std::endl;
        break;
    }
  }
};


/* Use of this partitioner is pretty pointless since no key is provided
 * in the produce() call. */
class MyHashPartitionerCb : public RdKafka::PartitionerCb {
 public:
  int32_t partitioner_cb (const RdKafka::Topic *topic, const std::string *key,
                          int32_t partition_cnt, void *msg_opaque) {
    return djb_hash(key->c_str(), key->size()) % partition_cnt;
  }
 private:

  static inline unsigned int djb_hash (const char *str, size_t len) {
    unsigned int hash = 5381;
    for (size_t i = 0 ; i < len ; i++)
      hash = ((hash << 5) + hash) + str[i];
    return hash;
  }
};

void usage(RdKafka::Conf *conf,char **argv) {
  std::string features;
  conf->get("builtin.features", features);

  std::cout << "Usage: ./producer -t <topic> "
            << "[-p <partition>] [-b <host1:port1,host2:port2,..>]\n"
            << "\n"
            << " Options:\n"
            << "  -t <topic>      Topic to fetch / produce\n"
            << "  -p <num>        Partition (random partitioner)\n"
            << "  -p <func>       Use partitioner:\n"
            << "                  random (default), hash\n"
            << "  -b <brokers>    Broker address (localhost:9092)\n"
            << "  -z <codec>      Enable compression:\n"
            << "                  none|gzip|snappy\n"
            << "  -o <offset>     Start offset (consumer)\n"
            << "  -e              Exit consumer when last message\n"
            << "                  in partition has been received.\n"
            << "  -d [facs..]     Enable debugging contexts:\n"
            << "                  %s\n"
            << "  -M <intervalms> Enable statistics\n"
            << "  -X <prop=name>  Set arbitrary librdkafka "
            << "configuration property\n"
            << "                  Properties prefixed with \"topic.\" "
            << "will be set on topic object.\n"
            << "                  Use '-X list' to see the full list\n"
            << "                  of supported properties.\n"
            << "  -f <flag>       Set option:\n"
            << "                     ccb - use consume_callback\n"
            << "\n"
            << " In Producer mode:\n"
            << "  reads messages from stdin and sends to broker\n"
            << "\n"
            << "\n"
            << "\n"
            << std::endl;
exit(1);
}



int main (int argc, char **argv) {
  std::string brokers = "localhost";
  std::string errstr;
  std::string topic_str;
  std::string mode;
  std::string debug;
  int32_t partition = RdKafka::Topic::PARTITION_UA;
  int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;
  bool do_conf_dump = false;
  int opt;
  MyHashPartitionerCb hash_partitioner;
  int use_ccb = 0;

  /*
   * Create configuration objects
   */
  RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

  while ((opt = getopt(argc, argv, "PCLt:p:b:z:qd:o:eX:AM:f:")) != -1) {
    switch (opt) {
    case 't':
      topic_str = optarg;
      break;
    case 'p':
      if (!strcmp(optarg, "random"))
        /* default */;
      else if (!strcmp(optarg, "hash")) {
        if (tconf->set("partitioner_cb", &hash_partitioner, errstr) !=
            RdKafka::Conf::CONF_OK) {
          std::cerr << errstr << std::endl;
          exit(1);
        }
      } else
        partition = std::atoi(optarg);
      break;
    case 'b':
      brokers = optarg;
      break;
    case 'o':
      if (!strcmp(optarg, "end"))
	start_offset = RdKafka::Topic::OFFSET_END;
      else if (!strcmp(optarg, "beginning"))
	start_offset = RdKafka::Topic::OFFSET_BEGINNING;
      else if (!strcmp(optarg, "stored"))
	start_offset = RdKafka::Topic::OFFSET_STORED;
      else
	start_offset = strtoll(optarg, NULL, 10);
      break;
    case 'd':
      debug = optarg;
      break;
    case 'M':
      if (conf->set("statistics.interval.ms", optarg, errstr) !=
          RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
      }
      break;
    default:
      usage(conf, argv);
    }
  }


  /*
   * Set configuration properties
   */
  conf->set("metadata.broker.list", brokers, errstr);

  if (!debug.empty()) {
    if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
      std::cerr << errstr << std::endl;
      exit(1);
    }
  }

  // ExampleEventCb ex_event_cb;
  // conf->set("event_cb", &ex_event_cb, errstr);

  // if (do_conf_dump) {
  //   int pass;

  //   for (pass = 0 ; pass < 2 ; pass++) {
  //     std::list<std::string> *dump;
  //     if (pass == 0) {
  //       dump = conf->dump();
  //       std::cout << "# Global config" << std::endl;
  //     } else {
  //       dump = tconf->dump();
  //       std::cout << "# Topic config" << std::endl;
  //     }

  //     for (std::list<std::string>::iterator it = dump->begin();
  //          it != dump->end(); ) {
  //       std::cout << *it << " = ";
  //       it++;
  //       std::cout << *it << std::endl;
  //       it++;
  //     }
  //     std::cout << std::endl;
  //   }
  //   exit(0);
  // }

  signal(SIGINT, sigterm);
  signal(SIGTERM, sigterm);


  // if (mode == "P") {
    /*
     * Producer mode
     */

  if(topic_str.empty())
    usage(conf, argv);
  
  // ExampleDeliveryReportCb ex_dr_cb;
  
  // /* Set delivery report callback */
  // conf->set("dr_cb", &ex_dr_cb, errstr);
  
  /*
   * Create producer using accumulated global configuration.
   */
  RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
  if (!producer) {
    std::cerr << "Failed to create producer: " << errstr << std::endl;
    exit(1);
  }

  std::cout << "% Created producer " << producer->name() << std::endl;

  /*
   * Create topic handle.
   */
  RdKafka::Topic *topic = RdKafka::Topic::create(producer, topic_str,
                                                 tconf, errstr);
  if (!topic) {
    std::cerr << "Failed to create topic: " << errstr << std::endl;
    exit(1);
  }

  /*
   * Read messages from stdin and produce to broker.
   */
  // for (std::string line; run && std::getline(std::cin, line);) {
  //   if (line.empty()) {
  //     producer->poll(0);
  //     continue;
  //   }

  //   /*
  //    * Produce message
  //    */
  //   RdKafka::ErrorCode resp =
  //     producer->produce(topic, partition,
  //                       RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
  //                       const_cast<char *>(line.c_str()), line.size(),
  //                       NULL, NULL);
  //   if (resp != RdKafka::ERR_NO_ERROR)
  //     std::cerr << "% Produce failed: " <<
  //       RdKafka::err2str(resp) << std::endl;
  //   else
  //     std::cerr << "% Produced message (" << line.size() << " bytes)" <<
  //       std::endl;

  //   producer->poll(0);
  // }

  uint64_t* data = new uint64_t[1024];
  for(int i = 0;i<1024;++i)
    data[i] = 2*i+1;

 
  RdKafka::ErrorCode resp =
    producer->produce(topic, partition,
                      RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                      reinterpret_cast<char*>(data), 1024*sizeof(uint64_t),
                      NULL, NULL);
  
  if (resp != RdKafka::ERR_NO_ERROR)
    std::cerr << "% Produce failed: " <<
      RdKafka::err2str(resp) << std::endl;
  else
    std::cerr << "% Produced message (" << sizeof(data) << " bytes)" <<
      std::endl;
  
  //   producer->poll(0);
  // }



  run = true;

  while (run && producer->outq_len() > 0) {
    std::cerr << "Waiting for " << producer->outq_len() << std::endl;
    producer->poll(1000);
  }

  delete topic;
  delete producer;


  // }
  //  else if (mode == "C") {
  //   /*
  //    * Consumer mode
  //    */

  //   if(topic_str.empty())
  //     goto usage;

  //   /*
  //    * Create consumer using accumulated global configuration.
  //    */
  //   RdKafka::Consumer *consumer = RdKafka::Consumer::create(conf, errstr);
  //   if (!consumer) {
  //     std::cerr << "Failed to create consumer: " << errstr << std::endl;
  //     exit(1);
  //   }

  //   std::cout << "% Created consumer " << consumer->name() << std::endl;

  //   /*
  //    * Create topic handle.
  //    */
  //   RdKafka::Topic *topic = RdKafka::Topic::create(consumer, topic_str,
  //       					   tconf, errstr);
  //   if (!topic) {
  //     std::cerr << "Failed to create topic: " << errstr << std::endl;
  //     exit(1);
  //   }

  //   /*
  //    * Start consumer for topic+partition at start offset
  //    */
  //   RdKafka::ErrorCode resp = consumer->start(topic, partition, start_offset);
  //   if (resp != RdKafka::ERR_NO_ERROR) {
  //     std::cerr << "Failed to start consumer: " <<
  //       RdKafka::err2str(resp) << std::endl;
  //     exit(1);
  //   }

  //   ExampleConsumeCb ex_consume_cb;

  //   /*
  //    * Consume messages
  //    */
  //   while (run) {
  //     if (use_ccb) {
  //       consumer->consume_callback(topic, partition, 1000,
  //                                  &ex_consume_cb, &use_ccb);
  //     } else {
  //       RdKafka::Message *msg = consumer->consume(topic, partition, 1000);
  //       msg_consume(msg, NULL);
  //       delete msg;
  //     }
  //     consumer->poll(0);
  //   }

  //   /*
  //    * Stop consumer
  //    */
  //   consumer->stop(topic, partition);

  //   consumer->poll(1000);

  //   delete topic;
  //   delete consumer;
  // }
  // else {
  //   /* Metadata mode */

  //   /*
  //    * Create producer using accumulated global configuration.
  //    */
  //   RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
  //   if (!producer) {
  //     std::cerr << "Failed to create producer: " << errstr << std::endl;
  //     exit(1);
  //   }

  //   std::cout << "% Created producer " << producer->name() << std::endl;

  //   /*
  //    * Create topic handle.
  //    */
  //   RdKafka::Topic *topic = NULL;
  //   if(!topic_str.empty()) {
  //     topic = RdKafka::Topic::create(producer, topic_str, tconf, errstr);
  //     if (!topic) {
  //       std::cerr << "Failed to create topic: " << errstr << std::endl;
  //       exit(1);
  //     }
  //   }

  //   while (run) {
  //     class RdKafka::Metadata *metadata;

  //     /* Fetch metadata */
  //     RdKafka::ErrorCode err = producer->metadata(topic!=NULL, topic,
  //                             &metadata, 5000);
  //     if (err != RdKafka::ERR_NO_ERROR) {
  //       std::cerr << "%% Failed to acquire metadata: " 
  //                 << RdKafka::err2str(err) << std::endl;
  //             run = 0;
  //             break;
  //     }

  //     metadata_print(topic_str, metadata);

  //     delete metadata;
  //     run = 0;
  //   }

  // }


  /*
   * Wait for RdKafka to decommission.
   * This is not strictly needed (when check outq_len() above), but
   * allows RdKafka to clean up all its resources before the application
   * exits so that memory profilers such as valgrind wont complain about
   * memory leaks.
   */
  RdKafka::wait_destroyed(5000);

  return 0;
}
