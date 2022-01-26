CC = gcc
CFLAGS = -g -Wall
OBJS1 = A.c
OBJS2 = B.c
OBJS3 = C.c
OBJS4 = D.c
OBJS5 = E.c
OBJS6 = F.c
OBJS7 = log.c

all : A.o B.o C.o D.o E.o F.o
A.o : $(OBJS1)
	$(CC) -o A.o $(OBJS1) $(OBJS7)
B.o : $(OBJS2)
	$(CC) -o B.o $(OBJS2) $(OBJS7)
C.o : $(OBJS3)
	$(CC) -o C.o $(OBJS3) $(OBJS7)
D.o : $(OBJS4)
	$(CC) -o D.o $(OBJS4) $(OBJS7)
E.o : $(OBJS5)
	$(CC) -o E.o $(OBJS5) $(OBJS7) -lpthread
F.o : $(OBJS6)
	$(CC) -o F.o $(OBJS6) $(OBJS7) -lpthread

clean :
	rm *.o

