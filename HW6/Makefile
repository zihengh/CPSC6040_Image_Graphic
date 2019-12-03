CC		= g++
C		= cpp

CFLAGS		= -g
LFLAGS		= -g

ifeq ("$(shell uname)", "Darwin")
  LDFLAGS     = -framework Foundation -framework GLUT -framework OpenGL -lOpenImageIO -lm
else
  ifeq ("$(shell uname)", "Linux")
    LDFLAGS   = -L /usr/lib64/ -lglut -lGL -lGLU -lOpenImageIO -lm
  endif
endif

all:  clean invmap.o OKWarp.o OKWarp

invmap.o : invmap.h
	${CC} ${CFLAGS} -c invmap.cpp

OKWarp.o : invmap.h
	${CC} ${CFLAGS} -c OKWarp.cpp

OKWarp : invmap.o OKWarp.o 
	${CC} ${CFLAGS} invmap.o OKWarp.o ${LDFLAGS} -o OKWarp

clean:
	rm -rf *.o
	rm -rf OKWarp
