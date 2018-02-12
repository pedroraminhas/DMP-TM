PROG := hashmap

SRCS += hashmap.c  \
	$(LIB)/thread.c \
	#$(LIB)/handler.c \
	$(LIB)/stm_src.c \

#
OBJS := ${SRCS:.c=.o}
