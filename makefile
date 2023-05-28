arrary_size=100000

out:pj3.out
	mpirun -np 4 ./$<  ${arrary_size} >> test.output 

pj3.out:pj3.cpp 
	mpic++   $<   -o $@

clean:
	rm -f *.o *.out*