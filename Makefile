SRCPATH = ./src
INF_SRCPATH = $(SRCPATH)/infrastructure
PI_SRCPATH = $(SRCPATH)/pi
BINPATH = ./bin
OBJPATH = ./obj
BOOSTPATH = ./lib/boost_1_87_0/

# CXX = /usr/bin/g++
CXX = g++
FLAGS = -std=c++17 -I $(INF_SRCPATH) -I $(PI_SRCPATH) -DNDEBUG
CFLAGS = -c 
OPTFLAGS = -O3
DEBUGFLAGS = -g
LINKFLAGS = -lm


INF_OBJS = units.o \
	cord.o
	

PI_OBJS = eqCktExtractor.o

_OBJS = main.o $(INF_OBJS) $(PI_OBJS)

OBJS = $(patsubst %,$(OBJPATH)/%,$(_OBJS))
DBG_OBJS = $(patsubst %.o, $(OBJPATH)/%_dbg.o, $(_OBJS))

all: asprun
debug: asprun_debug

asprun: $(OBJS)
	$(CXX) $(FLAGS) $(LINKFLAGS) $^ -o $(BINPATH)/$@

$(OBJPATH)/main.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(CFLAGS) $(OPTFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

$(OBJPATH)/%.o: $(INF_SRCPATH)/%.cpp $(INF_SRCPATH)/%.h
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(CFLAGS) $(OPTFLAGS) $< -o $@

$(OBJPATH)/%.o: $(PI_SRCPATH)/%.cpp $(PI_SRCPATH)/%.h
	$(CXX) $(FLAGS) $(CFLAGS) $(OPTFLAGS) $< -o $@



asorun_debug: $(DBG_OBJS)
	$(CXX) $(FLAGS) $(DEBUGFLAGS) $(LINKFLAGS) $^ -o $(BINPATH)/$@

$(OBJPATH)/main_dbg.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

$(OBJPATH)/%_dbg.o: $(INF_SRCPATH)/%.cpp $(INF_SRCPATH)/%.h
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/%_dbg.o: $(PI_SRCPATH)/%.cpp $(PI_SRCPATH)/%.h
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -I $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJPATH)/* $(BINPATH)/* 