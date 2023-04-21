
out:pj1.out
	./pj1.out  1000  > test.output
pj1.out:pj1.cpp
	g++  $<  -fopenmp -o $@

clean:
	rm -f *.o *.out*