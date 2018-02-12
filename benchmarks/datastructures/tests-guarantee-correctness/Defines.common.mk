PROG := main

SRCS += main.c \
	$(wildcard $(LIB)/*.c)
	#$(LIB)/mt19937ar.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \
	$(LIB)/handler.c \
	$(LIB)/stm_src.c \

#
OBJS := ${SRCS:.c=.o}
