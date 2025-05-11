CXX = g++
INCLUDES = -Isrc/include
LIBS = -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SOURCES = Main.cpp ConvertSketch.cpp DrawMap.cpp DrawText.cpp Ghost.cpp GhostManager.cpp MapCollision.cpp Pacman.cpp
OBJECTS = Main.o ConvertSketch.o DrawMap.o DrawText.o Ghost.o GhostManager.o MapCollision.o Pacman.o
TARGET = sfml-app

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

Main.o: Main.cpp
	$(CXX) $(INCLUDES) -c Main.cpp -o Main.o

ConvertSketch.o: ConvertSketch.cpp
	$(CXX) $(INCLUDES) -c ConvertSketch.cpp -o ConvertSketch.o

DrawMap.o: DrawMap.cpp
	$(CXX) $(INCLUDES) -c DrawMap.cpp -o DrawMap.o

DrawText.o: DrawText.cpp
	$(CXX) $(INCLUDES) -c DrawText.cpp -o DrawText.o

Ghost.o: Ghost.cpp
	$(CXX) $(INCLUDES) -c Ghost.cpp -o Ghost.o

GhostManager.o: GhostManager.cpp
	$(CXX) $(INCLUDES) -c GhostManager.cpp -o GhostManager.o

MapCollision.o: MapCollision.cpp
	$(CXX) $(INCLUDES) -c MapCollision.cpp -o MapCollision.o

Pacman.o: Pacman.cpp
	$(CXX) $(INCLUDES) -c Pacman.cpp -o Pacman.o

clean:
	-@rm -f *.o $(TARGET)
