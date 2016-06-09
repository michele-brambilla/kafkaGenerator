
#include "nexus_reader.hpp"
#include "generator.hpp"


//typedef ZmqGen generator_t;
typedef KafkaGen generator_t;




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

  NeXusSource<Rita2,uint64_t> stream(input);
  stream.read();
  return 0;


  Generator<generator_t,HeaderJson,uint64_t> g(input);

  uint64_t* d = new uint64_t[1024];
  
  for(int i =0;i<1024;++i)
    d[i] = 2*i+1;

  // read NeXus or mcstas

  g.run(d,1024);

  return 0;
}

