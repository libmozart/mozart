/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/timer>

std::chrono::time_point<std::chrono::high_resolution_clock>
mpp::timer::m_timer(std::chrono::high_resolution_clock::now());
