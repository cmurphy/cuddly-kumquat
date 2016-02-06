all: mm

mm: file.cpp main.cpp
	g++ -std=c++11 -Wall file.cpp main.cpp -o mm
