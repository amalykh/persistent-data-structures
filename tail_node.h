#pragma once
#include "shared.h"
#include "vector_persistent_node.h"
#include <memory>
#include <vector>

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class TailNode : public PersistentNode<T, Allocator> {
		using TNode = TailNode<T, Allocator>;
		using TNodePtr = shared_ptr<TNode>;
	public:
		T& operator[] (int x) {
			return data[x];
		}
		TailNode(VersionID version) : version(version) {
			data.resize(C_SIZE);
		}
		TailNode(T first_val, VersionID version) : TailNode(version) {
			data[0] = first_val;
		}
		TailNode(std::vector<T, Allocator>& source_data, int len, VersionID version) : TailNode(version) {
			std::copy(source_data.begin(), source_data.begin() + len, data.begin());
			last_change = len - 1;
		}
		TailNode(std::vector<T, Allocator>& source_data, int len, int ind, T first_val, VersionID version) : TailNode(source_data, len, version) {
			data[ind] = first_val;
		}
		TailNode(TailNode<T, Allocator>& source_tail, int len, VersionID version) : TailNode(version) {
			if (len == 0) len = 32;
			std::copy(source_tail.getData().begin(), source_tail.getData().begin() + len, data.begin());
			last_change = len - 1;
		}
		TNodePtr push(T& value, int ind, VersionID version) {
			if (version != NO_VERSION && this->version == version) {
				++last_change;
				data[ind] = value;
				return make_shared<TNode>(*this);
			}

			// this is not thread-safe!
			if (ind == last_change + 1) {
				data[++last_change] = value;
				return make_shared<TNode>(*this);
			}

			return make_shared<TNode>(data, ind + 1, ind, value, version);
		}
		TNodePtr change(int ind, T& value, VersionID version) {
			if (version != NO_VERSION && this->version == version) {
				data[ind] = T;
				return make_shared<TNode>(*this);
			}
			return make_shared<TNode>(data, last_change + 1, ind, value, version);
		}
		TNodePtr pop(VersionID version, int cnt) {
			if (version != NO_VERSION && this.version == version) {
				last_change--;
				return make_shared<TNode>(*this);
			}
			return make_shared<TNode>(data, cnt - 1, version);
		}
		bool equalsData(TailNode<T, Allocator>& other, int len) {
			T* other_data = other.getData();
			for (int i = 0; i < len; ++i) {
				if (other_data[i] != data[i]) return false;
			}
			return true;
		}
		std::vector<T, Allocator> getData() { return data; }
		VersionID getVersion() { return version; }
	private:
		VersionID version;
		int32_t last_change;
		std::vector<T, Allocator> data;
	};
} // namespace persistent