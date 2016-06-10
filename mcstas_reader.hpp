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

#include "uparam.hpp"

namespace mcstas {
  
  template<typename Instrument>
  struct McStasSource {
    typedef McStasSource self_t;
    typedef uint64_t value_type;

    std::vector<value_type>::iterator begin() { return data.begin(); }
    std::vector<value_type>::iterator end() { return data.end(); }
    std::vector<value_type>::const_iterator begin() const { return data.begin(); }
    std::vector<value_type>::const_iterator end() const { return data.end(); }
  
    McStasSource(uparam::Param p) {

    }

    int count() const { return data.size(); }

  private:
    Instrument instrum;
    std::vector<value_type> data;

  };


  struct D1 {

    D1(std::string& s) : in(s), n_col(0) { }

    void operator ()(const int t, const int n) {
      extract_values(t,n);
      // std::string value;
      // while( in.good() ) {
      //   std::getline (in,value);
        
      //   int counter = 0;
      //   char *token = std::strtok(&value[0], " ");
      //   while (token != NULL) {
      //     if(counter == t || counter == n) { 
      //       std::cout << atof(token) << '\t';
      //       n_col++;
      //     ++counter;
      //     token = std::strtok(NULL, " ");
      //   }        
      // }
    }
    
  private:
    std::ifstream in;
    int n_col;

    void extract_values(const int t, const int n) {
      std::string value;
      while( in.good() ) {
        std::getline (in,value);
        
        int counter = 0;
        char *token = std::strtok(&value[0], " ");
        while (token != NULL) {
          if(counter == t || counter == n) { 
            std::cout << atof(token) << '\t';
            n_col++;
            ++counter;
            token = std::strtok(NULL, " ");
          }
        }
      }
    }
  };


  struct D2 {

    D2(std::string& s) : in(s), n_row(0), n_col(0) { }

    void operator ()(const int n) {
      extract_values(n);
      // std::string value;
      // bool is_value=false;
      // int block=-1;
      
      // while( in.good() ) {
      //   std::getline (in,value);
      //   if ( value[0] == '#' ) {
      //     is_value=false;
      //     continue;
      //   }
      //   if( is_value == false ) {
      //     ++block;
      //     is_value = true;
      //   }
      //   if(block != n) continue;
      //   n_row++;
      //   char *token = std::strtok(&value[0], " ");
      //   while (token != NULL) {
      //     std::cout << atof(token) << "\t";
      //     token = std::strtok(NULL, " ");
      //     n_col++;
      //   }
      // }
      
    }
      
  private:
    std::ifstream in;
    int n_row,n_col;

    void extract_values(const int n) {
      std::string value;
      bool is_value=false;
      int block=-1;
      
      while( in.good() ) {
        std::getline (in,value);
        if ( value[0] == '#' ) {
          is_value=false;
          continue;
        }
        if( is_value == false ) {
          ++block;
          is_value = true;
        }
        if(block != n) continue;
        n_row++;
        char *token = std::strtok(&value[0], " ");
        while (token != NULL) {
          std::cout << atof(token) << "\t";
          token = std::strtok(NULL, " ");
          n_col++;
        }
      }
      
    }

  };
  

} // namespace


#endif //NEXUS_READER_H
