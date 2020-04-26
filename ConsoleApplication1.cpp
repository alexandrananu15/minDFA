#include <iostream>
#include <map>
#include <string>
#include <set>
#include <fstream>
#include <vector>
#include "Dsu.h"
#include <queue>

using namespace std;

class DFA
{
	set<int> Q, F;
	set<char> Sigma;
	int q0;
	map<pair<int, char>, int> delta;

public:
	DFA() { this->q0 = 0; }
	DFA(set<int> Q, set<char> Sigma, map<pair<int, char>, int> delta, int q0, set<int> F)
	{
		this->Q = Q;
		this->Sigma = Sigma;
		this->delta = delta;
		this->q0 = q0;
		this->F = F;
	}

	set<int> getQ() const { return this->Q; }
	set<int> getF() const { return this->F; }
	set<char> getSigma() const { return this->Sigma; }
	int getInitialState() const { return this->q0; }
	map<pair<int, char>, int> getDelta() const { return this->delta; }

	friend istream& operator >> (istream&, DFA&);

	bool isFinalState(int);
	int deltaStar(int, string);
	void minDFA();
};

bool DFA::isFinalState(int q)
{
	return F.find(q) != F.end();
}

int DFA::deltaStar(int q, string w)
{
	if (w.length() == 1)
	{
		return delta[{q, (char)w[0]}];
	}

	int new_q = delta[{q, (char)w[0]}];
	return deltaStar(new_q, w.substr(1, w.length() - 1));
}

istream& operator >> (istream& f, DFA& M)
{
	int noOfStates;
	f >> noOfStates;
	for (int i = 1; i <= noOfStates; ++i)
		M.Q.insert(i);

	int noOfLetters;
	f >> noOfLetters;
	for (int i = 0; i < noOfLetters; ++i)
	{
		char ch;
		f >> ch;
		M.Sigma.insert(ch);
	}

	int noOfTransitions;
	f >> noOfTransitions;
	for (int i = 0; i < noOfTransitions; ++i)
	{
		int s, d;
		char ch;
		f >> s >> ch >> d;
		M.delta[{s, ch}] = d;
	}

	f >> M.q0;

	int noOfFinalStates;
	f >> noOfFinalStates;
	for (int i = 0; i < noOfFinalStates; ++i)
	{
		int q;
		f >> q;
		M.F.insert(q);
	}

	return f;
}

//parcurgere in adam=ncime pentru pasii 4 si 5 descrisi
void DFS(vector <bool> &dead, set <int> &stari, map< int, set <int>> &rev)
{
	for (int i : stari)
	{
		if (dead[i] == true)
		{
			dead[i] = false;
			DFS(dead, rev[i], rev);
		}
	}
}


void DFA::minDFA()
{

	//Pasul 1: gasim starile echivalente

	vector < vector < bool > > equivalent(Q.size() + 1, vector < bool >(Q.size() + 1, true));	//construim matricea de adiacenta

	for (int i = 1; i <= Q.size(); ++i)
	{
		for (int j = i + 1; j <= Q.size(); ++j)
		{
			bool x = F.find(i) != F.end();			//daca i e stare finala/nefinala
			bool y = F.find(j) != F.end();			//daca j e stare finala/nefinala
			if (x != y)
				equivalent[i][j] = false;			//marcam cu false daca i stare finala si j stare nefinala
		}
	}


	//marchez cu false toate perechile (q, r) pentru care (delta(q, litera), delta(r, litera)) sunt marcate cu false
	bool notFinished = true;
	while (notFinished)
	{
		notFinished = false;														//semnalizator
		for (int i = 1; i <= Q.size(); ++i)							
			for (int j = i + 1; j <= Q.size(); ++j)
			{
				for (const char& x : Sigma)											//pentru fiecare litera din alfabet, verific in matrice
				{
					if (equivalent[delta[{i, x}]][delta[{j, x}]] == false && equivalent[i][j] == true)
					{
						equivalent[i][j] = false;									//marchez in matrice cu fals
						notFinished = true;
					}
				}
			}
	}

	//Pasul 2: Grupam starile echivalente rezultate din matricea de echivalenta intr-o unica stare. Tranzitiile vor fi aceleasi cu ale automatului initial dar tinand cont de aceasta grupare.

	Dsu dsu(Q.size() + 1);									//padure disjuncta de multimi
	for (int i = 1; i < Q.size(); ++i)
		for (int j = i + 1; j <= Q.size(); ++j)
			if (equivalent[i][j])
				dsu.link(i, j);								//starile i si j sunt plasate in aceeasi partitie

	map<pair<int, char>, int> deltaMin;
	for (auto& k : delta)
		deltaMin[{k.first.first, k.first.second}] = dsu.getParent(k.second);

	//Pasul 3: Obtinem starea initiala si starile finale: Starea initiala devine starea ce contine starea initiala a automatului original.
	//Starile finale sunt toate starile compuse din stari finale.

	set < int > Fmin;
	vector < bool > used(Q.size() + 1);
	int dfaMinInitialState = 0;									//starea initiala

	for (int i = 1; i <= Q.size(); ++i)
	{
		int state = dsu.getParent(i);							//state este reprezentantul partitiei
		if (i == q0)						
		{
			dfaMinInitialState = state;							//obtin starea initiala, ca fiind starea care contine starea initiala ale DFA-ului minimizat
		}

		if (not used[state])
		{
			used[state] = true;
			if (F.find(state) != F.end())
			{
				Fmin.insert(state);								//obtin starile finale ale DFA-ului minimizat
			}
		}
	}

	set <int> Qmin;
	for (int i = 1; i <= Q.size(); i++)
	{
		Qmin.insert(dsu.getParent(i));
	}


	// Pasul 4: remove dead-end states: parcurgere inversa de la starile finale la starea curenta => folosim BFS
	//O stare qk este dead-end daca nu exista niciun drum de la aceasta stare la o stare finala.
	
	map<int, set<int>> deltaMinSimplified;					//simplific deltaMin ca sa fac parcurgere in adancime incepand in 
	map<int, set<int>> deltaMinSimplifiedReversed;			//inversez deltaMin ca sa fac o parcurgere in latime din starile finale ptr pasul 4

	for (auto& k : deltaMin)								//parcurg deltaMin pentru a face seturile simplificate derivate din deltaMin
	{
		deltaMinSimplified[k.first.first].insert(k.second);
		deltaMinSimplifiedReversed[k.second].insert(k.first.first);
	}

	vector < bool > deadEnd(Q.size() + 1, true);			//vectorul semnalizator deadEnd care ne spune daca o stare a fost traversata in DFS sau nu (daca nu a fost traversata, atunci dead[i] = true)
	set <int> stergere;										//folosit ca sa stergem din Qmin, starile Dead-End

	DFS(deadEnd, Fmin, deltaMinSimplifiedReversed);			//parcurgerea in adancime

	for (int i : Qmin)										//retinem in setul stergere starile Dead-End
	{
		if (deadEnd[i] == true)								
			stergere.insert(i);
	}

	for (int i : stergere)									//stergem din Qmin starile Dead-End
	{
		Qmin.erase(i);
	}

	//Pasul 5: eliminam starile neaccesibile. O stare qk este neaccesibila daca nu exista niciun drum de la starea initala q0 pana la qk.

	set <int> initial;										//implementare asemanatoare cu cea de la Dead-Ends, doar ca de data asta pornim din starea initiala
	initial.insert(dfaMinInitialState);

	vector < bool > deadEndIN(Q.size() + 1, true);
	set <int> stergereIN;

	DFS(deadEndIN, initial, deltaMinSimplified);

	for (int i : Qmin)
	{
		if (deadEndIN[i] == true)
			stergereIN.insert(i);
	}

	for (int i : stergereIN)
	{
		Qmin.erase(i);
	}

	map <pair <int, char>, int> deltaMinFinal;
	for (auto& k : delta)
	{
		if (Qmin.find(dsu.getParent(k.first.first)) != Qmin.end() && Qmin.find(dsu.getParent(k.second)) != Qmin.end())
			deltaMinFinal[{dsu.getParent(k.first.first), k.first.second}] = dsu.getParent(k.second);

	}

	//afisare
	cout << "Q: ";
	for (int i : Qmin)
		cout << i << ' ';
	cout << endl<<endl;
	cout << "Alfabet" << ": ";
	for (auto ch : Sigma)
		cout << ch << ' ';
	cout << endl << endl;
	cout << "Starea initiala: " << dfaMinInitialState << endl << endl;
	cout << "Tranzitii: " << endl;
	for (auto& k : deltaMinFinal)
	{
		cout << k.first.first << ' ' << k.first.second << ' ' << k.second << endl;
	}
	cout << endl << endl;
	cout << "Starile finale: ";
	for (int f : Fmin)
		cout << f << ' ';
	cout << endl<<endl;
	cout << "Gruparea:" << endl;
	for (int i : Q)
	{
		cout << i << ' ' << dsu.getParent(i) << endl;
	}
}


int main()
{
	DFA M;

	ifstream fin("Text.txt");
	fin >> M;
	fin.close();

	M.minDFA();

	return 0;
}