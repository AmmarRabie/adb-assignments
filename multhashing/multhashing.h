#ifndef MULTI_HASHING_H_
#define MULTI_HASHING_H_

#include "../readfile.h"

//Data Record inside the file
struct DataItem {
   int valid;    //) means invalid record, 1 = valid record
   int data;
   int key;
};


//Each bucket contains number of records
struct Bucket {
	struct DataItem  dataItem[RECORDSPERBUCKET];

};



#endif