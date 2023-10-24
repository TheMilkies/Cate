#include "Parser/Parser.hpp" //Parser.hpp includes everything we need, including Util.hpp
#include <fstream>
#include <string.h>

//errors_exist is needed to show all errors and exit afterwards
//system_blocked is the -D option, only affects `system(String)` in parser
bool errors_exist = false, is_root = false;
i32 thread_count  = std::thread::hardware_concurrency() * 2;

string default_file, default_directory = "cate";
void parse_catel();

void init_project(string name);

void help();
inline const char* shift_arg(int &argc, char** &argv) {
	argc--; *argv++;
	
	if (argc <= 0 || argv[0] == NULL)
		return "";

	return argv[0];
}

#define shift_args() shift_arg(argc, argv)

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

extern vector<string> opened_files;
extern std::vector<std::unique_ptr<Class>> classes;

int main(int argc, char *argv[])
{
	std::ios_base::sync_with_stdio(false); //this is a massive speed boost in some cases.
	using namespace Util;

	if(file_exists(".catel") || file_exists(PLATFORM_CATEL))
		parse_catel();

	if(argc < 2 && default_file.empty())
	{
		if(!default_file_exists())
		{
			help();
			return 1;
		}
	}

	is_root = getuid() == 0;
	const char* arg = shift_args();

	std::vector<string> file_names;
	while (argc > 0){
		if(arg[0] != '-')
		{
			string file = arg;
			add_cate_ending_to(file);
			string file_in_folder = default_directory + "/" + file;

			if(file_exists(file_in_folder.c_str()))
				file_names.emplace_back(file_in_folder);
			else
				file_names.emplace_back(file);

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
				command_error("Missing argument after \"-t\".");

			i32 new_count = atoi(num);
			if (new_count <= 0)
				command_error("Can't set thread count to \"" + string(num) + "\".");

			thread_count = new_count;
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
				if (p.path().extension() == ".cate")
				{
					if(catefiles_found) all += ", ";
					if(p.path() == default_file)
						all += BOLD PURPLE + p.path().stem().string() + list_color;
					else
						all += p.path().stem().string();
					catefiles_found = true;
				}
			}

			if (catefiles_found)
				cout << list_color << all << COLOR_RESET "\n";
			else
				cerr << BOLD_RED "No catefiles were found" COLOR_RESET "\n";

			return !catefiles_found; //lazy error
		} break;

		case 'h':
		case '?': //shows help
			help();
			exit(0); //exit after
			break;
		
		case 'D': //disable system()
			global_values.options |= system_blocked;
			break;

		case 'd': //all system calls are No.
			global_values.options |= dry_run;
			break;
		
		case 'S': //disable system
			global_values.options |= force_smol;
			break;

		case 'B':
		case 'f': //force rebuild
			global_values.options |= force_rebuild;
			break;
		
		case 'y':
			global_values.options |= always_allow_install;
			if(!is_root) {
				Util::warn("You're using the `" BOLD_GREEN "-d" COLOR_RESET
				"` flag without being root.");
			}
			break;

		case 'n':
			global_values.options |=  always_deny_install;
			break;

		case 'i':
			char* name;

			if(argv[0][2])
				name = argv[0]+2;
			else if(argv[1] != NULL)
				name = (char*)shift_args(); //skip next because it's an int
			else
				command_error("Missing argument after \"-i\".");
			
			init_project(name);
			return 0;
			break;
	
		default: //unknown
			cerr << "Unknown argument \"" << argv[0] << "\"\n"
				"Use " BOLD BLUE "cate " GREEN "-h " COLOR_RESET "to see valid arguments\n"
				;
			break;
		}

		arg = shift_args();
	}

	if(default_file.empty() && !default_file_exists())
		command_error("No default file found.");

	if(flag(always_deny_install) && flag(always_allow_install)) {
		command_error("Usage of `-n` and `-y` at the same time is impossible.\n"
					  "Will ask if to install or not.");
		//disable the flags
		global_values.options &= (Flags)
		 (~(always_deny_install | always_allow_install));
	}

	if(file_names.empty())
		file_names.emplace_back(default_file);


	Util::generate_object_dir_name(); //save some time
	//start building yay
	classes.reserve(file_names.size()*4);
	for(const auto& name : file_names) {
		Parser p(name);	

		//clear for next run
		opened_files.clear();
		classes.clear();
	}

	return 0;
}

static void create_file(string file_name, string_view text) {
	std::ofstream f(file_name);
	if(f.fail()) {
		Util::error("Failed to create file \"" + file_name + "\" because: " + 
		strerror(errno));
	}
	f << text;
}

void init_project(string name) {
	if(Util::file_exists(".catel") && Util::file_exists("src/main.cpp"))
		Util::command_error("Project already inited.");

	Util::create_folder("src");
	Util::create_folder("include");
	Util::create_folder("cate");

	create_file("src/main.cpp",
	"#include <iostream>\n\n"
	"int main(int argc, const char* argv[])\n"
	"{\n\t\n"
	"}");

	create_file(".catel","def debug");

	string basic_catefile = 
	"cc = \"g++\"\n"
	"Project " + name + "\n"
	".files = {recursive(\"src/*.cpp\")}\n";

	create_file("cate/debug.cate", basic_catefile +
									".flags = \"-g\"\n"
									".build()");
	create_file("cate/release.cate", "smol = true\n" +
									basic_catefile +
									".flags = \"-O2\"\n"
									".build()");
}