#if !defined(Class_HPP)
#define Class_HPP
#include "Util.hpp"
#include "robin_hood.hpp"

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
	string all_object_files, all_libraries, all_library_paths, all_include_paths;

	//user defined in .cate file
	string name, flags, out_name, out_dir, compiler, final_flags;
	vector<string> files, libraries, object_files, include_paths;

	//filled from build();
	robin_hood::unordered_set<string> library_paths;

	bool is_static; //only in library
	bool already_built = false, needs_rebuild = false;
	bool link = true, threading = false;
	
	Class()
	{
		//these should be enough for most small/medium-sized projects
		name.reserve(16);
		files.reserve(32);
		object_files.reserve(32);
		libraries.reserve(8);
		library_paths.reserve(8);
		all_libraries.reserve(128);
		include_paths.reserve(32);
		threads.reserve(thread_count * 8);

		all_include_paths.reserve(256);
		all_libraries.reserve(8*16);
		all_object_files.reserve(32*16);
		all_library_paths.reserve(8*16);

		compiler.reserve(16);
		final_flags.reserve(32);
		out_name.reserve(32);
		flags.reserve(32);
	}
	
	virtual ~Class() {};
	virtual void build() = 0; //class defined
	void clean();

	void check(); //check if everything is okay
	void setup(); // set up directories and object files
	void object_setup(); //set up files and object files.
	void library_setup();
	void build_objects(); //build the objects

	//general and self-explanitory
	vector<string>& get_array_property(int32_t line, string& property);
	void clear_property(int32_t line, string& property);
	void add_to_property(int32_t line, string_view property, string_view value);
	void set_property(int32_t line, string& property, string& value);

	//threading
	void build_object(int32_t i); 
	vector<std::thread>threads;
	string command_template;
};

#endif