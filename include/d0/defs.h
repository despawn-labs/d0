#pragma once

#include <cstdint>

/* Platform Definitions */
#ifdef _WIN32
#define D0_WINDOWS 1
#endif

#ifdef __linux__
#define D0_LINUX 1
#endif

/* API Definitions */
#ifdef D0_SHARED
#ifdef D0_EXPORT
#define D0_API __declspec(dllexport)
#else
#define D0_API __declspec(dllimport)
#endif
#else
#define D0_API
#endif

/* Numeric Types */
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;

using f32 = float;

#ifdef D0_X64
using u64 = uint64_t;
using i64 = int64_t;

using usize = u64;
using isize = i64;

using f64 = double;
#else
using usize = u32;
using isize = i32;
#endif

using uptr = usize;
using iptr = isize;