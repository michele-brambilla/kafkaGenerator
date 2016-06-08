#ifndef _KAFKA_GENERATOR_H
#define _KAFKA_GENERATOR_H

#include <string>
#include <sstream>
#include <assert.h>
#include <librdkafka/rdkafkacpp.h>

#include "uparam.hpp"


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





///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 15:19:16 2016
struct KafkaGen {
  
  KafkaGen(uparam::Param p) : brokers(p["brokers"]), topic_str(p["topic"]) {
    conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    conf->set("metadata.broker.list", brokers, errstr);
    if (!debug.empty()) {
      if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
      }
    }

    if(topic_str.empty()) {
      
    }
    
    producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
      std::cerr << "Failed to create producer: " << errstr << std::endl;
      exit(1);
    }
    std::cout << "% Created producer " << producer->name() << std::endl;

    topic = RdKafka::Topic::create(producer, topic_str,
                                   tconf, errstr);
    if (!topic) {
      std::cerr << "Failed to create topic: " << errstr << std::endl;
      exit(1);
    }
  }

  template<typename T>
  void send(T* data,const int size, const int flag = 0) {

    RdKafka::ErrorCode resp =
      producer->produce(topic, partition,
                        RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                        data, size*sizeof(T),
                        NULL, NULL);
    if (resp != RdKafka::ERR_NO_ERROR)
      std::cerr << "% Produce failed: " <<
        RdKafka::err2str(resp) << std::endl;
      std::cerr << "% Produced message (" << sizeof(data) << " bytes)" <<
        std::endl;
  }

private:
  std::string brokers;
  std::string topic_str;
  std::string errstr;
  std::string debug;
  
  int32_t partition = RdKafka::Topic::PARTITION_UA;
  int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;

  int opt;
  MyHashPartitionerCb hash_partitioner;
  RdKafka::Conf *conf;
  RdKafka::Conf *tconf;
  RdKafka::Producer *producer;
  RdKafka::Topic *topic;


  void serialise() {

  }


};




#endif //KAFKA_GENERATOR_H
