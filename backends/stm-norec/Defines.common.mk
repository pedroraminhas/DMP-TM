STM := ../../../stms/norec/
CC       := gcc
CFLAGS   += -std=c++11 -g -w -pthread -fpermissive -mcpu=power8 -mtune=power8
CFLAGS   += -I /home/shady/lib/boost/include
CFLAGS   += -O2
CFLAGS   += -I$(LIB) -I$(STM)
CPP      := g++
CPPFLAGS += $(CFLAGS)
LD       := g++
LIBS     += -lpthread

LIB := ../lib
