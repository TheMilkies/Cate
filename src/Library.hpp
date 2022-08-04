#if !defined(Library_HPP)
#define Library_HPP
#include "Class.hpp"

class Library : public Class
{
public:
	void build() override;
	Library();
	~Library();
};

#endif
