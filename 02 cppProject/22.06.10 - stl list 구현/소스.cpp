#include <iostream>
#include <list>
#include "j_list.h"
using namespace std;

class Pos {
public:
	Pos(int x = 0, int y = 0) : x(x), y(y) {}

	int x;
	int y;
};

int main() {

	{
		j_list<Pos> a;

		a.push_back(Pos(1, 2));
		a.push_back(Pos(3, 4));
		a.push_back(Pos(5, 6));
		a.push_back(Pos(7, 8));

		j_list<Pos>::iterator iter;
		iter = a.begin();
		iter++;
		iter++;

		a.erase(iter);
		a.erase(iter);

		for (iter = a.begin(); iter != a.end(); iter++) {
			cout << (&(*iter))->x << ", " << (&(*iter))->y << endl;
		}

		cout << "a.size : " << a.size() << endl;
	}


	cout << endl << endl << "정상종료";
}