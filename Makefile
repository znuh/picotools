
all:
	gcc -rdynamic -lps5000 -Wall -o pico main.c handlers.c scope.c `pkg-config --cflags --libs libglade-2.0`

clean:
	rm -f pico
