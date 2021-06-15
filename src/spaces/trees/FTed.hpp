/*
 * Copyright (C) 2009 Arnoldo Jose Muller Molina
 *  This program is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version. This program is distributed
 * in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received
 * a copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef FTED_HPP
#define FTED_HPP
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

#include "SliceLexer.hpp"
#include "SliceParser.hpp"
#include "SliceAST.hpp"


ANTLR_USING_NAMESPACE(antlr)
using namespace std;


class FTed{
public:

	vector<RefSliceAST> data;
	int size; // size of the DB

	
  int getSize(){
		return size;
	}
	/**
   * Loads the trees of the given file into memory.
   * 
   */ 
	void loadDB(string url){
		ifstream myfile (url.c_str(), ios::in);
		string line;
		int lines = 0;
		if (myfile.is_open())  {
			while (! myfile.eof() ) {
					getline (myfile,line);
					lines++;
			}
			myfile.close();
			cout << "Loading : " << lines << " trees: " << endl;
		}
		else {
			cout << "Unable to open file for newline count"; 
		}
		size = lines;
		// now we can load the trees.
		//data = new RefSliceAST[lines + 1];
		data = vector<RefSliceAST>(lines + 1);
		
		ifstream myfile2 (url.c_str(), ifstream::in);
		lines = 1;
		if (myfile2.is_open())  {
			while (! myfile2.eof() && lines <= size) {
					getline (myfile2,line);
					if(! (line == "\n") && !line.empty()){
						data[lines] = createSliceAST(line);
						lines++;
					}
					if((lines % 10000) == 0){
						cout << "Loading : " << lines << endl;					
					}
			}
			cout << "Loaded DB!" << endl;
			myfile2.close();
		}
		else {
			cout << "Unable to open file file for loading trees"; 
		}
	}
	void setQuery(string x){
		setQuery(createSliceAST(x));
	}
	/**
   * Sets the query
   */ 
	void setQuery(RefSliceAST query){
		if(data[0] != NULL){
			//data.erase(data.begin());
		}
		data[0] = query;
	}
  /*
   * Clean all data
   */
  void clean(){
		cout << "will delete data..." << endl;
		data.erase(data.begin(), data.end());
		cout << "deleted data" << endl;
	}

	/**
   * Distance based on the object ids.
   */
	int ted(int a, int b){
		return tedAux(data[a], data[b]);
	}

  
  int ted(string a, string b){
    return tedAux(createSliceAST(a), createSliceAST(b));
  }

	int tedAux(RefSliceAST a, RefSliceAST b){
  // a->updateDecendantInformation();
  // b->updateDecendantInformation();
  list<RefSliceAST> aExpanded = a->depthFirst();
  list<RefSliceAST> bExpanded = b->depthFirst();
	list<RefSliceAST> bExpanded2 = b->depthFirst();
	
  int Na = aExpanded.size() * 2;
  int Nb = bExpanded.size() * 2;
  list<RefSliceAST>::iterator ait = aExpanded.begin();
  int res = 0;
  while(ait !=aExpanded.end()  ){
    RefSliceAST aTree = *ait++;
    list<RefSliceAST>::iterator bit = bExpanded.begin();
    while(bit != bExpanded.end()){
      RefSliceAST bTree = *bit;
      if(aTree->fEq(bTree)){
				res++;
        bExpanded.erase(bit);
				break;
      }
      bit++;
    }

		bit = bExpanded2.begin();
		while (bit != bExpanded2.end()) {
			RefSliceAST bTree = *bit;
			if (aTree->getText() == bTree->getText()) {
				res++;
				bExpanded2.erase(bit);
				break;
			}
			bit++;
		}

		
  }

  //tedsAmount++;
  //return ceil((float)( (Na + Nb) - ( 2 * res) ) / 2);
  //return max(Na, Nb) - res;
	int r = (int)(((Na + Nb) - (2 * res)) / 2);
	assert(r >= 0);
	return r;
}

 
  /**
   * Create a Tree from a string.
   */
  RefSliceAST createSliceAST(string x){
    istringstream in;
    in.str(x);
    SliceLexer lexer(in);
    SliceParser parser(lexer);
    // set up the ast factory to use a custom AST type per default
    // note that here the Ref prefix for the reference counter is
    // strippped off.
    ASTFactory ast_factory("SliceAST", SliceAST::factory);
    // let the parser add it's stuff to the factory...
    parser.initializeASTFactory(ast_factory);
    parser.setASTFactory(&ast_factory);
    parser.slice();
    RefSliceAST res(parser.getAST());
    res->updateDecendantInformation();
    return res;
  }

};


#endif
