all: mycald

mycald: tserver.c
	gcc `xml2-config --cflags --libs` tserver.c -o mycald -lpthread -I /usr/include/libxml2/ -Wall

clean:
	rm -f mycald