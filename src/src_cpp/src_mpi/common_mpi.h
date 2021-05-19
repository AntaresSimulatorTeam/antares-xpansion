#pragma once

#define __ACTIVE_CHECK__ 0

#include "common.h"
#include <boost/mpi.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace mpi = boost::mpi;

typedef std::pair<int, std::string> mps_order;