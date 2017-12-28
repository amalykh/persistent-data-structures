#pragma once

#include "../Shared/shared.h"
#include "hashmap_persistent_node.h"
#include <exception>
#include "collision_array.h"
#include "abstract_hm.h"

namespace persistent {
template<class K, class V, class Allocator = std::allocator<pair<K, V>>>
class PersistentHashMap : public AbstractHM {
  using MPC = MCollisionCollection<K, V, Allocator>;
  using MPCPtr = shared_ptr<MPC>;
  using MPN = MPersistentNode <K, V, Allocator>;
  using MPNPtr = shared_ptr<MPN>;
  using CA = CollisionArray<K, V, Allocator>;
  using CAPtr = shared_ptr<CA>;
  using PD = PersistentDictionary<K, V, Allocator>;
public:
  PersistentDictionary(MPNPtr root, int count)
              : AbstractHM(root, count) {}
  PersistentDictionary() : AbstractHM(nullptr, 0) {}
  
  PD add(K& key, V& value) {
    if (count == 0) {
      auto idx = getHash(key) & ABITS;
      return PD(createValueNode(idx, key, value), 1);
    }
    
    int set = SS_CHANGEDITEM | SS_NEWITEM;
    auto new_root = adding(0, root, getHash(key), key, value, set);
    return PV(new_root, count + (set & SS_NEWITEM) ? 1 : 0);
  }
  
  PD remove(K& key) {
    if (count == 0) throw "no such value";
    
    auto new_root = removing(0, root, getHash(key), key);
    
    if (count == 1) return PV();
    else if (new_root->valueCount() == 1 && new_root->referenceCount() == 0) {
      new_root = new_root->makeRoot(version);
    }
    
    PV(new_root, count - 1);
  }
  
  V& operator[] (K& k) {
    return getValue(k);
  }
};
}
