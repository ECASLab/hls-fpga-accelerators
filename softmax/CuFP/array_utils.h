#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H


// *****************************************************************************************************************

// Function to get the maximum of two values
template<typename T>
T max_value(T a, T b) {
    return (a > b) ? a : b;
}

// Template structure to find the max in an array
template<typename T, int N>
struct MaxFinder {
    static T find_max(const T array[N]) {
#pragma HLS INLINE
        T left_max = MaxFinder<T, N/2>::find_max(array);
        T right_max = MaxFinder<T, N-N/2>::find_max(array + N/2);
        return max_value(left_max, right_max);
    }
};

// Specialization for the base case when N == 1
template<typename T>
struct MaxFinder<T, 1> {
    static T find_max(const T array[1]) {
#pragma HLS INLINE
        return array[0];
    }
};

// Wrapper function to make it easier to call
template<typename T, int N>
T find_max(const T array[N]) {
#pragma HLS INLINE
    return MaxFinder<T, N>::find_max(array);
}

// *****************************************************************************************************************

// Function to get the maximum of two values
template<typename T>
T _sum(T a, T b) {
    return a + b;
}

// Template structure to find the max in an array
template<typename T, int N>
struct CalcSum {
    static T calc_sum(const T array[N]) {
#pragma HLS INLINE
        T left_max = CalcSum<T, N/2>::calc_sum(array);
        T right_max = CalcSum<T, N-N/2>::calc_sum(array + N/2);
        return _sum(left_max, right_max);
    }
};

// Specialization for the base case when N == 1
template<typename T>
struct CalcSum<T, 1> {
    static T calc_sum(const T array[1]) {
#pragma HLS INLINE
        return array[0];
    }
};

// Wrapper function to make it easier to call
template<typename T, int N>
T array_sum(const T array[N]) {
#pragma HLS INLINE
    return CalcSum<T, N>::calc_sum(array);
}

// *****************************************************************************************************************


#endif // ARRAY_UTILS_H
