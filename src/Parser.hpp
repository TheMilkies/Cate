#if !defined(Parser_HPP)
#define Parser_HPP
#include "Util.hpp"
#include "Lexer.hpp"
#include "ParserTokens.hpp"
#include "Project.hpp"
#include "Library.hpp"

extern bool parser_exit;

class Parser
{
private:
	void define(ParserToken::ParserTokens type, string &identifier);
	inline bool is_defined(string& identifier) {return (classes.find(identifier) != classes.end());}
	void array(string& child);
	void recursive(string &child, bool keep_path = true);
	Class *current_class;
private:
	ParserToken current;
	vector<ParserToken> tokens;
	map<string, Class*> classes;
	int index = -1;
	inline ParserToken next() {return tokens[++index];}
	void expect(ParserToken::ParserTokens type);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3, ParserToken::ParserTokens type4);
public:
	void parse();
	Parser(const char* file_name);
	~Parser();
};


#endif