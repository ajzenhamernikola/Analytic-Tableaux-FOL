#ifndef _TABLEAUX_H
#define _TABLEAUX_H

#include <algorithm>
#include <deque>
#include <iostream>

#include "fol.hpp"

class BaseSignedFormula;

typedef shared_ptr<BaseSignedFormula> SignedFormula;

class BaseSignedFormula
{
public:
	BaseSignedFormula(const Formula & f, bool sign)
		:_f(f), _sign(sign)
	{}

	Formula getFormula() const;
	bool getSign() const;
	void printSignedFormula(ostream & ostr) const;

	~BaseSignedFormula()
	{}

private:
	Formula _f;
	bool _sign;
};

ostream & operator<<(ostream & ostr, SignedFormula sf);

class Tableaux
{
public:
	Tableaux(const Formula & root);

	bool getResult() const;

	~Tableaux()
	{}

private:
	SignedFormula _root;
	bool _result;

	bool prove(deque<SignedFormula> & deque = deque<SignedFormula>(), int tabs = 0) const;
	bool atomRules(deque<SignedFormula> & deque, const SignedFormula & f, int tabs) const;
	bool notRules(deque<SignedFormula> & deque, const SignedFormula & f, int tabs) const;
	bool andRules(deque<SignedFormula> & deque, const SignedFormula & f, int tabs) const;
	bool orRules(deque<SignedFormula> & deque, const SignedFormula & f, int tabs) const;
	bool impRules(deque<SignedFormula> & deque, const SignedFormula & f, int tabs) const;
};

#endif // _TABLEAUX_H