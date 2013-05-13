CXX=$(shell root-config --cxx)
CFLAGS=-c -g -Wall $(shell root-config --cflags) -I./src -I ./include
LDLIBS=$(shell root-config --glibs)
LDFLAGS=$(shell root-config --ldflags)
#SOURCES=./src/SL_Event.cc ./src/FileManager.cc ./src/Filter.cc
SOURCES=$(shell ls ./src/*.cc)
TEMP=$(shell ls ./src/*.cc~)
TEMP2=$(shell ls ./include/*.hh~)
OBJECTS=$(SOURCES:.cc=.o) 

EXE= GammaBuilder
EXECUTABLE= $(EXE)$(LDFLAGS)
MAIN=$(addsuffix .C,$(EXE))
MAINO=./src/$(addsuffix .o,$(EXE))


INCLUDEPATH=include
SRCPATH=src
ROOTCINT=rootcint

EVENTLIBPATH=/mnt/daqtesting/lenda/sam_analysis/LendaEvent/
LIB=LendaEvent
EVENTLIB=$(addsuffix $(LDFLAGS),$(LIB))

.PHONY: clean get put all test sclean

all: $(EXECUTABLE) 

$(EXECUTABLE) : $(OBJECTS) $(MAINO)
	@echo "Building target" $@ "..." 
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(MAINO) $(LDLIBS) -L$(EVENTLIBPATH) -l$(EVENTLIB)
	@echo
	@echo "Build succeed"


.cc.o:
	@echo "Compiling" $< "..."
	@$(CXX) $(CFLAGS) $< -o $@ 

$(MAINO): $(MAIN)
	@echo "Compiling" $< "..."
	@$(CXX) $(CFLAGS) $< -o $@  



clean:
	-rm -f ./$(OBJECTS)
	-rm -f ./$(EXECUTABLE)
	-rm -f ./$(MAINO)

sclean:
	-rm  ./$(TEMP)
	-rm  ./$(TEMP2)
	-rm  ./$(OBJECTS)
	-rm  ./$(EXECUTABLE)
	-rm  ./$(MAINO)

