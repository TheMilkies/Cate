#include "Parser/Parser.hpp" //Parser.hpp includes everything we need, including Util.hpp

#ifdef MODS
#include "Mods/mods.h"
#endif // MODS

//parser_exit is needed to show all errors and exit afterwards
//system_allowed is the -D option, only affects `system(String)` in parser
bool parser_exit = false, system_allowed = true,
	 force_rebuild = false, force_smol = false;

int32_t thread_count = std::thread::hardware_concurrency() * 2;

void help();

string file_name, dir = (fs::is_directory("cate") == true) ? "cate" : "./";

void parse_catel(); // Catel.cpp
bool get_default_file_name();

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
		#ifdef CALL_COMPETITOR
			if(call_competitor()) return 0;
		#endif // CALL_COMPETITOR
			cout << "No catefiles detected, Have some help\n";
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
			case 'j': //uncomment this if you want compatibility with nake
			case 't': {
				char* num;

				if(argv[i][2] != NULL)
					num = argv[i]+2;
				else if(argv[i+1] != NULL)
					num = argv[++i]; //skip next because it's an int
				else
					command_error("Missing argument \"-t\"");

				int sub = atoi(num); //get everything after "-t"
				if (sub != 0) //if 0 or invalid
					thread_count = sub;

				//cout << thread_count << '\n;//debug

			}	break;

			case 'v': //cate version
				cout << CATE_VERSION "\n";
				return 0; //exit after
				break;

			case 'l':{ //list directory
				bool catefiles = false;

				if(catel_exists) parse_catel();

				string all; all.reserve(128);
				for (const auto &p : fs::directory_iterator(dir)) //iterate over the files
				{
					if(catefiles) all += ", ";
					if (p.path().extension() == ".cate")
					{
						all += p.path().stem().string();
						catefiles = true;
					}
				}
				if (catefiles)
					cout << CYAN << all << COLOR_RESET "\n";
				else
					cout << BOLD RED "No catefiles found" COLOR_RESET "\n";

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

			case 'B': //make compatibility
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

	if (!file_name.empty() && !ends_with(file_name, ".cate"))
		file_name += ".cate";

	if(catel_exists) parse_catel();

	if(file_name.empty())
	{
		if (!get_default_file_name() || file_name.empty())
			command_error("No input file");
	#ifdef CALL_COMPETITOR
		else if(call_competitor()) return 0; 
	#endif
	}

	//cout << file_name << '\n; //debug

	Parser* parser = new Parser(file_name);

	delete parser; //yay

#ifdef TRACK_ALLOCS
	cout << "Total allocs: " << allocs_count << "\n";
#endif // TRACK_ALLOCS

	return 0;
}

bool get_default_file_name()
{
	using namespace Util;
	if (file_exists("build.cate"))
		file_name = "build.cate";
	else if(file_exists("cate/build.cate"))
		file_name = "cate/build.cate";
	else
		return false;
	return true;
}
