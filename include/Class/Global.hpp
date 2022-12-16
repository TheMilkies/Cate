#pragma once
#include "Util.hpp"

struct Global
{
	string compiler = "cc", standard, object_dir;
};

extern Global global_values;