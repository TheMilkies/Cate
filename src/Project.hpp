#if !defined(Project_HPP)
#define Project_HPP
#include "Class.hpp"

class Project : public Class
{
public:
	void build() override;
	Project();
	~Project();
};

#endif
