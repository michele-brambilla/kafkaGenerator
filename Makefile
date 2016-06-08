#SHELL = /bin/tcsh

CC = gcc
CXX = g++

NEXUS_PATH=/afs/psi.ch/project/sinq/sl6-64

CCFLAGS = 
CXXFLAGS = -std=c++11 -I. -I${KAFKA_INCLUDE}  -I${NEXUS_PATH}/include

LDFLAGS = -L${KAFKA_LIBRARY_PATH} -L${NEXUS_PATH}/lib
LIBS = -lrdkafka -lrdkafka++ -lNeXus -lNeXusCPP

.SUFFIXES: 
.SUFFIXES: .cpp .o

targets = consumer producer rdkafka_example main
objects = consumer.o producer.o rdkafka_example.o main.o

all : $(targets)

consumer: kafka_consumer.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

producer: kafka_producer.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

rdkafka_example: rdkafka_example.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

main: main.o generator.hpp zmq_generator.hpp kafka_generator.hpp uparam.hpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -I. -IcJSON $(LDFLAGS) -LcJSON $(LIBS) -lcjson -lzmq -lsodium

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY : clean
clean:
	rm -f $(targets) $(objects)
