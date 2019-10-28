#ifndef CHAINING_H_
#define CHAINING_H_

#include "../readfile.h"
#include<utility>

//Data Record inside the file
struct DataItem {
   int valid;    //) means invalid record, 1 = valid record
   int data;
   int key;
};

struct OverflowRecord : public DataItem{
   int next = -1;
};


//Each bucket contains number of records
struct Bucket {
	struct DataItem dataItem[RECORDSPERBUCKET];
   int recordPointer = -1;
   int valid = 1; // so if you write any bucket, it will be valid by default
};

struct OverflowBucket{
	struct OverflowRecord dataItem[RECORDSPERBUCKET];
};

#define OVERFLOWBUCKETS 5
#define MAINBUCKETS 5


#endif