#if !defined(Class_HPP)
#define Class_HPP
#include "Util.hpp"

/*
	why hello there!
	i know most of this is bad, i honestly don't care too much because it's my first project.
*/

extern bool parser_exit;

class Class
{
public:
	string all_object_files, all_libraries, all_library_paths, all_include_paths;
	string name, flags, out_name, out_dir, compiler;
	vector<string> files, libraries, object_files, include_paths;
	robin_hood::unordered_set<string> library_paths;

	bool is_static;
	bool already_built = false, needs_rebuild = false;
	
	Class() {}
	virtual ~Class() {};
	void build_objects();
	virtual void build() = 0;

	void setup();
	bool clear_property(int line, string& property);
	void add_to_property(int line, string& property, string value);
	void set_property(int line, string& property, string& value);
	void check();
};

#endif
