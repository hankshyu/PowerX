SRCPATH = ./src
TEXO_SRCPATH = $(SRCPATH)/texo
PI_SRCPATH = $(SRCPATH)/pi
BINPATH = ./bin
OBJPATH = ./obj
BOOSTPATH = ./lib/boost_1_87_0

CADICAL_LIB_PATH = ./lib/cadical/build
CADICAL_HEADER_PATH = ./lib/cadical/src

CBC_LIB_INCLUDE = /opt/homebrew/opt/cbc/include/cbc #Cbc include files
CBCDEP_COINUTILS_INCLUDE = /opt/homebrew/opt/coinutils/include/coinutils/coin #Cbc dependencies: Coinutils
CBCDEP_OSI_INCLUDE = /opt/homebrew/opt/osi/include/osi/coin # Cbc depnedencies: osi
CBCDEP_CLP_INCLUDE = /opt/homebrew/opt/clp/include/clp/coin # Cbc dependencies: clp
CBC_LIB_PATH =  /opt/homebrew/opt/cbc/lib #Cbc library file
CBCDEP_LIB_PATH =  /opt/homebrew/lib/
CBC_LINKS = -lCbc -lCbcSolver -lOsiClp -lClp -lCoinUtils -lOsi #Cbc link libraries


# CXX = /usr/bin/g++
CXX = g++
FLAGS = -std=c++17 -I$(SRCPATH) -I$(TEXO_SRCPATH) -I$(PI_SRCPATH) -I$(BOOSTPATH) -I$(CADICAL_HEADER_PATH) -I$(CBC_LIB_INCLUDE) -I$(CBCDEP_COINUTILS_INCLUDE) -I$(CBCDEP_OSI_INCLUDE) -I$(CBCDEP_CLP_INCLUDE)
OPTFLAGS = -O3
LINKFLAGS = -L$(CADICAL_LIB_PATH) -L$(CBC_LIB_PATH)  -L$(CBCDEP_LIB_PATH) -lm -lcadical $(CBC_LINKS)

INF_OBJS =	isotropy.o units.o interval.o cord.o fcord.o segment.o rectangle.o doughnutPolygon.o doughnutPolygonSet.o \
			tile.o line.o lineTile.o orderedSegment.o \
			rectilinear.o cornerStitching.o 
	
PI_OBJS =	technology.o eqCktExtractor.o signalType.o ballOut.o pinMap.o microBump.o c4Bump.o \
			aStarBaseline.o


_OBJS = main.o timeProfiler.o visualiser.o $(INF_OBJS) $(PI_OBJS)

OBJS = $(patsubst %,$(OBJPATH)/%,$(_OBJS))
DBG_OBJS = $(patsubst %.o, $(OBJPATH)/%_dbg.o, $(_OBJS))

all: pwrx
debug: pwrx_dbg

pwrx: $(OBJS)
	$(CXX) $(FLAGS) $(LINKFLAGS) $^ -o $(BINPATH)/$@

$(OBJPATH)/main.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) $(OPTFLAGS) -c -DCOMPILETIME="\"`date`\"" $^ -o $@

$(OBJPATH)/%.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(OPTFLAGS) -c $< -o $@

$(OBJPATH)/%.o: $(TEXO_SRCPATH)/%.cpp $(TEXO_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(OPTFLAGS) -c $< -o $@

$(OBJPATH)/%.o: $(PI_SRCPATH)/%.cpp $(PI_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(OPTFLAGS) -c $< -o $@



pwrx_dbg: $(DBG_OBJS)
	$(CXX) $(FLAGS) $(LINKFLAGS) $^ -g -o $(BINPATH)/$@

$(OBJPATH)/main_dbg.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) -g -c -DCOMPILETIME="\"`date`\"" $^ -o $@

$(OBJPATH)/%_dbg.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.hpp
	$(CXX) $(FLAGS) -g -c $< -o $@

$(OBJPATH)/%_dbg.o: $(TEXO_SRCPATH)/%.cpp $(TEXO_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) -g -c $< -o $@

$(OBJPATH)/%_dbg.o: $(PI_SRCPATH)/%.cpp $(PI_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) -g -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJPATH)/* $(BINPATH)/* 