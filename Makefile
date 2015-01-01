# 64 bit
MARCH=-m64
# release vs debug build
OPT=-g3 -O1
#OPT=-O3 -DNDEBUG


INCLUDE=-Iraytracer-core -I3rdparty/glm

LINK= -lSDL2 -lassimp -lgomp

CXX=g++

CXXFLAGS=-fopenmp -w -std=c++11 $(OPT) $(MARCH) $(INCLUDE)


LIB_CXXSRCS = $(wildcard raytracer-core/*.cpp)
LIBOBJS = $(addprefix obj/, $(subst /,/,$(LIB_CXXSRCS:.cpp=.obj)))

BIN_CXXSRCS = $(wildcard raytracer-sdl/*.cpp)
BINOBJS = $(addprefix obj/, $(subst /,/,$(BIN_CXXSRCS:.cpp=.obj)))


bin/raytracer: dirs $(BINOBJS) lib/raytracer-core.a
	$(CXX) $(CXXFLAGS) -o $@ $(BINOBJS) lib/raytracer-core.a $(LINK)

lib/raytracer-core.a: $(LIBOBJS) dirs
	@echo "Building the core rendering library"
	@ar rcs $@ $(LIBOBJS)

obj/raytracer-core/%.obj: raytracer-core/%.cpp dirs
	@echo "Building core library object $@"
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/raytracer-sdl/%.obj: raytracer-sdl/%.cpp dirs
	@echo "Building program object $@"
	$(CXX) $(CXXFLAGS) -o $@ -c $<

dirs:
	@mkdir -p obj
	@mkdir -p obj/raytracer-core
	@mkdir -p obj/raytracer-sdl
	@mkdir -p lib
	@mkdir -p bin

clean:
	rm ./bin -r
	rm ./obj -r
	rm ./lib
