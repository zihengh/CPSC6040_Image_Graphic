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

all:  clean convolution.o convolve.o convolve

convolution.o : convolution.h
	${CC} ${CFLAGS} -c convolution.cpp

convolve.o : convolution.h
	${CC} ${CFLAGS} -c convolve.cpp

convolve : convolution.o convolve.o 
	${CC} ${CFLAGS} convolution.o convolve.o ${LDFLAGS} -o convolve

clean:
	rm -rf *.o
	rm -rf convolve
