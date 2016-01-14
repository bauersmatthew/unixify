// NO, I AM NOT PROUD OF THIS CODE.
//
/*
 * File: unixify.cpp
 * Edited: 14 Jan 2016
 * Author: Matthew Bauer
 */

#include <ostream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "optionparser.h"

enum optionIndex {
	UNKNOWN,
	HELP,
	USE_STDIN,
	USE_STDOUT
};
const option::Descriptor usage[] =
{
	{UNKNOWN, 0, "", "", option::Arg::None, "Usage: unixify [options] [file1] [file2] [...]\n\n"
						"Description: a program that modifies text files to remove the windows-style '\\r\\n' line endings and replace them with unix-style '\\n' endings.\n\n"
						"Options:\t\t(note: unrecognized options are automatically ignored)\n"},
	{HELP, 0, "", "help", option::Arg::None, "\t--help\n"
						 "\t\tPrints usage.\n\n"},
	{USE_STDIN, 0, "i", "stdin", option::Arg::None, "\t-i | --stdin\n"
							"\t\tReads file from stdin. Incompatible with ANY file arguments. Implies --stdout.\n\n"},
	{USE_STDOUT, 0, "o", "stdout", option::Arg::None, "\t-o | --stdout\n"
							  "\t\tOutputs to stdout. Incompatible with MULTIPLE file arguments.\n\n"},
	
	{0, 0, 0, 0, 0, 0} // descriptor table end
};

int main(int argc, char **argv)
{
	// vars
	bool u_in = false;
	bool u_out = false;
	std::vector<std::string> paths;

	// parse command line
	argc--; argv += 1; // skip program name (argv[0])
	option::Stats opt_stats(usage, argc, argv);
	option::Option opt_options[opt_stats.options_max], opt_buffer[opt_stats.buffer_max];
	option::Parser opt_parse(usage, argc, argv, opt_options, opt_buffer);
	if(opt_parse.error())
	{
		// something went wrong
		std::cerr << "e: argument parser failed" << std::endl;
		return -1;
	}
	if(!argc) // no args
	{
		std::cerr << "e: no args" << std::endl;
		option::printUsage(std::cerr, usage);
		return -2;
	}
	if(opt_options[HELP]) // --help
	{
		option::printUsage(std::cout, usage);
		return 0;
	}
	if(opt_options[USE_STDIN]) // --stdin
	{
		u_in = true;
		u_out = true; // set them both; --stdin implies --stdout
	}
	if(opt_options[USE_STDOUT]) // --stdout
	{
		u_out = true;
	}
	// input paths
	for(int i = 0; i < opt_parse.nonOptionsCount(); i++)
	{
		paths.push_back(opt_parse.nonOption(i));
	}
	// check that no options given are incompatible
	if (
		u_in && !paths.empty() || // too many inputs
		paths.empty() && !u_in || // no inputs given
		u_out && paths.size() > 1 ) // too many inputs (>1) per output (1)
	{
		std::cerr << "e: bad args given" << std::endl;
		option::printUsage(std::cerr, usage);
		return -3;
	}
	
	// loop through inputs
	std::istream *curr_in;
	std::ostream *curr_out;
	int curr_file = 0;
	while(true)
	{
		// decide input
		if(u_in)
		{
			curr_in = &(std::cin);
		}
		else
		{
			// next file
			curr_in = new std::ifstream(paths.at(curr_file));
			if(!(*curr_in)) // couldnt load
			{
				std::cerr << "e: couldn't open file '" << paths.at(curr_file) << "', continuing..." << std::endl;
				delete curr_in;
				curr_file++;
				continue;
			}
		}
		
		// load, edit as we go
		std::string contents = "";
		char c;
		c = curr_in->get(); // prime
		while(!curr_in->eof())
		{
			if(c != '\r') // process/add
				contents += c;
			c = curr_in->get(); // get next
		}
		
		// close file if needbe
		if(!u_in)
		{
			((std::ifstream*)curr_in)->close();
			delete curr_in;
		}
		
		// decide output
		if(u_out)
		{
			curr_out = &(std::cout);
		}
		else
		{
			// next file
			curr_out = new std::ofstream(paths.at(curr_file)); // deletes everything already in there
			if(!(*curr_out)) // couldn;'t load
			{
				std::cerr << "e: couldn't open file '" << paths.at(curr_file) << "', continuing..." << std::endl;
				delete curr_out;
				curr_file++;
				continue;
			}
		}
		
		// write
		curr_out->write(contents.c_str(), contents.size());
		
		// close file if needbe
		if(!u_out)
		{
			((std::ofstream*)curr_out)->close();
			delete curr_out;
		}
		
		// increment/check file counter if needbe
		if( !(u_in || u_out) )
		{
			curr_file++;
			if(curr_file >= paths.size()) // we done
				break;
		}
		else
		{
			// we done
			break;
		}
	}
	
	// ok so hopefully we're done at this point. lol.
	return 0;
}
