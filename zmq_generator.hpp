#ifndef _ZMQ_GENERATOR_H
#define _ZMQ_GENERATOR_H

#include <string>
#include <sstream>
#include <assert.h>
#include <zmq.h>

#include "uparam.hpp"

// #ifdef HAVE_CXX11
// using std::to_string;
// #else
// std::string to_string(const int& value) {
//   std::stringstream ss;
//   ss << value;
//   return ss.str();
// }
// #endif


/*! Uses 0MQ as data streamer.
*
*  \author Michele Brambilla <mib.mic@gmail.com>
*  \date Wed Jun 08 15:19:27 2016
*/

struct ZmqGen {
  
  ZmqGen(uparam::Param p) {
    context = zmq_ctx_new ();
    socket = zmq_socket (context, ZMQ_PUSH);
    std::cout << "tcp://*:"+p["port"] << std::endl;
    int rc = zmq_bind(socket,("tcp://*:"+p["port"]).c_str());
    assert (rc == 0);
    //    socket.setsockopt(zmq::SNDHWM, 100);
  }

  template<typename T>
  void send(const T* data,const int size, const int flag = 0) {
    zmq_send(socket,data,size,flag);
  }

private:
  void* context;
  void* socket;
};

#endif //ZMQ_GENERATOR_H
