CC = gcc
CFLAGS = -g mWall
OBJS1 = A.c
OBJS2 = B.c
OBJS3 = C.c
OBJS4 = D.c
OBJS5 = E.c
OBJS6 = F.c

all : A.o B.o C.o D.o E.o F.o
A.o : $(OBJS1)
	$(CC) -o A.o $(OBJS1)
B.o : $(OBJS2)
	$(CC) -o B.o $(OBJS2)
C.o : $(OBJS3)
	$(CC) -o C.o $(OBJS3)
D.o : $(OBJS4)
	$(CC) -o D.o $(OBJS4)
E.o : $(OBJS5)
	$(CC) -o E.o $(OBJS5) -lpthread
F.o : $(OBJS6)
	$(CC) -o F.o $(OBJS6) -lpthread

clean :
	rm *.o

