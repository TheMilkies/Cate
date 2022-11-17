#if !defined(Class_HPP)
#define Class_HPP
#include "Util.hpp"
#include "nonstd/robin_hood.hpp"

/*
	why hello there!
	i know most of this is bad, i honestly don't care too much because it's my first project. -yogurt
*/

extern bool parser_exit;
extern int32_t thread_count;

class Class
{
public:
	//filled by setup() and build()
	string all_object_files, all_libraries,
		   all_library_paths, all_include_paths,
		   all_definitions;

	//user defined in .cate file
	string name, flags, out_name, out_dir, compiler, final_flags, standard;
	vector<string> files, object_files;

	//filled from build();
	robin_hood::unordered_set<string> library_paths;

	bool is_static; //only in library
	bool already_built = false, needs_rebuild = false;
	bool link = true, threading = false, smol = false;
	
	Class();
	
	virtual ~Class() {};
	virtual void build() = 0; //class defined
	virtual void generate_name() = 0; //class defined

	void clean();
	void setup(); // set up directories and object files
	void check(); //check if everything is okay
	
	void build_objects(); //build the objects
	void create_directories();

	//general and self-explanitory
	void add_library(string& value);
	void set_property(int32_t line, string& property, string& value);

	//other
	void smolize();

	inline void add_include(const string& path) {
		all_include_paths += "-I" + path + ' ';
	}

	inline void print_done_message_with(string_view name) {
		cout << GREEN "Done building \"" << name << "\"" COLOR_RESET "\n";
	}
private:
	//threading
	void build_object(int32_t i); 
	vector<std::thread>threads;
	string command_template;

	void object_setup(); //set up files and object files.

	string get_stripped_name();
};

#endif