%option c++ noinput nounput noyywrap

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

"#" {
	int c;
	while((c = yyinput()) != 0 && c != '\n');

	++lexer_line;
}

\. {return DOT;}
\, {return COMMA;}
";" {}

\"[^"]+\" {return STRING_LITERAL;}
\"\" {Util::error(lexer_line, "Empty string literals are not allowed.");}
\" {Util::error(lexer_line, "Missing ending quote.");}

"Project" {return PROJECT;}
"Library" {return LIBRARY;}
"dynamic" {return DYNAMIC;}
"static" {return STATIC;}
"system" {return SYSTEM;}
"recursive" {return RECURSIVE;}
"iterate" {return RECURSIVE;}
"true" {return B_TRUE;}
"false" {return B_FALSE;}
"subcate" {return SUBCATE;}
[a-zA-Z_0-9]+ {return IDENTIFIER;}

"(" {return LPAREN;}
")" {return RPAREN;}
"=" {return ASSIGN;}
"{" {return LCURLY;}
"}" {return RCURLY;}

. {Util::error(lexer_line, "Unknown character");}

%%