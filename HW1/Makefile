CC		= g++
C		= cpp
H		= h
CFLAGS		= -Wall -g
LDFLAGS		= -lm

ifeq ("$(shell uname)", "Darwin")
  LDFLAGS     = -framework Foundation -framework GLUT -framework OpenGL -lOpenImageIO -lm
else
  ifeq ("$(shell uname)", "Linux")
    LDFLAGS   = -L /usr/lib64/ -lglut -lGL -lGLU -lOpenImageIO -lm
  endif
endif

all:  clean imgOperator.o main.o imgview

imgOperator.o : imgOperator.h
	${CC} ${CFLAGS} -c imgOperator.cpp

main.o : imgOperator.h
	${CC} ${CFLAGS} -c main.cpp

imgview : imgOperator.o main.o 
	${CC} ${CFLAGS} imgOperator.o main.o ${LDFLAGS} -o imgview

clean:
	rm -rf *.o
	rm -rf imgview
