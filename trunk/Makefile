
#-finstrument-functions -lSaturn -pg 

S3LIB = ../libs3-1.4/build/lib/libs3.a

all: sepdp-misc.o sepdp.h sepdp-keys.o sepdp-file.o sepdp-app.c 
	gcc -g -Wall -O3 -lcrypto -o sepdp sepdp-app.c sepdp-misc.o sepdp-keys.o sepdp-file.o

sepdp-s3: sepdp-misc.o sepdp.h sepdp-keys.o sepdp-file.o sepdp-s3.o sepdp-app.c 
	gcc -DUSE_S3 -g -Wall -O3 -lpthread -lcurl -lxml2 -lz -lcrypto -o sepdp-s3 sepdp-app.c sepdp-misc.o sepdp-keys.o sepdp-file.o sepdp-s3.o $(S3LIB)

sepdp-keys.o: sepdp-keys.c sepdp.h
	gcc -g -Wall -O3 -c sepdp-keys.c

sepdp-misc.o: sepdp-misc.c sepdp.h
	gcc -g -Wall -O3 -c sepdp-misc.c

sepdp-file.o: sepdp-file.c sepdp.h
	gcc -g -Wall -O3 -c sepdp-file.c

sepdp-s3.o: sepdp-s3.c sepdp.h ../libs3-1.4/build/include/libs3.h
	gcc -DUSE_S3 -g -Wall -O3 -I../libs3-1.4/build/include/ -c sepdp-s3.c

sepdplib: sepdp-core.o sepdp-misc.o sepdp-keys.o sepdp-file.o
	ar -rv sepdplib.a sepdp-misc.o sepdp-keys.o sepdp-file.o

clean:
	rm -rf *.o *.tok sepdp.dSYM sepdp