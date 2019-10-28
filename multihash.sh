g++ multhashing/multhashing.cpp readfile.cpp multhashing/main.cpp -o bin/multhashing.out

rm ./db/multhashing
./bin/multhashing.out tests/$1.txt