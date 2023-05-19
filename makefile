arrary_size=10000000

out:pj2.out
	mpirun -np 1 ./$<  ${arrary_size} >> test.output 
	mpirun -np 2 ./$<  ${arrary_size} >> test.output 
	mpirun -np 4 ./$<  ${arrary_size} >> test.output 
	mpirun -np 6 ./$<  ${arrary_size} >> test.output 
	mpirun -np 7 ./$<  ${arrary_size} >> test.output 
	mpirun -np 8 ./$<  ${arrary_size} >> test.output 


pj2.out:pj2.cpp 
	mpic++   $<   -o $@

clean:
	rm -f *.o *.out*