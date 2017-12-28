#pragma once

#include "../Shared/shared.h"
#include "hashmap_persistent_node.h"
#include <exception>
#include "collision_array.h"

const int SS_NEWITEM = 1;
const int SS_CHANGEDITEM = 2;

namespace persistent {
template<class K, class V, class Allocator = std::allocator<pair<K, V>>>
class AbstractHM {
  using MPC = MCollisionCollection<K, V, Allocator>;
  using MPCPtr = shared_ptr<MPC>;
  using MPN = MPersistentNode <K, V, Allocator>;
  using MPNPtr = shared_ptr<MPN>;
  using CA = CollisionArray<K, V, Allocator>;
  using CAPtr = shared_ptr<CA>;
public:
  
  bool empty() { return count == 0; }
  
  int32_t size() { return count; }
  
  MPNPtr createValueNode(int idx, K& key, V& value, VersionID version = NO_VERSION) {
    return make_shared<MPN>(idx, key, value, version);
  }
  
  AbstractHM(MPNPtr root, int32_t count, VersionID version = NO_VERSION) :
    root(root), count(count), version(version) {}
  
  MPNPtr createCommonPath(uint32_t h1, uint32_t h2, int i, int shift, MPNPtr node, K& key, V& value) {
    auto i1 = (int)(h1 >> shift) & ABITS;
    auto i2 = (int)(h2 >> shift) & ABITS;
    if (i1 != i2) return node->createNewNodeFrom(i, key, value, i1, i2, version);
    // Creating longer path
    vector<int> s;
    do {
      s.push_back(i1);
      shift += C_BITS;
      i1 = (int)(h1 >> shift) & ABITS;
      i2 = (int)(h2 >> shift) & ABITS;
    } while (i1 == i2);
    auto new_node = node.createNewNodeFrom(i, key, value, i1, i2, version);
    // Creating path
    for (auto idx : s) {
      new_node = node.createReferenceNode(idx, new_node, version);
    }
    return new_node;
  }
  
  MPNPtr adding(int shift, MPNPtr node, uint32_t hash, K& key, V& value, int set) {
    auto idx = (int)((hash >> shift) & ABITS);
    auto state = node->getNodeStateAt(idx);
    
    if (state == M_Reference) {
      // On position is reference node
      auto referenced_node = node->getReferenceAt(idx);
      auto new_node = adding(shift + C_BITS, referenced_node, hash, key, value, set);
      
      return (new_node == referenced_node || (set & SS_CHANGEDITEM))
          ? node->changeReference(idx, new_node, version)
          : node;
    }
    
    if (state == M_Nil) return node->addValueItemAt(idx, key, value, version);
    
    // On position is value node or collision collection
    auto relation = node->relationWithNodeAt(key, idx, state);
    
    if (relation == M_Equal) {
      // Value with the same key already exists
      auto n = node->getValueAt(idx, state, key);
      set &= ~SS_NEWITEM;
      if (n.Equals(value)) {
        set &= ~SS_CHANGEDITEM;
        return node;
      }
      return node->ChangeValue(idx, state, key, value, versionID);
    }
    if (relation == M_Collide) {
      // Hash collision
      return (state == M_Value)
          ? node->createCollisionAt(idx, key, value, version)
          : node->addToColisionAt(idx, key, value, version);
    }
    // Hashes are different, we create longer path
    
    return node->createReference(
          idx,
          createCommonPath(node->getHashCodeAt(idx, state), hash, idx, shift + C_BITS, node, key, value),
          state,
          version
          );
  }
  
  MPNPtr removing(int shift, MPNPtr node, uint32_t hash, K& key) {
    auto idx = (int)((hash >> shift) & ABITS);
    auto state = node.GetNodeStateAt(idx);
    if (state == M_Nil) throw "no such key in map";
    if (state == M_Value || state == M_Collision) {
      auto relation = node.RelationWithNodeAt(key, idx, state);
      if (relation == MR_Equal) {
        return node->removeValue(idx, state, key, version);
      }
      else throw "no such value";
    }
    auto new_node = removing(shift + 5, node->getReferenceAt(idx), hash, key);
    if (new_node->valueCount == 1 && new_node->referenceCount == 0) {
      return (node->referenceCount > 1 || node->valueCount != 0)
          ? node->Merge(new_node, idx, version)
          : new_node;
    }
    else {
      return node->changeReference(idx, new_node, version);
    }
  }
  
  V& getValue(K& key) {
    auto hash = getHash(key);
    auto node = root;
    
    for (int32_t shift = 0; shift < C_SIZE; shift += C_BITS) {
      auto idx = (hash >> shift) & ABITS;
      auto state = node->getNodeStateAt(idx);
      
      if (state == M_Reference) node = node->getReferenceAt(idx);
      else if (state == M_Value || state == M_Collision) {
        return node->getValueAt(idx, state, key);
      }
      else throw "no such value";
    }
    assert(false);
  }
  
  bool containsKey(K& key) {
    if (root == nullptr) return false;
    auto hash = getHash(key);
    auto node = root;
    for (int shift = 0; shift < C_SIZE; shift += C_BITS) {
      auto idx = (hash >> shift) & ABITS;
      auto state = node->getNodeStateAt(idx);
      if (state == M_Value || state == M_Collision) {
        return node->isKeyAt(idx, state, key);
      }
      
      if (state == M_Nil) return false;
      
      node = node->GetReferenceAt(idx);
    }
    
    assert(false);
  }
  
private:
  MPNPtr root;
  int32_t count;
  VersionID version;
};
} // namespace persistent
