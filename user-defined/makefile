include Makefile.inc

CFLAGS += -std=c++11 -I$(DYNINST_INC)
LDFLAGS += -L$(DYNINST_LIB) -L. -ldyninstAPI -lboost_system

CC=gcc
CXX=g++

TARGET=mutator
OBJS=mutator.o
SRC=mutator.cpp

all : $(TARGET) libmyfunc.so mutatee

$(TARGET) : $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

$(OBJS) : $(SRC)
	$(CXX) -c $(CFLAGS) $(SRC)

# aux lib
libmyfunc.so : myfunc.c
	$(CC) -fPIC -shared myfunc.c -o libmyfunc.so 

# mutatee
mutatee : mutatee.c
	$(CC) mutatee.c -no-pie -o mutatee


clean:
	rm -f $(OBJS) $(TARGET) libmyfunc.so mutatee mutatee-rewritten

