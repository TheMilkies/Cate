#if !defined(Parser_HPP)
#define Parser_HPP
#include "Util.hpp"
#include "Parser/ParserTokens.hpp"
#include "Class/Class.hpp"

//#include <unordered_map>

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

	void include_array();
	void library_array();
	void files_array();
	void definitions_array();
	void expect_string_array();
	void expect_string_recursive_array();
	
	void declare();
	void declare_library();

	void recursive();
	void recursive_setup();
	void include_recursive();

	Class *current_class;
	ParserTokenKind temp_type;

	void void_function(); //expects '(' ')' with nothing inside
	ParserToken string_function(); //expects '(' STRING_LITERAL ')' and then returns the STRING_LITErAL token
	
	bool special_case(); //`type` and `link` 
	bool object_method(); //all object methods

	string child;
private:
	ParserToken current;
	vector<ParserToken> tokens;
	std::unordered_map<string, Class*> classes;

	int32_t index = -1; //will be incremented to 0
	inline const ParserToken next() {
		return tokens[++index];
	}

	//there are MANY better ways of doing this... but i'm lazy
	bool expect_bool();
	bool expect_type();

	void expect(ParserTokenKind type);
	void expect(ParserTokenKind type, ParserTokenKind type2);
	void expect_and_then(ParserTokenKind type, ParserTokenKind type2);
	void expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3);
	void expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3, ParserTokenKind type4);
	void expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3, ParserTokenKind type4, ParserTokenKind type5);

	void parse(); //called from constructor, should be rewritten to not do that
public:
	Parser(const string& file_name);
	~Parser();
};

#endif