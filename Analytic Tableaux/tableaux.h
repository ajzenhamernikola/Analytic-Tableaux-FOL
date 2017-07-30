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
private:
	Formula _f;
	bool _sign;
public:
	enum TableauxType {
		TT_ALPHA, TT_BETA, TT_GAMMA, TT_DELTA, TT_ATOM
	};

	BaseSignedFormula(const Formula & f, bool sign)
		:_f(f), _sign(sign)
	{}

	Formula getFormula() const;
	bool getSign() const;
	void printSignedFormula(ostream & ostr) const;
	TableauxType getType() const;

	~BaseSignedFormula()
	{}
};

class Tableaux
{
private:
	SignedFormula _root;
	bool _result;

	bool prove(deque<SignedFormula> && d_formulae = deque<SignedFormula>(), deque<FunctionSymbol> && d_constants = deque<FunctionSymbol>(), int tabs = 0) const;
	
	bool checkIfExistsComplementaryPairOfLiterals(deque<SignedFormula> & d_formulae) const;
	bool checkIfExistsNonGammaRule(deque<SignedFormula> & d_formulae, SignedFormula & rule, BaseSignedFormula::TableauxType & ruleType) const;
	bool checkIfShouldBranchBeOpenForGammaRule(deque<SignedFormula> & d_formulae, deque<FunctionSymbol> & d_constants) const;

	bool atomRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const;
	bool notRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const;
	bool andRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const;
	bool orRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const;
	bool impRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const;
	bool forallRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const;
	bool existsRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const;
	FunctionSymbol getUniqueConstantSymbol(deque<SignedFormula> & d_formulae = deque<SignedFormula>()) const;
public:
	Tableaux(const Formula & root);

	string getResult() const;

	~Tableaux()
	{}
};

ostream & operator << (ostream & ostr, SignedFormula sf);

inline bool operator==(const SignedFormula& lhs, const SignedFormula& rhs);

template<class T>
ostream & operator << (ostream & ostr, deque<T> & d_T);

bool checkIfAlreadyExistsSuchNode(deque<SignedFormula> & d_nextFormulaeNode);

#endif // _TABLEAUX_H