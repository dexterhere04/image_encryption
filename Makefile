CXX = g++
CXXFLAGS = -Wall -O2 `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`

TARGET = image_scramble
OBJS = problem1.o arnold_cat_map.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

problem1.o: problem1.cpp arnold_cat_map.h
	$(CXX) $(CXXFLAGS) -c problem1.cpp

arnold_cat_map.o: arnold_cat_map.cpp arnold_cat_map.h
	$(CXX) $(CXXFLAGS) -c arnold_cat_map.cpp

clean:
	rm -f $(TARGET) $(OBJS) *.pgm *.bin
