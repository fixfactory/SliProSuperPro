#!/bin/sh
find ./Source -iname *.cpp -o -iname *.h | xargs clang-format -style=File -i -verbose