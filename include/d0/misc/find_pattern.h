#pragma once

#include "d0/defs.h"

#include <string>

namespace d0 {

uptr FindPattern(uptr start, uptr end, const std::string &pattern);

}