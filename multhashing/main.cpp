//============================================================================
// Name        : hashskeleton.cpp
// Author      : 
// Version     :
// Copyright   : Code adapted From https://www.tutorialspoint.com/
// Description : Hashing using open addressing
//============================================================================
#include <string>
#include<fstream>
#include <libgen.h>
#include "../readfile.h"
#include "multhashing.h"
#include<iostream>
using namespace std;

void insert(int key,int data);
int deleteItem(int key);
struct DataItem * search(int key);


int filehandle;   //handler for the database file

/* DBMS (DataBase Management System) needs to store its data in something non-volatile
 * so it stores its data into files (manteqy :)).

 * Some DBMS or even file-systems constraints the size of that file. 

 * for the efficiency of storing and retrieval, DBMS uses hashing 
 * as hashing make it very easy to store and retrieve, it introduces 
 * another type of problem which is handling conflicts when two items
 * have the same hash index

 * In this exercise we will store our database into a file and experience
 * how to retrieve, store, update and delete the data into this file 

 * This file has a constant capacity and uses external-hashing to store records,
 * however, it suffers from hashing conflicts.
 * 
 * You are required to help us resolve the hashing conflicts 

 * For simplification, consider the database has only one table 
 * which has two columns key and data (both are int)

 * Functions in this file are just wrapper functions, the actual functions are in openAddressing.cpp

*/
fstream resultFile;
int main(int argc, char *argv[]){

//here we create a sample test to read and write to our database file

  //1. Create Database file or Open it if it already exists, check readfile.cpp
   char* dbPath = "db/multhashing";
   if (argc >= 3) {
     dbPath = argv[1];
   }

   filehandle = createFile(FILESIZE, dbPath);
   string writePath = string("tests/res/") + "m_" + string(basename(argv[1]));
   resultFile.open(writePath, fstream::out);
   fstream testfile(argv[1], fstream::in);

  char operation;
  testfile >> operation;
   while (!testfile.eof()){
     int key, value;
     switch (operation)
     {
     case 'V':
     resultFile << "* view file operation\n";
       DisplayFile(filehandle);
       DisplayFile(filehandle, resultFile);
       break;
     case 'I':
       testfile >> key >> value;
      resultFile << "* insert operation of " << key << ", " << value << endl;
       insert(key, value);
       break;
     case 'S':
       testfile >> key;
        resultFile << "* search operation of " << key << endl;
       search(key);
       break;
     case 'D':
       testfile >> key;
        resultFile << "* delete operation of " << key << endl;
       deleteItem(key);
       break;
     default:
       break;
     }
     testfile >> operation;
     resultFile << "================================================\n\n";
   }
   testfile.close();
   close(filehandle);
   return 0;
   ////////////////////////////////////////

  //2. Display the database file, check openAddressing.cpp
   DisplayFile(filehandle);

  
  //3. Add some data in the table
   insert(1, 20);         //1  h(key) = 1.0
   insert(2, 70);         //2  h(key) = 2.0
   insert(42, 80);        //3  h(key) = 2.0, 2.1
   insert(4, 25);         //4  h(key) = 4.0
   insert(12, 44);        //5  h(key) = 2.0, 2.1, 3.0
   insert(14, 32);        //6  h(key) = 4.0, 4.1
   insert(17, 11);        //7  h(key) = 7.0
   insert(13, 78);        //8  h(key) = 3.0, 3.1
   insert(37, 97);        //9  h(key) = 7.0, 7.1
   insert(11, 34);        //10 h(key) = 1.0, 1.1
   insert(22, 730);       //11 h(key) = 2.0, 2.1, 3.0, 3.1, 4.0, 4.1, 5.0
   insert(46, 840);       //12 h(key) = 6.0
   insert(9, 83);         //13 h(key) = 9.0
   insert(21, 424);       //14 h(key) = 1.0, 1.1, 2.0, 2.1, 3.0, 3.1, 4.0, 4.1, 5.0, 5.1 
   insert(41, 115);       //15 h(key) = 1.0, 1.1, 2.0, 2.1, 3.0, 3.1, 4.0, 4.1, 5.0, 5.1, 6.0, 6.1
   insert(71, 47);        //16 h(key) = 1.0, 1.1, 2.0, 2.1, 3.0, 3.1, 4.0, 4.1, 5.0, 5.1, 6.0, 6.1, 7.0, 7.1, 8.0
   insert(31, 92);        //17 h(key) = 1.0, 1.1, 2.0, 2.1, 3.0, 3.1, 4.0, 4.1, 5.0, 5.1, 6.0, 6.1, 7.0, 7.1, 8.0, 8.1
   insert(73, 45);        //18 h(key) = 3.0, 3.1, 4.0, 4.1, 5.0, 5.1, 6.0, 6.1, 7.0, 7.1, 8.0, 8.1, 9.0, 9.1

   //4. Display the database file again
   DisplayFile(filehandle);

   //5. Search the database
   search(13);

   //6. delete an item from the database
   deleteItem(31);

   //7. Display the final data base
   DisplayFile(filehandle);
   // And Finally don't forget to close the file.
   close(filehandle);
   return 0;



}
/* functionality: insert the (key,data) pair into the database table
                  and print the number of comparisons it needed to find
    Input: key, data
    Output: print statement with the no. of comparisons
*/
void insert(int key,int data){
     struct DataItem item ;
     item.data = data;
     item.key = key;
     item.valid = 1;
     int result= insertItem(filehandle,item);
     resultFile << "Insert: No. of searched records " << abs(result) << endl;
     printf("Insert: No. of searched records:%d\n",abs(result));
}

/* Functionality: search for a data in the table using the key

   Input:  key
   Output: the return data Item
*/
struct DataItem * search(int key)
{
  struct DataItem* item = (struct DataItem *) malloc(sizeof(struct DataItem));
     item->key = key;
     int diff = 0;
     int Offset= searchItem(filehandle,item,&diff); //this function is implemented for you in openAddressing.cpp
     resultFile << "Search: No of records searched is " << diff << endl;
     printf("Search: No of records searched is %d\n",diff);
     if(Offset <0){  //If offset is negative then the key doesn't exists in the table
       printf("Search: Item not found\n");
       resultFile << "Search: Item not found" << endl;
     }
     else
     {
        printf("Item found at Offset: %d,  Data: %d and key: %d\n",Offset,item->data,item->key);
        resultFile << "Item found at Offset: " << Offset 
        << ",  Data: " << item->data 
        << " and key: " << item->key << endl;
     }
  return item;
}

/* Functionality: delete a record with a certain key

   Input:  key
   Output: return 1 on success and -1 on failure
*/
int deleteItem(int key){
   struct DataItem* item = (struct DataItem *) malloc(sizeof(struct DataItem));
   item->key = key;
   int diff = 0;
   int Offset= searchItem(filehandle,item,&diff);
   printf("Delete: No of records searched is %d\n",diff);
   resultFile << "Delete: No of records searched is " << diff << endl;
   if(Offset >=0 )
   {
    return deleteOffset(filehandle,Offset);
   }
   return -1;
}