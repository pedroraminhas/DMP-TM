CC       := gcc
CFLAGS   += -std=c++11 -g -w -pthread -fpermissive -mcpu=power8 -mtune=power8 -L/home/shady/lib/boost/lib -lboost_system
CFLAGS   += -O2
CFLAGS   += -I$(LIB)  -I ../../../stms/norec-p8/
CPP      := g++
CPPFLAGS += $(CFLAGS)
LD       := g++
LIBS     += -lpthread

# Remove these files when doing clean
OUTPUT +=

LIB := ../lib
