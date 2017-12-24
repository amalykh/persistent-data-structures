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
	PersistentVector<int> a = PersistentVector<int>::getEmpty();
	auto b = a;
	auto t = a;
	for (int i = 0; i < 900; i++) {
		if (i == 450) {
			t = b;
		}
		b = b.push_back(i);
	}
	Log::toString(t);
	cout << endl;
	Log::toString(b);
	int w;
	cin >> w;
	return 0;
}