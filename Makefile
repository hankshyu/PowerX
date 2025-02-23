SRCPATH = ./src
TEXO_SRCPATH = $(SRCPATH)/texo
PI_SRCPATH = $(SRCPATH)/pi
BINPATH = ./bin
OBJPATH = ./obj
BOOSTPATH = ./lib/boost_1_87_0/

# CXX = /usr/bin/g++
CXX = g++
FLAGS = -std=c++17 -I $(TEXO_SRCPATH) -I $(PI_SRCPATH) -DNDEBUG
CFLAGS = -c 
OPTFLAGS = -O3
DEBUGFLAGS = -g
LINKFLAGS = -lm


INF_OBJS = isotropy.o units.o cord.o fcord.o interval.o

	
PI_OBJS = eqCktExtractor.o

_OBJS = main.o $(INF_OBJS) $(PI_OBJS)

OBJS = $(patsubst %,$(OBJPATH)/%,$(_OBJS))
DBG_OBJS = $(patsubst %.o, $(OBJPATH)/%_dbg.o, $(_OBJS))

all: pwrx
debug: pwrx_dbg

pwrx: $(OBJS)
	$(CXX) $(FLAGS) $(LINKFLAGS) $^ -o $(BINPATH)/$@

$(OBJPATH)/main.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(CFLAGS) $(OPTFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

$(OBJPATH)/%.o: $(TEXO_SRCPATH)/%.cpp $(TEXO_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(CFLAGS) $(OPTFLAGS) $< -o $@

$(OBJPATH)/%.o: $(PI_SRCPATH)/%.cpp $(PI_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(CFLAGS) $(OPTFLAGS) $< -o $@



pwrx_dbg: $(DBG_OBJS)
	$(CXX) $(FLAGS) $(DEBUGFLAGS) $(LINKFLAGS) $^ -o $(BINPATH)/$@

$(OBJPATH)/main_dbg.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

$(OBJPATH)/%_dbg.o: $(TEXO_SRCPATH)/%.cpp $(TEXO_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -I $(BOOSTPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/%_dbg.o: $(PI_SRCPATH)/%.cpp $(PI_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -I $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJPATH)/* $(BINPATH)/* 