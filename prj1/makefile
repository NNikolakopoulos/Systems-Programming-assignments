main:main.o list.o hash.o bloom.o slist.o functions.o
	gcc -g3 -O0 -o main main.o list.o hash.o bloom.o slist.o functions.o


main.o:main.c
	gcc -g3 -O0 -c main.c
	
list.o:list.c
	gcc -g3 -O0 -c list.c

hash.o:hash.c
	gcc -g3 -O0 -c hash.c

bloom.o:bloom.c
	gcc -g3 -O0 -c bloom.c

slist.o:slist.c
	gcc -g3 -O0 -c slist.c

functions.o:functions.c
	gcc -g3 -O0 -c functions.c


clean:
	rm -f list.o main.o hash.o bloom.o main slist.o functions.o
