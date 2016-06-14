#SHELL = /bin/tcsh

MAKE = make

CC = gcc
CXX = g++

NEXUS_PATH=/afs/psi.ch/project/sinq/sl6-64

CCFLAGS = 
CXXFLAGS = -std=c++11 -I. -I${KAFKA_INCLUDE}  -I${NEXUS_PATH}/include -pthread

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

main: main.o
	$(MAKE) -C cJSON/
	$(CXX) $(CXXFLAGS) -o $@ $^ -I. -IcJSON $(LDFLAGS) -LcJSON $(LIBS) -lcjson -lzmq -lsodium

doc:
	cd docs; doxygen Doxyfile; cd ..

%.o : %.cpp generator.hpp zmq_generator.hpp kafka_generator.hpp nexus_reader.hpp mcstas_reader.hpp uparam.hpp control.hpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY : clean
clean:
	rm -f $(targets) $(objects)
