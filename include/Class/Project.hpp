#if !defined(Project_HPP)
#define Project_HPP
#include "Class/Class.hpp"

class Project : public Class
{
public:
	void build() override;
	void single_file_build();

	inline string get_install_path() override {
		return "/usr/local/bin/" + get_stripped_name();
	};
	void generate_name() override;
	
	void set_type(i32 line, bool is_static) override;

	Project(string_view ident);
	~Project();
	/// @brief Generate the long command with `objects` as the objects
	/// @param objects 
	/// @return the builds command with `objects`
	string generate_command_for(string& objects);
};

#endif
