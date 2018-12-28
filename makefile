CXX = g++
CFLAGS = 
LIBS = -lpthread
crows: crows.c
	$(CXX) crows.c -o crows $(CFLAGS) $(LIBS)
clean:
	rm crows