#if !defined(Parser_HPP)
#define Parser_HPP
#include "Util.hpp"
#include "ParserTokens.hpp"
#include "Project.hpp"
#include "Library.hpp"
#include <unordered_map>

//from main.cpp
extern bool parser_exit;
extern bool system_allowed;

//most comments in Parser.cpp

class Parser
{
private:
	void define(const string &identifier);
	inline bool is_defined(const string& identifier) {return (classes.find(identifier) != classes.end());}
	void array();
	void declare();
	void declare_library();
	void recursive();
	Class *current_class;
	ParserToken::ParserTokens temp_type;

	void void_function(); //expects '(' ')' with nothing inside
	ParserToken string_function(); //expects '(' STRING_LITERAL ')' and then returns the STRING_LITErAL token
	
	bool object_method(); //all object methods

	string child;
private:
	ParserToken current;
	vector<ParserToken> tokens;
	std::unordered_map<string, Class*> classes;

	int32_t index = -1; //will be incremented to 0
	inline ParserToken next() {
		return tokens[++index];
	}

	//there are MANY better ways of doing this... but i'm lazy
	void expect(ParserToken::ParserTokens type);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3, ParserToken::ParserTokens type4);
	void expect(ParserToken::ParserTokens type, ParserToken::ParserTokens type2, ParserToken::ParserTokens type3, ParserToken::ParserTokens type4, ParserToken::ParserTokens type5);

	void parse(); //called from constructor, should be rewritten to not do that
public:
	Parser(const string& file_name);
	~Parser();
};


#endif