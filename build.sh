#!/bin/bash
if [ ! -d "bin" ]; then
	mkdir bin
fi
g++ -std=c++1y src/unixify.cpp -o bin/unixify
