%option c++  noinput nounput 

%{
	#include <iostream>
	#include "ParserTokens.hpp"
	#include "Util.hpp"
	using namespace std;
	int lexer_line = 1;
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
	while((c = yyinput()) != 0 && c != '\n');

	++lexer_line;
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
"static" {return ParserToken::STATIC;}
"dynamic" {return ParserToken::DYNAMIC;}
"recursive" {return ParserToken::RECURSIVE;}
"system" {return ParserToken::SYSTEM;}
[ \t] {;}
\r\n {++lexer_line;}
\n {++lexer_line;}

\"[^"]+\" {return ParserToken::STRING_LITERAL;}
\"\" {Util::lexer_error("Empty string literals are not allowed.");}
\" {Util::lexer_error("Missing ending quote.");}
[a-zA-Z_0-9]+ {return ParserToken::IDENTIFIER;}

. {Util::lexer_error("Unknown character");}

%%