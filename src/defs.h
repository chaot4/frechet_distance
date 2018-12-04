#pragma once

#include <iostream>

// helper macro

#define FRECHET_NOP do { } while (0)

// printing macros

#ifdef NDEBUG
#define DEBUG(x) FRECHET_NOP
#else
#define DEBUG(x) do { std::cout << x << std::endl; } while (0)
#endif

#if defined(NVERBOSE) && defined(NDEBUG)
#define PRINT(x) FRECHET_NOP
#else
#define PRINT(x) do { std::cout << x << std::endl; } while (0)
#endif

#define ERROR(x) do { std::cerr << "Error: " << x << std::endl;\
                      std::exit(EXIT_FAILURE); } while (0)

// force assert for now (FIXME: remove on release)
// #ifdef NDEBUG
//     #undef NDEBUG
//     #include <cassert>
//     #define NDEBUG
// #else
//     #include <cassert>
// #endif
#include <cassert>
