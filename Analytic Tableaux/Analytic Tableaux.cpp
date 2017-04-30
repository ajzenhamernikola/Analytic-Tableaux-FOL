// Analytic Tableaux.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "fol.hpp"
#include "tableaux.h"

extern int yyparse();

/* Ovaj pokazivac ce nakon parsiranja dobiti vrednost
adrese parsirane formule. */
extern Formula parsed_formula;

int main()
{
	/*yyparse();

	if (parsed_formula.get() != 0)
		cout << parsed_formula;*/

	Formula p = make_shared<Atom>("p");
	Formula q = make_shared<Atom>("q");
	Formula r = make_shared<Atom>("r");

	// Dokazujemo da vazi:
	// |= ((P => Q) => P) => P

	Formula pom_1 = make_shared<Imp>(p, q);
	Formula pom_2 = make_shared<Imp>(pom_1, p);
	Formula F1 = make_shared<Imp>(pom_2, p);

	cout << "1. Dokazati:" << endl;
	cout << "|= " << F1 << endl << endl;
	
	Tableaux t1(F1);
	bool solution = t1.getResult();
	
	cout << "Resenje: " << solution << endl << endl;

	// Dokazujemo da vazi:
	// |= !(p /\ q) => !p \/ !q

	Formula pom_3 = make_shared<And>(p, q);
	Formula pom_4 = make_shared<Not>(pom_3);
	Formula pom_5 = make_shared<Not>(p);
	Formula pom_6 = make_shared<Not>(q);
	Formula pom_7 = make_shared<Or>(pom_5, pom_6);
	Formula F2 = make_shared<Imp>(pom_4, pom_7);

	cout << "2. Dokazati:" << endl;
	cout << "|= " << F2 << endl << endl;

	Tableaux t2(F2);
	solution = t2.getResult();

	cout << "Resenje: " << solution << endl << endl;

	// Ispitujemo zadovoljivost formule:
	// P /\ !P
	Formula F3 = make_shared<And>(p, pom_5);

	cout << "3. Ispitati zadovoljivost formule: " << endl;
	cout << F3 << endl << endl;

	Tableaux t3(F3);
	solution = t3.getResult();

	cout << "Resenje: " << solution << endl << endl;

	// Dokazujemo:
	// |= (P \/ (Q /\ R)) => ((P \/ Q) /\ (P \/ R)
	Formula pom_8 = make_shared<And>(q, r);
	Formula pom_9 = make_shared<Or>(p, pom_8);
	Formula pom_10 = make_shared<Or>(p, q);
	Formula pom_11 = make_shared<Or>(p, r);
	Formula pom_12 = make_shared<And>(pom_10, pom_11);
	Formula F4 = make_shared<Imp>(pom_9, pom_12);

	cout << "4. Dokazati: " << endl;
	cout << "|= " << F4 << endl << endl;

	Tableaux t4(F4);
	solution = t4.getResult();

	cout << "Resenje: " << solution << endl << endl;

	return 0;
}