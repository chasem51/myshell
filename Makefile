$(shell export LD_LIBRARY_PATH=$PWD:LD_LIBRARY_PATH)

all: myshell matmult_p matmult_t matformatter

matmult_p: matmult_p.c 
	gcc -o matmult_p matmult_p.c 
matmult_t: matmult_t.c 
	gcc -o matmult_t matmult_t.c
matformatter: matformatter.c 
	gcc -o matformatter matformatter.c 
myshell: myshell.c
	gcc -g -o myshell myshell.c -lreadline
clean:
	-rm -f *.a *.o *~ *.so myshell matmult_t matmult_p matformatter