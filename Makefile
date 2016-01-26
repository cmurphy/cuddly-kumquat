all: mm

mm: main.c
	gcc -Wall main.c -o mm
