#include "chaining.h"
#include "../readfile.h"
#include <iostream>
using namespace std;
/* Hash function to choose bucket
 * Input: key used to calculate the hash
 * Output: HashValue;
 */
const int BUCKETSIZE = sizeof(Bucket);
const int OVEFLOWBUCKETSIZE = sizeof(OverflowBucket);
const int FILESIZE = BUCKETSIZE*MAINBUCKETS + OVERFLOWBUCKETS * OVEFLOWBUCKETSIZE;

#define BucketOffset(o) (o / BUCKETSIZE) * BUCKETSIZE

OverflowRecord readOverflowRecord(int fd, int bucketOffset, int & count);
int hashCode(int key);
int getFreeOffsetInOverflow(int fd, int *count);
OverflowRecord getLastRecordChain(int overflowOffset, int *lastRecordOffset, int &count);
int searchChain(int fd, DataItem* item, int *count, int overflowOffset);
int displayMainBucket(int fd, Bucket b, bool, int bucketOffset, ostream& out);
Bucket readMainBucket(int fd, int bucketOffset);
OverflowRecord readOverflowRecord(int fd, int bucketOffset);


int hashCode(int key){
   return key % MAINBUCKETS;
}

int getFreeOffsetInOverflow(int fd, int *count) {
	cout << "getFreeOffsetInOverflow called ";
	int offset = BUCKETSIZE * MAINBUCKETS;
	while (offset <= FILESIZE){
		OverflowRecord res;
		ssize_t readResult = pread(fd, &res, sizeof(OverflowRecord), offset);
		(*count)++;
		if (readResult < 0)
		{
			perror("some error occurred in pread");
			cout << "-1" << endl;
			return -1;
		}
		else if (readResult == 0 || res.valid == 0){cout << offset; return offset;}
		offset += sizeof(OverflowRecord);
	}
	cout << -1 << endl;
	return -1;
}

OverflowRecord getLastRecordChain(int fd, int overflowOffset, int *lastRecordOffset, int &count) {
	int offset = overflowOffset;
	while (offset != -1)
	{
		auto r = readOverflowRecord(fd, offset, count);
		if (r.next == -1)
		{
			(*lastRecordOffset) = offset;
			return r;
		}
		offset = r.next;
	}
	(*lastRecordOffset) = -1;
	return OverflowRecord();
}

int searchChain(int fd, DataItem* item, int *count, int overflowOffset) {
	int offset = overflowOffset;
	while (offset != -1)
	{
		auto r = readOverflowRecord(fd, offset, *count);
		if (r.valid == 1 && r.key == item->key)
		{
			item->data = r.data;
			return offset;
		}
		offset = r.next;
	}
	return -1;
}

int displayMainBucket(int fd, Bucket b, bool empty, int bucketOffset, ostream &out) {
	int index = bucketOffset / BUCKETSIZE;
	for(int i = 0; i < RECORDSPERBUCKET; i++){
		if (b.dataItem[i].valid == 0)
		{
			out << "Bucket: " << index 
				<< ", Offset: " << bucketOffset + i * sizeof(DataItem)
				<< ", ~"
				<< endl;
		}
		else
		{
			out << "Bucket: " << index 
				<< ", Offset: " << bucketOffset + i * sizeof(DataItem)
				<< ", Data: " << b.dataItem[i].data 
				<< ", key: " << b.dataItem[i].key 
				<< endl;
		}
		
	}
	out << "Bucket: " << index << "==>";
	int next = b.recordPointer;
	while(next != -1 && b.valid){
		int overflowBucketIndex = (next - MAINBUCKETS * BUCKETSIZE) / sizeof(OverflowBucket);
		int overflowRecordIndex = ((next - MAINBUCKETS * BUCKETSIZE) - overflowBucketIndex * sizeof(OverflowBucket)) / sizeof(OverflowRecord);
		auto overflowBucket = readOverflowRecord(fd, next);
		out << "OF: " << overflowBucketIndex << '.' << overflowRecordIndex
			<< " Offset: " << next 
			<< ", Data: " << overflowBucket.data 
			<< ", key: " << overflowBucket.key 
			<< "==>";
		next = overflowBucket.next;
	}
	out << "NULL" << endl;
	return 0;
}

Bucket readMainBucket(int fd, int bucketOffset) {
	Bucket res;
	ssize_t readResult = pread(fd, &res, sizeof(Bucket), bucketOffset);
	if(readResult < 0){ perror("some error occurred in pread"); return res; }
	return res;
}

OverflowRecord readOverflowRecord(int fd, int bucketOffset) {
	OverflowRecord res;
	ssize_t readResult = pread(fd, &res, sizeof(OverflowRecord), bucketOffset);
	if(readResult < 0){ perror("some error occurred in pread"); return res; }
	return res;
}

OverflowRecord readOverflowRecord(int fd, int bucketOffset, int & count) {
	count++;
	return readOverflowRecord(fd, bucketOffset);
}
/* Functionality insert the data item into the correct position
 * Input:  fd: filehandler which contains the db
 *         item: the dataitem which should be inserted into the database
 *
 * Output: No. of record searched
 */
int insertItem(int fd, DataItem item){
   //Definitions
	struct DataItem data;   //a variable to read in it the records from the db
	int count = 0;				//No of accessed records
	int rewind = 0;			//A flag to start searching from the first bucket
	int hashIndex = hashCode(item.key);  				//calculate the Bucket index
	int startingOffset = hashIndex*sizeof(Bucket);		//calculate the starting address of the bucket
	int Offset = startingOffset;						//Offset variable which we will use to iterate on the db

	cout << "try to insert " << item.key << ", " << item.data << " in bucket " << hashIndex << endl;;

	ssize_t readResult, writeResult; // store the result of pread, pwrite

	// search for main bucket first for free records
	Bucket mainBucket;
	readResult = pread(fd, &mainBucket, sizeof(Bucket), Offset);
	if(readResult < 0) return -1;
	if (readResult == 0 || !mainBucket.valid) {
		mainBucket.dataItem[0] = item;
		mainBucket.recordPointer = -1;
		mainBucket.valid = 1;
		writeResult = pwrite(fd, &mainBucket, sizeof(Bucket), Offset);
		return ++count;
	}
	int next = mainBucket.recordPointer; // next pointer to overflow bucket
	if (next == -1) {
		// may be there is a free space here in the main bucket
		for(int i = 0; i < RECORDSPERBUCKET; i++) {
			count++;
			if (mainBucket.dataItem[i].valid == 0) {
				writeResult = pwrite(fd, &item, sizeof(DataItem), Offset + i * sizeof(DataItem));
				return count;
			}
		}
	}
	// get empty space in the overflow
	int writeOffset = getFreeOffsetInOverflow(fd, &count);
	if (writeOffset == -1) return -1; // no empty spaces

	// write the new record, point to first record in the old chain (even if null)
	OverflowRecord newrecord;
	newrecord.data = item.data;
	newrecord.key = item.key;
	newrecord.valid = 1;
	newrecord.next = next;
	writeResult = pwrite(fd, &newrecord, sizeof(OverflowRecord), writeOffset);

	// update the main bucket, point to new record
	mainBucket.recordPointer = writeOffset;
	writeResult = pwrite(fd, &mainBucket, sizeof(Bucket), startingOffset);
	return count;
}


/* Functionality: using a key, it searches for the data item
 *          1. use the hash function to determine which bucket to search into
 *          2. search for the element starting from this bucket and till it find it.
 *          3. return the number of records accessed (searched)
 *
 * Input:  fd: filehandler which contains the db
 *         item: the dataitem which contains the key you will search for
 *               the dataitem is modified with the data when found
 *         count: No. of record searched
 *
 * Output: the in the file where we found the item
 */

int searchItem(int fd, struct DataItem* item, int *count)
{
	//Definitions
	struct DataItem data;   //a variable to read in it the records from the db
	*count = 0;				//No of accessed records
	int hashIndex = hashCode(item->key);  				//calculate the Bucket index
	int startingOffset = hashIndex*sizeof(Bucket);		//calculate the starting address of the bucket
	int Offset = startingOffset;						//Offset variable which we will use to iterate on the db

	ssize_t readResult, writeResult; // store the result of pread, pwrite


	// search for main bucket first for free records
	Bucket mainBucket;
	readResult = pread(fd, &mainBucket, sizeof(Bucket), Offset);
	if(readResult <= 0 || !mainBucket.valid) return -1;
	
	for(int i = 0; i < RECORDSPERBUCKET; i++) {
		DataItem currentRecord = mainBucket.dataItem[i];
		(*count)++;
		if (currentRecord.valid == 1 && currentRecord.key == item->key) {
			item->data = currentRecord.data;
			return Offset + i * sizeof(DataItem);
		}
	}
	// not in the main bucket
	int next = mainBucket.recordPointer; // next pointer to overflow bucket
	if (next == -1) return -1; // no overflow, so the element doesn't exist

	return searchChain(fd, item, count, next);
}


/* Functionality: Display all the file contents
 *
 * Input:  fd: filehandler which contains the db
 *
 * Output: no. of non-empty records
 */
int DisplayFile(int fd, ostream &out){

	struct Bucket mainBucket;
	int count = 0;
	int currentOffset = 0;
	for(int i = 0; i < MAINBUCKETS; i++, currentOffset += sizeof(Bucket))
	{
		ssize_t result = pread(fd, &mainBucket, sizeof(Bucket), currentOffset);
		if(result < 0){ perror("some error occurred in pread"); return -1; }
		// cout << "result is " << result << endl;
		displayMainBucket(fd, mainBucket, result == 0, currentOffset, out);
	}
	
	// TODO: implement it correctly
	return count;
}


/* Functionality: Delete item at certain offset
 *
 * Input:  fd: filehandler which contains the db
 *         Offset: place where it should delete
 *
 * Hint: you could only set the valid key and write just and integer instead of the whole record
 */
int deleteOffset(int fd, int Offset)
{
	OverflowRecord dummyItem;
	
	ssize_t readResult, writeResult; // store the result of pread, pwrite

	if (Offset >= BUCKETSIZE * MAINBUCKETS) {
		// overflow offset
		// traverse main buckets with chains tell find the pointer who point to it

		// int bucketOffset = (((Offset - BUCKETSIZE * MAINBUCKETS) / OVEFLOWBUCKETSIZE) * OVEFLOWBUCKETSIZE) + BUCKETSIZE * MAINBUCKETS;
		// int recordIndex = (Offset - bucketOffset) / sizeof(DataItem);

		int currentOffset = 0;
		for(int i = 0; i < MAINBUCKETS; i++, currentOffset += sizeof(Bucket)) {
			Bucket mainBucket;
			readResult = pread(fd, &mainBucket, sizeof(Bucket), currentOffset);
			if(readResult < 0){ perror("some error occurred in pread"); return -1; }
			if (readResult == 0 || !mainBucket.valid) continue;
			int next = mainBucket.recordPointer;
			if (next == Offset) {
				// we found it, main bucket should change
				auto overflowRecord = readOverflowRecord(fd, next);
				mainBucket.recordPointer = overflowRecord.next;
				writeResult = pwrite(fd, &mainBucket, sizeof(Bucket), currentOffset);
				if (writeResult == -1) return -1;
				break;
			} else {
				bool found = false;
				while (next != -1) {
					auto overflowRecord = readOverflowRecord(fd, next);
					int nextNext = overflowRecord.next;
					if (nextNext == Offset) {
						// we found it, main bucket will not change
						auto nextOverflowRecord = readOverflowRecord(fd, nextNext);
						overflowRecord.next = nextOverflowRecord.next;
						writeResult = pwrite(fd, &overflowRecord, sizeof(OverflowRecord), next);
						if (writeResult == -1) return -1;
						found = true;
						break;
					}
					next = nextNext;
				}
				if (found) break;
			}
		}
	} else {
		// main bucket
		int bucketOffset = (Offset / BUCKETSIZE) * BUCKETSIZE;
		int recordIndex = (Offset - bucketOffset) / sizeof(DataItem);
		Bucket b = readMainBucket(fd, bucketOffset);
		// if (b.recordPointer != -1) {
		// 	// there is a pointer, get the first one and put it in the empty space, make the recordPointer of the bucket point to the pointer of record pointer of the this first one
		// 	OverflowRecord overflowRecord;
		// 	readResult = pread(fd, &overflowRecord, sizeof(OverflowRecord), b.recordPointer);
		// 	b.recordPointer = overflowRecord.next;
		// 	b.dataItem[recordIndex] = overflowRecord;
		// 	writeResult = pwrite(fd, &b, sizeof(Bucket), bucketOffset);
		// 	if (writeResult == -1) return -1;
		// }
		if(b.recordPointer != -1){
			auto overflowRecord = readOverflowRecord(fd, b.recordPointer);
			b.dataItem[recordIndex] = overflowRecord;
			b.recordPointer = overflowRecord.next;
			return pwrite(fd, &b, sizeof(Bucket), bucketOffset);
		}
	}
	dummyItem.valid = 0;
	dummyItem.key = -1;
	dummyItem.data = 0;
	writeResult = pwrite(fd, &dummyItem, sizeof(OverflowRecord), Offset);
	return writeResult;
}





int deleteKey(int fd, int key)
{
	struct DataItem dummyItem;
	
	//Definitions
	int hashIndex = hashCode(key);  				//calculate the Bucket index
	int startingOffset = hashIndex*sizeof(Bucket);		//calculate the starting address of the bucket
	int Offset = startingOffset;						//Offset variable which we will use to iterate on the db

	ssize_t readResult, writeResult; // store the result of pread, pwrite


	// search for main bucket first for free records
	Bucket mainBucket;
	readResult = pread(fd, &mainBucket, sizeof(Bucket), Offset);
	if(readResult <= 0) return -1;
	for(int i = 0; i < RECORDSPERBUCKET; i++) {
		DataItem & currentRecord = mainBucket.dataItem[i];
		if (currentRecord.valid == 1 && currentRecord.key == key) {
			return deleteOffset(fd, Offset + (i * sizeof(DataItem)));
		}
	}
	// not in the main bucket
	
	int next = mainBucket.recordPointer;
	auto overflowRecord = readOverflowRecord(fd, next);
	if (overflowRecord.key == key) {
		// we found it, main bucket should change
		mainBucket.recordPointer = overflowRecord.next;
		writeResult = pwrite(fd, &mainBucket, sizeof(Bucket), Offset);
		return writeResult;
	} else {
		while (next != -1) {
			int nextNext = overflowRecord.next;
			auto nextOverflowRecord = readOverflowRecord(fd, nextNext);
			if (nextOverflowRecord.key == key) {
				overflowRecord.next = nextOverflowRecord.next;
				writeResult = pwrite(fd, &overflowRecord, sizeof(OverflowRecord), next);
				return writeResult;
			}
			next = nextNext;
		}
	}
	return -1;
}
