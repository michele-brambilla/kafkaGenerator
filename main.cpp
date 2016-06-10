
// #include "nexus_reader.hpp"
// #include "generator.hpp"

// typedef Rita2 Instrument;
// typedef NeXusSource<Instrument> Source;

// typedef ZmqGen generator_t;
// //typedef KafkaGen generator_t;


#include <iostream>
#include <fstream>

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

  std::string value;

  std::ifstream in("../mcstasGenerator/sample/boa.tof");
  while( in.good() ) {
    std::getline (in,value);
    if ( value[0] == '#' ) continue;
    int counter = 0;
    char *token = std::strtok(&value[0], " ");
    while (token != NULL) {
      if(counter == 0 || counter == 3) 
        std::cout << atof(token) << '\t';
      ++counter;
      token = std::strtok(NULL, " ");
    }
    std::cout << std::endl;
  }
  in.close();


  in.open("../mcstasGenerator/sample/boa.2d");
  bool is_value=false;
  int block=-1,n_row=0,n_col=0;
  while( in.good() ) {
    std::getline (in,value);
    if ( value[0] == '#' ) {
      is_value=false;
      continue;
    }
    if( is_value == false ) {
      ++block;
      is_value = true;
      std:: cout << "\nnuovo blocco(" << block << ")" << std::endl;
    }
    if(block != 2) continue;
    n_row++;
    char *token = std::strtok(&value[0], " ");
    while (token != NULL) {
      std::cout << atof(token) << "\t";
      token = std::strtok(NULL, " ");
      n_col++;
    }
    std::cout << std::endl;
  }
  std::cout << "n blocks = " << block << "\t"
            << "n rows = "   << n_row << "\t"
            << "n col = "    << (n_col/=n_row) << "\t"
            << std::endl;
  in.close();




  
  // Source stream(input);

  // Generator<generator_t,HeaderJson,uint64_t> g(input);

  // g.run(&(stream.begin()[0]),stream.count());

  return 0;
}

