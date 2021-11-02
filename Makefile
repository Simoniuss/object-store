CC 			=	gcc
CFLAGS 		= 	-Wall
OPTFLAGS 	= 	-O3 -g
AR 			=	ar
ARFLAGS		=	rvs
INCLUDE 	= 	-I./include
LIBFLAG 	= 	-L./lib
LIBS		=	-lpthread -lUtils -lServer_op -lObjStr #-lList 
MDIR		=	./lib
OUT 		=	./testout.log

TARGET 		=	server_objstr  \
				client_objstr

.PHONY : all clean dolib test

% : %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBFLAG) $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

server_objstr : server_objstr.c dolib 
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBFLAG) $(LIBS) -o $@ $<

client_objstr : client_objstr.c dolib
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBFLAG) $(LIBS) -o $@ $<
	
all : $(TARGET)

dolib :
	$(MAKE) all -C $(MDIR)

clean :
	$(MAKE) clean -C $(MDIR)
	rm -f server_objstr client_objstr $(OUT) 
	rm -rf ./data

test :
	echo "Start test..." 1>$(OUT);
	number=1 ; while [[ $$number -le 50 ]] ; do \
    	./client_objstr client_$$number 1 1>>$(OUT) ; \
    	((number = number + 1)) ; \
    done 
	wait
	number=1 ; while [[ $$number -le 30 ]] ; do \
    	./client_objstr client_$$number 2 1>>$(OUT) ; \
    	((number = number + 1)) ; \
    done 
	number=31 ; while [[ $$number -le 50 ]] ; do \
    	./client_objstr client_$$number 3 1>>$(OUT) ; \
    	((number = number + 1)) ; \
    done








