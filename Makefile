.PHONY: all clean

all:
	gcc -o exam main.c -lreadline

clean:
	rm -rf exam