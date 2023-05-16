
out:pj2.out
	mpirun -np 1 ./$<  10000000 >> test.output 
	mpirun -np 2 ./$<  10000000 >> test.output 
	mpirun -np 4 ./$<  10000000 >> test.output 
	mpirun -np 6 ./$<  10000000 >> test.output 
	mpirun -np 7 ./$<  10000000 >> test.output 
	mpirun -np 8 ./$<  10000000 >> test.output 


pj2.out:pj2.cpp 
	mpic++   $<   -o $@

clean:
	rm -f *.o *.out*