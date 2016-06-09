
#include "nexus_reader.hpp"
#include "generator.hpp"

typedef Rita2 Instrument;
typedef NeXusSource<Instrument> Source;

typedef ZmqGen generator_t;
//typedef KafkaGen generator_t;




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

  Source stream(input);

  Generator<generator_t,HeaderJson,uint64_t> g(input);

  g.run(&(stream.begin()[0]),stream.count());

  return 0;
}

