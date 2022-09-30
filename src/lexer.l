%option c++ noinput nounput 

%{
	#include <iostream>
	#include "ParserTokens.hpp"
	#include "Util.hpp"
	using namespace std;
	int lexer_line = 1;
%}

%%

[ \t] {;}
\r\n {++lexer_line;}
\n {++lexer_line;}

"/*" {
	int c;

	while((c = yyinput()) != 0)
	{
		lexer_line += (c == '\n');

		if(c == '*')
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
	while((c = yyinput()) != 0 && c != '\n');

	++lexer_line;
}

\. {return ParserToken::DOT;}
\, {return ParserToken::COMMA;}
";" {return ParserToken::SEMICOLON;}

\"[^"]+\" {return ParserToken::STRING_LITERAL;}
\"\" {Util::lexer_error("Empty string literals are not allowed.");}
\" {Util::lexer_error("Missing ending quote.");}

"Project" {return ParserToken::PROJECT;}
"Library" {return ParserToken::LIBRARY;}
"dynamic" {return ParserToken::DYNAMIC;}
"static" {return ParserToken::STATIC;}
"system" {return ParserToken::SYSTEM;}
"recursive" {return ParserToken::RECURSIVE;}
"true" {return ParserToken::TRUE;}
"false" {return ParserToken::FALSE;}
[a-zA-Z_0-9]+ {return ParserToken::IDENTIFIER;}

"(" {return ParserToken::LPAREN;}
")" {return ParserToken::RPAREN;}
"=" {return ParserToken::ASSIGN;}
"{" {return ParserToken::LCURLY;}
"}" {return ParserToken::RCURLY;}

. {Util::lexer_error("Unknown character");}

%%