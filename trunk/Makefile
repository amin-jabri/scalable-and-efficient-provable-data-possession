
#-finstrument-functions -lSaturn -pg 

all: sepdp-misc.o sepdp.h sepdp-keys.o sepdp-file.o sepdp-app.c 
	gcc -g -Wall -O3 -lcrypto -o sepdp sepdp-app.c sepdp-misc.o sepdp-keys.o sepdp-file.o

sepdp-keys.o: sepdp-keys.c sepdp.h
	gcc -g -Wall -O3 -c sepdp-keys.c

sepdp-misc.o: sepdp-misc.c sepdp.h
	gcc -g -Wall -O3 -c sepdp-misc.c

sepdp-file.o: sepdp-file.c sepdp.h
	gcc -g -Wall -O3 -c sepdp-file.c

sepdplib: sepdp-core.o sepdp-misc.o sepdp-keys.o sepdp-file.o
	ar -rv sepdplib.a sepdp-misc.o sepdp-keys.o sepdp-file.o

clean:
	rm -rf *.o *.tok sepdp.dSYM sepdp