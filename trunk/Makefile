
#-finstrument-functions -lSaturn -pg 

all: scpdp-misc.o scpdp.h scpdp-core.o scpdp-keys.o scpdp-file.o scpdp-app.c 
	gcc -g -Wall -O3 -lcrypto -o scpdp scpdp-app.c scpdp-core.o scpdp-misc.o scpdp-keys.o scpdp-file.o

scpdp-core.o: scpdp-core.c scpdp.h
	gcc -g -Wall -O3 -c scpdp-core.c

scpdp-keys.o: scpdp-keys.c scpdp.h
	gcc -g -Wall -O3 -c scpdp-keys.c

scpdp-misc.o: scpdp-misc.c scpdp.h
	gcc -g -Wall -O3 -c scpdp-misc.c

scpdp-file.o: scpdp-file.c scpdp.h
	gcc -g -Wall -O3 -c scpdp-file.c

scpdplib: scpdp-core.o scpdp-misc.o scpdp-keys.o scpdp-file.o
	ar -rv scpdplib.a scpdp-core.o scpdp-misc.o scpdp-keys.o scpdp-file.o

clean:
	rm -rf *.o *.tok scpdp.dSYM scpdp