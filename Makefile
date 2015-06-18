
FLAGS = -std=c++11 -O2
OBJS = mashing.cpp
OBJ_NAME = mashing
LINKER_FLAGS = -lSDL2 -lSDL2_ttf
all : $(OBJS)
	g++ $(FLAGS) $(OBJS) $(LINKER_FLAGS) -o $(OBJ_NAME)
