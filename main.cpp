#ifndef _MAIN_H
#define _MAIN_H

#include "nexus_reader.hpp"
#include "mcstas_reader.hpp"
#include "generator.hpp"

// typedef nexus::Rita2 Instrument;
// typedef nexus::NeXusSource<Instrument> Source;

typedef mcstas::Rita2 Instrument;
typedef mcstas::McStasSource<Instrument> Source;

typedef ZmqGen generator_t;
//typedef KafkaGen generator_t;


#include <iostream>
#include <fstream>
#include <iterator>

#include <sstream>
#include <cstring>


#include "uparam.hpp"


///////////////////////////////////////////////
///////////////////////////////////////////////
///
/// Main program for using the flexible event generator
///
///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 15:14:10 2016
int main() {

  uparam::Param input;
  input["port"] = "1235";
  input["control"] = "control.in";
  input["topic"] = "test_0";
  input["brokers"] = "localhost";
  
  input["filename"] = "../../neventGenerator/rita22012n006190.hdf";
  input["1D"] = "sample/boa.tof";
  input["2D"] = "sample/boa.2d";

  
  Source stream(input);
  std::copy(stream.begin(),stream.end(),std::ostream_iterator<uint64_t>(std::cout, "\n")); 
  

  Generator<generator_t,HeaderJson,FileControl> g(input);

  g.run(&(stream.begin()[0]),stream.count());

  return 0;
}


#endif //MAIN_H
