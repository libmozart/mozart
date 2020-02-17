/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2020 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#include <mozart++/any>

template <typename T>
mpp::any::default_allocator<mpp::any::stor_impl<T>> mpp::any::stor_impl<T>::allocator;
