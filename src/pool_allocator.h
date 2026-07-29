#pragma once
#include <vector>
#include <type_traits>

namespace itchbook {

template<typename T>
class PoolAllocator {
    requires std::is_default_constructible_v<T>
public:

private:
    std::vector<T> 

};

}