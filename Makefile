CXX=$(shell root-config --cxx)
CFLAGS=-c -g -Wall $(shell root-config --cflags) -I./src -I ./include
LDLIBS=$(shell root-config --glibs)
LDFLAGS=$(shell root-config --ldflags)
#SOURCES=./src/SL_Event.cc ./src/FileManager.cc ./src/Filter.cc
SOURCES=$(shell ls ./src/*.cc)
TEMP=$(shell ls ./src/*.cc~)
TEMP2=$(shell ls ./include/*.hh~)
OBJECTS=$(SOURCES:.cc=.o) 
MAIN=GammaBuilder.C
MAINO=./src/GammaBuilder.o

EXECUTABLE=GammaBuilder

INCLUDEPATH=include
SRCPATH=src
ROOTCINT=rootcint
DICTNAME    = SL_Event
DICTOBJ     = $(addsuffix Dictionary.o, $(DICTNAME))

EVENTLIB=/mnt/daqtesting/lenda/sam_analysis/LendaEvent/
.PHONY: clean get put all test sclean

all: $(EXECUTABLE) 

$(EXECUTABLE) : $(OBJECTS) $(MAINO)
	@echo "Building target" $@ "..." 
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(MAINO) $(LDLIBS) -L$(EVENTLIB) -lLendaEvent
	@echo
	@echo "Build succeed"


.cc.o:
	@echo "Compiling" $< "..."
	@$(CXX) $(CFLAGS) $< -o $@ 

$(MAINO): $(MAIN)
	@echo "Compiling" $< "..."
	@$(CXX) $(CFLAGS) $< -o $@  


##%Dictionary.o: include/%.hh src/%LinkDef.h
##	$(ROOTCINT) -f $(patsubst %.o,%.cc,$@) -c $^;
##	$(CXX) -p -fPIC $(CFLAGS) -c $(patsubst %.o,%.cc,$@) $(patsubst %.o,%.h,$@)



clean:
	-rm ./$(OBJECTS)
	-rm ./$(EXECUTABLE)
	-rm ./$(MAINO)

sclean:
	-rm ./$(TEMP)
	-rm ./$(TEMP2)
	-rm ./$(OBJECTS)
	-rm ./$(EXECUTABLE)
	-rm ./$(MAINO)

