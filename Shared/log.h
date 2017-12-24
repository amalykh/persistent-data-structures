#pragma once

#include "shared.h"
#include "../PersistentVector/vector_persistent_node.h"
#include <memory>
#include <vector>
#include "../PersistentVector/tail_node.h"
#include "../PersistentVector/reference_node.h"
#include "../PersistentVector/abstract_pv.h"
#include "../PersistentVector/data_node.h"
#include "nodes_converter.h"

namespace persistent {
	class Log {
	public:
		template <class T, class Allocator = std::allocator<T>>
		static void toString(AbstractPV<T, Allocator>& v, std::ostream& out = std::cout) {
			int32_t sz = v.size();
			for (int i = 0; i < sz; i++) {
				out << v[i] << " ";
			}
		}
	};
}