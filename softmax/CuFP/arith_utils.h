#ifndef ARITH_UTILS_H
#define ARITH_UTILS_H

#include <ap_int.h>

namespace CuFl {

template<int W>
void lshu(ap_uint<W> &x, const unsigned char &shift)
{
#pragma HLS inline
	const bool b0 = shift & static_cast<const unsigned char>(1);
	const bool b1 = shift & static_cast<const unsigned char>(2);
	const bool b2 = shift & static_cast<const unsigned char>(4);
	const bool b3 = shift & static_cast<const unsigned char>(8);
	const bool b4 = shift & static_cast<const unsigned char>(16);
	const bool b5 = shift & static_cast<const unsigned char>(32);
	x <<= b0 ? 1 : 0;
	x <<= b1 ? 2 : 0;
	x <<= b2 ? 4 : 0;
	x <<= b3 ? 8 : 0;
	x <<= b4 ? 16 : 0;
	x <<= b5 ? 32 : 0;
}

template<int W>
void lsh(ap_int<W> &x, const unsigned char &shift)
{
//#pragma HLS inline
	const bool b0 = shift & static_cast<const unsigned char>(1);
	const bool b1 = shift & static_cast<const unsigned char>(2);
	const bool b2 = shift & static_cast<const unsigned char>(4);
	const bool b3 = shift & static_cast<const unsigned char>(8);
	const bool b4 = shift & static_cast<const unsigned char>(16);
	const bool b5 = shift & static_cast<const unsigned char>(32);
	x <<= b0 ? 1 : 0;
	x <<= b1 ? 2 : 0;
	x <<= b2 ? 4 : 0;
	x <<= b3 ? 8 : 0;
	x <<= b4 ? 16 : 0;
	x <<= b5 ? 32 : 0;
}

template<int W>
void rshu(ap_uint<W> &x, const unsigned char &shift)
{
#pragma HLS inline
	const bool b0 = shift & static_cast<const unsigned char>(1);
	const bool b1 = shift & static_cast<const unsigned char>(2);
	const bool b2 = shift & static_cast<const unsigned char>(4);
	const bool b3 = shift & static_cast<const unsigned char>(8);
	const bool b4 = shift & static_cast<const unsigned char>(16);
	const bool b5 = shift & static_cast<const unsigned char>(32);
	x >>= b0 ? 1 : 0;
	x >>= b1 ? 2 : 0;
	x >>= b2 ? 4 : 0;
	x >>= b3 ? 8 : 0;
	x >>= b4 ? 16 : 0;
	x >>= b5 ? 32 : 0;
}

template<int W>
void rsh(ap_int<W> &x, const unsigned char &shift)
{
//#pragma HLS inline
	const bool b0 = shift & static_cast<const unsigned char>(1);
	const bool b1 = shift & static_cast<const unsigned char>(2);
	const bool b2 = shift & static_cast<const unsigned char>(4);
	const bool b3 = shift & static_cast<const unsigned char>(8);
	const bool b4 = shift & static_cast<const unsigned char>(16);
	const bool b5 = shift & static_cast<const unsigned char>(32);
	x >>= b0 ? 1 : 0;
	x >>= b1 ? 2 : 0;
	x >>= b2 ? 4 : 0;
	x >>= b3 ? 8 : 0;
	x >>= b4 ? 16 : 0;
	x >>= b5 ? 32 : 0;
}

template<int W>
short enc(const ap_uint<W> &x)
{
	if (W == 2)
		return x[1] ? 1 : x[0] ? 0 : 0;
	else if (W == 3)
		return x[2] ? 2 : x[1] ? 1 : x[0] ? 0 : 0;
	else if (W == 4)
		return x[3] ? 3 : x[2] ? 2 : x[1] ? 1 : x[0] ? 0 : 0;
	else if (W == 5)
		return x[4] ? 4 : x[3] ? 3 : x[2] ? 2 : x[1] ? 1 : x[0] ? 0 : 0;
	else if (W == 6)
		return x[5] ? 5 : x[4] ? 4 : x[3] ? 3 : x[2] ? 2 : x[1] ? 1 : x[0] ? 0 : 0;
	else if (W == 7)
		return x[6] ? 6 : x[5] ? 5 : x[4] ? 4 : x[3] ? 3 : x[2] ? 2 : x[1] ? 1 : x[0] ? 0 : 0;
	else if (W == 8)
		return x[7] ? 7 : x[6] ? 6 : x[5] ? 5 : x[4] ? 4 : x[3] ? 3 : x[2] ? 2 : x[1] ? 1 : x[0] ? 0 : 0;
	else {
//        #error "This W is not supported!"
		return 0;
	}
}

}


#endif // ARITH_UTILS_H
