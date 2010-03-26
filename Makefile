
all:
	gcc -rdynamic -lps5000 -lSDL -lSDL_gfx -lSDL_ttf -Wall -g -o pico main.c handlers.c scope.c wview/wview.c wview/sdl_display.c wview/scrollbar.c `pkg-config --cflags --libs libglade-2.0 gthread-2.0`

clean:
	rm -f pico *~
