
# $Id: Makefile,v 1.6 2013-09-24 19:07:20-07 sglaser Exp $

BIN=/home/sglaser/bin

all : $(BIN)/c2ps $(BIN)/count

$(BIN)/c2ps: c2ps.cpp Makefile
	g++ -o $(BIN)/c2ps -O c2ps.cpp

$(BIN)/count : count.cpp Makefile
	g++ -o $(BIN)/count -O count.cpp

print : print.pdf

SRC = Makefile count.cpp c2ps.cpp

print.ps : $(BIN)/c2ps $(SRC)
	$(BIN)/c2ps -o $@ $(SRC) 

print.pdf : print.ps
	ps2pdf print.ps
	rm -f print.ps

clean :
	rm -f print.ps print.pdf

