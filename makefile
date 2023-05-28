dim=512

out:pj3.out
	mpirun -np 1 ./$<  ${dim} >> test.output 
	mpirun -np 4 ./$<  ${dim} >> test.output
	mpirun -np 16 ./$<  ${dim} >> test.output  
pj3.out:pj3.cpp 
	mpic++   $<   -o $@

clean:
	rm -f *.o *.out*