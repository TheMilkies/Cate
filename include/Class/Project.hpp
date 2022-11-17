#if !defined(Project_HPP)
#define Project_HPP
#include "Class/Class.hpp"

class Project : public Class
{
public:
	void build() override;
	void single_file_build();
	void generate_name() override;

	Project();
	~Project();
	/// @brief Generate the long command with `objects` as the objects
	/// @param objects 
	/// @return the builds command with `objects`
	string generate_command_for(string& objects);
};

#endif
