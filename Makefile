SRCPATH = ./src
TEXO_SRCPATH = $(SRCPATH)/texo
PI_SRCPATH = $(SRCPATH)/pi
PRESSUREMODEL_SRCPATH = $(SRCPATH)/pressureModel
DIFFUSIONMODEL_SRCPATH = $(SRCPATH)/diffusionModel
BINPATH = ./bin
OBJPATH = ./obj
BOOSTPATH = ./lib/boost_1_88_0

FLUTE_LIB_PATH = ./lib/flute/build
FLUTE_HEADER_PATH = ./lib/flute

GEOS_LIB_PATH = ./lib/geos/build/lib
GEOS_HEADER_PATH = ./lib/geos/include

GUROBI_INCLUDE_PATH = /Library/gurobi1203/macos_universal2/include
GUROBI_LIB_PATH = /Library/gurobi1203/macos_universal2/lib

OPENMP_LIB_PATH = /opt/homebrew/opt/libomp
OPENMP_COMPILE_FLAGS = -Xpreprocessor -fopenmp -I$(OPENMP_LIB_PATH)/include
OPENMP_LINK_FLAGS = -L$(OPENMP_LIB_PATH)/lib -lomp

# CXX = /opt/homebrew/opt/gcc/bin/g++-15
CXX = clang++

OPTFLAGS = -O2
FLAGS = -std=c++17 -stdlib=libc++ -I$(SRCPATH) -I$(TEXO_SRCPATH) -I$(PI_SRCPATH) -I$(PRESSUREMODEL_SRCPATH) -I$(DIFFUSIONMODEL_SRCPATH) \
		-I$(BOOSTPATH) -I$(FLUTE_HEADER_PATH) -I$(GEOS_HEADER_PATH) -I$(GUROBI_INCLUDE_PATH) $(OPENMP_COMPILE_FLAGS) -D_Alignof=alignof

LINKFLAGS = -L$(FLUTE_LIB_PATH) -L$(GEOS_LIB_PATH) -L$(GUROBI_LIB_PATH) $(OPENMP_LINK_FLAGS) -lm -lgurobi_c++ -lgurobi120 \
		$(FLUTE_LIB_PATH)/libflute.a $(GEOS_LIB_PATH)/libgeos.a $(GEOS_LIB_PATH)/libgeos_c.a

INF_OBJS =	isotropy.o interval.o cord.o fcord.o segment.o rectangle.o doughnutPolygon.o doughnutPolygonSet.o \
			tile.o line.o lineTile.o orderedSegment.o \
			rectilinear.o cornerStitching.o
	
PI_OBJS =	technology.o eqCktExtractor.o signalType.o ballOut.o objectArray.o c4Bump.o microBump.o \
			powerDistributionNetwork.o \
			voronoiPDNGen.o

PRESSUREMODEL_OBJS = 	fpoint.o fbox.o fpolygon.o fmultipolygon.o \
						viaBody.o softBody.o \
						pressureSimulator.o

DIFFUSIONMODEL_OBJS =	diffusionChamber.o metalCell.o viaCell.o flowNode.o flowEdge.o diffusionEngine.o

_OBJS = main.o timeProfiler.o visualiser.o units.o $(INF_OBJS) $(PI_OBJS) $(PRESSUREMODEL_OBJS) $(DIFFUSIONMODEL_OBJS)

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

$(OBJPATH)/%.o: $(PRESSUREMODEL_SRCPATH)/%.cpp $(PRESSUREMODEL_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) $(OPTFLAGS) -c $< -o $@

$(OBJPATH)/%.o: $(DIFFUSIONMODEL_SRCPATH)/%.cpp $(DIFFUSIONMODEL_SRCPATH)/%.hpp
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

$(OBJPATH)/%_dbg.o: $(PRESSUREMODEL_SRCPATH)/%.cpp $(PRESSUREMODEL_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) -O0 -g -c $< -o $@

$(OBJPATH)/%_dbg.o: $(DIFFUSIONMODEL_SRCPATH)/%.cpp $(DIFFUSIONMODEL_SRCPATH)/%.hpp
	$(CXX) $(FLAGS) -O0 -g -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJPATH)/* $(BINPATH)/* 