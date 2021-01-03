
/*main.cpp*/

//
// myDB project using AVL trees
//
// <<Bhavana Laxmi Radharapu>>
// U. of Illinois, Chicago
// CS 251: Fall 2019
// Project #04
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>

#include "avl.h"
#include "util.h"

using namespace std;

  streamoff pos2 = 0;  // first record at offset 0:
  string    value2;
  ifstream data;
  streamoff key;

//
// tokenize
//
// Given a string, breaks the string into individual tokens (words) based
// on spaces between the tokens.  Returns the tokens back in a vector.
//
// Example: "select * from students" would be tokenized into a vector
// containing 4 strings:  "select", "*", "from", "students".
//

int GetCol(string indexcol,vector<string> values)
{
	if(indexcol == "*")
		return -2;
	for(size_t k= 0 ;k<values.size();k++)
	{
		if(values[k] == indexcol)
			return k; // found !
	}
	return -1; //not found....
}

bool isIndex(string colName, vector<string> names, vector<int> pos)
{
   for(size_t i=0;i<names.size();i++)
   {
     if ( (colName == names.at(i)) && (pos.at(i) == 1) )
		 return true;
   }
   return false;
}

vector<string> tokenize(string line)
{
  vector<string> tokens;
  stringstream  stream(line);
  string token;

  while (getline(stream, token, ' '))
  {
    tokens.push_back(token);
  }

  return tokens;
}

avltree<string,streamoff> BuildIndexTree(string tablename,int recordSize , int i)
{
  string   datafilename = tablename + ".data";
  ifstream data(datafilename, ios::in | ios::binary);
  avltree<string,streamoff> avl;
	
	data.seekg(0, data.end);  // move to the end to get length of file:
    streamoff length2 = data.tellg();
	streamoff pos2 = 0;
	if (!data.good())
  {
    cout << "**Error: couldn't open data file '" << datafilename << "'." << endl;
    exit(0);
  }
   string value2;
	while(pos2<length2){
		
		data.seekg(pos2, data.beg);
 for (int j = 0; j < i+1; ++j)  // read values, one per column:
    {
      data >> value2;
    }
		  avl.insert(value2, pos2); // using the pos of the match value
		pos2 += recordSize;
	}
 return avl;
}
	


int main()
{
  string tablename; // = "students";

  cout << "Welcome to myDB, please enter tablename> ";
  getline(cin, tablename);

  cout << "Reading meta-data..."<< endl;
  //
  // TODO:
  //
  string metafilename = tablename + ".meta";
  ifstream meta(metafilename, ios::in | ios::binary);

	int recordSize = 0;
	int numColumns = 0;
	streamoff pos1 = 0;
	string    value;
	meta.seekg(0, meta.end);  // move to the end to get length of file:
    streamoff length1 = meta.tellg();
     
	if (!meta.good())
     {
        cout << "**Error: couldn't open data file '" << metafilename << "'." << endl;
        exit(0);
     }
	meta.seekg(pos1, meta.beg);
	meta >> recordSize >> numColumns;

	int record;
	vector<string> values; // records the index column nae
	vector<int> records;  // records the values beside index column
	vector<avltree<string, streamoff>>avl;  // contains trees of index column
	
	avltree<string,streamoff> tree;
	while(pos1 < length1)
	{
	  for (int i = 0; i < numColumns; ++i)  // read values, one per column:
	  {
 	 	meta >> value; 
 		meta >> record ; 
		values.push_back(value);
		records.push_back(record);
	   }
       pos1 += recordSize;  // move offset to start of next record: 
	}
//-------------------------------------------------------------------
	cout << "Building index tree(s)..." << endl;
	for(int i=0;i<numColumns;i++)
	{
		if(records.at(i) == 1)
		{
			tree = BuildIndexTree(tablename, recordSize, i);
			cout<<"Index column: "<< values.at(i)<<endl;
			cout << "  Tree size: " << tree.size() << endl << "  Tree height: "<<tree.height() << endl;
		}
	  avl.push_back(tree);
	}
	
//-----------------------------------------------------------------------------------------	
  
	string query;  
    cout << endl;
    cout << "Enter query> ";
    getline(cin, query);

  while (query != "exit")
  {
        vector<string> tokens = tokenize(query);
	    string requireddata;
	    vector<streamoff> requiredpos;
	    // Check for errors 
	    if(tokens.size() < 8)
		{
			if (tokens[0] == "select")
				cout << "Invalid select query, ignored..." << endl;
		    else 
				cout << "Unknown query, ignored..." << endl;
		}
		else if(tokens[0]!="select" )
			cout << "Unknown query, ignored..." << endl;
		else if(GetCol(tokens[1],values)==-1)
			cout << "Invalid select column, ignored..." << endl;
		else if(tokens[2]!="from")
			cout << "Unknown query, ignored..." << endl;
		else if(tokens[3]!=tablename)
			cout << "Invalid table name, ignored..." << endl;
		else if(tokens[4]!="where")
			cout << "Unknown query, ignored..." << endl;
		else if (GetCol(tokens[5],values) == -1)
			cout << "Invalid where column, ignored..." << endl;
		else if(tokens[6]!="=")
			cout << "Unknown query, ignored..." << endl;
	    else
		{
			   if(tokens.at(1) == "*")
			   {
				   if(isIndex(tokens.at(5),values,records)) // We have to search in a tree
				   {
					   avltree<string,streamoff> requiredtree;
					   requiredtree = avl[GetCol(tokens.at(5),values)];
					   streamoff* position = requiredtree.search(tokens.at(7));
		               if(position == nullptr)      // Not found in the tree
						   cout << "Not found..." << endl;
					   else
					   {
						   vector<string> Val = GetRecord(tablename,*position,numColumns);
						   for(size_t j=0; j<Val.size();j++) // output the values of from get record with their respective title from values 
							   cout << values[j] << ": " << Val[j] << endl;
					   }
				   }
				   else
				   {
					 vector<streamoff> streams = LinearSearch(tablename,recordSize,numColumns,tokens.at(7),GetCol(tokens.at(5),values));
					 if(streams.size() == 0)
						 cout << "Not found..." << endl;  // not found 
					 else
					 {
						 for(size_t k=0;k<streams.size();k++)
						 {
							 vector<string> Val = GetRecord(tablename,streams[k],numColumns);
							 for(size_t j=0;j<Val.size();j++)
								 cout << values[j] << ": " << Val[j] << endl;
						 }
					 }
				   }
			     }
			   else
			   {
				   int column = GetCol(tokens.at(5),values);
				   int select_column = GetCol(tokens.at(1),values);// index of the select column
				   
				   if(isIndex(tokens.at(5),values,records)) // We have to search in a tree
				   {
					  avltree<string,streamoff> requiredtree;
					  requiredtree = avl[column];
					  streamoff* position = requiredtree.search(tokens.at(7));
					  if(position == nullptr)
						   cout << "Not found..." << endl; // not found 
					  else
					  {
						  string filename = tablename + ".data";
						  ifstream Data(filename, ios::in | ios::binary);
						  if (!Data.good())
						  {
							 cout << "**Error: couldn't open data file '" << metafilename << "'." << endl;
							 exit(0);
						  }
						  string value;
						  Data.seekg(*position,Data.beg);  // takes to the beginning of the specific record
						  Data >> value;
						  for(int j=0;j<select_column;j++)
							  Data >> value; // reads the data
						  cout << values[select_column] << ": " << value << endl;
					  }
				  }
				  else
				  {
					 
					 vector<streamoff> streams = LinearSearch(tablename,recordSize,numColumns,tokens.at(7),column); // Linear search s there is no tree
					 if(streams.size() == 0)
						 cout << "Not found..." << endl; //not found
					 else
					 {
						 for(size_t j=0;j<streams.size();j++)
						 {
							 string filename = tablename + ".data";
							 ifstream Data(filename, ios::in | ios::binary);
							 if (!Data.good()) 
							 {
								 cout << "**Error: couldn't open data file '" << metafilename << "'." << endl;
								 exit(0);
							 }
							 string value;
							 Data.seekg(streams[j],Data.beg);  
							 Data >> value; // reads data if selected column is zero
							 for(int k=0;k<select_column;k++)
								Data >> value; // reading if column is greater than zero
							 cout << values[select_column] << ": " << value << endl; // output with respective to the title of specific detail
						 }
					 }
				  }
			   }
			}
	

    cout << endl;
    cout << "Enter query> ";
    getline(cin, query);
  }
  //
  // done:
  //
  return 0;
}
