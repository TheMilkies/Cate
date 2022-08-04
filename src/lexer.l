%option c++  noinput nounput 

%{
	#include <iostream>
	#include "ParserTokens.hpp"
	#include "Util.hpp"
	using namespace std;
	int lexer_line = 1;
	bool lexer_exit = false;
%}

%%

"/*" {
	int c;

	while((c = yyinput()) != 0)
	{
		if(c == '\n')
			++lexer_line;

		else if(c == '*')
		{
			if((c = yyinput()) == '/')
				break;
			else
				unput(c);
		}
	}
}

"//" {
	int c;

	while((c = yyinput()) != 0)
	{
		if(c == '\n')
		{
			++lexer_line;
			break;
		}
	}
}

\. {return ParserToken::DOT;}
\, {return ParserToken::COMMA;}
";" {return ParserToken::SEMICOLON;}

"=" {return ParserToken::ASSIGN;}
"(" {return ParserToken::LPAREN;}
")" {return ParserToken::RPAREN;}
"{" {return ParserToken::LCURLY;}
"}" {return ParserToken::RCURLY;}
"Project" {return ParserToken::PROJECT;}
"Library" {return ParserToken::LIBRARY;}
"string" {return ParserToken::STRING;}
"static" {return ParserToken::STATIC;}
"dynamic" {return ParserToken::DYNAMIC;}
"recursive" {return ParserToken::RECURSIVE;}
"system" {return ParserToken::SYSTEM;}
[ \t] {;}
\n {++lexer_line;}

\/\/[^.*]\n {;}
\"[^"]+\" {return ParserToken::STRING_LITERAL;}
\"\" {Util::lexer_error("Empty string literals are not allowed.");}
\" {Util::lexer_error("Missing ending quote.");}
[a-zA-Z_0-9w]+ {return ParserToken::IDENTIFIER;}

. {Util::lexer_error("Unknown character");}

%%

