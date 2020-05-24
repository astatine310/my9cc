CFLAGS=-static -g -Wall -std=c++1z
SRCS=main.cc tokenize.cc parser.cc codegen.cc mylib.cc
OBJS=$(SRCS:.cc=.o)

my9cc: $(OBJS)
	g++ -o my9cc $(OBJS) $(LDFLAGS)

main.o: main.cc my9cc.h parser.h mylib.h
	g++ -c -o $@ $< $(CFLAGS)

tokenize.o: tokenize.cc my9cc.h mylib.h
	g++ -c -o $@ $< $(CFLAGS)

parser.o: parser.cc my9cc.h parser.h
	g++ -c -o $@ $< $(CFLAGS)

codegen.o: codegen.cc my9cc.h
	g++ -c -o $@ $< $(CFLAGS)

mylib.o: mylib.cc mylib.h
	g++ -c -o $@ $< $(CFLAGS)

test: my9cc
	./test.sh

clean:
	rm -f my9cc *.o *~ tmp*

.PHONY: test clean
