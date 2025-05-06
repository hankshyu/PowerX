SRCPATH = ./src
TEXO_SRCPATH = $(SRCPATH)/texo
PI_SRCPATH = $(SRCPATH)/pi
BINPATH = ./bin
OBJPATH = ./obj
BOOSTPATH = ./lib/boost_1_87_0

FLUTE_LIB_PATH = ./lib/flute/build
FLUTE_HEADER_PATH = ./lib/flute

GEOS_LIB_PATH = ./lib/geos/geos-install/lib
GEOS_HEADER_PATH = ./lib/geos/geos-install/include

OPENMP_OPT = -fopenmp


CXX = /opt/homebrew/bin/g++-14
# CXX = g++
OPTFLAGS = -O3
FLAGS = -std=c++17 -I$(SRCPATH) -I$(TEXO_SRCPATH) -I$(PI_SRCPATH) \
		-I$(BOOSTPATH) -I$(FLUTE_HEADER_PATH) -I$(GEOS_HEADER_PATH) $(OPENMP_OPT) -D_Alignof=alignof

LINKFLAGS = -L$(FLUTE_LIB_PATH) -L$(GEOS_LIB_PATH) -lm -lflute -lgeos -Wl,-rpath,lib/geos/geos-install/lib

INF_OBJS =	isotropy.o units.o interval.o cord.o fcord.o segment.o rectangle.o doughnutPolygon.o doughnutPolygonSet.o \
			tile.o line.o lineTile.o orderedSegment.o \
			rectilinear.o cornerStitching.o
	
PI_OBJS =	technology.o eqCktExtractor.o signalType.o ballOut.o objectArray.o c4Bump.o microBump.o \
			powerDistributionNetwork.o voronoiPDNGen.o
# 			powerGrid.o aStarBaseline.o 


_OBJS = main.o timeProfiler.o visualiser.o $(INF_OBJS) $(PI_OBJS)

OBJS = $(patsubst %,$(OBJPATH)/%,$(_OBJS))
DBG_OBJS = $(patsubst %.o, $(OBJPATH)/%_dbg.o, $(_OBJS))

all: pwrx
debug: pwrx_dbg

pwrx: $(OBJS)
	$(CXX) $(FLAGS) $(LINKFLAGS) $^ -o $(BINPATH)/$@

$(OBJPATH)/main.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) $(OPTFLAGS) -c -DCOMPILETIME="\"`date`\""  $^ -o $@

$(OBJPATH)/%.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(OPTFLAGS) -c $< -o $@

$(OBJPATH)/%.o: $(TEXO_SRCPATH)/%.cpp $(TEXO_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(OPTFLAGS) -c $< -o $@

$(OBJPATH)/%.o: $(PI_SRCPATH)/%.cpp $(PI_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(OPTFLAGS) -c $< -o $@



pwrx_dbg: $(DBG_OBJS)
	$(CXX) $(FLAGS) $(LINKFLAGS) $^ -g -o $(BINPATH)/$@

$(OBJPATH)/main_dbg.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) -O0 -g -c -DCOMPILETIME="\"`date`\"" $^ -o $@

$(OBJPATH)/%_dbg.o: $(SRCPATH)/%.cpp $(SRCPATH)/%.hpp
	$(CXX) $(FLAGS) -O0 -g -c $< -o $@

$(OBJPATH)/%_dbg.o: $(TEXO_SRCPATH)/%.cpp $(TEXO_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) -O0 -g -c $< -o $@

$(OBJPATH)/%_dbg.o: $(PI_SRCPATH)/%.cpp $(PI_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) -O0 -g -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJPATH)/* $(BINPATH)/* 