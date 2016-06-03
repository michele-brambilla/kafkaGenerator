#SHELL = /bin/tcsh

CC = gcc
CXX = g++

CCFLAGS = 
CXXFLAGS = -I. -I${KAFKA_INCLUDE} -std=c++11

LDFLAGS = -L${KAFKA_LIBRARY_PATH}
LIBS = -lrdkafka -lrdkafka++

.SUFFIXES: 
.SUFFIXES: .cpp .o

targets = consumer producer rdkafka_example
objects = consumer.o producer.o rdkafka_example.o

all : $(targets)

consumer: kafka_consumer.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

producer: kafka_producer.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

rdkafka_example: rdkafka_example.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $<

.PHONY : clean
clean:
	rm -f $(targets) $(objects)
