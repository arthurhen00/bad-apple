#!/bin/bash

rm -rf build
cmake -B build
cmake --build build

rm -rf build-web
emcmake cmake -B build-web
cmake --build build-web