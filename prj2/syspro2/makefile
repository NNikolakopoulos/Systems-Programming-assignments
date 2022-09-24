all:travelMonitor monitor


monitor:monitor.o list.o hash.o bloom.o slist.o pipes.o msg.o protocol.o functions.o  logs.o global.o signals_child.o
	gcc -g3 -O0 -o monitor monitor.o list.o hash.o bloom.o slist.o pipes.o msg.o protocol.o functions.o signals_child.o logs.o global.o

travelMonitor:travelMonitor.o list.o hash.o bloom.o slist.o pipes.o msg.o protocol.o functions.o  logs.o global.o signals_parent.o
	gcc -g3 -O0 -o travelMonitor travelMonitor.o list.o hash.o bloom.o slist.o pipes.o msg.o protocol.o functions.o  logs.o global.o signals_parent.o


travelMonitor.o:travelMonitor.c
	gcc -g3 -O0 -c travelMonitor.c
	
list.o:list.c
	gcc -g3 -O0 -c list.c

hash.o:hash.c
	gcc -g3 -O0 -c hash.c

bloom.o:bloom.c
	gcc -g3 -O0 -c bloom.c

slist.o:slist.c
	gcc -g3 -O0 -c slist.c

pipes.o:pipes.c
	gcc -g3 -O0 -c pipes.c

monitor.o:monitor.c
	gcc -g3 -O0 -c monitor.c

msg.o:msg.c
	gcc -g3 -O0 -c msg.c

protocol.o:protocol.c
	gcc -g3 -O0 -c protocol.c

functions.o:functions.c
	gcc -g3 -O0 -c functions.c

signals_parent.o:signals_parent.c
	gcc -g3 -O0 -c signals_parent.c

signals_child.o:signals_child.c
	gcc -g3 -O0 -c signals_child.c

logs.o:logs.c
	gcc -g3 -O0 -c logs.c

global.o:global.c
	gcc -g3 -O0 -c global.c


clean:
	rm -f list.o travelMonitor.o hash.o bloom.o travelMonitor slist.o pipes.o monitor.o monitor msg.o protocol.o functions.o signals_child.o signals_parent.o logs.o global.o
