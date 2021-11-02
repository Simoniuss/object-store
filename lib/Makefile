CC 			=	gcc
CFLAGS 		= 	-Wall
OPTFLAGS 	= 	-O3 -g
AR 			=	ar
ARFLAGS		=	rvs
INCLUDE 	= 	-I../include
LIBFLAG 	= 	-L.
LIB 		= 	-lpthread

TARGET 		=	libUtils.a 		\
				libServer_op.a  \
				libObjStr.a 	\
				#libList.a

.PHONY : all clean


% : %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBFLAG) $(LIB)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

libUtils.a : my_utils.o
	$(AR) $(ARFLAGS) $@ $<

libServer_op.a : server_op.o
	$(AR) $(ARFLAGS) $@ $<

libObjStr.a : obj_str.o
	$(AR) $(ARFLAGS) $@ $<

#libList.a : list.o list_iterator.o list_node.o
#	$(AR) $(ARFLAGS) $@ $^

all : $(TARGET)

clean :
	rm -f *.o *.a