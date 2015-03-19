
run: sample/sample
	$<

sample/sample: asynctest.h asynctest.cpp sample/sample.cpp
	gcc -O0 -m32 -std=c++11 -lc++ -I. -o $@ asynctest.cpp sample/sample.cpp
