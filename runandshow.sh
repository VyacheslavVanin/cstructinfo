#! /bin/bash
./run data/example.c | dot -Tpng -o a.png && feh a.png
