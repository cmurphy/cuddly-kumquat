all: mm

mm: file.cpp song.cpp main.cpp
	g++ -g -std=c++11 -Wall file.cpp song.cpp main.cpp -o mm
