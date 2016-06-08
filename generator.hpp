#ifndef _GENERATOR_H
#define _GENERATOR_H

#include <iostream>
#include <fstream>
#include <map>
#include <assert.h>

#include <stdlib.h>
#include <time.h>

#include "zmq_generator.hpp"
#include "kafka_generator.hpp"

extern "C" {
#include "cJSON/cJSON.h"
}


///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 15:19:52 2016
template<typename Streamer, typename Header>
struct Generator {
  typedef  Generator self_t;

Generator(const uparam::Param& p,
            const int& m = 1) : streamer(p), 
                                multiplier(m), 
                                head("header.in") {
get_control();
  }

  template<class T>
  void run(T* stream, int nev = 0) {
    
    int pulseID = 0;
    int count = 0;
    int start = time(0);

    head.set(pulseID,time(0),1234567,nev,atoi(ctl["rate"].c_str()));

    while(ctl["run"] != "stop") {
      
      if(ctl["run"] == "run") {
        streamer.send(&head.get()[0],head.size(),ZMQ_SNDMORE);
        streamer.send(stream,nev,0);
      }
      else {
        streamer.send(&head.get()[0],head.size(),0);
      }        
   
      ++count;
      ++pulseID;

      if(time(0) - start > 10) {
        std::cout << "Sent "       << count 
                  << " packets @ " << count*nev*sizeof(T)/(10*1e6)
                  << "MB/s" 
                  << std::endl;
        std::cin >> count;

        get_control();
        head.set(pulseID,time(0),1234567,nev,atoi(ctl["rate"].c_str()));

        count = 0;
        start = time(0);
      }
    }


 
  }


private:
  int multiplier;
  std::ifstream icf;
  uparam::Param ctl;
  Streamer streamer;

  Header head;

  void get_control() {
    ctl.read("control.in");
  }


};






struct HeaderJson {

  HeaderJson(std::string s) {
    std::ifstream in(s);
    std::string dummy;
    while(in.good()) {
      in >> dummy;
      content += dummy;
    }
    in.close();
    len = content.size();
    std::cout << (content+="\n");
    
  } 

  const std::string& set(const int pid, 
                         const int st,
                         const int ts,
                         const int nev,
                         const int tr) {
    
    
    cJSON* root = cJSON_Parse(content.c_str());

    //////////////////////
    // !!! cJSON does not modify valueint vars: use valuedouble
    cJSON_GetObjectItem(root,"pid")->valuedouble = pid;
    cJSON_GetObjectItem(root, "st")->valuedouble = st;
    cJSON_GetObjectItem(root, "ts")->valuedouble = ts;
    cJSON_GetObjectItem(root, "tr")->valuedouble = tr;

    cJSON* item = cJSON_GetObjectItem(root,"ds");
    cJSON_GetArrayItem(item,1) -> valuedouble = nev;

    return content = std::string(cJSON_Print(root));;
  }

  //  const std::string get() { return content; }
  std::string get() { return content; }

  const int size() { return len; }

  std::string content;
  int len;
};



#endif //GENERATOR_H

