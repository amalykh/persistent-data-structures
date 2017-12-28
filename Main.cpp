#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "PersistentVector/tail_node.h"
#include "Shared/shared.h"
#include "PersistentVector/data_node.h"
#include "PersistentVector/reference_node.h"
#include "PersistentVector/abstract_pv.h"
#include "PersistentVector/persistent_vector.h"
#include "Shared/log.h"
#include <map>
#include "PersistentHashMap/persistent_hashmap.h"

using namespace std;
using namespace persistent;

int main() {
	freopen("output.txt", "w", stdout);
  PersistentHashMap<int> a();
  PersistentHashMap<int> b = a.add(14, 15);
  std::cout << a.size() << " " << b.size();
  
  int w;
  cin >> w;
	return 0;
}
