#if !defined(Library_HPP)
#define Library_HPP
#include "Class.hpp"

class Library : public Class
{
private:
	string build_type;
public:
	void build() override;
	Library();
	~Library();
};

#endif
