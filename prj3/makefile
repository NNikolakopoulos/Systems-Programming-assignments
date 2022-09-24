all:travelMonitorClient monitor


monitor:monitor.o list.o hash.o bloom.o slist.o pipes.o msg.o protocol.o functions.o  logs.o global.o  cyclic.o
	gcc -g3 -O0 -o monitor monitor.o list.o hash.o bloom.o slist.o pipes.o msg.o protocol.o functions.o  logs.o global.o cyclic.o -pthread

travelMonitorClient:travelMonitorClient.o list.o hash.o bloom.o slist.o pipes.o msg.o protocol.o functions.o  logs.o global.o 
	gcc -g3 -O0 -o travelMonitorClient travelMonitorClient.o list.o hash.o bloom.o slist.o pipes.o msg.o protocol.o functions.o  logs.o global.o  -pthread


travelMonitorClient.o:travelMonitorClient.c
	gcc -g3 -O0 -c travelMonitorClient.c
	
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

logs.o:logs.c
	gcc -g3 -O0 -c logs.c

global.o:global.c
	gcc -g3 -O0 -c global.c

cyclic.o:cyclic.c
	gcc -g3 -O0 -c cyclic.c -pthread

clean:
	rm -f list.o travelMonitorClient.o hash.o bloom.o travelMonitorClient slist.o pipes.o monitor.o monitor msg.o protocol.o functions.o logs.o global.o cyclic.o
