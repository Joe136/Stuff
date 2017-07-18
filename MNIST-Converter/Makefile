
PKG_CONFIG_PATH=/opt/OpenCV/master/lib/pkgconfig

all: mnist-converter

mnist-converter: mnist-converter.cpp
	g++ -std=c++14 -Wall -o mnist-converter mnist-converter.cpp $(shell pkg-config --libs --cflags opencv)
