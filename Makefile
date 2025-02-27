SRCPATH = ./src
TEXO_SRCPATH = $(SRCPATH)/texo
PI_SRCPATH = $(SRCPATH)/pi
BINPATH = ./bin
OBJPATH = ./obj
BOOSTPATH = ./lib/boost_1_87_0/

# OpenCV Paths
OPENCV_INCLUDE = -I /opt/homebrew/opt/opencv/include/opencv4
OPENCV_LIBS = -L /opt/homebrew/opt/opencv/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

# CXX = /usr/bin/g++
CXX = g++
FLAGS = -std=c++17 -I $(TEXO_SRCPATH) -I $(PI_SRCPATH) -DNDEBUG $(OPENCV_INCLUDE)
CFLAGS = -c 
OPTFLAGS = -O3
DEBUGFLAGS = -g
LINKFLAGS = -lm $(OPENCV_LIBS)

INF_OBJS =	isotropy.o units.o cord.o fcord.o interval.o line.o rectangle.o tile.o #doughnutPolygon.o doughnutPolygonSet.o 

	
PI_OBJS = eqCktExtractor.o

_OBJS = main.o $(INF_OBJS) $(PI_OBJS)

OBJS = $(patsubst %,$(OBJPATH)/%,$(_OBJS))
DBG_OBJS = $(patsubst %.o, $(OBJPATH)/%_dbg.o, $(_OBJS))

all: pwrx
debug: pwrx_dbg

pwrx: $(OBJS)
	$(CXX) $(FLAGS) $(LINKFLAGS) $(OPENCV_FLAGS) $^ -o $(BINPATH)/$@

$(OBJPATH)/main.o: $(SRCPATH)/main.cpp 
	$(CXX) $(FLAGS) -I $(BOOSTPATH) $(CFLAGS) $(OPENCV_FLAGS) $(OPTFLAGS) -DCOMPILETIME="\"`date`\"" $^ -o $@

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