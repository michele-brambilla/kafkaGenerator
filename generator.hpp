#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <assert.h>

#include <stdlib.h>
#include <time.h>

//#include <zmq.hpp>
#include <zmq.h>

#include "uparam.hpp"

extern "C" {
#include "cJSON/cJSON.h"
}

#ifdef HAVE_CXX11
using std::to_string;
#else
std::string to_string(const int& value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

#endif



template<typename Streamer, typename Header>
struct Generator {
  typedef  Generator self_t;

  Generator(const int& p, 
            std::string control, 
            const int& m = 1) : port(p), 
                                multiplier(m), 
                                head("header.in") {
    context = zmq_ctx_new ();
    socket = zmq_socket (context, ZMQ_PUSH);
    std::cout << "tcp://*:"+to_string(port) << std::endl;
    int rc = zmq_bind(socket,("tcp://*:"+to_string(port)).c_str());
    assert (rc == 0);
    //    socket.setsockopt(zmq::SNDHWM, 100);

    get_control();

    return;
  }

  template<class T>
  void run(const T* stream, int nev = 0) {
    
    int pulseID = 0;
    int count = 0;
    int start = time(0);

    head.set(pulseID,time(0),1234567,nev,atoi(ctl["rate"].c_str()));

    while(ctl["run"] != "stop") {
      
      if(ctl["run"] == "run") {
        zmq_send(socket,head.get().c_str(),head.size(),ZMQ_SNDMORE);
        zmq_send(socket,stream,nev,0);
      }
      else {
        zmq_send(socket,head.get().c_str(),head.size(),0);
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
  int port, multiplier;
  std::ifstream icf;
  uparam::Param ctl;
  Streamer streamer;

  void* context;
  void* socket;
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

  const std::string get() { return content; }

  const int size() { return len; }

  std::string content;
  int len;
};




struct ZmqGen {

};
