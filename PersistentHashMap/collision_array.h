#pragma once

#include "../Shared/shared.h"
#include "hashmap_persistent_node.h"
#include <exception>


namespace persistent {
template<class K, class V, class Allocator = std::allocator<pair<K, V>>>
class CollisionArray : public MCollisionCollection {
public:
  using CA = CollisionArray<K, V, Allocator>;
  CollisionArray(vector<pair<K, V>, Allocator>& collisions, int32_t hash, VersionID& version) : 
    collisions(collisions), version(version), hash(hash) {}
  CollisionArray(VersionID version, vector<pair<K, V>, Allocator> pp) :
    version(version), collisions(pp) {
    assert(pp.size() >= 2);
    hash = getHash(pp.front().first);
  }
  CA add(K& key, V& value, VersionID version) {
    auto cc = collisions;
    cc.push_back({key, value});
    return CA(cc, hash, version);
  }
  CA remove(K& key, VersionID version) {
    assert(collisions.size() >= 2);
    auto g = collisions;
    for (int i = 0; i < collisions.size(); i++) {
      K& kk = g[i].first;
      if (kk == key) {
        g.erase(g.begin() + i);
        break;
      }
    }
    return CA(g, hash, version);
  }
  CA change(K& key, V& value, VersionID version) {
    if (version != NO_VERSION && version == this->version) {
      for (int i = 0; i < collisions.size(); i++) {
        if (collisions[i].first == key) {
          collisions[i] = {key, value};
          return make_shared<MPNPtr>(*this);
        }
      }
      assert(false);
    }
    auto g = collisions;
    for (int i = 0; i < collisions.size(); i++) {
      K& kk = g[i].first;
      if (kk == key) {
        g.erase(g.begin() + i);
        break;
      }
    }
    g.push_back({key, value});
    return CA(g, hash, version);
  }
  pair<K, V> getItem(K& key) {
    auto g = collisions;
    for (int i = 0; i < collisions.size(); i++) {
      K& kk = g[i].first;
      if (kk == key) {
        return g[i];
      }
    }
    throw "no such key";
  }
  bool hasItemWithKey(persistent::K &key) {
    for (int i = 0; i < collisions.size(); i++) {
      K& kk = g[i].first;
      if (kk == key) {
        return true;
      }
    }
    return false;
  }
  int32_t size() {
    return collisions.size();
  }

private:
  vector<pair<K, V>, Allocator> collisions;
  VersionID version;
  int32_t hash;
};
} // namespace persistent
