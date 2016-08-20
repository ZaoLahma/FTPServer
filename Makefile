CCOMMAND = g++
CFLAGS = -Wall -c -Wextra --std=c++11 
LINKARGS = -lpthread
SRC_FILES = ./src/*.cpp
INC_DIRS = -I./inc
EXE_NAME = FTPServer

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LINKARGS += ./lib_linux_x86/jobdispatcher.a
endif
ifeq ($(UNAME_S),Darwin)
	LINKARGS += ./lib_osx/jobdispatcher.a ./lib_osx/socketwrapper.a
endif

all: compile link

compile:
	$(CCOMMAND) $(CFLAGS) $(INC_DIRS) $(SRC_FILES)
	
link:
	$(CCOMMAND) -o $(EXE_NAME) ./*.o $(LINKARGS)
	
clean:
	rm -rf ./*.o
	rm ./$(EXE_NAME)