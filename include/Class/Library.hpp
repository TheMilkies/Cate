#if !defined(Library_HPP)
#define Library_HPP
#include "Class/Class.hpp"

class Library : public Class
{
public:
	void build() override;
	void generate_name() override;

	Library();
	~Library();
};

#endif
