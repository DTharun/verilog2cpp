prefix=/usr/local

CC      = gcc
CXX     = g++
LEX     = flex
YACC    = bison
INSTALL = cp

LDFLAGS = 
CXXFLAGS = -g


all: Verilog2C++ v2v

clean:
	rm Verilog2C++
	rm v2v
	rm *.o
	rm parse.tab.h parse.tab.c parse.output
	rm _lexor.cc lexor.cc

install:
	$(INSTALL) Verilog2C++ $(prefix)/bin
	$(INSTALL) v2v         $(prefix)/bin
	$(INSTALL) EmVer.hh    $(prefix)/include

archive:
	mkdir Verilog2C++
	cp Makefile		Verilog2C++
	cp verilog.l		Verilog2C++
	cp verilog.y		Verilog2C++
	cp verilog.h		Verilog2C++
	cp Verilog.cc		Verilog2C++
	cp Verilog.hh		Verilog2C++
	cp Verilog2C++.cc	Verilog2C++
	cp v2v.cc		Verilog2C++
	cp EmVer.hh		Verilog2C++
	tar zcvf Verilog2C++.tgz 	Verilog2C++
	rm -r 			Verilog2C++

Verilog2C++: parse.tab.o lexor.o Verilog.o Verilog2C++.cc Verilog.hh
	$(CXX) $(CXXFLAGS) Verilog.o parse.tab.o lexor.o Verilog2C++.cc -o Verilog2C++ -lfl

## Verilog2C: parse.tab.o lexor.o Verilog.o Verilog2C.cc Verilog.hh
##	$(CXX) $(CXXFLAGS) Verilog.o parse.tab.o lexor.o Verilog2C.cc -o Verilog2C -lfl
## Verilog2SystemC: parse.tab.o lexor.o Verilog.o Verilog2SystemC.cc Verilog.hh
##	$(CXX) $(CXXFLAGS) Verilog.o parse.tab.o lexor.o Verilog2SystemC.cc -o Verilog2SystemC -lfl

v2v: lexor.o parse.tab.o Verilog.o v2v.cc
	$(CXX) $(CXXFLAGS) Verilog.o parse.tab.o lexor.o v2v.cc -o v2v -lfl


parse.tab.o: verilog.y Verilog.hh verilog.h
	$(YACC) --verbose -t -p verilog_ -d verilog.y -b parse
	$(CXX) $(CXXFLAGS) -c parse.tab.c

lexor.o: verilog.l Verilog.hh
	$(LEX) -Pverilog_ -o_lexor.cc verilog.l
	cat Verilog.hh _lexor.cc > lexor.cc
	$(CXX) $(CXXFLAGS) -c lexor.cc

Verilog.o: Verilog.cc Verilog.hh
	$(CXX) $(CXXFLAGS) -c Verilog.cc

