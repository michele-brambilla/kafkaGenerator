#ifndef _NEXUS_READER_H
#define _NEXUS_READER_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <cstring>

#include <nexus/NeXusFile.hpp>

#include "uparam.hpp"

///  \author Michele Brambilla <mib.mic@gmail.com>
///  \date Wed Jun 08 16:49:17 2016
template<typename Instrument,typename EventRule>
struct NeXusSource {
  typedef NeXusSource self_t;
  typedef uint64_t value_type;

  NeXusSource(uparam::Param p) {
    file.open(p["filename"]);
    std::vector<NeXus::AttrInfo> attr_infos = file.getAttrInfos();
    std::cout << "Number of global attributes: " 
              << attr_infos.size() 
              << std::endl;
 }

  void read() {
    file.openPath(Instrument.data[0]);
  }



private:
  NeXus::File file;
};



struct Rita2 {
  static const int n_monitor = 2;
  
  Rita2() {
    data.push_back("/entry1/RITA-2/detector/counts");
    data.push_back("random");
  }

  std::vector<std::string> data;
};


struct AMOR {
  static const int n_monitor = 2;
  
  AMOR() {
    data.push_back("/entry1/AMOR/area_detector/data");
    data.push_back("/entry1/AMOR/area_detector/time_binning");
  }

  std::vector<std::string> data;
};


struct FOCUS {
  static const int n_monitor = 3;
  
  FOCUS() {
    data.push_back("/entry1/FOCUS/merged/counts");
    data.push_back("/entry1/FOCUS/merged/time_binning");
    data.push_back("/entry1/FOCUS/tof_monitor");
  }

  std::vector<std::string> data;
};






#endif //NEXUS_READER_H
