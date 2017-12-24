#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "tail_node.h"
#include "shared.h"
#include "data_node.h"
#include "reference_node.h"
#include "abstract_pv.h"
#include "persistent_vector.h"
#include "Log.h"

using namespace std;
using namespace persistent;

int main() {
	freopen("output.txt", "w", stdout);
	PersistentVector<int> a = PersistentVector<int>::getEmpty();
	PersistentVector<double> k(100);
	Log::toString(k);
	cout << endl;
	const int n = 100000;
	const int D = 10;
	vector<PersistentVector<int>> v(D, PersistentVector<int>::getEmpty());
	for (int i = 0; i < D; i++) v[i] = a;
	for (int i = 0; i < n; i++) {
		v[rand() % D] = v[rand() % D].push_back(rand());
	}
	for (int j = 0; j < D; j++) {
		Log::toString(v[j]);
		std::cout << endl;
	}
	return 0;
}