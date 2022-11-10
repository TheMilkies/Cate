%option c++ noinput nounput

%{
	#include <iostream>
	#include "Parser/ParserTokens.hpp"
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

\. {return DOT;}
\, {return COMMA;}
";" {}

\"[^"]+\" {return STRING_LITERAL;}
\"\" {Util::lexer_error("Empty string literals are not allowed.");}
\" {Util::lexer_error("Missing ending quote.");}

"Project" {return PROJECT;}
"Library" {return LIBRARY;}
"dynamic" {return DYNAMIC;}
"static" {return STATIC;}
"system" {return SYSTEM;}
"recursive" {return RECURSIVE;}
"iterate" {return RECURSIVE;}
"true" {return B_TRUE;}
"false" {return B_FALSE;}
[a-zA-Z_0-9]+ {return IDENTIFIER;}

"(" {return LPAREN;}
")" {return RPAREN;}
"=" {return ASSIGN;}
"{" {return LCURLY;}
"}" {return RCURLY;}

. {Util::lexer_error("Unknown character");}

%%