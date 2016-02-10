all: mm

mm: file.cpp file.h song.cpp song.h main.cpp
	g++ -g -std=c++11 -Wall file.cpp song.cpp main.cpp -o mm
