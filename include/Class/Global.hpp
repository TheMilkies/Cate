#pragma once
#include "Util.hpp"

struct Global
{
	string compiler = "cc", standard, object_dir;
	bool smol = false, threading = false;
};

inline Global global_values;