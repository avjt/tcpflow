
CC = gcc -g -O3
CXX = g++ -g -O3 -std=c++17
INCLUDE = 

TARGET = tcpflow

MAIN_OBJECTS = tcpflow.o
CXX_OBJECTS = $(MAIN_OBJECTS)
C_OBJECTS = 

HEADERS = 

OBJECTS = $(C_OBJECTS) $(CXX_OBJECTS)

default: $(TARGET)

$(CXX_OBJECTS): %.o: %.cc $(HEADERS)
	$(CXX) -c $(INCLUDE) -o $@ $<

$(C_OBJECTS): %.o: %.c $(HEADERS)
	$(CC) -c -o $@ $<

tcpflow: $(MAIN_OBJECTS)
	$(CXX) -o $@ $(MAIN_OBJECTS) -lpthread

clean:
	rm -rf $(OBJECTS) $(TARGET)

