all: mm

mm: main.cpp
	g++ -std=c++11 -Wall file.cpp main.cpp -o mm
