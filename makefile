
out:pj2.out
	 mpirun -np 8 ./$<  10000000 > test.output 
	
pj2.out:pj2.cpp 
	mpic++   $<   -o $@

clean:
	rm -f *.o *.out*