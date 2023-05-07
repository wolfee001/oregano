#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

namespace oregano {
namespace test {
    template <typename T>
    std::unique_ptr<T> make_strict_mock_unique_ptr(::testing::StrictMock<T>*& p_raw_pointer)
    {
        auto ret_val = std::make_unique<::testing::StrictMock<T>>();
        p_raw_pointer = ret_val.get();
        ::testing::Mock::AllowLeak(p_raw_pointer);
        return ret_val;
    }
} // namespace test
} // namespace oregano
