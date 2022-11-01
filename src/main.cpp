#include "Parser/Parser.hpp" //Parser.hpp includes everything we need, including Util.hpp

//parser_exit is needed to show all errors and exit afterwards
//system_allowed is the -D option, only affects `system(String)` in parser
bool parser_exit = false, system_allowed = true,
	 force_rebuild = false, force_smol = false;

int32_t thread_count = std::thread::hardware_concurrency() * 2;

void help();

string file_name, dir = (fs::is_directory("cate") == true) ? "cate" : "./";

void parse_catel(); // Catel.cpp
bool get_default_file_name();

//bool call_competitor();

#ifdef TRACK_ALLOCS
int allocs_count = 0;
void* operator new(size_t size)
{
	++allocs_count;
	return malloc(size);
}
#endif // TRACK_ALLOCS

int main(int argc, char *argv[])
{
	std::ios_base::sync_with_stdio(false); //this is a massive speed boost in some cases.
	using namespace Util;

	file_name.reserve(64);

	bool catel_exists = file_exists(".catel");

	//get default file
	if (argc < 2 && !catel_exists)
	{
		if (get_default_file_name());
		else if (fs::is_directory("cate"))
		{
			if(catel_exists) command_error("Found catefiles directory, but default catefile doesn't exist.\nMaybe update your .catel file?");
			else command_error("Found catefiles directory, but default catefile doesn't exist.\nMaybe create a .catel file?");
		}
		else
		{
			//if(call_competitor()) return 0;
			std::cout << "No catefiles detected, Have some help\n";
			help();
			return 1;
		}
	}
	//get specified
	else
	{
		for (int32_t i = ARGC_START; i < argc; ++i)
		{
			if (argv[i][0] == '-')
			{
				switch (argv[i][1]) //check the second character of the argument
				{
				case 't': {
					if (argv[i+1] == NULL && argv[i][2] == NULL) //if just "-t"
						command_error("Missing argument \"-t\"");
					
					int sub = atoi((char*) (argv[i] + 2)); //get everything after "-t"
					if (sub != 0) //if 0 or invalid
						thread_count = sub;
					
				}	break;

				case 'v': //cate version
					std::cout << CATE_VERSION "\n";
					return 0; //exit after
					break;

				case 'l':{ //list directory
					bool catefiles = false;

					if(catel_exists) parse_catel();

					std::cout << CYAN;
					for (auto &p : fs::directory_iterator(dir)) //iterate over the files
					{
						if (p.path().extension() == ".cate")
						{
							std::cout << p.path().stem().string() << ", ";
							catefiles = true;
						}
					}
					if (catefiles)
						std::cout << COLOR_RESET "\n";
					else
						std::cout << BOLD RED "No catefiles found" COLOR_RESET "\n";

					return 0;
				} break;
				
				case 'h':
				case '?': //shows help
					help();
					return 0; //exit after
					break;
				
				case 'D': //disable system
					system_allowed = false;
					break;
				
				case 'S': //disable system
					force_smol = true;
					break;

				case 'f': //force rebuild
					force_rebuild = true;
					break;
			
				default: //unknown
					command_error(
						string("Unknown argument \"") + argv[i] + "\", "
							   "Use " BOLD BLUE "cate " GREEN "-h " COLOR_RESET "to see valid arguments"
					);
					break;
				}
			}
			else //must be a filename now
			{
				if (file_name.empty())
					file_name = argv[i];
				else
					command_error("Cannot build more than one catefile per command.");
			}
		}
	}

	if (!file_name.empty() && !Util::ends_with(file_name, ".cate"))
		file_name += ".cate";

	if(catel_exists) parse_catel();

	if(file_name.empty())
	{
		if (get_default_file_name())
		{
			if(file_name.empty()) goto no_file; // don't ask
		}
		else
		{
			no_file:
			command_error("No input file");
		}
	}

	Parser* parser = new Parser(file_name);

	delete parser; //yay

#ifdef TRACK_ALLOCS
	std::cout << "Total allocs: " << allocs_count << "\n";
#endif // TRACK_ALLOCS

	return 0;
}

bool get_default_file_name()
{
	using namespace Util;
	if (file_exists("build.cate"))
	{
		file_name = "build.cate";
	}
	else if(file_exists("cate/build.cate"))
	{
		file_name = "cate/build.cate";
	}
	else
	{
		return false;
	}
	return true;
}

//this is a maybe
/*bool call_competitor()
{
	using namespace Util;
	if(file_exists("Makefile") || file_exists("makefile"))
	{
		std::cout << "No catefiles found, but found a makefile.\nRunning Make\n";
		system("make -j" + std::to_string(thread_count));
		return true;
	}
}*/