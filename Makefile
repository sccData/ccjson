OBJS = test1.o test2.o
XX = g++
CFLAGS = -Wall -O -g

test : $(OBJS)
	$(XX) $(OBJS) -o test

test1.o : ccjson.cpp ccjson.h
	$(XX) $(CFLAGS) -c ccjson.cpp -o test1.o

test2.o : main.cpp minunit.h
	$(XX) $(CFLAGS) -c main.cpp -o test2.o

clean:
	rm -rf *.o test