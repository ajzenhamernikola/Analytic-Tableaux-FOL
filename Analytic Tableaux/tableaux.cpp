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
	_root = make_shared<BaseSignedFormula>(root->releaseIff(), false);
	_result = prove();
}

bool Tableaux::getResult() const
{
	return _result;
}

bool Tableaux::prove(deque<SignedFormula> & deque, int tabs) const
{
	if (!deque.empty())
	{
		SignedFormula f = deque.front();
		BaseFormula::Type type = f->getFormula()->getType();

		// Writing the current state of tableaux to the standard output
		cout << string(tabs, '\t');
		cout << "{ ";
		auto b = deque.cbegin();
		auto e = deque.cend();
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
		case BaseFormula::T_TRUE:
		case BaseFormula::T_FALSE:
			throw new exception("Not applicable!");
			break;
		case BaseFormula::T_ATOM:
		{
			// Check if there are only signed atoms in the current branch
			bool onlyAtoms = true;
			for (SignedFormula & sf : deque)
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
				std::deque<SignedFormula> tmp(deque);

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
				for (SignedFormula & sf : deque)
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
			deque.pop_front();
			deque.push_back(f);
			return prove(deque, tabs);
		}
		case BaseFormula::T_NOT:
			// If !X is true, then X is false.
			if (f->getSign())
			{
				deque.push_back(make_shared<BaseSignedFormula>(((Not*)f->getFormula().get())->getOperand(), false));
			}
			// If !X is false, then X is true.
			else
			{
				deque.push_back(make_shared<BaseSignedFormula>(((Not*)f->getFormula().get())->getOperand(), true));
			}
			deque.pop_front();
			return prove(deque, tabs);
			break;
		case BaseFormula::T_AND:
			// If X /\ Y is true, then X and Y are both true.
			if (f->getSign())
			{
				deque.pop_front();
				deque.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand1(), true));
				deque.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand2(), true));
				return prove(deque, tabs);
			}
			// If X /\ Y is false, then either X is false or Y is false.
			else
			{
				bool res1, res2;
				std::deque<SignedFormula> tmp(deque);

				// first, check what happens if X is false
				deque.pop_front();
				deque.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand1(), false));
				res1 = prove(deque, tabs + 1);
				cout << string(tabs + 1, '\t') << " X (" << (res1 ? "true" : "false") << ")" << endl;

				deque = tmp;

				// if the first branch is closed, then...
				if (res1)
				{
					// ... check what happens if Y is false
					deque.pop_front();
					deque.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand2(), false));
					res2 = prove(deque, tabs + 1);
					cout << string(tabs + 1, '\t') << " X (" << (res2 ? "true" : "false") << ")" << endl;

					deque = tmp;

					// both branches have to be closed to close their superbranch
					return res1 && res2;
				}
				// if the first branch is not closed, then its superbranch cannot be closed
				else
				{
					return res1; // false
				}
			}
		case BaseFormula::T_OR:
			// If X /\ Y is true, then either X is true or Y is true.
			if (f->getSign())
			{
				bool res1, res2;
				std::deque<SignedFormula> tmp(deque);

				// first, check if X is true
				deque.pop_front();
				deque.push_back(make_shared<BaseSignedFormula>(((Or*)f->getFormula().get())->getOperand1(), true));
				res1 = prove(deque, tabs + 1);
				cout << string(tabs + 1, '\t') << " X (" << (res1 ? "true" : "false") << ")" << endl;

				deque = tmp;

				// if the first branch is closed, then...
				if (res1)
				{
					// ... check what happens if Y is true
					deque.pop_front();
					deque.push_back(make_shared<BaseSignedFormula>(((Or*)f->getFormula().get())->getOperand2(), true));
					res2 = prove(deque, tabs + 1);
					cout << string(tabs + 1, '\t') << " X (" << (res2 ? "true" : "false") << ")" << endl;

					deque = tmp;

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
				deque.pop_front();
				deque.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand1(), false));
				deque.push_back(make_shared<BaseSignedFormula>(((And*)f->getFormula().get())->getOperand2(), false));
				return prove(deque, tabs);
			}
		case BaseFormula::T_IMP:
			// If X => Y is true, then either X is false or Y is true.
			if (f->getSign())
			{
				bool res1, res2;
				std::deque<SignedFormula> tmp(deque);
				
				// first, check if X is false
				deque.pop_front();
				deque.push_back(make_shared<BaseSignedFormula>(((Imp*)f->getFormula().get())->getOperand1(), false));
				res1 = prove(deque, tabs + 1);
				cout << string(tabs + 1, '\t') << " X (" << (res1 ? "true" : "false") << ")" << endl;

				deque = tmp;

				// if the first branch is closed, then...
				if (res1)
				{
					// ... check what happens if Y is true
					deque.pop_front();
					deque.push_back(make_shared<BaseSignedFormula>(((Imp*)f->getFormula().get())->getOperand2(), true));
					res2 = prove(deque, tabs + 1);
					cout << string(tabs + 1, '\t') << " X (" << (res2 ? "true" : "false") << ")" << endl;

					deque = tmp;

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
				deque.pop_front();
				deque.push_back(make_shared<BaseSignedFormula>(((Imp*)f->getFormula().get())->getOperand1(), true));
				deque.push_back(make_shared<BaseSignedFormula>(((Imp*)f->getFormula().get())->getOperand2(), false));
				return prove(deque, tabs);
			}
		case BaseFormula::T_IFF:
		case BaseFormula::T_FORALL:
		case BaseFormula::T_EXISTS:
		default:
			throw new exception("Not applicable!");
			break;
		}
	}
	else
	{
		deque.push_back(_root);
		return prove(deque, tabs);
	}
}

// END Tableaux
// ----------------------------------------------------------------------------
