/**
 * @file sn.hpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include <type_traits>


namespace litertp {
namespace sn{



    template <unsigned long M>                                    // NOLINT
    inline unsigned long add(unsigned long a, unsigned long b) {  // NOLINT
        unsigned long t = M - b % M;  // NOLINT
        unsigned long res = a - t;    // NOLINT
        if (t > a)
            return res + M;
        return res;
    }

    template <unsigned long M>                                         // NOLINT
    inline unsigned long subtract(unsigned long a, unsigned long b) {  // NOLINT
        unsigned long sub = b % M;  // NOLINT
        if (a < sub)
            return M - (sub - a);
        return a - sub;
    }

    // Calculates the forward difference between two wrapping numbers.
    //
    // Example:
    // uint8_t x = 253;
    // uint8_t y = 2;
    //
    // ForwardDiff(x, y) == 5
    //
    //   252   253   254   255    0     1     2     3
    // #################################################
    // |     |  x  |     |     |     |     |  y  |     |
    // #################################################
    //          |----->----->----->----->----->
    //
    // ForwardDiff(y, x) == 251
    //
    //   252   253   254   255    0     1     2     3
    // #################################################
    // |     |  x  |     |     |     |     |  y  |     |
    // #################################################
    // -->----->                              |----->---
    //
    // If M > 0 then wrapping occurs at M, if M == 0 then wrapping occurs at the
    // largest value representable by T.
    template <typename T, T M>
    inline typename std::enable_if<(M > 0), T>::type forward_diff(T a, T b) {
        static_assert(std::is_unsigned<T>::value,"Type must be an unsigned integer.");
        
        return a <= b ? b - a : M - (a - b);
    }

    template <typename T, T M>
    inline typename std::enable_if<(M == 0), T>::type forward_diff(T a, T b) {
        static_assert(std::is_unsigned<T>::value, "Type must be an unsigned integer.");
        return b - a;
    }

    template <typename T>
    inline T forward_diff(T a, T b) {
        return forward_diff<T, 0>(a, b);
    }

    // Calculates the reverse difference between two wrapping numbers.
    //
    // Example:
    // uint8_t x = 253;
    // uint8_t y = 2;
    //
    // ReverseDiff(y, x) == 5
    //
    //   252   253   254   255    0     1     2     3
    // #################################################
    // |     |  x  |     |     |     |     |  y  |     |
    // #################################################
    //          <-----<-----<-----<-----<-----|
    //
    // ReverseDiff(x, y) == 251
    //
    //   252   253   254   255    0     1     2     3
    // #################################################
    // |     |  x  |     |     |     |     |  y  |     |
    // #################################################
    // ---<-----|                             |<-----<--
    //
    // If M > 0 then wrapping occurs at M, if M == 0 then wrapping occurs at the
    // largest value representable by T.
    template <typename T, T M>
    inline typename std::enable_if<(M > 0), T>::type reverse_diff(T a, T b) {
        static_assert(std::is_unsigned<T>::value,"Type must be an unsigned integer.");
        
        return b <= a ? a - b : M - (b - a);
    }

    template <typename T, T M>
    inline typename std::enable_if<(M == 0), T>::type reverse_diff(T a, T b) {
        static_assert(std::is_unsigned<T>::value,"Type must be an unsigned integer.");
        return a - b;
    }

    template <typename T>
    inline T reverse_diff(T a, T b) {
        return reverse_diff<T, 0>(a, b);
    }

    // Calculates the minimum distance between to wrapping numbers.
    // The minimum distance is defined as min(ForwardDiff(a, b), ReverseDiff(a, b))
    template <typename T, T M = 0>
    inline T min_diff(T a, T b) {
        static_assert(std::is_unsigned<T>::value,"Type must be an unsigned integer.");
        return std::min(forward_diff<T, M>(a, b), reverse_diff<T, M>(a, b));
    }




    // Test if the sequence number `a` is ahead or at sequence number `b`.
    //
    // If `M` is an even number and the two sequence numbers are at max distance
    // from each other, then the sequence number with the highest value is
    // considered to be ahead.
    template <typename T, T M>
    inline typename std::enable_if<(M > 0), bool>::type ahead_or_at(T a, T b) {
        static_assert(std::is_unsigned<T>::value, "Type must be an unsigned integer.");
        const T maxDist = M / 2;
        if (!(M & 1) && min_diff<T, M>(a, b) == maxDist)
            return b < a;
        return forward_diff<T, M>(b, a) <= maxDist;
    }

    template <typename T, T M>
    inline typename std::enable_if<(M == 0), bool>::type ahead_or_at(T a, T b) {
        static_assert(std::is_unsigned<T>::value,"Type must be an unsigned integer.");
        const T max_dist = std::numeric_limits<T>::max() / 2 + T(1);
        if (a - b == max_dist)
            return b < a;
        return forward_diff(b, a) < max_dist;
    }

    template <typename T>
    inline bool ahead_or_at(T a, T b) {
        return ahead_or_at<T, 0>(a, b);
    }

    // Test if the sequence number `a` is ahead of sequence number `b`.
    //
    // If `M` is an even number and the two sequence numbers are at max distance
    // from each other, then the sequence number with the highest value is
    // considered to be ahead.
    template <typename T, T M = 0>
    inline bool ahead_of(T a, T b) {
        static_assert(std::is_unsigned<T>::value,"Type must be an unsigned integer.");
        return a != b && ahead_or_at<T, M>(a, b);
    }

}
}




