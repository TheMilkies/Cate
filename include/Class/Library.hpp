#if !defined(Library_HPP)
#define Library_HPP
#include "Class/Class.hpp"

class Library : public Class
{
public:
	void build() override;
	void generate_name() override;
	void set_type(int32_t line, bool is_static) override;

	Library();
	~Library();
};

#endif
