#ifndef INC_SliceParserTokenTypes_hpp_
#define INC_SliceParserTokenTypes_hpp_

/* $ANTLR 2.7.7 (2006-11-01): "Slice.g" -> "SliceParserTokenTypes.hpp"$ */

#ifndef CUSTOM_API
# define CUSTOM_API
#endif

#ifdef __cplusplus
struct CUSTOM_API SliceParserTokenTypes {
#endif
	enum {
		EOF_ = 1,
		CONSTRUCT_NAME = 4,
		LEFT_PAREN = 5,
		PARAM_SEPARATOR = 6,
		RIGHT_PAREN = 7,
		NUMBER = 8,
		WS = 9,
		NULL_TREE_LOOKAHEAD = 3
	};
#ifdef __cplusplus
};
#endif
#endif /*INC_SliceParserTokenTypes_hpp_*/
