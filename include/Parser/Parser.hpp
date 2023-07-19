#if !defined(Parser_HPP)
#define Parser_HPP
#include "Util.hpp"
#include "Parser/Tokens.hpp"
#include "Class/Class.hpp"
#include "Class/Global.hpp"
#define fatal(text) Util::fatal_error(current.line, text)

//from main.cpp
extern bool errors_exist;

//most comments in Parser.cpp

class Parser
{
private:
	void define();
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
	Token string_function(); //expects '(' STRING_LITERAL ')' and then returns the STRING_LITErAL token
	
	bool special_case(); //`type` and `link` 
	void object_method(); //all object methods

	bool global();

	string child;
private:
	Token current = END;
	vector<Token> tokens;
	Class* get_class(std::string_view name);

	i32 index = -1; //will be incremented to 0
	inline void next() {
		if(tokens[++index].type != END)
			current = tokens[index];
		else current.type = END;
	}

	//there are MANY better ways of doing this... but i'm lazy
	bool expect_bool();
	bool expect_type();

	void expect(TokenKind type);
	void expect(TokenKind type, TokenKind type2);
	void expect(TokenKind type, TokenKind type2, TokenKind type3);

	void optional_rparen();

	inline TokenKind peek(i32 how_much = 1) {return tokens[index + how_much].type;}
	inline void skip(i32 how_much = 1) {current = tokens[index += how_much];}
	inline void prev() {current = tokens[index -= 1];}

	void parse(); //called from constructor, should be rewritten to not do that
public:
	Parser(const string& file_name);
	~Parser();
};

#endif