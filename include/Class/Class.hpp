#if !defined(Class_HPP)
#define Class_HPP
#include "Util.hpp"
#include "Class/Global.hpp"

/*
	why hello there!
	i know most of this is bad, i honestly don't care too much because it's my first project. -yogurt
*/

extern bool errors_exist;
extern i32 thread_count;

class Class
{
public:
	//filled by setup() and build()
	string all_object_files, all_libraries,
		   all_include_paths, all_definitions;

	//user defined in .cate file
	string name, flags, out_name, object_dir, compiler, final_flags, standard;
	vector<string> files, object_files;

	//filled from build();
	vector<string> loaded_library_paths;
	bool is_library_defined(string_view name);

	bool is_static; //only in library
	bool already_built = false, needs_link = false;
	bool link = true, threading, smol = false;
	
	Class(string_view ident);
	
	virtual ~Class() {};
	virtual void build() = 0; //class defined
	virtual void generate_name() = 0; //class defined
	virtual void set_type(i32 line, bool is_static) = 0; //class defined

	void clean();
	void setup(); // set up directories and object files
	void check(); //check if everything is okay
	
	void build_objects(); //build the objects

	//general and self-explanitory
	void create_directories();
	void add_library(string value);
	void set_property(i32 line, string& property, string& value);

	//other
	void smolize();
	bool ask_to_install();

	inline void add_include(const string& path) {
		all_include_paths += "-I" + path + ' ';
	}

	inline void print_done_message_with(string_view name) {
		if(flag(dry_run)) return;
		cout << GREEN "Done building \"" << name << "\"" COLOR_RESET "\n";
	}

	void install();
	string get_stripped_name();
	virtual string get_install_path() = 0;

private:
	//threading
	void build_object(i32 i); 
	vector<std::thread> threads;
	string command_template;

	void build_error(string_view problem);

	void setup_objects(); //set up files and object files.
};

#endif