#if !defined(Parser_HPP)
#define Parser_HPP
#include "Util.hpp"
#include "Parser/ParserTokens.hpp"
#include "Class/Class.hpp"
#include "Class/Global.hpp"
#define fatal(text) Util::fatal_error(current.line, text)

//from main.cpp
extern bool errors_exist;
extern bool system_blocked;

//most comments in Parser.cpp

class Parser
{
private:
	void define();
	inline bool is_defined(const string& identifier) {return (classes.find(identifier) != classes.end());}
	void array();

	void include_array();
	void library_array();
	void files_array();
	void definitions_array();
	void expect_string_array();
	void expect_string_recursive_array();
	void expect_library_recursive_array();
	
	void   recursive_setup();
	string extension_recursive();
	void   files_recursive();
	void   include_recursive();
	void   library_recursive();

	Class *current_class = nullptr;

	void 		void_function(); //expects '(' ')' with nothing inside
	ParserToken string_function(); //expects '(' STRING_LITERAL ')' and then returns the STRING_LITErAL token
	
	bool special_case(); //`type` and `link` 
	void object_method(); //all object methods

	bool global();

	string child;
private:
	ParserToken current;
	vector<ParserToken> tokens;
	std::unordered_map<string, Class*> classes;

	int32_t index = -1; //will be incremented to 0
	inline void next() {
		current = tokens[++index];
	}

	//there are MANY better ways of doing this... but i'm lazy
	bool expect_bool();
	bool expect_type();

	void expect(ParserTokenKind type);
	void expect(ParserTokenKind type, ParserTokenKind type2);
	void expect_and_then(ParserTokenKind type, ParserTokenKind type2);
	void expect(ParserTokenKind type, ParserTokenKind type2, ParserTokenKind type3);

	void optional_rparen();

	inline ParserTokenKind peek(int32_t how_much = 1) {return tokens[index + how_much].type;}
	inline void skip(int32_t how_much = 1) {current = tokens[index += how_much];}

	void parse(); //called from constructor, should be rewritten to not do that
public:
	Parser(const string& file_name);
	~Parser();
};

#endif