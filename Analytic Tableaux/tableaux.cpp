#include "stdafx.h"
#include "tableaux.h"

// ----------------------------------------------------------------------------
// BaseSignedFormula

Formula BaseSignedFormula::getFormula() const
{
	return _f;
}

bool BaseSignedFormula::getSign() const
{
	return _sign;
}

void BaseSignedFormula::printSignedFormula(ostream & ostr) const
{
	ostr << (getSign() ? "T " : "F ") << "(" << getFormula() << ")";
}

BaseSignedFormula::TableauxType BaseSignedFormula::getType() const
{
	// Alpha-type formulae are...
	if (
		// T ~X
		_sign && _f->getType() == BaseFormula::T_NOT ||
		// F ~X
		!_sign && _f->getType() == BaseFormula::T_NOT ||
		// T (X /\ Y)
		_sign && _f->getType() == BaseFormula::T_AND ||
		// F (X \/ Y)
		!_sign && _f->getType() == BaseFormula::T_OR ||
		// F (X => Y)
		!_sign && _f->getType() == BaseFormula::T_IMP
	)
	{
		return TT_ALPHA;
	}

	// Beta-type formulae are...
	if (
		// F (X /\ Y)
		!_sign && _f->getType() == BaseFormula::T_AND ||
		// T (X \/ Y)
		_sign && _f->getType() == BaseFormula::T_OR ||
		// T (X => Y)
		_sign && _f->getType() == BaseFormula::T_IMP
		)
	{
		return TT_BETA;
	}

	// Gamma-type formulae are...
	if (
		// T (Av)X(v)
		_sign && _f->getType() == BaseFormula::T_FORALL ||
		// F (Ev)X(v)
		!_sign && _f->getType() == BaseFormula::T_EXISTS
		)
	{
		return TT_GAMMA;
	}

	// Delta-type formulae are...
	if (
		// F (Av)X(v)
		!_sign && _f->getType() == BaseFormula::T_FORALL ||
		// T (Ev)X(v)
		_sign && _f->getType() == BaseFormula::T_EXISTS
		)
	{
		return TT_DELTA;
	}

	throw "Not applicable";
}

ostream & operator<<(ostream & ostr, SignedFormula sf)
{
	sf->printSignedFormula(ostr);
	return ostr;
}

// END BaseSignedFormula
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Tableaux

Tableaux::Tableaux(const Formula & root)
{
	// The original formula should be transformed to match the correct input for tableaux
	Formula transformed;

	// First, eliminate all equivalents from the formula, and then eliminate all constants from the formula
	transformed = root->releaseIff()->absorbConstants();

	// If the transformed formula is a logic constant true, then...
	if (transformed->getType() == BaseFormula::T_TRUE)
	{
		// ... transform the formula into its equivalent form without logic constants
		transformed = ((True*)transformed.get())->transformToDisjunction();
	}
	// If the transformed formula is a logic constant false, then...
	else if (transformed->getType() == BaseFormula::T_FALSE)
	{
		// ... transform the formula into its equivalent form without logic constants
		transformed = ((False*)transformed.get())->transformToConjunction();
	}
	// Otherwise, do nothing

	_root = make_shared<BaseSignedFormula>(transformed, false);
	/* By here, the formula _root is equivalent to the beginning formula root,
	so if the formula _root is unsatisfiable, then the formula root is unsatisfiable */
	_result = prove();
}

string Tableaux::getResult() const
{
	return _result ? "TAUTOLOGY" : "NOT A TAUTOLOGY";
}

bool Tableaux::prove(deque<SignedFormula>&& d_formulae, deque<FunctionSymbol>&& d_constants, int tabs) const
{
	if (!d_formulae.empty())
	{
		SignedFormula f = d_formulae.front();
		BaseFormula::Type type = f->getFormula()->getType();

		// Writing the current state of tableaux to the standard output
		cout << string(tabs, '\t');
		cout << "{ ";
		auto b = d_formulae.cbegin();
		auto e = d_formulae.cend();
		for (; b != e; b++)
		{
			cout << *b;
			if (b + 1 != e)
			{
				cout << ", ";
			}
		}
		cout << " }" << endl;

		/* Depending on the type of formula, there are several sets of rules that can be applied */
		switch (type)
		{
		case BaseFormula::T_ATOM:
			return atomRules(move(d_formulae), move(d_constants), f, tabs);
		case BaseFormula::T_NOT:
			return notRules(move(d_formulae), move(d_constants), f, tabs);
		case BaseFormula::T_AND:
			return andRules(move(d_formulae), move(d_constants), f, tabs);
		case BaseFormula::T_OR:
			return orRules(move(d_formulae), move(d_constants), f, tabs);
		case BaseFormula::T_IMP:
			return impRules(move(d_formulae), move(d_constants), f, tabs);
		//case BaseFormula::T_TRUE:
		//case BaseFormula::T_FALSE:
		//case BaseFormula::T_IFF:
		//case BaseFormula::T_FORALL:
		//case BaseFormula::T_EXISTS:
		default:
			throw "Not applicable!";
			break;
		}
	}
	else
	{
		d_formulae.push_back(_root);
		deque<FunctionSymbol> d_constantsInRoot;
		return prove(move(d_formulae), move(d_constants), tabs);
	}
}

bool Tableaux::atomRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const
{
	// Check if there are only signed atoms in the current branch
	bool onlyAtoms = true;
	for (SignedFormula & sf : d_formulae)
	{
		if (sf->getFormula()->getType() != BaseFormula::T_ATOM)
		{
			onlyAtoms = false;
			break;
		}
	}
	// If there are only signed atoms, then...
	if (onlyAtoms)
	{
		SignedFormula tmp_f(f);
		std::deque<SignedFormula> tmp(d_formulae);

		// ... for each signed atom, check...
		do
		{
			// ... if there is his complementary signed atom in the same branch, then close the current branch
			for (SignedFormula & sf : tmp)
			{
				if (((Atom*)sf->getFormula().get())->getSymbol() == ((Atom*)tmp_f->getFormula().get())->getSymbol() &&
					sf->getSign() != tmp_f->getSign())
				{
					return true;
				}
			}

			//// For debugging: write the contents of temporary queue to the standard output
			//cout << string(tabs + 1, '\t');
			//std::for_each(tmp.cbegin(), tmp.cend(), [&cout = cout](SignedFormula x) { cout << x << " "; });
			//cout << endl;

			tmp_f = tmp.front();
			tmp.pop_front();
		} while (!tmp.empty());

		// If there are no complementary signed atoms, then the branch cannot be closed
		return false;
	}
	// If there is at least one signed formula other than a signed atom, then...
	else
	{
		// If there are T P and F P signed formulas, where P is an atom, then close the current branch
		for (SignedFormula & sf : d_formulae)
		{
			if (sf->getFormula()->getType() == BaseFormula::T_ATOM &&
				((Atom*)sf->getFormula().get())->getSymbol() == ((Atom*)f->getFormula().get())->getSymbol() &&
				sf->getSign() != f->getSign())
			{
				return true;
			}
		}
	}
	/* Otherwise, place the signed atom at the end of the queue,
	so that other signed formulas can be checked */
	d_formulae.pop_front();
	d_formulae.push_back(f);
	return prove(move(d_formulae), move(d_constants), tabs);
}

bool Tableaux::notRules(deque<SignedFormula>&& d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const
{
	// If !X is true, then X is false.
	if (f->getSign())
	{
		d_formulae.push_back(make_shared<BaseSignedFormula>(((Not*)f->getFormula().get())->getOperand(), false));
	}
	// If !X is false, then X is true.
	else
	{
		d_formulae.push_back(make_shared<BaseSignedFormula>(((Not*)f->getFormula().get())->getOperand(), true));
	}
	d_formulae.pop_front();
	return prove(move(d_formulae), move(d_constants), tabs);
}

bool Tableaux::andRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const
{
	// If X /\ Y is true, then X and Y are both true.
	if (f->getSign())
	{
		d_formulae.pop_front();
		d_formulae.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand1(), true));
		d_formulae.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand2(), true));
		return prove(move(d_formulae), move(d_constants), tabs);
	}
	// If X /\ Y is false, then either X is false or Y is false.
	else
	{
		bool res1, res2;
		std::deque<SignedFormula> tmp(d_formulae);

		// first, check what happens if X is false
		d_formulae.pop_front();
		d_formulae.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand1(), false));
		res1 = prove(move(d_formulae), move(d_constants), tabs + 1);
		cout << string(tabs + 1, '\t') << (res1 ? "X" : "O") << endl;

		d_formulae = tmp;

		// if the first branch is closed, then...
		if (res1)
		{
			// ... check what happens if Y is false
			d_formulae.pop_front();
			d_formulae.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand2(), false));
			res2 = prove(move(d_formulae), move(d_constants), tabs + 1);
			cout << string(tabs + 1, '\t') << (res2 ? "X" : "O") << endl;

			move(d_formulae) = tmp;

			// both branches have to be closed to close their superbranch
			return res1 && res2;
		}
		// if the first branch is not closed, then its superbranch cannot be closed
		else
		{
			return res1; // false
		}
	}
}

bool Tableaux::orRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const
{
	// If X /\ Y is true, then either X is true or Y is true.
	if (f->getSign())
	{
		bool res1, res2;
		std::deque<SignedFormula> tmp(d_formulae);

		// first, check if X is true
		d_formulae.pop_front();
		d_formulae.push_back(make_shared<BaseSignedFormula>(((Or*)f->getFormula().get())->getOperand1(), true));
		res1 = prove(move(d_formulae), move(d_constants), tabs + 1);
		cout << string(tabs + 1, '\t') << (res1 ? "X" : "O") << endl;

		d_formulae = tmp;

		// if the first branch is closed, then...
		if (res1)
		{
			// ... check what happens if Y is true
			d_formulae.pop_front();
			d_formulae.push_back(make_shared<BaseSignedFormula>(((Or*)f->getFormula().get())->getOperand2(), true));
			res2 = prove(move(d_formulae), move(d_constants), tabs + 1);
			cout << string(tabs + 1, '\t') << (res2 ? "X" : "O") << endl;

			d_formulae = tmp;

			// both branches have to be closed to close their superbranch
			return res1 && res2;
		}
		// if the first branch is not closed, then its superbranch cannot be closed
		else
		{
			return res1; // false
		}
	}
	// If X \/ Y is false, then X and Y are both false.
	else
	{
		d_formulae.pop_front();
		d_formulae.push_back(make_shared<BaseSignedFormula>(((Or*)f->getFormula().get())->getOperand1(), false));
		d_formulae.push_back(make_shared<BaseSignedFormula>(((Or*)f->getFormula().get())->getOperand2(), false));
		return prove(move(d_formulae), move(d_constants), tabs);
	}
}

bool Tableaux::impRules(deque<SignedFormula> && d_formulae, deque<FunctionSymbol> && d_constants, const SignedFormula & f, int tabs) const
{
	// If X => Y is true, then either X is false or Y is true.
	if (f->getSign())
	{
		bool res1, res2;
		std::deque<SignedFormula> tmp(d_formulae);

		// first, check if X is false
		d_formulae.pop_front();
		d_formulae.push_back(make_shared<BaseSignedFormula>(((Imp*)f->getFormula().get())->getOperand1(), false));
		res1 = prove(move(d_formulae), move(d_constants), tabs + 1);
		cout << string(tabs + 1, '\t') << (res1 ? "X" : "O") << endl;

		d_formulae = tmp;

		// if the first branch is closed, then...
		if (res1)
		{
			// ... check what happens if Y is true
			d_formulae.pop_front();
			d_formulae.push_back(make_shared<BaseSignedFormula>(((Imp*)f->getFormula().get())->getOperand2(), true));
			res2 = prove(move(d_formulae), move(d_constants), tabs + 1);
			cout << string(tabs + 1, '\t') << (res2 ? "X" : "O") << endl;

			d_formulae = tmp;

			// both branches have to be closed to close their superbranch
			return res1 && res2;
		}
		// if the first branch is not closed, then its superbranch cannot be closed
		else
		{
			return res1; // false
		}
	}
	// If X => Y is false, then X is true and Y is false.
	else
	{
		d_formulae.pop_front();
		d_formulae.push_back(make_shared<BaseSignedFormula>(((Imp*)f->getFormula().get())->getOperand1(), true));
		d_formulae.push_back(make_shared<BaseSignedFormula>(((Imp*)f->getFormula().get())->getOperand2(), false));
		return prove(move(d_formulae), move(d_constants), tabs);
	}
}

// END Tableaux
// ----------------------------------------------------------------------------
