#if !defined(Project_HPP)
#define Project_HPP
#include "Class/Class.hpp"

class Project : public Class
{
public:
	void build() override;

	Project();
	~Project();
	/// @brief Generate the long command with `objects` as the objects
	/// @param objects 
	/// @return the builds command with `objects`
	string command_gen(string& objects);
};

#endif
