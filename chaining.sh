g++ chaining/chaining.cpp readfile.cpp chaining/main.cpp -o bin/chaining.out

rm ./db/chaining
./bin/chaining.out tests/$1.txt