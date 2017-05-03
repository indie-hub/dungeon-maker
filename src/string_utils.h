#pragma once

#include <string>
#include <locale>
#include <algorithm>

#define STRTOLOWER(x) std::transform (x.begin(), x.end(), x.begin(), ::tolower)
