#include "Parser/Parser.hpp" //Parser.hpp includes everything we need, including Util.hpp

//parser_exit is needed to show all errors and exit afterwards
//system_allowed is the -D option, only affects `system(String)` in parser
bool parser_exit   = false, system_allowed = true,
	 force_rebuild = false, force_smol     = false,
	 dont_ask_install = false;
int32_t thread_count = std::thread::hardware_concurrency() * 2;

string default_file, default_directory = "cate";
void parse_catel();
Global global_values;

void help();
inline const char* shift_arg(int &argc, char** &argv) {
	argc--; *argv++;
	
	if (argc <= 0 || argv[0] == NULL)
		return "";

	return argv[0];
}

#define shift_args() shift_arg(argc, argv)

#ifdef TRACK_ALLOCS
int allocs_count = 0;
void* operator new(size_t size)
{
	++allocs_count;
	return malloc(size);
}
#endif // TRACK_ALLOCS

bool default_file_exists()
{
	using Util::file_exists;
	if(file_exists("cate/build.cate"))
		default_file = "cate/build.cate";
	else if(file_exists("build.cate"))
		default_file = "build.cate";
	else return false;

	return true;
}

int main(int argc, char *argv[])
{
	std::ios_base::sync_with_stdio(false); //this is a massive speed boost in some cases.
	using namespace Util;

	if(file_exists(".catel") || file_exists(PLATFORM_CATEL))
		parse_catel();

	if(argc < (ARGC_START+1) && default_file.empty())
	{
		if(!default_file_exists())
		{
			help();
			return 1;
		}
	}

	const char* arg = shift_args();
	std::vector<string> file_names;
	while (argc > 0){
		if(arg[0] != '-')
		{
			string file_in_folder = default_directory + "/" + arg;
			add_cate_ending(file_in_folder);

			if(file_exists(file_in_folder.c_str()))
				file_names.emplace_back(file_in_folder);
			else
			{
				// This might look weird, but it saves one alloc.
				auto& file = file_names.emplace_back(argv[0]);
				add_cate_ending(file);
			}
			arg = shift_args();
			continue;
		}

		switch (argv[0][1]) //check the second character of the argument
		{
		case 'j':
		case 't': {
			char* num;

			if(argv[0][2])
				num = argv[0]+2;
			else if(argv[1] != NULL)
				num = (char*)shift_args(); //skip next because it's an int
			else
				command_error("Missing argument \"-t\".");
			
			int32_t sub = atoi(num); //get everything after "-t"
			if (sub > 0) //if 0 or invalid
				thread_count = sub;
			else
				command_error("Can't set thread count to \"" + string(num) + "\".");
		}	break;

		case 'v': //cate version
			cout << CATE_VERSION << '\n';
			return 0; //exit after
			break;

		case 'l':{ //list directory
			bool catefiles_found = false;

			string all; all.reserve(128);
			for (const auto &p : fs::directory_iterator(default_directory)) //iterate over the files
			{
				if(catefiles_found) all += ", ";
				if (p.path().extension() == ".cate")
				{
					all += p.path().stem().string();
					catefiles_found = true;
				}
			}
			if (catefiles_found)
				cout << CYAN << all << COLOR_RESET "\n";
			else
				cerr << BOLD RED "No catefiles were found" COLOR_RESET "\n";

			exit(0);
		} break;
		
		case 'h':
		case '?': //shows help
			help();
			exit(0); //exit after
			break;
		
		case 'D': //disable system
			system_allowed = false;
			break;
		
		case 'S': //disable system
			force_smol = true;
			break;

		case 'B':
		case 'f': //force rebuild
			force_rebuild = true;
			break;
		
		case 'y':
			dont_ask_install = true;
			break;
	
		default: //unknown
			cerr << "Unknown argument \"" << argv[0] << "\", "
				"Use " BOLD BLUE "cate " GREEN "-h " COLOR_RESET "to see valid arguments\n"
				;
			break;
		}

		arg = shift_args();
	}

	if(default_file.empty() && !default_file_exists())
	{
		command_error("No default file found.");
	}
	
	if(file_names.empty()) file_names.emplace_back(default_file);
	for(auto& name : file_names)
		Parser p(name);	
		
	return 0;
}