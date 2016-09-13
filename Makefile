CCOMMAND = g++
CFLAGS = -Wall -c -Wextra --std=c++11 
LINKARGS = -lpthread
SOURCES = $(wildcard ./src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
EXE_NAME = FTPServer

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_S),Linux)
ifeq ($(UNAME_M),armv7l)
	LINKARGS += ./lib_linux_arm/jobdispatcher.a ./lib_linux_arm/socketwrapper.a
endif
endif

ifeq ($(UNAME_S),Darwin)
	LINKARGS += ./lib_osx/jobdispatcher.a ./lib_osx/socketwrapper.a
endif


$(EXE_NAME): $(OBJECTS)
	$(CCOMMAND) $(OBJECTS) $(LINKARGS) -o $(EXE_NAME)

%.o: %.cpp
	$(CCOMMAND) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(EXE_NAME) $(OBJECTS)
