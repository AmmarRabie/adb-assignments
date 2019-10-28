/*
 * readfile.h
 *
 *  Created on: Sep 20, 2016
 *      Author: dina
 */

#ifndef READFILE_H_
#define READFILE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include<iostream>

#define MBUCKETS 10					//Number of BUCKETS
#define RECORDSPERBUCKET 2				//No. of records inside each Bucket

//Data Record inside the file
struct DataItem;
//Each bucket contains number of records
struct Bucket;


// #define BUCKETSIZE sizeof(Bucket)		//Size of the bucket (in bytes)
// #define FILESIZE BUCKETSIZE*MBUCKETS    //Size of the file

extern const int BUCKETSIZE;
extern const int FILESIZE;

//Check the create File
int createFile(int size, char *);

//interefacing of each algorithm
int deleteItem(int key);
int insertItem(int fd,DataItem item);
int DisplayFile(int fd, std::ostream &out = std::cout);
int deleteOffset(int filehandle, int Offset);
int searchItem(int filehandle,struct DataItem* item,int *count);




#endif /* READFILE_H_ */
