# build options
CXX=g++
LD=g++
DEBUG 	= -O0 -g -pg
OPTIM	= -O3

# flags common to all builds
CFLAGS = -std=c++14 -Wall -Werror -pedantic -Wno-sign-compare -Wno-unused
LINKFLAGS=

ifeq ($(BUILD), debug)
BUILDFLAGS = $(CFLAGS) $(DEBUG)
else
BUILDFLAGS = $(CFLAGS) $(OPTIM)
endif

BRP_EXE  = brp

all: $(BRP_EXE)

BRP_SRC = \
brpstate.cpp \
branchandbound.cpp \
brppolicy.cpp \
dfbb.cpp \
fastmeta.cpp \
genpolicy.cpp \
glah.cpp \
la_n.cpp \
main.cpp \
petering.cpp \
pilotmethod.cpp \
rakesearch.cpp \
safemoves.cpp \
subsequence.cpp \

BRP_OBJ = $(BRP_SRC:%.cpp=%.o) 

$(BRP_EXE):  $(BRP_OBJ)
	$(LD) $(LINKFLAGS) $(BRP_OBJ) -o $(BRP_EXE)

%.o:%.cpp *.h
	$(CXX) -c $(BUILDFLAGS) $< -o $(<:%.cpp=%.o)

clean:
	rm -f *.o $(BRP_EXE) && chmod -x *.h *.cpp Makefile
