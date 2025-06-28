#pragma once

#include "Missile.h"
#include <unordered_map>
#include <mutex>

inline std::unordered_map<std::string, ParsedMissileData> globalMissiles;
