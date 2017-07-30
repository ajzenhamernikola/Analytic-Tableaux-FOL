#ifndef _FOL_H
#define _FOL_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <deque>

using namespace std;

typedef string FunctionSymbol;
typedef string PredicateSymbol;
typedef string Variable;

class BaseTerm;
typedef shared_ptr<BaseTerm> Term;

class BaseTerm : public enable_shared_from_this<BaseTerm>
{
public:
	enum Type { TT_VARIABLE, TT_FUNCTION };
	virtual Type getType() const = 0;
	virtual void printTerm(ostream & ostr) const = 0;
	virtual void getConstants(deque<FunctionSymbol> & d_constants) const = 0;
	virtual bool equalTo(const Term & t) const = 0;
	virtual Term instantiate(const Variable & v, const Term & t) = 0;

	virtual ~BaseTerm() {}
};

class VariableTerm : public BaseTerm
{
private:
	Variable _v;
public:
	VariableTerm(const Variable & v);

	virtual Type getType() const;
	const Variable & getVariable() const;
	virtual void printTerm(ostream & ostr) const;
	virtual void getConstants(deque<FunctionSymbol> & d_constants) const;
	virtual bool equalTo(const Term & t) const;
	virtual Term instantiate(const Variable & v, const Term & t);
};

class FunctionTerm : public BaseTerm
{
private:
	FunctionSymbol _f;
	vector<Term> _ops;
public:
	FunctionTerm(const FunctionSymbol & f,
		const vector<Term> & ops = vector<Term>());

	virtual Type getType() const;
	const FunctionSymbol & getSymbol() const;
	const vector<Term> & getOperands() const;
	virtual void printTerm(ostream & ostr) const;
	virtual void getConstants(deque<FunctionSymbol> & d_constants) const;
	virtual bool equalTo(const Term & t) const;
	virtual Term instantiate(const Variable & v, const Term & t);
};

class BaseFormula;

typedef shared_ptr<BaseFormula> Formula;

class BaseFormula : public enable_shared_from_this<BaseFormula>
{
public:
	enum Type {
		T_TRUE, T_FALSE, T_ATOM, T_NOT,
		T_AND, T_OR, T_IMP, T_IFF, T_FORALL, T_EXISTS
	};

	virtual void printFormula(ostream & ostr) const = 0;
	virtual Type getType() const = 0;
	virtual Formula releaseIff() = 0;
	virtual Formula absorbConstants() = 0;
	virtual void getConstants(deque<FunctionSymbol> & d_constants) const = 0;
	virtual bool equalTo(const Formula & f) const = 0;
	virtual Formula instantiate(const Variable & v, const Term & t) = 0;

	virtual ~BaseFormula() {}
};

class AtomicFormula : public BaseFormula
{
public:
	virtual Formula releaseIff();
	virtual Formula absorbConstants();
};

class LogicConstant : public AtomicFormula
{
public:
	virtual void getConstants(deque<FunctionSymbol> & d_constants) const;
	virtual bool equalTo(const Formula & f) const;
	virtual Formula instantiate(const Variable & v, const Term & t);
};

class True : public LogicConstant
{
public:
	virtual void printFormula(ostream & ostr) const;
	virtual Type getType() const;
	Formula transformToDisjunction() const;
};

class False : public LogicConstant
{
public:
	virtual void printFormula(ostream & ostr) const;
	virtual Type getType() const;
	Formula transformToConjunction() const;
};

class Atom : public AtomicFormula
{
protected:
	PredicateSymbol _p;
	vector<Term> _ops;
public:
	Atom(const PredicateSymbol & p,
		const vector<Term> & ops = vector<Term>());

	const PredicateSymbol & getSymbol() const;
	const vector<Term> & getOperands() const;

	virtual void printFormula(ostream & ostr) const;
	virtual Type getType() const;
	virtual void getConstants(deque<FunctionSymbol> & d_constants) const;
	virtual bool equalTo(const Formula & f) const;
	virtual Formula instantiate(const Variable & v, const Term & t);
};

class Equality : public Atom
{
public:
	Equality(const Term & lop, const Term & rop)
		:Atom("=", vector<Term>())
	{
		_ops.push_back(lop);
		_ops.push_back(rop);
	}

	const Term & getLeftOperand() const
	{
		return _ops[0];
	}

	const Term & getRightOperand() const
	{
		return _ops[1];
	}

	virtual void printFormula(ostream & ostr) const
	{
		_ops[0]->printTerm(ostr);
		ostr << " = ";
		_ops[1]->printTerm(ostr);
	}

	virtual Formula releaseIff()
	{
		throw "Not applicable!";
	}
	
	virtual Formula absorbConstants()
	{
		throw "Not applicable!";
	}
	
	virtual Formula instantiate(const Variable & v, const Term & t)
	{
		throw "Not applicable!";
	}
};

class Disequality : public Atom
{
public:
	Disequality(const Term & lop, const Term & rop)
		:Atom("~=", vector<Term>())
	{
		_ops.push_back(lop);
		_ops.push_back(rop);
	}

	const Term & getLeftOperand() const
	{
		return _ops[0];
	}

	const Term & getRightOperand() const
	{
		return _ops[1];
	}

	virtual void printFormula(ostream & ostr) const
	{

		_ops[0]->printTerm(ostr);
		ostr << " ~= ";
		_ops[1]->printTerm(ostr);
	}

	virtual Formula releaseIff()
	{
		throw "Not applicable!";
	}
	
	virtual Formula absorbConstants()
	{
		throw "Not applicable!";
	}
	
	virtual Formula instantiate(const Variable & v, const Term & t)
	{
		throw "Not applicable!";
	}
};

class UnaryConjective : public BaseFormula
{
protected:
	Formula _op;
public:
	UnaryConjective(const Formula & op);

	const Formula & getOperand() const;

	virtual void getConstants(deque<FunctionSymbol> & d_constants) const;
};

class Not : public UnaryConjective
{
public:
	Not(const Formula & op);

	virtual void printFormula(ostream & ostr) const;
	virtual Type getType() const;
	virtual Formula releaseIff();
	virtual Formula absorbConstants();
	virtual bool equalTo(const Formula & f) const;
	virtual Formula instantiate(const Variable & v, const Term & t);
};

class BinaryConjective : public BaseFormula
{
protected:
	Formula _op1, _op2;
public:
	BinaryConjective(const Formula & op1, const Formula & op2);

	const Formula & getOperand1() const;
	const Formula & getOperand2() const;

	virtual void getConstants(deque<FunctionSymbol> & d_constants) const;
};

class And : public BinaryConjective
{
public:
	And(const Formula & op1, const Formula & op2);

	virtual void printFormula(ostream & ostr) const;
	virtual Type getType() const;
	virtual Formula releaseIff();
	virtual Formula absorbConstants();
	virtual bool equalTo(const Formula & f) const;
	virtual Formula instantiate(const Variable & v, const Term & t);
};

class Or : public BinaryConjective
{
public:
	Or(const Formula & op1, const Formula & op2);

	virtual void printFormula(ostream & ostr) const;
	virtual Type getType() const;
	virtual Formula releaseIff();
	virtual Formula absorbConstants();
	virtual bool equalTo(const Formula & f) const;
	virtual Formula instantiate(const Variable & v, const Term & t);
};

class Imp : public BinaryConjective
{
public:
	Imp(const Formula & op1, const Formula & op2);

	virtual void printFormula(ostream & ostr) const;
	virtual Type getType() const;
	virtual Formula releaseIff();
	virtual Formula absorbConstants();
	virtual bool equalTo(const Formula & f) const;
	virtual Formula instantiate(const Variable & v, const Term & t);
};

class Iff : public BinaryConjective
{
public:
	Iff(const Formula & op1, const Formula & op2);

	virtual void printFormula(ostream & ostr) const;
	virtual Type getType() const;
	virtual Formula releaseIff();
	virtual Formula absorbConstants();
	virtual bool equalTo(const Formula & f) const;
	virtual Formula instantiate(const Variable & v, const Term & t);
};

class Quantifier : public BaseFormula
{
protected:
	Variable _v;
	Formula  _op;
public:
	Quantifier(const Variable & v, const Formula & op);

	const Variable & getVariable() const;
	const Formula & getOperand() const;
	virtual void getConstants(deque<FunctionSymbol> & d_constants) const;
};

class Forall : public Quantifier
{
public:
	Forall(const Variable & v, const Formula & op);

	virtual Type getType() const;
	virtual void printFormula(ostream & ostr) const;
	virtual bool equalTo(const Formula & f) const;
	virtual Formula releaseIff();
	virtual Formula absorbConstants();
	virtual Formula instantiate(const Variable & v, const Term & t);
};

class Exists : public Quantifier
{
public:
	Exists(const Variable & v, const Formula & op);

	virtual Type getType() const;
	virtual void printFormula(ostream & ostr) const;
	virtual bool equalTo(const Formula & f) const;
	virtual Formula releaseIff();
	virtual Formula absorbConstants();
	virtual Formula instantiate(const Variable & v, const Term & t);
};

ostream & operator << (ostream & ostr, const Term & t);
ostream & operator << (ostream & ostr, const Formula & f);

FunctionSymbol getUniqueConstantSymbol(deque<FunctionSymbol> & d_constants);

extern Formula parsed_formula;

#endif // _FOL_H
