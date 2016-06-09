#ifndef _NEXUS_READER_H
#define _NEXUS_READER_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

#include <cstring>
#include <stdlib.h>

#include <nexus/napi.h>

#include "uparam.hpp"

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 16:49:17 2016
template<typename Instrument,typename EventRule>
struct NeXusSource {
  typedef NeXusSource self_t;
  typedef uint64_t value_type;

  NeXusSource(uparam::Param p) {
    if(NXopen(p["filename"].c_str(),NXACC_READ,&handle) != NX_OK){
      throw std::runtime_error("Failed to open NeXus file "+p["filename"]);
    }
  }
  
  void read() {
    instrum(handle, data);
    NXclose(&handle);    
  }



private:
  NXhandle handle;
  Instrument instrum;
  std::vector<value_type> data;
};



struct Rita2 {
  static const int n_monitor = 2;
  std::vector<std::string>::iterator begin() { return path.begin(); }
  std::vector<std::string>::iterator end() { return path.end(); }
  std::vector<std::string>::const_iterator begin() const { return path.begin(); }
  std::vector<std::string>::const_iterator end() const { return path.end(); }

  Rita2() {
    path.push_back("/entry1/RITA-2/detector/counts");
    path.push_back("random");
  }

  template<typename T>
  void operator()(const NXhandle& handle, std::vector<T>& stream) {
    int rank, type, size;
    
    if(NXopenpath(handle,path[0].c_str()) != NX_OK){
      throw std::runtime_error("Error reading NeXus data");
    }
    NXgetinfo(handle,&rank,dim,&type);

    std::cout << "rank: " << rank << "\n"
              << "type: " << type << "\n"
              << "dim: " << dim[0] << "\t" << dim[1] << "\t" << dim[2] << "\n";
    size = dim[0];
    for(int i = 1; i < rank; i++) size *= dim[i];
    dim[rank] = dim[rank-1];
    
    std::cout << "size: " << size << "\n"    ;

    data = new int32_t [size];
    std::cout << "data: " << data << "\n"    ;

    NXgetdata(handle,&data[0]);
    NXclose(&handle);

    toEventFmt<T>(stream);
  }
  
  std::vector<std::string> path;

private:
  int32_t* data = NULL;
  int32_t dim[3+1];

  template<typename T>
  void toEventFmt(std::vector<T>& signal) {
    int offset, nCount;
    T value;
    
    for(int i = 0; i < dim[1]; ++i){
      offset = i*dim[2];
      for(int j = 0; j < dim[2]; ++j) {
        nCount = data[i];
        for(int l = 0;l< nCount; ++l) {
          //          int val = rand();
          value = ( rand() << 32 |1 << 31 | 1 << 30 | 1 << 29 | 1 << 28 | 2 << 24 | i << 12 | j );
          std::cout << value << std::endl;
          signal.push_back(value);
        }
      }
    }
    
  }

};


// struct AMOR {
//   static const int n_monitor = 2;
//   std::vector<std::string>::iterator begin() { return data.begin(); }
//   std::vector<std::string>::iterator end() { return data.end(); }
//   std::vector<std::string>::const_iterator begin() const { return data.begin(); }
//   std::vector<std::string>::const_iterator end() const { return data.end(); }

//   AMOR() {
//     data.push_back("/entry1/AMOR/area_detector/data");
//     data.push_back("/entry1/AMOR/area_detector/time_binning");
//   }

//   std::vector<std::string> data;
// };


// struct FOCUS {
//   static const int n_monitor = 3;
//   std::vector<std::string>::iterator begin() { return data.begin(); }
//   std::vector<std::string>::iterator end() { return data.end(); }
//   std::vector<std::string>::const_iterator begin() const { return data.begin(); }
//   std::vector<std::string>::const_iterator end() const { return data.end(); }

//   FOCUS() {
//     data.push_back("/entry1/FOCUS/merged/counts");
//     data.push_back("/entry1/FOCUS/merged/time_binning");
//     data.push_back("/entry1/FOCUS/tof_monitor");
//   }
//   std::vector<std::string> data;
// };






#endif //NEXUS_READER_H
