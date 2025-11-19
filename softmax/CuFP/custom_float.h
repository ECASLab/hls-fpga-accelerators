#ifndef CUSTOM_FLOAT_H
#define CUSTOM_FLOAT_H

#include <math.h>
#include <iostream>
#include <ap_fixed.h>
#include "array_utils.h"
#include "arith_utils.h"

// Macros to parametrize the pragmas
#define PRAGMA_SUB(x) _Pragma (#x)
#define PRAGMA_HLS(x) PRAGMA_SUB(x)

#define EN_ROUNDING

namespace CuFl {

template <int WL, int MS>
class CustomFloat;

template<int WR, int MR, int WX, int MX, int WY, int MY>
CustomFloat<WR, MR> sum(const CustomFloat<WX, MX> &x, const CustomFloat<WY, MY> &y);

template<int WR, int MR, int WX, int MX, int WY, int MY>
CustomFloat<WR, MR> mul(const CustomFloat<WX, MX> &x, const CustomFloat<WY, MY> &y);

template<int WR, int MR, int WX, int MX, int WY, int MY>
CustomFloat<WR, MR> sub(const CustomFloat<WX, MX> &x, const CustomFloat<WY, MY> &y);
    




union Transfer {
    double d;
    unsigned long long i;
};

union TransferFloat {
    float f;
    unsigned int i;
};

union TransferHalf {
    half h;
    unsigned short i; // 16 bits
};

template <int WL, int MS>
class CustomFloat
{
public:
    ap_uint<MS+1> mnts;
    ap_uint<WL-MS> exp;
    bool sign;

    CustomFloat() :
    	mnts(0),
		exp(0),
		sign(0)
    {}

    CustomFloat(double val)
    {   
        if (val == 0.0) {
            mnts = 0;
            exp = 0;
            sign = 0;
            return;
        }
        
    	const ap_int<WL-MS> biasExp((1ull << (WL - MS - 1 - 1)) - 1ull);

        Transfer t;
        t.d = val;

        // Mantissa
        ap_uint<64> ee(t.i);

    	bool Sb = ee.range(52 - MS - 2, 0) != 0;
    	bool Rb = ee[52 - MS - 1] == 1;
    	bool Gb = ee[52 - MS ] == 1;
    	mnts = ee.range(52 - 1, 52 - MS);
    	if ((Gb && Rb && !Sb) || (Rb && Sb)) {
    		mnts += ap_int<MS+1>(1);
    	}

        // Exponent
        ap_int<WL-MS> expo(((t.i & 0x7FF0000000000000ull) >> 52) - 1023);
        if (expo > biasExp)
            expo = biasExp;
        else if (expo < - biasExp)
            expo = - biasExp;

        exp = expo + biasExp;

        // Sign
        sign = t.i >> 63;
    }

    CustomFloat(const CustomFloat<WL, MS>& copy)
    {
        this->mnts      = copy.mnts;
        this->exp       = copy.exp;
        this->sign      = copy.sign;
    }

    CustomFloat<WL, MS>& operator=(const CustomFloat<WL, MS>& other) {
        this->mnts = other.mnts;
        this->exp = other.exp;
        this->sign = other.sign;
        return *this;
    }

    CustomFloat<WL, MS> operator-() const {
        CustomFloat<WL, MS> result = *this;
        result.sign = !this->sign;
        return result;
    }
    
    template<int W_other, int M_other>
    CustomFloat<WL, MS> operator+(const CustomFloat<W_other, M_other>& other) const {
        return sum<WL, MS, WL, MS, W_other, M_other>(*this, other);
    }

    template<int W_other, int M_other>
    CustomFloat<WL, MS> operator-(const CustomFloat<W_other, M_other>& other) const {
        return sub<WL, MS, WL, MS, W_other, M_other>(*this, other);
    }

    template<int W_other, int M_other>
    CustomFloat<WL, MS> operator*(const CustomFloat<W_other, M_other>& other) const {
        return mul<WL, MS, WL, MS, W_other, M_other>(*this, other);
    }

    template<int W_other, int M_other>
    CustomFloat<WL, MS>& operator+=(const CustomFloat<W_other, M_other>& other) {
        *this = *this + other;
        return *this;
    }

    template<int W_other, int M_other>
    CustomFloat<WL, MS>& operator-=(const CustomFloat<W_other, M_other>& other) {
        *this = *this - other;
        return *this;
    }
    
    template<int W_other, int M_other>
    CustomFloat<WL, MS>& operator*=(const CustomFloat<W_other, M_other>& other) {
        *this = *this * other;
        return *this;
    }


    CustomFloat<WL, MS> reciprocal() const {
    #pragma HLS inline

        

        static const ap_int<WL-MS> bias = (ap_int<WL-MS>(1) << (WL - MS - 2)) - 1;
        
        ap_int<WL-MS> real_exp = ap_int<WL-MS>(this->exp) - bias;
        
        ap_int<WL-MS> recip_real_exp = -real_exp;

        ap_uint<MS+1> den = (ap_uint<MS+1>(1) << MS) | this->mnts.range(MS-1, 0);

        static const int DivWidth = 2*MS + 2; 
        ap_uint<DivWidth> num = (ap_uint<DivWidth>(1) << (2 * MS));
        ap_uint<DivWidth> den_wide = den; 

        ap_uint<DivWidth> qq = num / den_wide;
        ap_uint<DivWidth> rem = num % den_wide;

        bool rem_tie = (rem << 1) == den_wide;
        bool rem_gt = (rem << 1) > den_wide;
        bool qq_odd = qq.test(0); 
        
        if (rem_gt || (rem_tie && qq_odd)) {
            ++qq;
        }

        if (!qq.test(MS)) {
            qq <<= 1;
            recip_real_exp -= 1;
        }

        
        CustomFloat<WL, MS> result;
        result.sign = this->sign; 
        result.exp = recip_real_exp + bias; 
        result.mnts = qq.range(MS - 1, 0); 

        
        if (result.mnts == 0 && qq.range(MS,0) == 0) {
             result.sign = 0;
             result.exp = 0;
        }

        return result;
    }
    

    template<int W_other, int M_other>
    CustomFloat<WL, MS> operator/(const CustomFloat<W_other, M_other>& other) const {
        return *this * other.reciprocal();
    }


    CustomFloat<WL, MS> operator/(int i) const {
        CustomFloat<WL, MS> reciprocal_i(1.0 / static_cast<half>(i));
        return *this * reciprocal_i;
    }

    template<int W_other, int M_other>
    CustomFloat<WL, MS>& operator/=(const CustomFloat<W_other, M_other>& other) {
        *this = *this / other;
        return *this;
    }
    CustomFloat<WL, MS>& operator/=(int i) {
        *this = *this / i;
        return *this;
    }





    double getDouble() const {
    	if (isZero()) {
        return 0.0;
        }
        ap_uint<WL-MS> biasExp((1ull << (WL - MS - 1 - 1)) - 1ull);

        Transfer t;
        t.i = (sign ? 1ull : 0ull) << 63;
        t.i = t.i | ap_uint<64>((ap_uint<64>(1023) + exp - biasExp) << 52).to_uint64();
        t.i = t.i | (mnts.to_uint64() << (52 - MS));

        return t.d;
    }


    
    half getHalf() const {
#pragma HLS INLINE off

    if (isZero()) {
        return (half)0.0;
    }

    static const ap_int<WL-MS> customBias = (1 << (WL - MS - 2)) - 1;
    const int halfBias = 15;

    ap_int<WL-MS> unbiased_exp = exp - customBias;
    ap_uint<5> half_exp = unbiased_exp + halfBias;

    ap_uint<10> half_mnts;
    if (MS > 10) {
        half_mnts = mnts >> (MS - 10);
    } else if (MS < 10) {
        half_mnts = mnts << (10 - MS);
    } else {
        half_mnts = mnts;
    }

    ap_uint<16> half_bits = (ap_uint<1>(sign), ap_uint<5>(half_exp), ap_uint<10>(half_mnts));

    return *reinterpret_cast<half*>(&half_bits);
}

    bool isZero() const {
#pragma HLS inline
        return (mnts == 0 && exp == 0);
    }
    void print() const
    {
        printf("The value is %4.8f\n", getDouble());
    }

    constexpr int getTotalWidth() const
    {
        return WL;
    }

    constexpr int getMantissaWidth() const
    {
        return MS;
    }
};

template<int WR, int MR, int WX, int MX, int WY, int MY>
CustomFloat<WR, MR> mul(const CustomFloat<WX, MX> &x, const CustomFloat<WY, MY> &y)
{
#pragma HLS inline
    if (x.isZero() || y.isZero()) {
        return CustomFloat<WR, MR>(); // Devuelve un CustomFloat inicializado a cero
    }
	const ap_int<WX-MX> biasExpX((ap_uint<WX-MX>(1) << (WX - MX - 1 - 1)) - 1);
	const ap_int<WY-MY> biasExpY((ap_uint<WY-MY>(1) << (WY - MY - 1 - 1)) - 1);
	const ap_int<WR-MR> biasExpR((ap_uint<WR-MR>(1) << (WR - MR - 1 - 1)) - 1);

    ap_uint<MX+1+MY+1> mnts(((ap_uint<MX+1>(1) << MX) | x.mnts) * ((ap_uint<MY+1>(1) << MY) | y.mnts));

    ap_uint<2> normBit( mnts >> (MX + MY + 1));
    mnts >>= normBit;

    const int shift = MX + MY - MR;
    if (shift > 0) {
#ifdef EN_ROUNDING
    	const bool Sb = (shift > 1) ? (mnts.range(shift - 2, 0) != 0) : 0;
    	const bool Rb = mnts[shift -1] == 1;
    	const bool Gb = mnts[shift] == 1;

    	rshu<MX+1+MY+1>(mnts, shift);

    	if ((Gb && Rb && !Sb) || (Rb && Sb)) {
    		mnts += 1;
        }
#else
        rshu<MX+1+MY+1>(mnts, shift);
#endif
    }
    else if (shift < 0) {
        mnts <<= - shift;
    }

    CustomFloat<WR, MR> r;
    r.mnts = mnts & ((ap_uint<MX+1+MY+1>(1) << MR) - 1);
    r.exp = ap_uint<WR-MR>((ap_int<WX-MX>(x.exp) - biasExpX) + (ap_int<WY-MY>(y.exp) - biasExpY) + biasExpR + normBit);
    r.sign = x.sign ^ y.sign;

    return r;
}

template<int WR, int MR, int WX, int MX, int WY, int MY>
CustomFloat<WR, MR> sum(const CustomFloat<WX, MX> &x, const CustomFloat<WY, MY> &y)
{
#pragma HLS inline

    if (x.isZero()) {
        CustomFloat<WR,MR> r(y.getDouble()); // Convierte y al formato de resultado
        return r;
    }
    if (y.isZero()) {
        CustomFloat<WR,MR> r(x.getDouble()); // Convierte x al formato de resultado
        return r;
    }
    CustomFloat<WR, MR> r;

    const ap_int<WX-MX> biasExpX = (ap_int<WX-MX>(1) << (WX - MX - 1 - 1)) - 1;
    const ap_int<WY-MY> biasExpY = (ap_int<WY-MY>(1) << (WY - MY - 1 - 1)) - 1;
    const ap_int<WR-MR> biasExpR = (ap_int<WR-MR>(1) << (WR - MR - 1 - 1)) - 1;

    const ap_int<WX-MX> x_exp = ap_int<WX-MX>(x.exp) - biasExpX;
    const ap_int<WY-MY> y_exp = ap_int<WY-MY>(y.exp) - biasExpY;

    const unsigned char maxm = (MX > MY) ? MX : MY;
    const unsigned char diff = abs(x_exp - y_exp);
    const bool xlty = x_exp < y_exp;
    const unsigned char lt_wm = xlty ? MX : MY;
    const unsigned char gt_wm = xlty ? MY : MX;

    // Define 1.(x mantissa) and 1.(y mantissa)
    ap_int<maxm+2> m1(xlty ? (x.mnts | ap_int<maxm+2>(1) << MX) : (y.mnts | ap_int<maxm+2>(1) << MY));
    ap_int<maxm+2> m2(xlty ? (y.mnts | ap_int<maxm+2>(1) << MY) : (x.mnts | ap_int<maxm+2>(1) << MX));

    const bool lt_sign = xlty ? x.sign : y.sign;
    const bool gt_sign = xlty ? y.sign : x.sign;
    const ap_int<WX-MX> gt_exp = xlty ? y_exp : x_exp;

    // Align the fraction of the smaller number to the larger number to unify their exponents
#ifdef EN_ROUNDING
    if (diff > lt_wm) {
    	r = xlty ? y : x;
    	return r;
    }
    else if (diff > 0) {
    	const bool Sb = (diff > 1) ? (m1.range(diff - 2, 0) != 0) : 0;
    	const bool Rb = m1[diff -1] == 1;
    	const bool Gb = m1[diff] == 1;

    	rsh<maxm+2>(m1, diff);

    	if ((Gb && Rb && !Sb) || (Rb && Sb)) {
        	m1 += 1;
        }
    }
#else
    // m1 >>= diff;
    rsh<maxm+2>(m1, diff);
#endif

    // Extend the shorter fraction to have the same size as the longer one
    unsigned char ash = abs(lt_wm - gt_wm);
    if (lt_wm < gt_wm) {
        lsh<maxm+2>(m1, ash);
    }
    else if (lt_wm > gt_wm) {
        lsh<maxm+2>(m2, ash);
    }

    // Apply the sign of the input fractions
    if (lt_sign) {
        m1 = - m1;
    }
    if (gt_sign) {
        m2 = - m2;
    }

    // Sum the two pre-processed fractions
    ap_int<maxm+3> s(m1 + m2);

    // Apply the fraction's sign
    r.sign = 0;
    if (s < 0) {
        r.sign = 1;
        s = - s;
    }

    // Post-process the fraction and exponent (normalizing and aligning)
    if (s == 0) {
        r.exp = 0;
    }
    else {
        r.exp = ap_uint<WR-MR>(gt_exp + biasExpR);

        // Align the fraction: Find the most left 1 bit and shift the fraction to form the right format (1.s)
        if (s.range(maxm+2, maxm) == 0) {
        	const int leadone = __builtin_clz(s.range(maxm-1, 0))+1-(8*sizeof(int))+maxm;
        	lsh(s, leadone);
        	r.exp -= leadone;
        }
        // Did overflow occur to the fraction? If yes, the fraction must be shifted 1-bit to the right and the exponent must be added by one
        else if ((s >> maxm) > 1) {
#ifdef EN_ROUNDING
        	const bool Sb = 0;
        	const bool Rb = s[0] == 1;
        	const bool Gb = s[1] == 1;
        	s >>= 1;
        	if ((Gb && Rb && !Sb) || (Rb && Sb)) {
        		s += 1;
        	}
#else
        	s >>= 1;
#endif
            r.exp += 1;
        }

        // Remove the excess '1' in the freaction (Convert the fraction from 1.s to 0.s)
        s = s & ((ap_int<maxm+3>(1) << maxm) - 1);

        // Fit the result into the output bit-width
        int cc = MR - maxm;
        if (cc > 0) {
        	// s <<= cc;
        	lsh<maxm+3>(s, cc);
        }
        else if (cc < 0) {
            cc = - cc;
#ifdef EN_ROUNDING
            const bool Sb = (cc > 1) ? (s.range(cc - 2, 0) != 0) : 0;
            const bool Rb = s[cc -1] == 1;
            const bool Gb = s[cc] == 1;

            rsh<maxm+3>(s, cc);

            if ((Gb && Rb && !Sb) || (Rb && Sb)) {
            	s += 1;
            }
#else
            rsh<maxm+3>(s, cc);
#endif
        }
    }
    r.mnts = ap_uint<MR+1>(s);

    return r;
}

template<int WR, int MR, int WX, int MX, int WY, int MY>
CustomFloat<WR, MR> sub(const CustomFloat<WX, MX> &x, const CustomFloat<WY, MY> &y)
{
	CustomFloat<WY, MY> my(y);
    my.sign = ! y.sign;
    return sum<WR, MR>(x, my);
}

template<int WR, int MR, int N>
CustomFloat<WR, MR> vsum(const CustomFloat<WR, MR> x[N])
{
#pragma HLS inline
	CustomFloat<WR, MR> r;

	PRAGMA_HLS(HLS ARRAY_PARTITION  variable=x complete dim=0)

	const int exbit = ((N <= 4) and (N > 2))     ? 2 :
			          ((N <= 8) and (N > 4))     ? 3 :
					  ((N <= 16) and (N > 8))    ? 4 :
					  ((N <= 32) and (N > 16))   ? 5 :
					  ((N <= 64) and (N > 32))   ? 6 :
					  ((N <= 128) and (N > 64))  ? 7 :
					  ((N <= 256) and (N > 128)) ? 8 : 0;

	const ap_int<WR-MR> bias_exp((ap_int<WR-MR>(1) << (WR - MR - 1 - 1)) - 1);

	ap_int<WR-MR> e[N];
	PRAGMA_HLS(HLS ARRAY_PARTITION  variable=e complete dim=0)
	L1: for (int i = 0; i < N; i ++) {
		PRAGMA_HLS(HLS unroll factor=N)
		PRAGMA_HLS(HLS PIPELINE II=1)
		e[i] = x[i].exp;
	}
	const ap_int<WR-MR> max_exp = find_max<ap_int<WR-MR>, N>(e);

	ap_int<WR-MR> d_exp[N];
	PRAGMA_HLS(HLS ARRAY_PARTITION  variable=d_exp complete dim=0)
	L2: for (int i = 0; i < N; i ++) {
		PRAGMA_HLS(HLS unroll factor=N)
		PRAGMA_HLS(HLS PIPELINE II=1)
		d_exp[i] = max_exp - x[i].exp;
	}

	ap_int<MR+exbit+2> m[N];
	PRAGMA_HLS(HLS ARRAY_PARTITION  variable=m complete dim=0)
	L3: for (int i = 0; i < N; i++) {
		PRAGMA_HLS(HLS unroll factor=N)
		PRAGMA_HLS(HLS PIPELINE II=1)
		m[i] = (x[i].mnts | (ap_uint<MR+1>(1) << MR)) >> d_exp[i];
	}

	L4: for (int i = 0; i < N; i ++) {
		PRAGMA_HLS(HLS unroll factor=N)
		PRAGMA_HLS(HLS PIPELINE II=1)
		m[i] = x[i].sign ? ap_int<MR+exbit+2>(- m[i]) : m[i];
	}

	ap_int<MR+exbit+2> s(array_sum<ap_int<MR+exbit+2>, N>(m));

	r.sign = 0;
	if (s < 0) {
		r.sign = 1;
		s = - s;
	}

	if (s == 0) {
		r.exp = 0;
	}
	else {
		r.exp = max_exp;

		// Align mantissa
		if (s.range(MR+exbit+1, MR) == 0) {
			const int leadone = __builtin_clz(s.range(MR-1, 0))+1-(8*sizeof(int))+MR;
			lsh(s, leadone);
			r.exp -= leadone;
		}

		short diff = enc<exbit+1>(ap_uint<exbit+1>(s.range(MR+exbit, MR)));
	    if (diff > 0) {
	    	bool Sb = (diff > 1) ? (s.range(diff - 2, 0) != 0) : 0;
	    	bool Rb = s[diff -1] == 1;
	    	bool Gb = s[diff] == 1;
	    	// s >>= diff;
	    	rsh<MR+exbit+2>(s, diff);
	    	if ((Gb && Rb && !Sb) || (Rb && Sb)) {
	    		s += ap_int<MR+exbit+2>(1);
	    	}
	    	r.exp += diff;
	    }

	    s = s & ((ap_int<MR+exbit+2>(1) << MR) - 1);
	}

	r.mnts = s;

    return r;
}

template<int WR, int MR, int N>
CustomFloat<WR, MR> dp(const CustomFloat<WR, MR> a[N], const CustomFloat<WR, MR> x[N])
{
	PRAGMA_HLS(HLS inline recursive)
	PRAGMA_HLS(HLS PIPELINE II=1)
	CustomFloat<WR, MR> v[N];
	for (int i = 0; i < N; i ++) {
		PRAGMA_HLS(HLS unroll factor=N)
		v[i] = mul<WR, MR>(a[i], x[i]);
	}
	return vsum<WR, MR, N>(v);
}

template<int WR, int MR, int N>
void mvm(const CuFl::CustomFloat<WR, MR> x[N][N], const CuFl::CustomFloat<WR, MR> y[N], CuFl::CustomFloat<WR, MR> z[N])
{
	PRAGMA_HLS(HLS inline recursive)
	PRAGMA_HLS(HLS PIPELINE II=1)
	for (int i = 0; i < N; i++) {
		PRAGMA_HLS(HLS unroll factor=N)
		z[i] = CuFl::dp<WR, MR, N>(&x[i][0], y);
	}
}

template<int WR, int MR, int N>
void matmul(const CuFl::CustomFloat<WR, MR> x[N][N], const CuFl::CustomFloat<WR, MR> y[N][N], CuFl::CustomFloat<WR, MR> z[N][N])
{
	PRAGMA_HLS(HLS inline recursive)
	PRAGMA_HLS(HLS PIPELINE II=1)
	for (int i = 0; i < N; i++) {
		PRAGMA_HLS(HLS PIPELINE II=1)
		for (int j = 0; j < N; j++) {
			PRAGMA_HLS(HLS unroll factor=N)
		    CustomFloat<WR, MR> vy[N];
		    for (int k = 0; k < N; k ++) {
		    	PRAGMA_HLS(HLS unroll factor=N)
		    	vy[k] = y[k][j];
		    }
			z[i][j] = CuFl::dp<WR, MR, N>(&x[i][0], vy);
		}
	}
}


#ifndef __SYNTHESIS__ // <-- Importante: evita que se sintetice
#include <ostream>
template <int WL, int MS>
std::ostream& operator<<(std::ostream& os, const CustomFloat<WL, MS>& val) {
    os << val.getDouble(); // Imprime la representación en double
    return os;
}
#endif

}

#endif // CUSTOM_FLOAT_H
