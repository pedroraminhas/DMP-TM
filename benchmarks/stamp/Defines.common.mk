CC       := gcc
CFLAGS   += -std=c++11 -g -w -pthread -mcpu=power8 -mtune=power8 -fpermissive
CFLAGS   += -O2
CFLAGS   += -I$(LIB) -I ../../../stms/dmp-tinystm/src/ -I ../../../stms/dmp-tinystm/include -I ../../../stms/dmp-tinystm/src/atomic_ops
CFLAGS   += -I /home/shady/lib/boost/include
CPP      := g++
CPPFLAGS += $(CFLAGS)
LD       := g++
LIBS     += -lpthread

# Remove these files when doing clean
OUTPUT +=

LIB := ../lib
