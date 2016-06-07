#include <generator.hpp>



int main() {

  Generator<ZmqGen,HeaderJson> g(1235,"control.in");


  uint64_t* d = new uint64_t[1024];
  
  for(int i =0;i<1024;++i)
    d[i] = 2*i+1;

  // read NeXus or mcstas


  

  g.run(d,1024);


  return 0;

}

