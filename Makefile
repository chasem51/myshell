$(shell export LD_LIBRARY_PATH=$PWD:LD_LIBRARY_PATH)

all: myshell

myshell: myshell.c
	gcc -g -o myshell myshell.c -lreadline
clean:
	-rm -f *.a *.o *~ *.so myshell