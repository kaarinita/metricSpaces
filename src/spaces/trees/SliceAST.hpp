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
#ifndef SLICEAST_HPP
#define SLICEAST_HPP


#include <antlr/CommonAST.hpp>
#include <antlr/AST.hpp>
#include <list>
#include <assert.h>

using namespace antlr;
using namespace std;



class SliceAST;
typedef ANTLR_USE_NAMESPACE(antlr)ASTRefCount<SliceAST> RefSliceAST;

/** 
 * Custom CommonAST class. Can be extended to support DRWM
 */
class SliceAST : public ANTLR_USE_NAMESPACE(antlr)CommonAST {
public:
   // copy constructor
   SliceAST( const SliceAST& other )
     : CommonAST(other), descendants(-1)
   {
   }
   // Default constructor
   SliceAST( void ) : CommonAST() {}
   virtual ~SliceAST( void ) {}

   /** the initialize methods are called by the tree building constructs
    * depending on which version is called the line number is filled in.
    * e.g. a bit depending on how the node is constructed it will have the
    * line number filled in or not (imaginary nodes!).
    */
   virtual void initialize(int t, const ANTLR_USE_NAMESPACE(std)string& txt)
   {
      CommonAST::initialize(t,txt);
   }
   virtual void initialize( ANTLR_USE_NAMESPACE(antlr)RefToken t )
   {
      CommonAST::initialize(t);
   }
   virtual void initialize( RefSliceAST ast )
   {
      CommonAST::initialize(ANTLR_USE_NAMESPACE(antlr)RefAST(ast));
   }
   // for convenience will also work without
   virtual void addChild( RefSliceAST c )
   {
      BaseAST::addChild( ANTLR_USE_NAMESPACE(antlr)RefAST(c) );
   }
   // for convenience will also work without
   void setNextSibling( RefSliceAST c )
   {
      BaseAST::setNextSibling( ANTLR_USE_NAMESPACE(antlr)RefAST(c) );
   }
   // provide a clone of the node (no sibling/child pointers are copied)
   virtual ANTLR_USE_NAMESPACE(antlr)RefAST clone( void )
   {
      return ANTLR_USE_NAMESPACE(antlr)RefAST(new SliceAST(*this));
   }
   static ANTLR_USE_NAMESPACE(antlr)RefAST factory( void )
   {
      return ANTLR_USE_NAMESPACE(antlr)RefAST(RefSliceAST(new SliceAST()));
   }

   int getDescendants(){
     assert( descendants != -1);
    	return descendants;
    }

   int getSize(){    
    	return getDescendants() + 1;
    }
   
   /*
     * little speed up to the normal equalsTree method
     * @see antlr.BaseAST#equalsTree(antlr.collections.AST)
     */
    virtual bool fEq(RefSliceAST t) {
			//      if(t->getSize() != this->getSize()){ // little speed up! ;)
      //	  return false;
      //	}else{
    	  return this->equalsTree(ANTLR_USE_NAMESPACE(antlr)RefAST(t));
				//	}
    }

    void depthFirstAux(list<RefSliceAST> &res){
    	res.push_back(RefSliceAST(this));
    	RefSliceAST down = RefSliceAST(this->getFirstChild());
    	if(down){
    		down->depthFirstAux(res);
    	}
    	RefSliceAST right = RefSliceAST(this->getNextSibling());
    	if(right){
    		right->depthFirstAux(res);
    	}
    }

    list<RefSliceAST> depthFirst(){
    	list<RefSliceAST> res;
    	depthFirstAux(res);
    	return res;
    }

    int updateDecendantInformation(){
    	descendants = 0;
    	RefSliceAST n = RefSliceAST(getFirstChild());
    	while(n){
  		descendants += n->updateDecendantInformation();
    		n = RefSliceAST(n->getNextSibling());
    	}
	return descendants + 1;
    }

 private:
    int descendants;
    
};

#endif
