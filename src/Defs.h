#pragma once

#include <variant>
#include <string>
#include <ranges>
#include <unordered_set>
#include <unordered_map>

#include "obse/PluginAPI.h"
#include "obse/GameAPI.h"
#include "obse/GameObjects.h"
#include "obse/GameData.h"
#include "obse/GameExtraData.h"
#include "obse_common/SafeWrite.h"
#include "obse/Utilities.h"

#include <common/IDebugLog.h>

#include "lib/string.hpp"
#include "lib/rng.hpp"
#include "lib/distribution.hpp"
#include "lib/numeric.hpp"
#include "lib/simpleINI.hpp"

#include "lib/srell.hpp"
#include "NiPoint.h"

namespace string = clib_util::string;
namespace dist = clib_util::distribution;
namespace numeric = clib_util::numeric;

using SeedRNG = clib_util::RNG;

template <class T>
using MinMax = std::pair<T, T>;
template <class T>
using RelData = std::pair<bool, MinMax<T>>;  //relative vs absolute

using FormIDStr = std::variant<std::uint32_t, std::string>;

using FormIDSet = std::unordered_set<std::uint32_t>;
using FormIDOrSet = std::variant<std::uint32_t, FormIDSet>;

template <class T>
using SwapMap = std::unordered_map<std::uint32_t, T>;
