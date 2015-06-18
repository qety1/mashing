
SYS := $(shell gcc -dumpmachine)

ifneq (, $(findstring linux, $(SYS)))
CC = g++
LINKER_FLAGS = -lSDL2 -lSDL2_ttf
else ifneq (, $(findstring cygwin, $(SYS)))
LINKER_FLAGS = -lmingw32 -lSDL2 -lSDL2_ttf -static-libgcc -static-libstdc++ -mwindows
CC = i686-pc-mingw32-g++.exe
else
$(error Platform is not supported)
endif

FLAGS = -std=c++11 -O2
OBJS = mashing.cpp
OBJ_NAME = mashing

all : $(OBJS)
	$(CC) $(FLAGS) $(OBJS) $(LINKER_FLAGS) -o $(OBJ_NAME)
