#ifndef INC_SliceParser_hpp_
#define INC_SliceParser_hpp_

#include <antlr/config.hpp>
/* $ANTLR 2.7.7 (2006-11-01): "Slice.g" -> "SliceParser.hpp"$ */
#include <antlr/TokenStream.hpp>
#include <antlr/TokenBuffer.hpp>
#include "SliceParserTokenTypes.hpp"
#include <antlr/LLkParser.hpp>

class CUSTOM_API SliceParser : public ANTLR_USE_NAMESPACE(antlr)LLkParser, public SliceParserTokenTypes
{
#line 1 "Slice.g"
#line 15 "SliceParser.hpp"
public:
	void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
protected:
	SliceParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k);
public:
	SliceParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf);
protected:
	SliceParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k);
public:
	SliceParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);
	SliceParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state);
	int getNumTokens() const
	{
		return SliceParser::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return SliceParser::tokenNames[type];
	}
	const char* const* getTokenNames() const
	{
		return SliceParser::tokenNames;
	}
	public: void slice();
	public: void internal();
public:
	ANTLR_USE_NAMESPACE(antlr)RefAST getAST()
	{
		return returnAST;
	}
	
protected:
	ANTLR_USE_NAMESPACE(antlr)RefAST returnAST;
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 10;
#else
	enum {
		NUM_TOKENS = 10
	};
#endif
	
};

#endif /*INC_SliceParser_hpp_*/
