CC		= g++
C		= cpp

CFLAGS		= -g -Wall 
LFLAGS		= -g 

ifeq ("$(shell uname)", "Darwin")
  LDFLAGS     = -framework Foundation -framework GLUT -framework OpenGL + -lm
else
  ifeq ("$(shell uname)", "Linux")
    LDFLAGS     = -L /usr/lib64/ -lglut -lGL  -lGLU -lOpenImageIO -lm
  endif
endif

all:  clean matrix.o NonlinearMap.o warper.o warper

NonlinearMap.o : NonlinearMap.h
	${CC} ${CFLAGS} -c NonlinearMap.cpp

matrix.o : matrix.h 
	${CC} ${CFLAGS} -c matrix.cpp

warper.o : matrix.h NonlinearMap.h
	${CC} ${CFLAGS} -c warper.cpp

warper : matrix.o NonlinearMap.o warper.o 
	${CC} ${CFLAGS} matrix.o NonlinearMap.o warper.o ${LDFLAGS} -o warper

clean:
	rm -rf *.o
	rm -rf warper
