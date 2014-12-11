all: myCald

myCald: tserver.c
	gcc `xml2-config --cflags --libs` tserver.c -o myCald -lpthread -I /usr/include/libxml2/ -Wall

clean:
	rm -f myCald