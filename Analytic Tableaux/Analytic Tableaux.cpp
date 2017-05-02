// Analytic Tableaux.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "fol.hpp"
#include "tableaux.h"

#include <string>
#include <fstream>

using namespace std;

extern int yyparse();

/* Ovaj pokazivac ce nakon parsiranja dobiti vrednost
adrese parsirane formule. */
extern Formula parsed_formula;

int main(int argc, char **argv)
{
	// Check if the number of arguments is inaccurate
	if (argc > 2)
	{
		cerr << "Too many arguments! The correct syntax for calling this program is:" << endl;
		cerr << "\tAnalytic Tableaux.exe" << endl;
		cerr << "\tAnalytic Tableaux.exe --help" << endl;
		exit(EXIT_FAILURE);
	}

	// Check if there is an additional argument "--help"
	if (argc == 2)
	{
		// If that argument is not "--help"
		if (string(argv[1]) != "--help")
		{
			cerr << "Unknown argument! The correct syntax for calling this program is:" << endl;
			cerr << "\tAnalytic Tableaux.exe" << endl;
			cerr << "\tAnalytic Tableaux.exe --help" << endl;
			exit(EXIT_FAILURE);
		}
		// Otherwise, show the help
		{
			ifstream infile;
			string fileloc[] = { ".\\help.txt", "..\\Analytic Tableaux\\help.txt" };
			string line;

			cout << "Welcome to Analytic Tableaux HELP!" << endl << endl;

			for (string & fileName : fileloc)
			{
				infile = ifstream(fileName.c_str());
				if (infile.good())
				{
					break;
				}
			}
			
			while (!infile.eof())
			{
				getline(infile, line);
				cout << line << endl;
			}
			infile.close();
			return 0;
		}
	}

	cout << "Welcome to Analytic Tableaux!" << endl;
	cout << "Please type in a proposition calculus formula to generate its tableaux." << endl;
	cout << "If you need help, run this program again with option --help." << endl << endl;

	yyparse();
	cout << endl;

	if (parsed_formula.get() != 0)
	{
		Tableaux t(parsed_formula);
		bool result = t.getResult();

		cout << "Your formula is " << (result ? "SATISTIFABLE" : "UNSATISFIABLE") << endl;
	}

	return 0;
}