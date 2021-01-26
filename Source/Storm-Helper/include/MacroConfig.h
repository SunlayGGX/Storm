#pragma once



// The storage type of the Singleton holder :
// 0 : a std::map -> This was the initial and safer implementation but slower.
// 1 : a std::vector (dynamic contiguous storage) -> The storage will be contiguous, therefore all get operation will be much faster. But it is allocated on the head (like map).
// 2 : a static buffer (static contiguous storage) -> The storage will be contiguous and allocated on the stack. But the size will be fixed, therefore we should set a big size we're sure all singletons will fit inside with their contiguous ids.
#define STORM_SINGLETON_HOLDER_STORAGE_TYPE_MODE 1



#define STORM_ALLOWS_OPENMP true
