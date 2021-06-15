#ifndef INC_SliceLexer_hpp_
#define INC_SliceLexer_hpp_

#include <antlr/config.hpp>
/* $ANTLR 2.7.7 (2006-11-01): "Slice.g" -> "SliceLexer.hpp"$ */
#include <antlr/CommonToken.hpp>
#include <antlr/InputBuffer.hpp>
#include <antlr/BitSet.hpp>
#include "SliceParserTokenTypes.hpp"
#include <antlr/CharScanner.hpp>
class CUSTOM_API SliceLexer : public ANTLR_USE_NAMESPACE(antlr)CharScanner, public SliceParserTokenTypes
{
#line 1 "Slice.g"
#line 15 "SliceLexer.hpp"
private:
	void initLiterals();
public:
	bool getCaseSensitiveLiterals() const
	{
		return true;
	}
public:
	SliceLexer(ANTLR_USE_NAMESPACE(std)istream& in);
	SliceLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib);
	SliceLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state);
	ANTLR_USE_NAMESPACE(antlr)RefToken nextToken();
	public: void mCONSTRUCT_NAME(bool _createToken);
	public: void mLEFT_PAREN(bool _createToken);
	public: void mRIGHT_PAREN(bool _createToken);
	public: void mPARAM_SEPARATOR(bool _createToken);
	public: void mNUMBER(bool _createToken);
	public: void mWS(bool _createToken);
private:
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
};

#endif /*INC_SliceLexer_hpp_*/
