g++ openaddressing/openAddressing.cpp readfile.cpp openaddressing/main.cpp -o bin/openaddressing.out

rm ./db/openaddressing
./bin/openaddressing.out tests/$1.txt