/* $ANTLR 2.7.7 (2006-11-01): "Slice.g" -> "SliceParser.cpp"$ */
#include "SliceParser.hpp"
#include <antlr/NoViableAltException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/ASTFactory.hpp>
#line 1 "Slice.g"
#line 8 "SliceParser.cpp"
SliceParser::SliceParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
}

SliceParser::SliceParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,3)
{
}

SliceParser::SliceParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
}

SliceParser::SliceParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,3)
{
}

SliceParser::SliceParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,3)
{
}

void SliceParser::slice() {
	returnAST = ANTLR_USE_NAMESPACE(antlr)nullAST;
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	ANTLR_USE_NAMESPACE(antlr)RefAST slice_AST = ANTLR_USE_NAMESPACE(antlr)nullAST;
	
	if ((LA(1) == CONSTRUCT_NAME) && (LA(2) == LEFT_PAREN)) {
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp1_AST = ANTLR_USE_NAMESPACE(antlr)nullAST;
		tmp1_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, tmp1_AST);
		match(CONSTRUCT_NAME);
		match(LEFT_PAREN);
		{
		switch ( LA(1)) {
		case CONSTRUCT_NAME:
		case LEFT_PAREN:
		case NUMBER:
		{
			internal();
			astFactory->addASTChild( currentAST, returnAST );
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == PARAM_SEPARATOR)) {
					match(PARAM_SEPARATOR);
					internal();
					astFactory->addASTChild( currentAST, returnAST );
				}
				else {
					goto _loop4;
				}
				
			}
			_loop4:;
			} // ( ... )*
			break;
		}
		case RIGHT_PAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RIGHT_PAREN);
		slice_AST = currentAST.root;
	}
	else if ((LA(1) == CONSTRUCT_NAME) && (LA(2) == PARAM_SEPARATOR || LA(2) == RIGHT_PAREN)) {
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp5_AST = ANTLR_USE_NAMESPACE(antlr)nullAST;
		tmp5_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, tmp5_AST);
		match(CONSTRUCT_NAME);
		slice_AST = currentAST.root;
	}
	else if ((LA(1) == CONSTRUCT_NAME) && (LA(2) == CONSTRUCT_NAME || LA(2) == NUMBER)) {
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp6_AST = ANTLR_USE_NAMESPACE(antlr)nullAST;
		tmp6_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, tmp6_AST);
		match(CONSTRUCT_NAME);
		slice();
		astFactory->addASTChild( currentAST, returnAST );
		slice_AST = currentAST.root;
	}
	else if ((LA(1) == NUMBER)) {
		ANTLR_USE_NAMESPACE(antlr)RefAST tmp7_AST = ANTLR_USE_NAMESPACE(antlr)nullAST;
		tmp7_AST = astFactory->create(LT(1));
		astFactory->makeASTRoot(currentAST, tmp7_AST);
		match(NUMBER);
		slice_AST = currentAST.root;
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	returnAST = slice_AST;
}

void SliceParser::internal() {
	returnAST = ANTLR_USE_NAMESPACE(antlr)nullAST;
	ANTLR_USE_NAMESPACE(antlr)ASTPair currentAST;
	ANTLR_USE_NAMESPACE(antlr)RefAST internal_AST = ANTLR_USE_NAMESPACE(antlr)nullAST;
	
	switch ( LA(1)) {
	case LEFT_PAREN:
	{
		match(LEFT_PAREN);
		{
		switch ( LA(1)) {
		case CONSTRUCT_NAME:
		case NUMBER:
		{
			slice();
			astFactory->addASTChild( currentAST, returnAST );
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == PARAM_SEPARATOR)) {
					match(PARAM_SEPARATOR);
					slice();
					astFactory->addASTChild( currentAST, returnAST );
				}
				else {
					goto _loop8;
				}
				
			}
			_loop8:;
			} // ( ... )*
			break;
		}
		case RIGHT_PAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RIGHT_PAREN);
		internal_AST = currentAST.root;
		break;
	}
	case CONSTRUCT_NAME:
	case NUMBER:
	{
		slice();
		astFactory->addASTChild( currentAST, returnAST );
		internal_AST = currentAST.root;
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	returnAST = internal_AST;
}

void SliceParser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory )
{
	factory.setMaxNodeType(9);
}
const char* SliceParser::tokenNames[] = {
	"<0>",
	"EOF",
	"<2>",
	"NULL_TREE_LOOKAHEAD",
	"CONSTRUCT_NAME",
	"LEFT_PAREN",
	"PARAM_SEPARATOR",
	"RIGHT_PAREN",
	"NUMBER",
	"WS",
	0
};



