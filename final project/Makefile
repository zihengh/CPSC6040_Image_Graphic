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

all:  clean MattingFunc.o SharedMatting.o  Matting.o matt  alphamask.o 

alphamask.o :  alphamask.h
	${CC} ${CFLAGS} -c alphamask.cpp

MattingFunc.o : MattingFunc.h
	${CC} ${CFLAGS} -c MattingFunc.cpp


SharedMatting.o :  SharedMatting.h MattingFunc.h
	${CC} ${CFLAGS} -c SharedMatting.cpp

Matting.o : SharedMatting.h 
	${CC} ${CFLAGS} -c Matting.cpp

matt : alphamask.o MattingFunc.o SharedMatting.o  Matting.o 
	${CC} ${CFLAGS} alphamask.o MattingFunc.o SharedMatting.o  Matting.o ${LDFLAGS} -o matt

clean:
	rm -rf *.o
	rm -rf matt
