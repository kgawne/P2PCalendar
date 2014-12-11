all: myCald

myCald: tserver.c
	gcc `xml2-config --cflags --libs` tserver.c -o myCald -lpthread -Wall

clean:
	rm -f myCald