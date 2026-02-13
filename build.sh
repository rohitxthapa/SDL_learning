#!/usr/bin/env bash
gcc game.c -o game -I. -I/usr/local/include -I/usr/include -std=c11 -Wall -lSDL3
./game
