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
options {
	language="Cpp";
}

class SliceParser extends Parser;

options {
        buildAST=true;
        k=3;
        defaultErrorHandler=false;
}

// ^ make it a sub-tree root
// ! take it out of the tree
// nothing means that it will become a leaf

/*slice : CONSTRUCT_NAME^
      | CONSTRUCT_NAME^ LEFT_PAREN!   RIGHT_PAREN!
      | CONSTRUCT_NAME^ LEFT_PAREN!   slice (PARAM_SEPARATOR! slice)*  RIGHT_PAREN!
      | CONSTRUCT_NAME^ slice
      | NUMBER^
      ;
*/



slice : CONSTRUCT_NAME^ LEFT_PAREN! (internal (PARAM_SEPARATOR! internal)*)?  RIGHT_PAREN!
      | CONSTRUCT_NAME^ 
      | CONSTRUCT_NAME^ slice
      | NUMBER^
      ;

internal :  LEFT_PAREN! (slice (PARAM_SEPARATOR! slice)*)? RIGHT_PAREN! 
         |  slice;


class SliceLexer extends Lexer;

options {
    // Allow any char but \uFFFF (16 bit -1)
    //charVocabulary='\u0000'..'\uFFFE';
     k=3;
}

// Words, which include our operators

CONSTRUCT_NAME  : ('a'..'z' | 'A'..'Z')+ | '$' | '%' | '^' | '#'
                ;


LEFT_PAREN  : '('
            ;
RIGHT_PAREN : ')'
            ;



PARAM_SEPARATOR : ','
                ;



 //('-')? ('0'..'9')+ ('.' ('0'..'9')+)?
NUMBER  :   ('-')? ('0'..'9')+ ('.' ('0'..'9')+)?  ('E' ('-')? ('0'..'9')+)?
		| '-' 'I''n''f''i''n''i''t''y'
        ;
    
// there is a bug in antlr, this rule has to be the last!!!!
WS
    : (' ' | '\t' | '\r' | '\n') { _ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP; }
    ;





