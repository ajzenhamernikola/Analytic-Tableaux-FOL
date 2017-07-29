#include "stdafx.h"
#include "fol.hpp"

// ----------------------------------------------------------------------------
// VariableTerm

VariableTerm::VariableTerm(const Variable & v)
	:_v(v)
{}

BaseTerm::Type VariableTerm::getType() const
{
	return TT_VARIABLE;
}

const Variable & VariableTerm::getVariable() const
{
	return _v;
}

void VariableTerm::printTerm(ostream & ostr) const
{
	ostr << _v;
}

void VariableTerm::getConstants(deque<FunctionSymbol> & d_constants) const
{
	(void)d_constants;
}

bool VariableTerm::equalTo(const Term & t) const
{
	if (t->getType() != BaseTerm::TT_VARIABLE)
	{
		return false;
	}

	if (((VariableTerm*)t.get())->getVariable() != _v)
	{
		return false;
	}

	return true;
}

Term VariableTerm::instantiate(const Variable & v, const Term & t)
{
	if (v == _v)
	{
		return t;
	}
	else
	{
		return shared_from_this();
	}
}

// END VariableTerm
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// FunctionTerm

FunctionTerm::FunctionTerm(const FunctionSymbol & f, const vector<Term> & ops)
	:_f(f),
	_ops(ops)
{}

BaseTerm::Type FunctionTerm::getType() const
{
	return TT_FUNCTION;
}

const FunctionSymbol & FunctionTerm::getSymbol() const
{
	return _f;
}

const vector<Term> & FunctionTerm::getOperands() const
{
	return _ops;
}

void FunctionTerm::printTerm(ostream & ostr) const
{
	ostr << _f;

	for (unsigned i = 0; i < _ops.size(); i++)
	{
		if (i == 0)
			ostr << "(";
		_ops[i]->printTerm(ostr);
		if (i != _ops.size() - 1)
			ostr << ",";
		else
			ostr << ")";
	}
}

void FunctionTerm::getConstants(deque<FunctionSymbol> & d_constants) const
{
	// If the FunctionTerm is a constant, and there is no such constant symbol in the deque ...
	deque<FunctionSymbol>::const_iterator iter = find(d_constants.cbegin(), d_constants.cend(), _f);
	if (_ops.size() == 0 && iter != d_constants.cend())
	{
		// ... then add the symbol in the deque
		d_constants.push_back(_f);
	}
	// Otherwise, go through the arguments and add constants found in them in the deque
	else
	{
		for (unsigned i = 0; i < _ops.size(); ++i)
		{
			_ops[i]->getConstants(d_constants);
		}
	}
}

bool FunctionTerm::equalTo(const Term & t) const
{
	if (t->getType() != BaseTerm::TT_FUNCTION)
	{
		return false;
	}

	FunctionTerm * ft = (FunctionTerm*)t.get();
	if (ft->getOperands().size() != _ops.size())
	{
		return false;
	}

	for (unsigned i = 0; i < _ops.size(); ++i)
	{
		if (!_ops[i]->equalTo(ft->getOperands()[i]))
		{
			return false;
		}
	}

	return true;
}

Term FunctionTerm::instantiate(const Variable & v, const Term & t)
{
	vector<Term> instOps;
	for (unsigned i = 0; i < _ops.size(); ++i)
	{
		instOps.push_back(_ops[i]->instantiate(v, t));
	}
	
	return make_shared<FunctionTerm>(_f, instOps);
}

// END FunctionTerm
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// AtomicFormula

Formula AtomicFormula::releaseIff()
{
	return shared_from_this();
}

Formula AtomicFormula::absorbConstants()
{
	return shared_from_this();
}

// END AtomicFormula
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// LogicConstant

void LogicConstant::getConstants(deque<FunctionSymbol> & d_constants) const
{
	(void)d_constants;
}

bool LogicConstant::equalTo(const Formula & f) const
{
	if (f->getType() != getType())
	{
		return false;
	}

	return true;
}

Formula LogicConstant::instantiate(const Variable & v, const Term & t)
{
	(void)v;
	(void)t;
	return shared_from_this();
}

// END LogicConstant
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// True

void True::printFormula(ostream & ostr) const
{
	ostr << "true";
}

BaseFormula::Type True::getType() const
{
	return T_TRUE;
}

// it is possible to regard true as an abbreviation for the formula p \/ ~p
Formula True::transformToDisjunction() const
{
	Formula p = make_shared<Atom>("p");
	return make_shared<Or>(p, make_shared<Not>(p));
}

// END True
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// False

void False::printFormula(ostream & ostr) const
{
	ostr << "false";
}

BaseFormula::Type False::getType() const
{
	return T_FALSE;
}

// it is possible to regard false as an abbreviation for the formula p /\ ~p
Formula False::transformToConjunction() const
{
	Formula p = make_shared<Atom>("p");
	return make_shared<And>(p, make_shared<Not>(p));
}

// END False
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Atom

Atom::Atom(const PredicateSymbol & p,
	const vector<Term> & ops)
	:_p(p),
	_ops(ops)
{}

const PredicateSymbol & Atom::getSymbol() const
{
	return _p;
}

const vector<Term> & Atom::getOperands() const
{
	return _ops;
}

void Atom::printFormula(ostream & ostr) const
{
	ostr << _p;
	for (unsigned i = 0; i < _ops.size(); i++)
	{
		if (i == 0)
			ostr << "(";
		_ops[i]->printTerm(ostr);
		if (i != _ops.size() - 1)
			ostr << ",";
		else
			ostr << ")";
	}
}

BaseFormula::Type Atom::getType() const
{
	return T_ATOM;
}

void Atom::getConstants(deque<FunctionSymbol> & d_constants) const
{
	for (unsigned i = 0; i < _ops.size(); ++i)
	{
		_ops[i]->getConstants(d_constants);
	}
}

bool Atom::equalTo(const Formula & f) const
{
	if (f->getType() != BaseFormula::T_ATOM)
	{
		return false;
	}

	Atom * fa = (Atom*)f.get();
	if (fa->getOperands().size() != _ops.size())
	{
		return false;
	}

	for (unsigned i = 0; i < _ops.size(); ++i)
	{
		if (!_ops[i]->equalTo(fa->getOperands()[i]))
		{
			return false;
		}
	}

	return true;
}

Formula Atom::instantiate(const Variable & v, const Term & t)
{
	vector<Term> instOps;
	for (unsigned i = 0; i < _ops.size(); ++i)
	{
		instOps.push_back(_ops[i]->instantiate(v, t));
	}
	
	return make_shared<Atom>(_p, instOps);
}

// END Atom
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// UnaryConjective

UnaryConjective::UnaryConjective(const Formula & op)
	:_op(op)
{}

const Formula & UnaryConjective::getOperand() const
{
	return _op;
}

void UnaryConjective::getConstants(deque<FunctionSymbol> & d_constants) const
{
	_op->getConstants(d_constants);
}

// END UnaryConjective
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Not

Not::Not(const Formula & op)
	:UnaryConjective(op)
{}

void Not::printFormula(ostream & ostr) const
{
	ostr << "~";
	Type op_type = _op->getType();

	if (op_type == T_AND || op_type == T_OR ||
		op_type == T_IMP || op_type == T_IFF)
		ostr << "(";

	_op->printFormula(ostr);

	if (op_type == T_AND || op_type == T_OR ||
		op_type == T_IMP || op_type == T_IFF)
		ostr << ")";
}

BaseFormula::Type Not::getType() const
{
	return T_NOT;
}

Formula Not::releaseIff()
{
	Formula releasedIffOp = _op->releaseIff();
	return make_shared<Not>(releasedIffOp);
}

Formula Not::absorbConstants()
{
	Formula absOp = _op->absorbConstants();

	if (absOp->getType() == T_TRUE)
	{
		return make_shared<False>();
	}
	else if (absOp->getType() == T_FALSE)
	{
		return make_shared<True>();
	}
	else
	{
		return make_shared<Not>(absOp);
	}
}

bool Not::equalTo(const Formula & f) const
{
	if (f->getType() != BaseFormula::T_NOT)
	{
		return false;
	}

	Not* fn = (Not*)f.get();
	if (!fn->getOperand()->equalTo(_op))
	{
		return false;
	}

	return true;
}

Formula Not::instantiate(const Variable & v, const Term & t)
{
	return make_shared<Not>(_op->instantiate(v, t));
}

// END Not
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// BinaryConjective

BinaryConjective::BinaryConjective(const Formula & op1, const Formula & op2)
	:_op1(op1),
	_op2(op2)
{}

const Formula & BinaryConjective::getOperand1() const
{
	return _op1;
}

const Formula & BinaryConjective::getOperand2() const
{
	return _op2;
}

void BinaryConjective::getConstants(deque<FunctionSymbol> & d_constants) const
{
	_op1->getConstants(d_constants);
	_op2->getConstants(d_constants);
}

// END BinaryConjective
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// And

And::And(const Formula & op1, const Formula & op2)
	:BinaryConjective(op1, op2)
{}

void And::printFormula(ostream & ostr) const
{
	Type op1_type = _op1->getType();
	Type op2_type = _op2->getType();

	if (op1_type == T_OR || op1_type == T_IMP ||
		op1_type == T_IFF)
		ostr << "(";

	_op1->printFormula(ostr);

	if (op1_type == T_OR || op1_type == T_IMP ||
		op1_type == T_IFF)
		ostr << ")";

	ostr << " & ";

	if (op2_type == T_OR || op2_type == T_IMP ||
		op2_type == T_IFF || op2_type == T_AND)
		ostr << "(";

	_op2->printFormula(ostr);

	if (op2_type == T_OR || op2_type == T_IMP ||
		op2_type == T_IFF || op2_type == T_AND)
		ostr << ")";
}

BaseFormula::Type And::getType() const
{
	return T_AND;
}

Formula And::releaseIff()
{
	Formula releasedIffOp1 = _op1->releaseIff();
	Formula releasedIffOp2 = _op2->releaseIff();
	return make_shared<And>(releasedIffOp1, releasedIffOp2);
}

Formula And::absorbConstants()
{
	Formula absOp1 = _op1->absorbConstants();
	Formula absOp2 = _op2->absorbConstants();

	if (absOp1->getType() == T_FALSE || absOp2->getType() == T_FALSE)
	{
		return make_shared<False>();
	}
	else if (absOp1->getType() == T_TRUE)
	{
		return absOp2;
	}
	else if (absOp2->getType() == T_TRUE)
	{
		return absOp1;
	}
	else
	{
		return make_shared<And>(absOp1, absOp2);
	}
}

bool And::equalTo(const Formula & f) const
{
	if (f->getType() != BaseFormula::T_AND)
	{
		return false;
	}

	And* fa = (And*)f.get();
	if (!fa->getOperand1()->equalTo(_op1))
	{
		return false;
	}
	if (!fa->getOperand2()->equalTo(_op2))
	{
		return false;
	}

	return true;
}

Formula And::instantiate(const Variable & v, const Term & t)
{
	return make_shared<And>(_op1->instantiate(v, t), _op2->instantiate(v, t));
}

// END And
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Or

Or::Or(const Formula & op1, const Formula & op2)
	:BinaryConjective(op1, op2)
{}

void Or::printFormula(ostream & ostr) const
{
	Type op1_type = _op1->getType();
	Type op2_type = _op2->getType();

	if (op1_type == T_IMP || op1_type == T_IFF)
		ostr << "(";

	_op1->printFormula(ostr);

	if (op1_type == T_IMP || op1_type == T_IFF)
		ostr << ")";

	ostr << " | ";

	if (op2_type == T_IMP ||
		op2_type == T_IFF || op2_type == T_OR)
		ostr << "(";

	_op2->printFormula(ostr);

	if (op2_type == T_IMP ||
		op2_type == T_IFF || op2_type == T_OR)
		ostr << ")";
}

BaseFormula::Type Or::getType() const
{
	return T_OR;
}

Formula Or::releaseIff()
{
	Formula releasedIffOp1 = _op1->releaseIff();
	Formula releasedIffOp2 = _op2->releaseIff();
	return make_shared<Or>(releasedIffOp1, releasedIffOp2);
}

Formula Or::absorbConstants()
{
	Formula absOp1 = _op1->absorbConstants();
	Formula absOp2 = _op2->absorbConstants();

	if (absOp1->getType() == T_TRUE || absOp2->getType() == T_TRUE)
	{
		return make_shared<True>();
	}
	else if (absOp1->getType() == T_FALSE)
	{
		return absOp2;
	}
	else if (absOp2->getType() == T_FALSE)
	{
		return absOp1;
	}
	else
	{
		return make_shared<Or>(absOp1, absOp2);
	}
}

bool Or::equalTo(const Formula & f) const
{
	if (f->getType() != BaseFormula::T_OR)
	{
		return false;
	}

	Or* fo = (Or*)f.get();
	if (!fo->getOperand1()->equalTo(_op1))
	{
		return false;
	}
	if (!fo->getOperand2()->equalTo(_op2))
	{
		return false;
	}

	return true;
}

Formula And::instantiate(const Variable & v, const Term & t)
{
	return make_shared<Or>(_op1->instantiate(v, t), _op2->instantiate(v, t));
}

// END Or
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Imp

Imp::Imp(const Formula & op1, const Formula & op2)
	:BinaryConjective(op1, op2)
{}

void Imp::printFormula(ostream & ostr) const
{
	Type op1_type = _op1->getType();
	Type op2_type = _op2->getType();

	if (op1_type == T_IFF)
		ostr << "(";

	_op1->printFormula(ostr);

	if (op1_type == T_IFF)
		ostr << ")";

	ostr << " => ";

	if (op2_type == T_IMP || op2_type == T_IFF)
		ostr << "(";

	_op2->printFormula(ostr);

	if (op2_type == T_IMP || op2_type == T_IFF)
		ostr << ")";
}

BaseFormula::Type Imp::getType() const
{
	return T_IMP;
}

Formula Imp::releaseIff()
{
	Formula releasedIffOp1 = _op1->releaseIff();
	Formula releasedIffOp2 = _op2->releaseIff();
	return make_shared<Imp>(releasedIffOp1, releasedIffOp2);
}

Formula Imp::absorbConstants()
{
	Formula absOp1 = _op1->absorbConstants();
	Formula absOp2 = _op2->absorbConstants();

	if (absOp1->getType() == T_TRUE)
	{
		return absOp2;
	}
	else if (absOp2->getType() == T_TRUE)
	{
		return make_shared<True>();
	}
	else if (absOp1->getType() == T_FALSE)
	{
		return make_shared<True>();
	}
	else if (absOp2->getType() == T_FALSE)
	{
		return make_shared<Not>(absOp1);
	}
	else
	{
		return make_shared<Imp>(absOp1, absOp2);
	}
}

bool Imp::equalTo(const Formula & f) const
{
	if (f->getType() != BaseFormula::T_IMP)
	{
		return false;
	}

	Imp* fi = (Imp*)f.get();
	if (!fi->getOperand1()->equalTo(_op1))
	{
		return false;
	}
	if (!fi->getOperand2()->equalTo(_op2))
	{
		return false;
	}

	return true;
}

Formula Imp::instantiate(const Variable & v, const Term & t)
{
	return make_shared<Imp>(_op1->instantiate(v, t), _op2->instantiate(v, t));
}

// END Imp
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Iff

Iff::Iff(const Formula & op1, const Formula & op2)
	:BinaryConjective(op1, op2)
{}

void Iff::printFormula(ostream & ostr) const
{
	Type op1_type = _op1->getType();
	Type op2_type = _op2->getType();

	_op1->printFormula(ostr);

	ostr << " => ";

	if (op2_type == T_IFF)
		ostr << "(";

	_op2->printFormula(ostr);

	if (op2_type == T_IFF)
		ostr << ")";
}

BaseFormula::Type Iff::getType() const
{
	return T_IFF;
}

Formula Iff::releaseIff()
{
	Formula releasedIffOp1 = _op1->releaseIff();
	Formula releasedIffOp2 = _op2->releaseIff();
	return make_shared<And>(
		make_shared<Imp>(releasedIffOp1, releasedIffOp2),
		make_shared<Imp>(releasedIffOp2, releasedIffOp1)
		);
}

Formula Iff::absorbConstants()
{
	Formula absOp1 = _op1->absorbConstants();
	Formula absOp2 = _op2->absorbConstants();

	if (absOp1->getType() == T_FALSE || absOp2->getType() == T_FALSE)
	{
		return make_shared<True>();
	}
	else if (absOp1->getType() == T_TRUE)
	{
		return absOp2;
	}
	else if (absOp2->getType() == T_TRUE)
	{
		return absOp1;
	}
	else if (absOp1->getType() == T_FALSE)
	{
		return make_shared<Not>(absOp2);
	}
	else if (absOp2->getType() == T_FALSE)
	{
		return make_shared<Not>(absOp1);
	}
	else
	{
		return make_shared<Iff>(absOp1, absOp2);
	}
}

bool Iff::equalTo(const Formula & f) const
{
	if (f->getType() != BaseFormula::T_IFF)
	{
		return false;
	}

	Iff* fi = (Iff*)f.get();
	if (!fi->getOperand1()->equalTo(_op1))
	{
		return false;
	}
	if (!fi->getOperand2()->equalTo(_op2))
	{
		return false;
	}

	return true;
}

Formula Iff::instantiate(const Variable & v, const Term & t)
{
	return make_shared<Iff>(_op1->instantiate(v, t), _op2->instantiate(v, t));
}

// END Iff
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Quantifier

Quantifier::Quantifier(const Variable & v, const Formula & op)
	:_v(v),
	_op(op)
{}

const Variable & Quantifier::getVariable() const
{
	return _v;
}

const Formula & Quantifier::getOperand() const
{
	return _op;
}

void Quantifier::getConstants(deque<FunctionSymbol> & d_constants) const
{
	_op->getConstants(d_constants);
}

// END Quantifier
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forall

Forall::Forall(const Variable & v, const Formula & op)
	:Quantifier(v, op)
{}

BaseFormula::Type Forall::getType() const
{
	return T_FORALL;
}

void Forall::printFormula(ostream & ostr) const
{
	cout << "![" << _v << "] : ";

	Type op_type = _op->getType();

	if (op_type == T_AND || op_type == T_OR ||
		op_type == T_IMP || op_type == T_IFF)
		ostr << "(";

	_op->printFormula(ostr);

	if (op_type == T_AND || op_type == T_OR ||
		op_type == T_IMP || op_type == T_IFF)
		ostr << ")";
}

bool Forall::equalTo(const Formula & f) const
{
	if (f->getType() != BaseFormula::T_FORALL)
	{
		return false;
	}

	Forall *ff = (Forall *)f.get();
	if (ff->getVariable() != _v)
	{
		return false;
	}
	if (!ff->getOperand()->equalTo(_op))
	{
		return false;
	}

	return true;
}

Formula Forall::releaseIff()
{
	Formula releasedIffOp = _op->releaseIff();
	
	return make_shared<Forall>(_v, releasedIffOp);
}

Formula Forall::absorbConstants()
{
	Formula absOp = _op->absorbConstants();
	
	if (absOp->getType() == BaseFormula::T_TRUE)
	{
		return make_shared<True>();
	}
	else if (absOp->getType() == BaseFormula::T_FALSE)
	{
		return make_shared<False>();
	}
	else
	{
		return make_shared<Exists>(_v, absOp);
	}
}

Formula Forall::instantiate(const Variable & v, const Term & t)
{
	if (_v == v)
	{
		return _op->instantiate();
	}
	else
	{
		return make_shared<Forall>(_v, _op->instantiate(v, t));
	}
}

// END Forall
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Exists

Exists::Exists(const Variable & v, const Formula & op)
	:Quantifier(v, op)
{}

BaseFormula::Type Exists::getType() const
{
	return T_EXISTS;
}

void Exists::printFormula(ostream & ostr) const
{
	cout << "?[" << _v << "] : ";

	Type op_type = _op->getType();

	if (op_type == T_AND || op_type == T_OR ||
		op_type == T_IMP || op_type == T_IFF)
		ostr << "(";

	_op->printFormula(ostr);

	if (op_type == T_AND || op_type == T_OR ||
		op_type == T_IMP || op_type == T_IFF)
		ostr << ")";
}

bool Exists::equalTo(const Formula & f) const
{
	if (f->getType() != BaseFormula::T_FORALL)
	{
		return false;
	}

	Exists *fe = (Exists *)f.get();
	if (fe->getVariable() != _v)
	{
		return false;
	}
	if (!fe->getOperand()->equalTo(_op))
	{
		return false;
	}

	return true;
}

Formula Exists::releaseIff()
{
	Formula releasedIffOp = _op->releaseIff();
	
	return make_shared<Exists>(_v, releasedIffOp);
}

Formula Exists::absorbConstants()
{
	Formula absOp = _op->absorbConstants();
	
	if (absOp->getType() == BaseFormula::T_TRUE)
	{
		return make_shared<True>();
	}
	else if (absOp->getType() == BaseFormula::T_FALSE)
	{
		return make_shared<False>();
	}
	else
	{
		return make_shared<Exists>(_v, absOp);
	}
}

Formula Forall::instantiate(const Variable & v, const Term & t)
{
	if (_v == v)
	{
		return _op->instantiate();
	}
	else
	{
		return make_shared<Exists>(_v, _op->instantiate(v, t));
	}
}

// END Exists
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Other functions

ostream & operator << (ostream & ostr, const Term & t)
{
	t->printTerm(ostr);
	return ostr;
}

ostream & operator << (ostream & ostr, const Formula & f)
{
	f->printFormula(ostr);
	return ostr;
}

FunctionSymbol getUniqueConstantSymbol(deque<FunctionSymbol> & d_constants)
{
	static unsigned i = 0;
	FunctionSymbol unique_constant = "uc" + to_string(i);

	deque<FunctionSymbol>::const_iterator b = d_constants.cbegin();
	deque<FunctionSymbol>::const_iterator e = d_constants.cend();

	for (; b != e; ++b)
	{
		if (*b == unique_constant)
		{
			++i;
			unique_constant = "uc" + to_string(i);
		}
	}

	return unique_constant;
}
