all: mm

mm: main.cpp
	g++ -std=c++11 -Wall main.cpp -o mm
