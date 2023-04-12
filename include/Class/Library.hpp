#if !defined(Library_HPP)
#define Library_HPP
#include "Class/Class.hpp"

class Library : public Class
{
public:
	void build() override;
	string get_install_path() override {
		return "/usr/local/lib/" + get_stripped_name();
	}
	
	inline const char* get_build_extension() {
		return (is_static) ? ".a" : DYNAMIC_EXTENSION;;
	}

	void generate_name() override;
	void set_type(i32 line, bool is_static) override;

	Library(string_view ident);
	~Library();
};

#endif
