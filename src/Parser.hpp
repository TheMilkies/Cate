#if !defined(Parser_HPP)
#define Parser_HPP
#include "Util.hpp"
#include "ParserTokens.hpp"
#include "Project.hpp"
#include "Library.hpp"

extern bool parser_exit;
extern bool system_allowed;

class Parser
{
private:
	void define(ParserToken::ParserTokens type, const string &identifier);
	inline bool is_defined(const string& identifier) {return (classes.find(identifier) != classes.end());}
	void array();
	void declare();
	void declare_library();
	void recursive();
	Class *current_class;
	ParserToken::ParserTokens temp_type;
	void void_function();
	ParserToken string_function();
	bool object_method();
	string child;
private:
	ParserToken current;
	vector<ParserToken> tokens;
	unordered_map<string, Class*> classes;
	int index = -1;
	inline ParserToken next() {return tokens[++index];}
	void expect(ParserToken::ParserTokens type);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3, ParserToken::ParserTokens type4);
public:
	void parse();
	Parser(const string& file_name);
	~Parser();
};


#endif