all: myCal

myCal: tclient.c
	gcc `xml2-config --cflags --libs` tclient.c -o myCal -lpthread -I /usr/include/libxml2/

clean:
	rm -f myCal