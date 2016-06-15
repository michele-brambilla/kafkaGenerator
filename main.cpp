#ifndef _MAIN_H
#define _MAIN_H

#include <iostream>
// #include <fstream>
// #include <iterator>

#include <unistd.h> // getopt

// #include <sstream>
// #include <cstring>

#include "uparam.hpp"
#include "nexus_reader.hpp"
#include "mcstas_reader.hpp"
#include "generator.hpp"

// typedef nexus::Rita2 Instrument;
// typedef nexus::NeXusSource<Instrument> Source;

typedef mcstas::Rita2 Instrument;
typedef mcstas::McStasSource<Instrument> Source;

//typedef control::FileControl Control;
typedef control::CommandlineControl Control;


typedef ZmqGen generator_t;
//typedef KafkaGen generator_t;



///////////////////////////////////////////////
///////////////////////////////////////////////
///
/// Main program for using the flexible event generator
///
///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 15:14:10 2016
int main(int argc, char **argv) {

  uparam::Param input;
  // default values
  input["port"] = "1235";
  input["control"] = "control.in";
  input["topic"] = "test_0";
  input["brokers"] = "localhost";
  input["filename"] = "../../neventGenerator/rita22012n006190.hdf";
  input["1D"] = "sample/boa.tof";
  input["2D"] = "sample/boa.2d";

  opterr = 0;
  int opt;
  while ((opt = getopt (argc, argv, "a:b:c:f:p:s:t:")) != -1) {
    switch (opt) {
    case 'a': //area
      input["2D"] = std::string(optarg);
      break;
    case 'b':
      input["brokers"] = std::string(optarg);
      break;
    case 'c':
      input["control"] = std::string(optarg);
      break;
    case 'f':
      input["filename"] = std::string(optarg);
      break;
    case 'p':
      input["port"] = std::string(optarg);
      break;
    case 's': // single dimension detector
      input["1D"] = std::string(optarg);
      break;
    case 't':
      input["topic"] = std::string(optarg);
      break;
    case '?':
      // TODO help
      return 1;
    }
  }
  
  Source stream(input);  

  Generator<generator_t,HeaderJson,Control> g(input);

  g.run(&(stream.begin()[0]),stream.count());

  return 0;
}


#endif //MAIN_H
