all: mycal

mycal: tclient.c
	gcc `xml2-config --cflags --libs` tclient.c -o mycal -lpthread -I /usr/include/libxml2/

clean:
	rm -f mycal