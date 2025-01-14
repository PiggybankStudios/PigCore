/*
File:   wasm_std_math_helpers.c
Author: Taylor Robbins
Date:   01\13\2025
Description: 
	** Holds helper functions and macros used in the implementation of various
	** functions that are exposed in math.h
*/

// +--------------------------------------------------------------+
// |                    Float Related Helpers                     |
// +--------------------------------------------------------------+
static const float_t tointf = 1/((FLT_EVAL_METHOD == 0) ? FLT_EPSILON : DBL_EPSILON);
static const float_t tointd = 1/DBL_EPSILON;

float fp_barrierf(float x)
{
	volatile float y = x;
	return y;
}
double fp_barrier(double x)
{
	volatile double y = x;
	return y;
}

float __math_divzerof(uint32_t sign)
{
	return fp_barrierf(sign ? -1.0f : 1.0f) / 0.0f;
}
double __math_divzero(uint32_t sign)
{
	//TODO: This used to have an fp_barrier call around the numerator. Do we need that in WASM?
	return fp_barrierf(sign ? -1.0 : 1.0) / 0.0;
}

uint32_t top16(double x)
{
	return asuint64(x) >> 48;
}
uint32_t top12(double value)
{
	return (asuint64(value) >> 52);
}
uint32_t top12f(float x)
{
	return (asuint(x) >> 20);
}

// +--------------------------------------------------------------+
// |                 __sindf __cosdf and __tandf                  |
// +--------------------------------------------------------------+
// |sin(value)/value - s(value)| < 2**-37.5 (~[-4.89e-12, 4.824e-12]).
static const double
	S1 = -0x15555554CBAC77.0p-55, // -0.166666666416265235595
	S2 =  0x111110896EFBB2.0p-59, //  0.0083333293858894631756
	S3 = -0x1A00F9E2CAE774.0p-65, // -0.000198393348360966317347
	S4 =  0x16CD878C3B46A7.0p-71; //  0.0000027183114939898219064

float __sindf(double value)
{
	double_t rVar, sVar, quad, square;
	// Try to optimize for parallel evaluation as in __tandf.c.
	square = value * value;
	quad = square * square;
	rVar = S3 + (square * S4);
	sVar = square * value;
	return (value + (sVar * (S1 + (square * S2)))) + (sVar * quad * rVar);
}

// |cos(value) - c(value)| < 2**-34.1 (~[-5.37e-11, 5.295e-11]).
static const double
	C0 = -0x1FFFFFFD0C5E81.0p-54, // -0.499999997251031003120
	C1 =  0x155553E1053A42.0p-57, //  0.0416666233237390631894
	C2 = -0x16C087E80F1E27.0p-62, // -0.00138867637746099294692
	C3 =  0x199342E0EE5069.0p-68; //  0.0000243904487962774090654

float __cosdf(double value)
{
	double_t rVar, quad, square;
	// Try to optimize for parallel evaluation as in __tandf.c.
	square = value * value;
	quad = square * square;
	rVar = C2 + (square * C3);
	return ((1.0 + (square * C0)) + (quad * C1)) + ((quad * square) * rVar);
}

// |tan(x)/x - t(x)| < 2**-25.5 (~[-2e-08, 2e-08]).
static const double T[] = {
  0x15554d3418c99f.0p-54, /* 0.333331395030791399758 */
  0x1112fd38999f72.0p-55, /* 0.133392002712976742718 */
  0x1b54c91d865afe.0p-57, /* 0.0533812378445670393523 */
  0x191df3908c33ce.0p-58, /* 0.0245283181166547278873 */
  0x185dadfcecf44e.0p-61, /* 0.00297435743359967304927 */
  0x1362b9bf971bcd.0p-59, /* 0.00946564784943673166728 */
};

float __tandf(double value, int odd)
{
	double_t square, rVar, quad, cube, tVar, uVar;

	square = value * value;
	// Split up the polynomial into small independent terms to give
	// opportunities for parallel evaluation.  The chosen splitting is
	// micro-optimized for Athlons (XP, X64).  It costs 2 multiplications
	// relative to Horner's method on sequential machines.
	// 
	// We add the small terms from lowest degree up for efficiency on
	// non-sequential machines (the lowest degree terms tend to be ready
	// earlier).  Apart from this, we don't care about order of
	// operations, and don't need to to care since we have precision to
	// spare.  However, the chosen splitting is good for accuracy too,
	// and would give results as accurate as Horner's method if the
	// small terms were added from highest degree down.
	rVar = T[4] + (square * T[5]);
	tVar = T[2] + (square * T[3]);
	quad = square * square;
	cube = square * value;
	uVar = T[0] + (square * T[1]);
	rVar = (value + (cube * uVar)) + ((cube * quad) * (tVar + (quad * rVar)));
	return (odd ? -1.0 / rVar : rVar);
}

// +--------------------------------------------------------------+
// |                    __sin __cos and __tan                     |
// +--------------------------------------------------------------+

static const double
	S1d = -1.66666666666666324348e-01, // 0xBFC55555, 0x55555549
	S2d =  8.33333333332248946124e-03, // 0x3F811111, 0x1110F8A6
	S3d = -1.98412698298579493134e-04, // 0xBF2A01A0, 0x19C161D5
	S4d =  2.75573137070700676789e-06, // 0x3EC71DE3, 0x57B1FE7D
	S5d = -2.50507602534068634195e-08, // 0xBE5AE5E6, 0x8A2B9CEB
	S6d =  1.58969099521155010221e-10; // 0x3DE5D93A, 0x5ACFD57C

double __sin(double value, double other, int odd)
{
	double_t square, rVar, vVar, fourth;
	square = value * value;
	fourth = square * square;
	rVar = S2d + (square * (S3d + (square * S4d))) + (square * fourth * (S5d + (square * S6d)));
	vVar = square * value;
	if (odd == 0) { return value + (vVar * (S1d + (square * rVar))); }
	else { return value - ((square*(0.5*other - vVar*rVar) - other) - vVar*S1d); }
}

static const double
	C1d =  4.16666666666666019037e-02, // 0x3FA55555, 0x5555554C
	C2d = -1.38888888888741095749e-03, // 0xBF56C16C, 0x16C15177
	C3d =  2.48015872894767294178e-05, // 0x3EFA01A0, 0x19CB1590
	C4d = -2.75573143513906633035e-07, // 0xBE927E4F, 0x809C52AD
	C5d =  2.08757232129817482790e-09, // 0x3E21EE9E, 0xBDB4B1C4
	C6d = -1.13596475577881948265e-11; // 0xBDA8FAE9, 0xBE8838D4

double __cos(double value, double other)
{
	double_t halfSquare, square, rVar, fourth;
	square = value * value;
	fourth = square * square;
	rVar = (square * (C1d + (square * (C2d + (square * C3d))))) + (fourth * fourth * (C4d + (square * (C5d + (square * C6d)))));
	halfSquare = 0.5 * square;
	fourth = 1.0 - halfSquare;
	return fourth + (((1.0 - fourth) - halfSquare) + ((square * rVar) - (value * other)));
}

static const double Td[] = {
	 3.33333333333334091986e-01, // 3FD55555, 55555563
	 1.33333333333201242699e-01, // 3FC11111, 1110FE7A
	 5.39682539762260521377e-02, // 3FABA1BA, 1BB341FE
	 2.18694882948595424599e-02, // 3F9664F4, 8406D637
	 8.86323982359930005737e-03, // 3F8226E3, E96E8493
	 3.59207910759131235356e-03, // 3F6D6D22, C9560328
	 1.45620945432529025516e-03, // 3F57DBC8, FEE08315
	 5.88041240820264096874e-04, // 3F4344D8, F2F26501
	 2.46463134818469906812e-04, // 3F3026F7, 1A8D1068
	 7.81794442939557092300e-05, // 3F147E88, A03792A6
	 7.14072491382608190305e-05, // 3F12B80F, 32F0A7E9
	-1.85586374855275456654e-05, // BEF375CB, DB605373
	 2.59073051863633712884e-05, // 3EFB2A70, 74BF7AD4
};
static const double tan_pio4 = 7.85398163397448278999e-01;   // 3FE921FB, 54442D18
static const double tan_pio4lo = 3.06161699786838301793e-17; // 3C81A626, 33145C07
double __tan(double value, double other, int odd)
{
	double_t square, rVar, vVar, quad, cube, aVar;
	double quad0, aVar0;
	uint32_t highWord;
	int big, sign;

	GET_HIGH_WORD(highWord, value);
	big = ((highWord & 0x7FFFFFFF) >= 0x3FE59428); // |value| >= 0.6744
	if (big)
	{
		sign = highWord>>31;
		if (sign)
		{
			value = -value;
			other = -other;
		}
		value = (tan_pio4 - value) + (tan_pio4lo - other);
		other = 0.0;
	}
	square = value * value;
	quad = square * square;
	// Break value^5*(Td[1]+value^2*Td[2]+...) into
	// value^5(Td[1]+value^4*Td[3]+...+value^20*Td[11]) +
	// value^5(value^2*(Td[2]+value^4*Td[4]+...+value^22*[T12]))
	rVar = Td[1] + quad * (Td[3] + quad * (Td[5] + quad * (Td[7] + quad * (Td[9] + quad * Td[11]))));
	vVar = square * (Td[2] + quad * (Td[4] + quad * (Td[6] + quad * (Td[8] + quad * (Td[10] + quad * Td[12])))));
	cube = square * value;
	rVar = other + square * (cube * (rVar + vVar) + other) + (cube * Td[0]);
	quad = value + rVar;
	if (big)
	{
		cube = 1 - (2 * odd);
		vVar = cube - 2.0 * (value + (rVar - quad * quad / (quad + cube)));
		return (sign ? -vVar : vVar);
	}
	if (!odd) { return quad; }
	// -1.0/(value+rVar) has up to 2ulp error, so compute it accurately
	quad0 = quad;
	SET_LOW_WORD(quad0, 0);
	vVar = rVar - (quad0 - value); // quad0+vVar = rVar+value
	aVar0 = aVar = -1.0 / quad;
	SET_LOW_WORD(aVar0, 0);
	return aVar0 + aVar*(1.0 + aVar0*quad0 + aVar0*vVar);
}

// +--------------------------------------------------------------+
// |         __rem_pio2_large __rem_pio2f and __rem_pio2          |
// +--------------------------------------------------------------+

static const int jkInitValues[] = {3, 4, 4, 6}; // initial value for jk

/*
	Table of constants for 2/pi, 396 Hex digits (476 decimal) of 2/pi
	             integer array, contains the (24*i)-th to (24*i+23)-th
	             bit of 2/pi after binary point. The corresponding
	             floating value is
	                     ipio2[i] * 2^(-24(i+1)).
	NB: This table must have at least (e0-3)/24 + jk terms.
	    For quad precision (e0 <= 16360, jk = 6), this is 686.
*/
static const int32_t ipio2[] =
{
	0xA2F983, 0x6E4E44, 0x1529FC, 0x2757D1, 0xF534DD, 0xC0DB62,
	0x95993C, 0x439041, 0xFE5163, 0xABDEBB, 0xC561B7, 0x246E3A,
	0x424DD2, 0xE00649, 0x2EEA09, 0xD1921C, 0xFE1DEB, 0x1CB129,
	0xA73EE8, 0x8235F5, 0x2EBB44, 0x84E99C, 0x7026B4, 0x5F7E41,
	0x3991D6, 0x398353, 0x39F49C, 0x845F8B, 0xBDF928, 0x3B1FF8,
	0x97FFDE, 0x05980F, 0xEF2F11, 0x8B5A0A, 0x6D1F6D, 0x367ECF,
	0x27CB09, 0xB74F46, 0x3F669E, 0x5FEA2D, 0x7527BA, 0xC7EBE5,
	0xF17B3D, 0x0739F7, 0x8A5292, 0xEA6BFB, 0x5FB11F, 0x8D5D08,
	0x560330, 0x46FC7B, 0x6BABF0, 0xCFBC20, 0x9AF436, 0x1DA9E3,
	0x91615E, 0xE61B08, 0x659985, 0x5F14A0, 0x68408D, 0xFFD880,
	0x4D7327, 0x310606, 0x1556CA, 0x73A8C9, 0x60E27B, 0xC08C6B,
	#if LDBL_MAX_EXP > 1024
	0x47C419, 0xC367CD, 0xDCE809, 0x2A8359, 0xC4768B, 0x961CA6,
	0xDDAF44, 0xD15719, 0x053EA5, 0xFF0705, 0x3F7E33, 0xE832C2,
	0xDE4F98, 0x327DBB, 0xC33D26, 0xEF6B1E, 0x5EF89F, 0x3A1F35,
	0xCAF27F, 0x1D87F1, 0x21907C, 0x7C246A, 0xFA6ED5, 0x772D30,
	0x433B15, 0xC614B5, 0x9D19C3, 0xC2C4AD, 0x414D2C, 0x5D000C,
	0x467D86, 0x2D71E3, 0x9AC69B, 0x006233, 0x7CD2B4, 0x97A7B4,
	0xD55537, 0xF63ED7, 0x1810A3, 0xFC764D, 0x2A9D64, 0xABD770,
	0xF87C63, 0x57B07A, 0xE71517, 0x5649C0, 0xD9D63B, 0x3884A7,
	0xCB2324, 0x778AD6, 0x23545A, 0xB91F00, 0x1B0AF1, 0xDFCE19,
	0xFF319F, 0x6A1E66, 0x615799, 0x47FBAC, 0xD87F7E, 0xB76522,
	0x89E832, 0x60BFE6, 0xCDC4EF, 0x09366C, 0xD43F5D, 0xD7DE16,
	0xDE3B58, 0x929BDE, 0x2822D2, 0xE88628, 0x4D58E2, 0x32CAC6,
	0x16E308, 0xCB7DE0, 0x50C017, 0xA71DF3, 0x5BE018, 0x34132E,
	0x621283, 0x014883, 0x5B8EF5, 0x7FB0AD, 0xF2E91E, 0x434A48,
	0xD36710, 0xD8DDAA, 0x425FAE, 0xCE616A, 0xA4280A, 0xB499D3,
	0xF2A606, 0x7F775C, 0x83C2A3, 0x883C61, 0x78738A, 0x5A8CAF,
	0xBDD76F, 0x63A62D, 0xCBBFF4, 0xEF818D, 0x67C126, 0x45CA55,
	0x36D9CA, 0xD2A828, 0x8D61C2, 0x77C912, 0x142604, 0x9B4612,
	0xC459C4, 0x44C5C8, 0x91B24D, 0xF31700, 0xAD43D4, 0xE54929,
	0x10D5FD, 0xFCBE00, 0xCC941E, 0xEECE70, 0xF53E13, 0x80F1EC,
	0xC3E7B3, 0x28F8C7, 0x940593, 0x3E71C1, 0xB3092E, 0xF3450B,
	0x9C1288, 0x7B20AB, 0x9FB52E, 0xC29247, 0x2F327B, 0x6D550C,
	0x90A772, 0x1FE76B, 0x96CB31, 0x4A1679, 0xE27941, 0x89DFF4,
	0x9794E8, 0x84E6E2, 0x973199, 0x6BED88, 0x365F5F, 0x0EFDBB,
	0xB49A48, 0x6CA467, 0x427271, 0x325D8D, 0xB8159F, 0x09E5BC,
	0x25318D, 0x3974F7, 0x1C0530, 0x010C0D, 0x68084B, 0x58EE2C,
	0x90AA47, 0x02E774, 0x24D6BD, 0xA67DF7, 0x72486E, 0xEF169F,
	0xA6948E, 0xF691B4, 0x5153D1, 0xF20ACF, 0x339820, 0x7E4BF5,
	0x6863B2, 0x5F3EDD, 0x035D40, 0x7F8985, 0x295255, 0xC06437,
	0x10D86D, 0x324832, 0x754C5B, 0xD4714E, 0x6E5445, 0xC1090B,
	0x69F52A, 0xD56614, 0x9D0727, 0x50045D, 0xDB3BB4, 0xC576EA,
	0x17F987, 0x7D6B49, 0xBA271D, 0x296996, 0xACCCC6, 0x5414AD,
	0x6AE290, 0x89D988, 0x50722C, 0xBEA404, 0x940777, 0x7030F3,
	0x27FC00, 0xA871EA, 0x49C266, 0x3DE064, 0x83DD97, 0x973FA3,
	0xFD9443, 0x8C860D, 0xDE4131, 0x9D3992, 0x8C70DD, 0xE7B717,
	0x3BDF08, 0x2B3715, 0xA0805C, 0x93805A, 0x921110, 0xD8E80F,
	0xAF806C, 0x4BFFDB, 0x0F9038, 0x761859, 0x15A562, 0xBBCB61,
	0xB989C7, 0xBD4010, 0x04F2D2, 0x277549, 0xF6B6EB, 0xBB22DB,
	0xAA140A, 0x2F2689, 0x768364, 0x333B09, 0x1A940E, 0xAA3A51,
	0xC2A31D, 0xAEEDAF, 0x12265C, 0x4DC26D, 0x9C7A2D, 0x9756C0,
	0x833F03, 0xF6F009, 0x8C402B, 0x99316D, 0x07B439, 0x15200C,
	0x5BC3D8, 0xC492F5, 0x4BADC6, 0xA5CA4E, 0xCD37A7, 0x36A9E6,
	0x9492AB, 0x6842DD, 0xDE6319, 0xEF8C76, 0x528B68, 0x37DBFC,
	0xABA1AE, 0x3115DF, 0xA1AE00, 0xDAFB0C, 0x664D64, 0xB705ED,
	0x306529, 0xBF5657, 0x3AFF47, 0xB9F96A, 0xF3BE75, 0xDF9328,
	0x3080AB, 0xF68C66, 0x15CB04, 0x0622FA, 0x1DE4D9, 0xA4B33D,
	0x8F1B57, 0x09CD36, 0xE9424E, 0xA4BE13, 0xB52333, 0x1AAAF0,
	0xA8654F, 0xA5C1D2, 0x0F3F0B, 0xCD785B, 0x76F923, 0x048B7B,
	0x721789, 0x53A6C6, 0xE26E6F, 0x00EBEF, 0x584A9B, 0xB7DAC4,
	0xBA66AA, 0xCFCF76, 0x1D02D1, 0x2DF1B1, 0xC1998C, 0x77ADC3,
	0xDA4886, 0xA05DF7, 0xF480C6, 0x2FF0AC, 0x9AECDD, 0xBC5C3F,
	0x6DDED0, 0x1FC790, 0xB6DB2A, 0x3A25A3, 0x9AAF00, 0x9353AD,
	0x0457B6, 0xB42D29, 0x7E804B, 0xA707DA, 0x0EAA76, 0xA1597B,
	0x2A1216, 0x2DB7DC, 0xFDE5FA, 0xFEDB89, 0xFDBE89, 0x6C76E4,
	0xFCA906, 0x70803E, 0x156E85, 0xFF87FD, 0x073E28, 0x336761,
	0x86182A, 0xEABD4D, 0xAFE7B3, 0x6E6D8F, 0x396795, 0x5BBF31,
	0x48D784, 0x16DF30, 0x432DC7, 0x356125, 0xCE70C9, 0xB8CB30,
	0xFD6CBF, 0xA200A4, 0xE46C05, 0xA0DD5A, 0x476F21, 0xD21262,
	0x845CB9, 0x496170, 0xE0566B, 0x015299, 0x375550, 0xB7D51E,
	0xC4F133, 0x5F6E13, 0xE4305D, 0xA92E85, 0xC3B21D, 0x3632A1,
	0xA4B708, 0xD4B1EA, 0x21F716, 0xE4698F, 0x77FF27, 0x80030C,
	0x2D408D, 0xA0CD4F, 0x99A520, 0xD3A2B3, 0x0A5D2F, 0x42F9B4,
	0xCBDA11, 0xD0BE7D, 0xC1DB9B, 0xBD17AB, 0x81A2CA, 0x5C6A08,
	0x17552E, 0x550027, 0xF0147F, 0x8607E1, 0x640B14, 0x8D4196,
	0xDEBE87, 0x2AFDDA, 0xB6256B, 0x34897B, 0xFEF305, 0x9EBFB9,
	0x4F6A68, 0xA82A4A, 0x5AC44F, 0xBCF82D, 0x985AD7, 0x95C7F4,
	0x8D4D0D, 0xA63A20, 0x5F57A4, 0xB13F14, 0x953880, 0x0120CC,
	0x86DD71, 0xB6DEC9, 0xF560BF, 0x11654D, 0x6B0701, 0xACB08C,
	0xD0C0B2, 0x485551, 0x0EFB1E, 0xC37295, 0x3B06A3, 0x3540C0,
	0x7BDC06, 0xCC45E0, 0xFA294E, 0xC8CAD6, 0x41F3E8, 0xDE647C,
	0xD8649B, 0x31BED9, 0xC397A4, 0xD45877, 0xC5E369, 0x13DAF0,
	0x3C3ABA, 0x461846, 0x5F7555, 0xF5BDD2, 0xC6926E, 0x5D2EAC,
	0xED440E, 0x423E1C, 0x87C461, 0xE9FD29, 0xF3D6E7, 0xCA7C22,
	0x35916F, 0xC5E008, 0x8DD7FF, 0xE26A6E, 0xC6FDB0, 0xC10893,
	0x745D7C, 0xB2AD6B, 0x9D6ECD, 0x7B723E, 0x6A11C6, 0xA9CFF7,
	0xDF7329, 0xBAC9B5, 0x5100B7, 0x0DB2E2, 0x24BA74, 0x607DE5,
	0x8AD874, 0x2C150D, 0x0C1881, 0x94667E, 0x162901, 0x767A9F,
	0xBEFDFD, 0xEF4556, 0x367ED9, 0x13D9EC, 0xB9BA8B, 0xFC97C4,
	0x27A831, 0xC36EF1, 0x36C594, 0x56A8D8, 0xB5A8B4, 0x0ECCCF,
	0x2D8912, 0x34576F, 0x89562C, 0xE3CE99, 0xB920D6, 0xAA5E6B,
	0x9C2A3E, 0xCC5F11, 0x4A0BFD, 0xFBF4E1, 0x6D3B8E, 0x2C86E2,
	0x84D4E9, 0xA9B4FC, 0xD1EEEF, 0xC9352E, 0x61392F, 0x442138,
	0xC8D91B, 0x0AFC81, 0x6A4AFB, 0xD81C2F, 0x84B453, 0x8C994E,
	0xCC2254, 0xDC552A, 0xD6C6C0, 0x96190B, 0xB8701A, 0x649569,
	0x605A26, 0xEE523F, 0x0F117F, 0x11B5F4, 0xF5CBFC, 0x2DBC34,
	0xEEBC34, 0xCC5DE8, 0x605EDD, 0x9B8E67, 0xEF3392, 0xB817C9,
	0x9B5861, 0xBC57E1, 0xC68351, 0x103ED8, 0x4871DD, 0xDD1C2D,
	0xA118AF, 0x462C21, 0xD7F359, 0x987AD9, 0xC0549E, 0xFA864F,
	0xFC0656, 0xAE79E5, 0x362289, 0x22AD38, 0xDC9367, 0xAAE855,
	0x382682, 0x9BE7CA, 0xA40D51, 0xB13399, 0x0ED7A9, 0x480569,
	0xF0B265, 0xA7887F, 0x974C88, 0x36D1F9, 0xB39221, 0x4A827B,
	0x21CF98, 0xDC9F40, 0x5547DC, 0x3A74E1, 0x42EB67, 0xDF9DFE,
	0x5FD45E, 0xA4677B, 0x7AACBA, 0xA2F655, 0x23882B, 0x55BA41,
	0x086E59, 0x862A21, 0x834739, 0xE6E389, 0xD49EE5, 0x40FB49,
	0xE956FF, 0xCA0F1C, 0x8A59C5, 0x2BFA94, 0xC5C1D3, 0xCFC50F,
	0xAE5ADB, 0x86C547, 0x624385, 0x3B8621, 0x94792C, 0x876110,
	0x7B4C2A, 0x1A2C80, 0x12BF43, 0x902688, 0x893C78, 0xE4C4A8,
	0x7BDBE5, 0xC23AC4, 0xEAF426, 0x8A67F7, 0xBF920D, 0x2BA365,
	0xB1933D, 0x0B7CBD, 0xDC51A4, 0x63DD27, 0xDDE169, 0x19949A,
	0x9529A8, 0x28CE68, 0xB4ED09, 0x209F44, 0xCA984E, 0x638270,
	0x237C7E, 0x32B90F, 0x8EF5A7, 0xE75614, 0x08F121, 0x2A9DB5,
	0x4D7E6F, 0x5119A5, 0xABF9B5, 0xD6DF82, 0x61DD96, 0x023616,
	0x9F3AC4, 0xA1A283, 0x6DED72, 0x7A8D39, 0xA9B882, 0x5C326B,
	0x5B2746, 0xED3400, 0x7700D2, 0x55F4FC, 0x4D5901, 0x8071E0,
	#endif
};

static const double PIo2[] = {
  1.57079625129699707031e+00, // 0x3FF921FB, 0x40000000
  7.54978941586159635335e-08, // 0x3E74442D, 0x00000000
  5.39030252995776476554e-15, // 0x3CF84698, 0x80000000
  3.28200341580791294123e-22, // 0x3B78CC51, 0x60000000
  1.27065575308067607349e-29, // 0x39F01B83, 0x80000000
  1.22933308981111328932e-36, // 0x387A2520, 0x40000000
  2.73370053816464559624e-44, // 0x36E38222, 0x80000000
  2.16741683877804819444e-51, // 0x3569F31D, 0x00000000
};

//TODO: Clean up this function!
int __rem_pio2_large(double* x, double* y, int e0, int nx, int prec)
{
	int32_t jz,jx,jv,jp,jk,carry,n,iq[20],i,j,k,m,q0,ih;
	double z,fw,f[20],fq[20],q[20];

	// initialize j
	jk = jkInitValues[prec];
	jp = jk;

	// determine jx,jv,q0, note that 3>q0
	jx = nx-1;
	jv = (e0-3)/24;  if(jv<0) jv=0;
	q0 = e0-24*(jv+1);

	// set up f[0] to f[jx+jk] where f[jx+jk] = ipio2[jv+jk]
	j = jv-jx; m = jx+jk;
	for (i=0; i<=m; i++,j++)
		f[i] = j<0 ? 0.0 : (double)ipio2[j];

	// compute q[0],q[1],...q[jk]
	for (i=0; i<=jk; i++) {
		for (j=0,fw=0.0; j<=jx; j++)
			fw += x[j]*f[jx+i-j];
		q[i] = fw;
	}

	jz = jk;
recompute:
	// distill q[] into iq[] reversingly
	for (i=0,j=jz,z=q[jz]; j>0; i++,j--) {
		fw    = (double)(int32_t)(0x1p-24*z);
		iq[i] = (int32_t)(z - 0x1p24*fw);
		z     = q[j-1]+fw;
	}

	// compute n
	z  = scalbn(z,q0);       // actual value of z
	z -= 8.0*floor(z*0.125); // trim off integer >= 8
	n  = (int32_t)z;
	z -= (double)n;
	ih = 0;
	if (q0 > 0) {  // need iq[jz-1] to determine n
		i  = iq[jz-1]>>(24-q0); n += i;
		iq[jz-1] -= i<<(24-q0);
		ih = iq[jz-1]>>(23-q0);
	}
	else if (q0 == 0) ih = iq[jz-1]>>23;
	else if (z >= 0.5) ih = 2;

	if (ih > 0) {  // q > 0.5
		n += 1; carry = 0;
		for (i=0; i<jz; i++) {  // compute 1-q
			j = iq[i];
			if (carry == 0) {
				if (j != 0) {
					carry = 1;
					iq[i] = 0x1000000 - j;
				}
			} else
				iq[i] = 0xffffff - j;
		}
		if (q0 > 0) {  // rare case: chance is 1 in 12
			switch(q0) {
			case 1:
				iq[jz-1] &= 0x7fffff; break;
			case 2:
				iq[jz-1] &= 0x3fffff; break;
			}
		}
		if (ih == 2) {
			z = 1.0 - z;
			if (carry != 0)
				z -= scalbn(1.0,q0);
		}
	}

	// check if recomputation is needed
	if (z == 0.0) {
		j = 0;
		for (i=jz-1; i>=jk; i--) j |= iq[i];
		if (j == 0) {  // need recomputation
			for (k=1; iq[jk-k]==0; k++);  // k = no. of terms needed

			for (i=jz+1; i<=jz+k; i++) {  // add q[jz+1] to q[jz+k]
				f[jx+i] = (double)ipio2[jv+i];
				for (j=0,fw=0.0; j<=jx; j++)
					fw += x[j]*f[jx+i-j];
				q[i] = fw;
			}
			jz += k;
			goto recompute;
		}
	}

	// chop off zero terms
	if (z == 0.0) {
		jz -= 1;
		q0 -= 24;
		while (iq[jz] == 0) {
			jz--;
			q0 -= 24;
		}
	} else { // break z into 24-bit if necessary
		z = scalbn(z,-q0);
		if (z >= 0x1p24) {
			fw = (double)(int32_t)(0x1p-24*z);
			iq[jz] = (int32_t)(z - 0x1p24*fw);
			jz += 1;
			q0 += 24;
			iq[jz] = (int32_t)fw;
		} else
			iq[jz] = (int32_t)z;
	}

	// convert integer "bit" chunk to floating-point value
	fw = scalbn(1.0,q0);
	for (i=jz; i>=0; i--) {
		q[i] = fw*(double)iq[i];
		fw *= 0x1p-24;
	}

	// compute PIo2[0,...,jp]*q[jz,...,0]
	for(i=jz; i>=0; i--) {
		for (fw=0.0,k=0; k<=jp && k<=jz-i; k++)
			fw += PIo2[k]*q[i+k];
		fq[jz-i] = fw;
	}

	// compress fq[] into y[]
	switch(prec) {
	case 0:
		fw = 0.0;
		for (i=jz; i>=0; i--)
			fw += fq[i];
		y[0] = ih==0 ? fw : -fw;
		break;
	case 1:
	case 2:
		fw = 0.0;
		for (i=jz; i>=0; i--)
			fw += fq[i];
		// TODO: drop excess precision here once double_t is used
		fw = (double)fw;
		y[0] = ih==0 ? fw : -fw;
		fw = fq[0]-fw;
		for (i=1; i<=jz; i++)
			fw += fq[i];
		y[1] = ih==0 ? fw : -fw;
		break;
	case 3:  // painful
		for (i=jz; i>0; i--) {
			fw      = fq[i-1]+fq[i];
			fq[i]  += fq[i-1]-fw;
			fq[i-1] = fw;
		}
		for (i=jz; i>1; i--) {
			fw      = fq[i-1]+fq[i];
			fq[i]  += fq[i-1]-fw;
			fq[i-1] = fw;
		}
		for (fw=0.0,i=jz; i>=2; i--)
			fw += fq[i];
		if (ih==0) {
			y[0] =  fq[0]; y[1] =  fq[1]; y[2] =  fw;
		} else {
			y[0] = -fq[0]; y[1] = -fq[1]; y[2] = -fw;
		}
	}
	return n&7;
}

// invpio2:  53 bits of 2/pi
// pio2_1:   first 25 bits of pi/2
// pio2_1d:  first  33 bit of pi/2
// pio2_1t:  pi/2 - pio2_1
// pio2_1dt:  pi/2 - pio2_1d
// pio2_2:   second 33 bit of pi/2
// pio2_2t:  pi/2 - (pio2_1+pio2_2)
// pio2_3:   third  33 bit of pi/2
// pio2_3t:  pi/2 - (pio2_1+pio2_2+pio2_3)
static const double
	tointd15 = 1.5/DBL_EPSILON,
	pio4     = 0x1.921FB6p-1,
	pio4d    = 0x1.921FB54442D18p-1,
	invpio2  = 6.36619772367581382433e-01, // 0x3FE45F30, 0x6DC9C883
	pio2_1   = 1.57079631090164184570e+00, // 0x3FF921FB, 0x50000000
	pio2_1d  = 1.57079632673412561417e+00, // 0x3FF921FB, 0x54400000
	pio2_1t  = 1.58932547735281966916e-08, // 0x3E5110b4, 0x611A6263
	pio2_1dt = 6.07710050650619224932e-11, // 0x3DD0B461, 0x1A626331
	pio2_2   = 6.07710050630396597660e-11, // 0x3DD0B461, 0x1A600000
	pio2_2t  = 2.02226624879595063154e-21, // 0x3BA3198A, 0x2E037073
	pio2_3   = 2.02226624871116645580e-21, // 0x3BA3198A, 0x2E000000
	pio2_3t  = 8.47842766036889956997e-32; // 0x397B839A, 0x252049C1

int __rem_pio2f(float value, double* outPntr)
{
	union { float value; uint32_t integer; } valueUnion = { value };
	double tx[1], ty[1]; //TODO: name these better
	double_t fn; //TODO: Name this better!
	uint32_t ix; //TODO: Name this better!
	int n, sign, e0; //TODO: name these better
	
	ix = (valueUnion.integer & 0x7FFFFFFF);
	// 25+53 bit pi is good enough for medium size 
	if (ix < 0x4dc90fdb) // |value| ~< 2^28*(pi/2), medium size
	{
		// Use a specialized rint() to get fn.
		fn = ((double_t)value * invpio2) + tointd15 - tointd15;
		n  = (int32_t)fn;
		*outPntr = value - (fn * pio2_1) - (fn * pio2_1t);
		// Matters with directed rounding.
		if (predict_false(*outPntr < -pio4))
		{
			n--;
			fn--;
			*outPntr = value - fn*pio2_1 - fn*pio2_1t;
		}
		else if (predict_false(*outPntr > pio4))
		{
			n++;
			fn++;
			*outPntr = value - fn*pio2_1 - fn*pio2_1t;
		}
		return n;
	}
	if (ix >= 0x7f800000) // value is inf or NaN
	{
		*outPntr = (value - value);
		return 0;
	}
	// scale value into [2^23, 2^24-1]
	sign = (valueUnion.integer >> 31);
	e0 = (ix >> 23) - (0x7f + 23);  // e0 = ilogb(|value|)-23, positive
	valueUnion.integer = ix - (e0 << 23);
	tx[0] = valueUnion.value;
	n  =  __rem_pio2_large(tx, ty, e0, 1, 0);
	if (sign)
	{
		*outPntr = -ty[0];
		return -n;
	}
	*outPntr = ty[0];
	return n;
}

// caller must handle the case when reduction is not needed: |value| ~<= pi/4
int __rem_pio2(double value, double *outPntr)
{
	union { double value; uint64_t integer; } valueUnion = { value };
	double_t z, w, t, r, fn; //TODO: Give these better names!
	double tx[3], ty[2]; //TODO: Give these better names!
	uint32_t ix; //TODO: Give these better names!
	int sign, n, ex, ey, i; //TODO: Give these better names!
	
	sign = (valueUnion.integer >> 63);
	ix = ((valueUnion.integer >> 32) & 0x7FFFFFFF);
	if (ix <= 0x400F6A7A) // |value| ~<= 5pi/4
	{
		if ((ix & 0xFFFFF) == 0x921FB) // |value| ~= pi/2 or 2pi/2
		{
			goto medium;  // cancellation -- use medium case
		}
		if (ix <= 0x4002D97C) // |value| ~<= 3pi/4
		{
			if (!sign)
			{
				z = value - pio2_1d; // one round good to 85 bits
				outPntr[0] = z - pio2_1dt;
				outPntr[1] = (z - outPntr[0]) - pio2_1dt;
				return 1;
			}
			else
			{
				z = value + pio2_1d;
				outPntr[0] = z + pio2_1dt;
				outPntr[1] = (z - outPntr[0]) + pio2_1dt;
				return -1;
			}
		}
		else
		{
			if (!sign)
			{
				z = value - 2*pio2_1d;
				outPntr[0] = z - 2*pio2_1dt;
				outPntr[1] = (z - outPntr[0]) - 2*pio2_1dt;
				return 2;
			}
			else
			{
				z = value + 2*pio2_1d;
				outPntr[0] = z + 2*pio2_1dt;
				outPntr[1] = (z - outPntr[0]) + 2*pio2_1dt;
				return -2;
			}
		}
	}
	if (ix <= 0x401C463B) // |value| ~<= 9pi/4
	{
		if (ix <= 0x4015FDBC) // |value| ~<= 7pi/4
		{
			if (ix == 0x4012D97C) // |value| ~= 3pi/2
			{
				goto medium;
			}
			if (!sign)
			{
				z = value - 3*pio2_1d;
				outPntr[0] = z - 3*pio2_1dt;
				outPntr[1] = (z - outPntr[0]) - 3*pio2_1dt;
				return 3;
			}
			else
			{
				z = value + 3*pio2_1d;
				outPntr[0] = z + 3*pio2_1dt;
				outPntr[1] = (z - outPntr[0]) + 3*pio2_1dt;
				return -3;
			}
		}
		else
		{
			if (ix == 0x401921FB) // |value| ~= 4pi/2
			{
				goto medium;
			}
			if (!sign)
			{
				z = value - 4*pio2_1d;
				outPntr[0] = z - 4*pio2_1dt;
				outPntr[1] = (z - outPntr[0]) - 4*pio2_1dt;
				return 4;
			}
			else
			{
				z = value + 4*pio2_1d;
				outPntr[0] = z + 4*pio2_1dt;
				outPntr[1] = (z - outPntr[0]) + 4*pio2_1dt;
				return -4;
			}
		}
	}
	if (ix < 0x413921FB) // |value| ~< 2^20*(pi/2), medium size
	{
medium:
		// rint(value/(pi/2))
		fn = ((double_t)value * invpio2) + tointd15 - tointd15;
		n = (int32_t)fn;
		r = value - (fn * pio2_1d);
		w = (fn * pio2_1dt); // 1st round, good to 85 bits
		// Matters with directed rounding.
		if (predict_false(r - w < -pio4d))
		{
			n--;
			fn--;
			r = value - fn*pio2_1d;
			w = fn*pio2_1dt;
		}
		else if (predict_false(r - w > pio4d))
		{
			n++;
			fn++;
			r = value - fn*pio2_1d;
			w = fn*pio2_1dt;
		}
		outPntr[0] = r - w;
		valueUnion.value = outPntr[0];
		ey = valueUnion.integer>>52 & 0x7FF;
		ex = ix>>20;
		if (ex - ey > 16) // 2nd round, good to 118 bits
		{
			t = r;
			w = fn*pio2_2;
			r = t - w;
			w = fn*pio2_2t - ((t-r)-w);
			outPntr[0] = r - w;
			valueUnion.value = outPntr[0];
			ey = valueUnion.integer>>52 & 0x7FF;
			if (ex - ey > 49) // 3rd round, good to 151 bits, covers all cases
			{
				t = r;
				w = fn*pio2_3;
				r = t - w;
				w = fn*pio2_3t - ((t-r)-w);
				outPntr[0] = r - w;
			}
		}
		outPntr[1] = (r - outPntr[0]) - w;
		return n;
	}
	
	// all other (large) arguments
	if (ix >= 0x7FF00000) // value is inf or NaN
	{
		outPntr[0] = outPntr[1] = (value - value);
		return 0;
	}
	// set z = scalbn(|value|,-ilogb(value)+23)
	valueUnion.value = value;
	valueUnion.integer &= ((uint64_t)-1 >> 12);
	valueUnion.integer |= (uint64_t)(0x3FF + 23)<<52;
	z = valueUnion.value;
	for (i=0; i < 2; i++)
	{
		tx[i] = (double)(int32_t)z;
		z     = (z-tx[i])*0x1p24;
	}
	tx[i] = z;
	// skip zero terms, first term is non-zero
	while (tx[i] == 0.0) { i--; }
	n = __rem_pio2_large(tx, ty, (int)(ix >> 20) - (0x3FF + 23), i+1, 1);
	if (sign)
	{
		outPntr[0] = -ty[0];
		outPntr[1] = -ty[1];
		return -n;
	}
	outPntr[0] = ty[0];
	outPntr[1] = ty[1];
	return n;
}

// +--------------------------------------------------------------+
// |                arc trig helpers and constants                |
// +--------------------------------------------------------------+

static const double pio2 = 1.570796326794896558e+00;

static const float
	pi     = 3.1415927410e+00, // 0x40490fdb
	pi_lo  = -8.7422776573e-08, // 0xb3bbbd2e
	pio2_hi = 1.5707962513e+00, // 0x3fc90fda
	pio2_lo = 7.5497894159e-08; // 0x33a22168
static const float // coefficients for asinf_helper(x^2)
	pS0 =  1.6666586697e-01,
	pS1 = -4.2743422091e-02,
	pS2 = -8.6563630030e-03,
	qS1 = -7.0662963390e-01;
static const double
	pio2d_hi = 1.57079632679489655800e+00, // 0x3FF921FB, 0x54442D18
	pio2d_lo = 6.12323399573676603587e-17; // 0x3C91A626, 0x33145C07
static const double // coefficients for asin_helper(x^2)
	pS0d =  1.66666666666666657415e-01, // 0x3FC55555, 0x55555555
	pS1d = -3.25565818622400915405e-01, // 0xBFD4D612, 0x03EB6F7D
	pS2d =  2.01212532134862925881e-01, // 0x3FC9C155, 0x0E884455
	pS3d = -4.00555345006794114027e-02, // 0xBFA48228, 0xB5688F3B
	pS4d =  7.91534994289814532176e-04, // 0x3F49EFE0, 0x7501B288
	pS5d =  3.47933107596021167570e-05, // 0x3F023DE1, 0x0DFDF709
	qS1d = -2.40339491173441421878e+00, // 0xC0033A27, 0x1C8A2D4B
	qS2d =  2.02094576023350569471e+00, // 0x40002AE5, 0x9C598AC8
	qS3d = -6.88283971605453293030e-01, // 0xBFE6066C, 0x1B8D0159
	qS4d =  7.70381505559019352791e-02; // 0x3FB3B8C5, 0xB12E9282

static const float atanhi[] = {
	4.6364760399e-01, // atan(0.5)hi 0x3eed6338
	7.8539812565e-01, // atan(1.0)hi 0x3f490fda
	9.8279368877e-01, // atan(1.5)hi 0x3f7b985e
	1.5707962513e+00, // atan(inf)hi 0x3fc90fda
};

static const float atanlo[] = {
	5.0121582440e-09, // atan(0.5)lo 0x31ac3769
	3.7748947079e-08, // atan(1.0)lo 0x33222168
	3.4473217170e-08, // atan(1.5)lo 0x33140fb4
	7.5497894159e-08, // atan(inf)lo 0x33a22168
};

static const float aT[] = {
	 3.3333328366e-01,
	-1.9999158382e-01,
	 1.4253635705e-01,
	-1.0648017377e-01,
	 6.1687607318e-02,
};

static const double atanhid[] = {
  4.63647609000806093515e-01, /* atan(0.5)hi 0x3FDDAC67, 0x0561BB4F */
  7.85398163397448278999e-01, /* atan(1.0)hi 0x3FE921FB, 0x54442D18 */
  9.82793723247329054082e-01, /* atan(1.5)hi 0x3FEF730B, 0xD281F69B */
  1.57079632679489655800e+00, /* atan(inf)hi 0x3FF921FB, 0x54442D18 */
};

static const double atanlod[] = {
  2.26987774529616870924e-17, /* atan(0.5)lo 0x3C7A2B7F, 0x222F65E2 */
  3.06161699786838301793e-17, /* atan(1.0)lo 0x3C81A626, 0x33145C07 */
  1.39033110312309984516e-17, /* atan(1.5)lo 0x3C700788, 0x7AF0CBBD */
  6.12323399573676603587e-17, /* atan(inf)lo 0x3C91A626, 0x33145C07 */
};

static const double aTd[] = {
  3.33333333333329318027e-01, /* 0x3FD55555, 0x5555550D */
 -1.99999999998764832476e-01, /* 0xBFC99999, 0x9998EBC4 */
  1.42857142725034663711e-01, /* 0x3FC24924, 0x920083FF */
 -1.11111104054623557880e-01, /* 0xBFBC71C6, 0xFE231671 */
  9.09088713343650656196e-02, /* 0x3FB745CD, 0xC54C206E */
 -7.69187620504482999495e-02, /* 0xBFB3B0F2, 0xAF749A6D */
  6.66107313738753120669e-02, /* 0x3FB10D66, 0xA0D03D51 */
 -5.83357013379057348645e-02, /* 0xBFADDE2D, 0x52DEFD9A */
  4.97687799461593236017e-02, /* 0x3FA97B4B, 0x24760DEB */
 -3.65315727442169155270e-02, /* 0xBFA2B444, 0x2C6A6C2F */
  1.62858201153657823623e-02, /* 0x3F90AD3A, 0xE322DA11 */
};

static float asinf_helper(float value)
{
	float_t pVar, qVar;
	pVar = value * (pS0 + (value * (pS1 + (value * pS2))));
	qVar = 1.0f + (value * qS1);
	return (pVar / qVar);
}

static double asin_helper(double value)
{
	double_t p, q;
	p = value * (pS0d + value * (pS1d + value * (pS2d + value * (pS3d + value * (pS4d + value * pS5d)))));
	q = 1.0 + value * (qS1d + value * (qS2d + value * (qS3d + value * qS4d)));
	return (p / q);
}

static float acosf_helper(float value)
{
	float_t numer, denom;
	numer = value * (pS0 + value * (pS1 + value * pS2));
	denom = 1.0f + (value * qS1);
	return (numer / denom);
}

static double acos_helper(double value)
{
	double_t numer, denom;
	numer = value * (pS0d + value * (pS1d + value * (pS2d + value * (pS3d + value * (pS4d + value * pS5d)))));
	denom = 1.0 + value * (qS1d + value * (qS2d + value * (qS3d + value * qS4d)));
	return (numer / denom);
}

// +--------------------------------------------------------------+
// |                         Pow Helpers                          |
// +--------------------------------------------------------------+
// Returns 0 if not int, 1 if odd int, 2 if even int.  The argument is
// the bit representation of a non-zero finite floating-point value.
int checkint32(uint32_t floatInt)
{
	int exponent = ((floatInt >> 23) & 0xFF);
	if (exponent < 0x7F)                                { return 0; }
	if (exponent > 0x7F + 23)                           { return 2; }
	if (floatInt & ((1 << (0x7F + 23 - exponent)) - 1)) { return 0; }
	if (floatInt & (1 << (0x7F + 23 - exponent)))       { return 1; }
	return 2;
}
int checkint64(uint64_t iy)
{
	int e = iy >> 52 & 0x7FF;
	if (e < 0x3FF) { return 0; }
	if (e > 0x3FF + 52) { return 2; }
	if (iy & ((1ULL << (0x3FF + 52 - e)) - 1)) { return 0; }
	if (iy & (1ULL << (0x3FF + 52 - e))) { return 1; }
	return 2;
}

// Returns 1 if input is the bit representation of 0, infinity or nan.
int zeroinfnan32(uint32_t floatInt)
{
	return ((2 * floatInt) - 1) >= ((2u * 0x7F800000) - 1);
}
int zeroinfnan64(uint64_t i)
{
	return ((2 * i) - 1) >= ((2 * asuint64(INFINITY)) - 1);
}

float __math_xflowf(uint32_t sign, float value)
{
	return eval_as_float(fp_barrierf(sign ? -value : value) * value);
}
double __math_xflow(uint32_t sign, double y)
{
	return eval_as_double(fp_barrier(sign ? -y : y) * y);
}
float __math_oflowf(uint32_t sign)
{
	return __math_xflowf(sign, 0x1p97F);
}
double __math_oflow(uint32_t sign)
{
	return __math_xflow(sign, 0x1p769);
}
float __math_uflowf(uint32_t sign)
{
	return __math_xflowf(sign, 0x1p-95F);
}
double __math_uflow(uint32_t sign)
{
	return __math_xflow(sign, 0x1p-767);
}

// +--------------------------------------------------------------+
// |                   sqrt and cbrt Constants                    |
// +--------------------------------------------------------------+
static inline uint32_t MultiplyU32Overflow(uint32_t left, uint32_t right)
{
	return ((uint64_t)left * right) >> 32;
}
// returns a*b*2^-64 - e, with error 0 <= e < 3.
static inline uint64_t MultiplyU64Overflow(uint64_t a, uint64_t b)
{
	uint64_t ahi = (a >> 32);
	uint64_t alo = (a & 0xFFFFFFFF);
	uint64_t bhi = (b >> 32);
	uint64_t blo = (b & 0xFFFFFFFF);
	return (ahi * bhi) + ((ahi * blo) >> 32) + ((alo * bhi) >> 32);
}

const uint16_t __rsqrt_table[128] = {
	0xb451,0xb2f0,0xb196,0xb044,0xaef9,0xadb6,0xac79,0xab43,
	0xaa14,0xa8eb,0xa7c8,0xa6aa,0xa592,0xa480,0xa373,0xa26b,
	0xa168,0xa06a,0x9f70,0x9e7b,0x9d8a,0x9c9d,0x9bb5,0x9ad1,
	0x99f0,0x9913,0x983a,0x9765,0x9693,0x95c4,0x94f8,0x9430,
	0x936b,0x92a9,0x91ea,0x912e,0x9075,0x8fbe,0x8f0a,0x8e59,
	0x8daa,0x8cfe,0x8c54,0x8bac,0x8b07,0x8a64,0x89c4,0x8925,
	0x8889,0x87ee,0x8756,0x86c0,0x862b,0x8599,0x8508,0x8479,
	0x83ec,0x8361,0x82d8,0x8250,0x81c9,0x8145,0x80c2,0x8040,
	0xff02,0xfd0e,0xfb25,0xf947,0xf773,0xf5aa,0xf3ea,0xf234,
	0xf087,0xeee3,0xed47,0xebb3,0xea27,0xe8a3,0xe727,0xe5b2,
	0xe443,0xe2dc,0xe17a,0xe020,0xdecb,0xdd7d,0xdc34,0xdaf1,
	0xd9b3,0xd87b,0xd748,0xd61a,0xd4f1,0xd3cd,0xd2ad,0xd192,
	0xd07b,0xcf69,0xce5b,0xcd51,0xcc4a,0xcb48,0xca4a,0xc94f,
	0xc858,0xc764,0xc674,0xc587,0xc49d,0xc3b7,0xc2d4,0xc1f4,
	0xc116,0xc03c,0xbf65,0xbe90,0xbdbe,0xbcef,0xbc23,0xbb59,
	0xba91,0xb9cc,0xb90a,0xb84a,0xb78c,0xb6d0,0xb617,0xb560,
};

static const unsigned
	B1 = 709958130, // B1 = (127-127.0/3-0.03306235651)*2**23
	B2 = 642849266; // B2 = (127-127.0/3-24/3-0.03306235651)*2**23
static const uint32_t
	Bd1 = 715094163, // B1 = (1023-1023/3-0.03306235651)*2**20
	Bd2 = 696219795; // B2 = (1023-1023/3-54/3-0.03306235651)*2**20
// |1/cbrt(x) - p(x)| < 2**-23.5 (~[-7.93e-8, 7.929e-8]).
static const double
	P0 =  1.87595182427177009643,  // 0x3ffe03e6, 0x0f61e692
	P1 = -1.88497979543377169875,  // 0xbffe28e0, 0x92f02420
	P2 =  1.621429720105354466140, // 0x3ff9f160, 0x4a49d6c2
	P3 = -0.758397934778766047437, // 0xbfe844cb, 0xbee751d9
	P4 =  0.145996192886612446982; // 0x3fc2b000, 0xd4e4edd7

// +--------------------------------------------------------------+
// |                    exp and expf Constants                    |
// +--------------------------------------------------------------+
#define EXP_TABLE_BITS 7
#define EXP_POLY_ORDER 5
#define EXP2_POLY_ORDER 5
#define exp_N (1 << EXP_TABLE_BITS)

const struct exp_data
{
	double invln2N;
	double shift;
	double negln2hiN;
	double negln2loN;
	double poly[4]; /* Last four coefficients.  */
	double exp2_shift;
	double exp2_poly[EXP2_POLY_ORDER];
	uint64_t tab[2*(1 << EXP_TABLE_BITS)];
} __exp_data =
{
	// N/ln2
	.invln2N = 0x1.71547652B82FEp0 * exp_N,
	// -ln2/N
	.negln2hiN = -0x1.62E42FEFA0000p-8,
	.negln2loN = -0x1.CF79ABC9E3B3Ap-47,
	// Used for rounding
	.shift = 0x1.8p52,
	// exp polynomial coefficients.
	.poly =
	{
		// abs error: 1.555*2^-66
		// ulp error: 0.509 (0.511 without fma)
		// if |x| < ln2/256+eps
		// abs error if |x| < ln2/256+0x1p-15: 1.09*2^-65
		// abs error if |x| < ln2/128: 1.7145*2^-56
		0x1.FFFFFFFFFFDBDp-2,
		0x1.555555555543Cp-3,
		0x1.55555CF172B91p-5,
		0x1.1111167A4D017p-7,
	},
	.exp2_shift = 0x1.8p52 / exp_N,
	// exp2 polynomial coefficients.
	.exp2_poly =
	{
		// abs error: 1.2195*2^-65
		// ulp error: 0.507 (0.511 without fma)
		// if |x| < 1/256
		// abs error if |x| < 1/128: 1.9941*2^-56
		0x1.62E42FEFA39EFp-1,
		0x1.EBFBDFF82C424p-3,
		0x1.C6B08D70CF4B5p-5,
		0x1.3B2ABD24650CCp-7,
		0x1.5D7E09B4E3A84p-10,
	},
	// 2^(k/N) ~= H[k]*(1 + T[k]) for int k in [0,N)
	// tab[2*k] = asuint64(T[k])
	// tab[2*k+1] = asuint64(H[k]) - (k << 52)/N
	.tab =
	{
		0x0, 0x3FF0000000000000,
		0x3C9B3B4F1A88BF6E, 0x3FEFF63DA9FB3335,
		0xBC7160139CD8DC5D, 0x3FEFEC9A3E778061,
		0xBC905E7A108766D1, 0x3FEFE315E86E7F85,
		0x3C8CD2523567F613, 0x3FEFD9B0D3158574,
		0xBC8BCE8023F98EFA, 0x3FEFD06B29DDF6DE,
		0x3C60F74E61E6C861, 0x3FEFC74518759BC8,
		0x3C90A3E45B33D399, 0x3FEFBE3ECAC6F383,
		0x3C979AA65D837B6D, 0x3FEFB5586CF9890F,
		0x3C8EB51A92FDEFFC, 0x3FEFAC922B7247F7,
		0x3C3EBE3D702F9CD1, 0x3FEFA3EC32D3D1A2,
		0xBC6A033489906E0B, 0x3FEF9B66AFFED31B,
		0xBC9556522A2FBD0E, 0x3FEF9301D0125B51,
		0xBC5080EF8C4EEA55, 0x3FEF8ABDC06C31CC,
		0xBC91C923B9D5F416, 0x3FEF829AAEA92DE0,
		0x3C80D3E3E95C55AF, 0x3FEF7A98C8A58E51,
		0xBC801B15EAA59348, 0x3FEF72B83C7D517B,
		0xBC8F1FF055DE323D, 0x3FEF6AF9388C8DEA,
		0x3C8B898C3F1353BF, 0x3FEF635BEB6FCB75,
		0xBC96D99C7611EB26, 0x3FEF5BE084045CD4,
		0x3C9AECF73E3A2F60, 0x3FEF54873168B9AA,
		0xBC8FE782CB86389D, 0x3FEF4D5022FCD91D,
		0x3C8A6F4144A6C38D, 0x3FEF463B88628CD6,
		0x3C807A05B0E4047D, 0x3FEF3F49917DDC96,
		0x3C968EFDE3A8A894, 0x3FEF387A6E756238,
		0x3C875E18F274487D, 0x3FEF31CE4FB2A63F,
		0x3C80472B981FE7F2, 0x3FEF2B4565E27CDD,
		0xBC96B87B3F71085E, 0x3FEF24DFE1F56381,
		0x3C82F7E16D09AB31, 0x3FEF1E9DF51FDEE1,
		0xBC3D219B1A6FBFFA, 0x3FEF187FD0DAD990,
		0x3C8B3782720C0AB4, 0x3FEF1285A6E4030B,
		0x3C6E149289CECB8F, 0x3FEF0CAFA93E2F56,
		0x3C834D754DB0ABB6, 0x3FEF06FE0A31B715,
		0x3C864201E2AC744C, 0x3FEF0170FC4CD831,
		0x3C8FDD395DD3F84A, 0x3FEEFC08B26416FF,
		0xBC86A3803B8E5B04, 0x3FEEF6C55F929FF1,
		0xBC924AEDCC4B5068, 0x3FEEF1A7373AA9CB,
		0xBC9907F81B512D8E, 0x3FEEECAE6D05D866,
		0xBC71D1E83E9436D2, 0x3FEEE7DB34E59FF7,
		0xBC991919B3CE1B15, 0x3FEEE32DC313A8E5,
		0x3C859F48A72A4C6D, 0x3FEEDEA64C123422,
		0xBC9312607A28698A, 0x3FEEDA4504AC801C,
		0xBC58A78F4817895B, 0x3FEED60A21F72E2A,
		0xBC7C2C9B67499A1B, 0x3FEED1F5D950A897,
		0x3C4363ED60C2AC11, 0x3FEECE086061892D,
		0x3C9666093B0664EF, 0x3FEECA41ED1D0057,
		0x3C6ECCE1DAA10379, 0x3FEEC6A2B5C13CD0,
		0x3C93FF8E3F0F1230, 0x3FEEC32AF0D7D3DE,
		0x3C7690CEBB7AAFB0, 0x3FEEBFDAD5362A27,
		0x3C931DBDEB54E077, 0x3FEEBCB299FDDD0D,
		0xBC8F94340071A38E, 0x3FEEB9B2769D2CA7,
		0xBC87DECCDC93A349, 0x3FEEB6DAA2CF6642,
		0xBC78DEC6BD0F385F, 0x3FEEB42B569D4F82,
		0xBC861246EC7B5CF6, 0x3FEEB1A4CA5D920F,
		0x3C93350518FDD78E, 0x3FEEAF4736B527DA,
		0x3C7B98B72F8A9B05, 0x3FEEAD12D497C7FD,
		0x3C9063E1E21C5409, 0x3FEEAB07DD485429,
		0x3C34C7855019C6EA, 0x3FEEA9268A5946B7,
		0x3C9432E62B64C035, 0x3FEEA76F15AD2148,
		0xBC8CE44A6199769F, 0x3FEEA5E1B976DC09,
		0xBC8C33C53BEF4DA8, 0x3FEEA47EB03A5585,
		0xBC845378892BE9AE, 0x3FEEA34634CCC320,
		0xBC93CEDD78565858, 0x3FEEA23882552225,
		0x3C5710AA807E1964, 0x3FEEA155D44CA973,
		0xBC93B3EFBF5E2228, 0x3FEEA09E667F3BCD,
		0xBC6A12AD8734B982, 0x3FEEA012750BDABF,
		0xBC6367EFB86DA9EE, 0x3FEE9FB23C651A2F,
		0xBC80DC3D54E08851, 0x3FEE9F7DF9519484,
		0xBC781F647E5A3ECF, 0x3FEE9F75E8EC5F74,
		0xBC86EE4AC08B7DB0, 0x3FEE9F9A48A58174,
		0xBC8619321E55E68A, 0x3FEE9FEB564267C9,
		0x3C909CCB5E09D4D3, 0x3FEEA0694FDE5D3F,
		0xBC7B32DCB94DA51D, 0x3FEEA11473EB0187,
		0x3C94ECFD5467C06B, 0x3FEEA1ED0130C132,
		0x3C65EBE1ABD66C55, 0x3FEEA2F336CF4E62,
		0xBC88A1C52FB3CF42, 0x3FEEA427543E1A12,
		0xBC9369B6F13B3734, 0x3FEEA589994CCE13,
		0xBC805E843A19FF1E, 0x3FEEA71A4623C7AD,
		0xBC94D450D872576E, 0x3FEEA8D99B4492ED,
		0x3C90AD675B0E8A00, 0x3FEEAAC7D98A6699,
		0x3C8DB72FC1F0EAB4, 0x3FEEACE5422AA0DB,
		0xBC65B6609CC5E7FF, 0x3FEEAF3216B5448C,
		0x3C7BF68359F35F44, 0x3FEEB1AE99157736,
		0xBC93091FA71E3D83, 0x3FEEB45B0B91FFC6,
		0xBC5DA9B88B6C1E29, 0x3FEEB737B0CDC5E5,
		0xBC6C23F97C90B959, 0x3FEEBA44CBC8520F,
		0xBC92434322F4F9AA, 0x3FEEBD829FDE4E50,
		0xBC85CA6CD7668E4B, 0x3FEEC0F170CA07BA,
		0x3C71AFFC2B91CE27, 0x3FEEC49182A3F090,
		0x3C6DD235E10A73BB, 0x3FEEC86319E32323,
		0xBC87C50422622263, 0x3FEECC667B5DE565,
		0x3C8B1C86E3E231D5, 0x3FEED09BEC4A2D33,
		0xBC91BBD1D3BCBB15, 0x3FEED503B23E255D,
		0x3C90CC319CEE31D2, 0x3FEED99E1330B358,
		0x3C8469846E735AB3, 0x3FEEDE6B5579FDBF,
		0xBC82DFCD978E9DB4, 0x3FEEE36BBFD3F37A,
		0x3C8C1A7792CB3387, 0x3FEEE89F995AD3AD,
		0xBC907B8F4AD1D9FA, 0x3FEEEE07298DB666,
		0xBC55C3D956DCAEBA, 0x3FEEF3A2B84F15FB,
		0xBC90A40E3DA6F640, 0x3FEEF9728DE5593A,
		0xBC68D6F438AD9334, 0x3FEEFF76F2FB5E47,
		0xBC91EEE26B588A35, 0x3FEF05B030A1064A,
		0x3C74FFD70A5FDDCD, 0x3FEF0C1E904BC1D2,
		0xBC91BDFBFA9298AC, 0x3FEF12C25BD71E09,
		0x3C736EAE30AF0CB3, 0x3FEF199BDD85529C,
		0x3C8EE3325C9FFD94, 0x3FEF20AB5FFFD07A,
		0x3C84E08FD10959AC, 0x3FEF27F12E57D14B,
		0x3C63CDAF384E1A67, 0x3FEF2F6D9406E7B5,
		0x3C676B2C6C921968, 0x3FEF3720DCEF9069,
		0xBC808A1883CCB5D2, 0x3FEF3F0B555DC3FA,
		0xBC8FAD5D3FFFFA6F, 0x3FEF472D4A07897C,
		0xBC900DAE3875A949, 0x3FEF4F87080D89F2,
		0x3C74A385A63D07A7, 0x3FEF5818DCFBA487,
		0xBC82919E2040220F, 0x3FEF60E316C98398,
		0x3C8E5A50D5C192AC, 0x3FEF69E603DB3285,
		0x3C843A59AC016B4B, 0x3FEF7321F301B460,
		0xBC82D52107B43E1F, 0x3FEF7C97337B9B5F,
		0xBC892AB93B470DC9, 0x3FEF864614F5A129,
		0x3C74B604603A88D3, 0x3FEF902EE78B3FF6,
		0x3C83C5EC519D7271, 0x3FEF9A51FBC74C83,
		0xBC8FF7128FD391F0, 0x3FEFA4AFA2A490DA,
		0xBC8DAE98E223747D, 0x3FEFAF482D8E67F1,
		0x3C8EC3BC41AA2008, 0x3FEFBA1BEE615A27,
		0x3C842B94C3A9EB32, 0x3FEFC52B376BBA97,
		0x3C8A64A931D185EE, 0x3FEFD0765B6E4540,
		0xBC8E37BAE43BE3ED, 0x3FEFDBFDAD9CBE14,
		0x3C77893B4D91CD9D, 0x3FEFE7C1819E90D8,
		0x3C5305C14160CC89, 0x3FEFF3C22B8F71F1,
	},
};

#define exp_InvLn2N   __exp_data.invln2N
#define exp_NegLn2hiN __exp_data.negln2hiN
#define exp_NegLn2loN __exp_data.negln2loN
#define exp_Shift     __exp_data.shift
#define exp_T         __exp_data.tab
#define exp_C2        __exp_data.poly[5 - EXP_POLY_ORDER]
#define exp_C3        __exp_data.poly[6 - EXP_POLY_ORDER]
#define exp_C4        __exp_data.poly[7 - EXP_POLY_ORDER]
#define exp_C5        __exp_data.poly[8 - EXP_POLY_ORDER]

// Shared between expf, exp2f and powf.
#define EXP2F_TABLE_BITS 5
#define EXP2F_POLY_ORDER 3
#define EXP2F_N (1 << EXP2F_TABLE_BITS)

const struct exp2f_data
{
	uint64_t tab[1 << EXP2F_TABLE_BITS];
	double shift_scaled;
	double poly[EXP2F_POLY_ORDER];
	double shift;
	double invln2_scaled;
	double poly_scaled[EXP2F_POLY_ORDER];
} __exp2f_data =
{
	// tab[i] = uint(2^(i/N)) - (i << 52-BITS)
	// used for computing 2^(k/N) for an int |k| < 150 N as
	// double(tab[k%N] + (k << 52-BITS))
	.tab =
	{
		0x3FF0000000000000, 0x3FEFD9B0D3158574, 0x3FEFB5586CF9890F, 0x3FEF9301D0125B51,
		0x3FEF72B83C7D517B, 0x3FEF54873168B9AA, 0x3FEF387A6E756238, 0x3FEF1E9DF51FDEE1,
		0x3FEF06FE0A31B715, 0x3FEEF1A7373AA9CB, 0x3FEEDEA64C123422, 0x3FEECE086061892D,
		0x3FEEBFDAD5362A27, 0x3FEEB42B569D4F82, 0x3FEEAB07DD485429, 0x3FEEA47EB03A5585,
		0x3FEEA09E667F3BCD, 0x3FEE9F75E8EC5F74, 0x3FEEA11473EB0187, 0x3FEEA589994CCE13,
		0x3FEEACE5422AA0DB, 0x3FEEB737B0CDC5E5, 0x3FEEC49182A3F090, 0x3FEED503B23E255D,
		0x3FEEE89F995AD3AD, 0x3FEEFF76F2FB5E47, 0x3FEF199BDD85529C, 0x3FEF3720DCEF9069,
		0x3FEF5818DCFBA487, 0x3FEF7C97337B9B5F, 0x3FEFA4AFA2A490DA, 0x3FEFD0765B6E4540,
	},
	.shift_scaled = 0x1.8p+52 / EXP2F_N,
	.poly = { 0x1.C6AF84B912394p-5, 0x1.EBFCE50FAC4F3p-3, 0x1.62E42FF0C52D6p-1, },
	.shift = 0x1.8p+52,
	.invln2_scaled = 0x1.71547652B82FEp+0 * EXP2F_N,
	.poly_scaled = { 0x1.C6AF84B912394p-5/EXP2F_N/EXP2F_N/EXP2F_N, 0x1.EBFCE50FAC4F3p-3/EXP2F_N/EXP2F_N, 0x1.62E42FF0C52D6p-1/EXP2F_N, },
};

#define exp2f_InvLn2N __exp2f_data.invln2_scaled
#define exp2f_SHIFT   __exp2f_data.shift
#define exp2f_T       __exp2f_data.tab
#define exp2f_C       __exp2f_data.poly_scaled

// +--------------------------------------------------------------+
// |                     exp Helper Functions                     |
// +--------------------------------------------------------------+
// Handle cases that may overflow or underflow when computing the result that
// is scale*(1+TMP) without intermediate rounding.  The bit representation of
// scale is in SBITS, however it has a computed exponent that may have
// overflown into the sign bit so that needs to be adjusted before using it as
// a double.  (int32_t)KI is the k used in the argument reduction and exponent
// adjustment of scale, positive k here means the result may overflow and
// negative k means the result may underflow.
double exp_specialcase(double_t tmp, uint64_t sbits, uint64_t ki)
{
	double_t scale, y;
	
	if ((ki & 0x80000000) == 0)
	{
		// k > 0, the exponent of scale might have overflowed by <= 460.
		sbits -= 1009ull << 52;
		scale = asdouble(sbits);
		y = 0x1p1009 * (scale + scale * tmp);
		return eval_as_double(y);
	}
	// k < 0, need special care in the subnormal range.
	sbits += 1022ull << 52;
	scale = asdouble(sbits);
	y = scale + scale * tmp;
	if (y < 1.0)
	{
		// Round y to the right precision before scaling it into the subnormal
		// range to avoid double rounding that can cause 0.5+E/2 ulp error where
		// E is the worst-case ulp error outside the subnormal range.  So this
		// is only useful if the goal is better than 1 ulp worst-case error.
		double_t hi, lo;
		lo = scale - y + scale * tmp;
		hi = 1.0 + y;
		lo = 1.0 - hi + y + lo;
		y = eval_as_double(hi + lo) - 1.0;
		// Avoid -0.0 with downward rounding.
		if (y == 0.0) { y = 0.0; }
		// The underflow exception needs to be signaled explicitly.
		fp_force_eval(fp_barrier(0x1p-1022) * 0x1p-1022);
	}
	y = 0x1p-1022 * y;
	return eval_as_double(y);
}

// +--------------------------------------------------------------+
// |                    log and logf constants                    |
// +--------------------------------------------------------------+
#define LOGF_TABLE_BITS 4
#define LOGF_POLY_ORDER 4
const struct logf_data
{
	struct
	{
		double invc, logc;
	} tab[1 << LOGF_TABLE_BITS];
	double ln2;
	double poly[LOGF_POLY_ORDER - 1]; /* First order coefficient is 1.  */
} __logf_data =
{
	.tab =
	{
		{ 0x1.661EC79F8F3BEp+0, -0x1.57BF7808CAADEp-2 },
		{ 0x1.571ED4AAF883Dp+0, -0x1.2BEF0A7C06DDBp-2 },
		{ 0x1.49539F0F010Bp+0,  -0x1.01EAE7F513A67p-2 },
		{ 0x1.3C995B0B80385p+0, -0x1.B31D8A68224E9p-3 },
		{ 0x1.30D190C8864A5p+0, -0x1.6574F0AC07758p-3 },
		{ 0x1.25E227B0B8EAp+0,  -0x1.1AA2BC79C81p-3   },
		{ 0x1.1BB4A4A1A343Fp+0, -0x1.A4E76CE8C0E5Ep-4 },
		{ 0x1.12358F08AE5BAp+0, -0x1.1973C5A611CCCp-4 },
		{ 0x1.0953F419900A7p+0, -0x1.252F438E10C1Ep-5 },
		{ 0x1p+0,               0x0p+0                },
		{ 0x1.E608CFD9A47ACp-1, 0x1.AA5AA5DF25984p-5  },
		{ 0x1.CA4B31F026AAp-1,  0x1.C5E53AA362EB4p-4  },
		{ 0x1.B2036576AFCE6p-1, 0x1.526E57720DB08p-3  },
		{ 0x1.9C2D163A1AA2Dp-1, 0x1.BC2860D22477p-3   },
		{ 0x1.886E6037841EDp-1, 0x1.1058BC8A07EE1p-2  },
		{ 0x1.767DCF5534862p-1, 0x1.4043057B6EE09p-2  },
	},
	.ln2 = 0x1.62E42FEFA39EFp-1,
	.poly = { -0x1.00EA348B88334p-2, 0x1.5575B0BE00B6Ap-2, -0x1.FFFFEF20A4123p-2, }
};

#define logf_T __logf_data.tab
#define logf_A __logf_data.poly
#define logf_Ln2 __logf_data.ln2
#define logf_N (1 << LOGF_TABLE_BITS)
#define logf_OFF 0x3F330000

#define LOG_TABLE_BITS 7
#define LOG_POLY_ORDER 6
#define LOG_POLY1_ORDER 12
const struct log_data
{
	double ln2hi;
	double ln2lo;
	double poly[LOG_POLY_ORDER - 1]; /* First coefficient is 1.  */
	double poly1[LOG_POLY1_ORDER - 1];
	struct
	{
		double invc, logc;
	} tab[1 << LOG_TABLE_BITS];
	struct
	{
		double chi, clo;
	} tab2[1 << LOG_TABLE_BITS];
} __log_data =
{
	.ln2hi = 0x1.62E42FEFA3800p-1,
	.ln2lo = 0x1.EF35793C76730p-45,
	.poly1 =
	{
		// relative error: 0x1.c04d76cp-63
		// in -0x1p-4 0x1.09p-4 (|log(1+x)| > 0x1p-4 outside the interval)
		-0x1p-1,
		0x1.5555555555577p-2,
		-0x1.FFFFFFFFFFDCBp-3,
		0x1.999999995DD0Cp-3,
		-0x1.55555556745A7p-3,
		0x1.24924A344DE3p-3,
		-0x1.FFFFFA4423D65p-4,
		0x1.C7184282AD6CAp-4,
		-0x1.999EB43B068FFp-4,
		0x1.78182F7AFD085p-4,
		-0x1.5521375D145CDp-4,
	},
	.poly =
	{
		// relative error: 0x1.926199e8p-56
		// abs error: 0x1.882ff33p-65
		// in -0x1.fp-9 0x1.fp-9
		-0x1.0000000000001p-1,
		0x1.555555551305Bp-2,
		-0x1.FFFFFFFEB459p-3,
		0x1.999B324F10111p-3,
		-0x1.55575E506C89Fp-3,
	},
	/*
	Algorithm:
			x = 2^k z
			log(x) = k ln2 + log(c) + log(z/c)
			log(z/c) = poly(z/c - 1)
		where z is in [1.6p-1; 1.6p0] which is split into N subintervals and z falls
		into the ith one, then table entries are computed as
			tab[i].invc = 1/c
			tab[i].logc = (double)log(c)
			tab2[i].chi = (double)c
			tab2[i].clo = (double)(c - (double)c)
		where c is near the center of the subinterval and is chosen by trying +-2^29
		floating point invc candidates around 1/center and selecting one for which
			1) the rounding error in 0x1.8p9 + logc is 0,
			2) the rounding error in z - chi - clo is < 0x1p-66 and
			3) the rounding error in (double)log(c) is minimized (< 0x1p-66).
		Note: 1) ensures that k*ln2hi + logc can be computed without rounding error,
		2) ensures that z/c - 1 can be computed as (z - chi - clo)*invc with close to
		a single rounding error when there is no fast fma for z*invc - 1, 3) ensures
		that logc + poly(z/c - 1) has small error, however near x == 1 when
		|log(x)| < 0x1p-4, this is not enough so that is special cased.
	*/
	.tab =
	{
		{ 0x1.734F0C3E0DE9Fp+0, -0x1.7CC7F79E69000p-2 },
		{ 0x1.713786A2CE91Fp+0, -0x1.76FEEC20D0000p-2 },
		{ 0x1.6F26008FAB5A0p+0, -0x1.713E31351E000p-2 },
		{ 0x1.6D1A61F138C7Dp+0, -0x1.6B85B38287800p-2 },
		{ 0x1.6B1490BC5B4D1p+0, -0x1.65D5590807800p-2 },
		{ 0x1.69147332F0CBAp+0, -0x1.602D076180000p-2 },
		{ 0x1.6719F18224223p+0, -0x1.5A8CA86909000p-2 },
		{ 0x1.6524F99A51ED9p+0, -0x1.54F4356035000p-2 },
		{ 0x1.63356AA8F24C4p+0, -0x1.4F637C36B4000p-2 },
		{ 0x1.614B36B9DDC14p+0, -0x1.49DA7FDA85000p-2 },
		{ 0x1.5F66452C65C4Cp+0, -0x1.445923989A800p-2 },
		{ 0x1.5D867B5912C4Fp+0, -0x1.3EDF439B0B800p-2 },
		{ 0x1.5BABCCB5B90DEp+0, -0x1.396CE448F7000p-2 },
		{ 0x1.59D61F2D91A78p+0, -0x1.3401E17BDA000p-2 },
		{ 0x1.5805612465687p+0, -0x1.2E9E2EF468000p-2 },
		{ 0x1.56397CEE76BD3p+0, -0x1.2941B3830E000p-2 },
		{ 0x1.54725E2A77F93p+0, -0x1.23EC58CDA8800p-2 },
		{ 0x1.52AFF42064583p+0, -0x1.1E9E129279000p-2 },
		{ 0x1.50F22DBB2BDDFp+0, -0x1.1956D2B48F800p-2 },
		{ 0x1.4F38F4734DED7p+0, -0x1.141679AB9F800p-2 },
		{ 0x1.4D843CFDE2840p+0, -0x1.0EDD094EF9800p-2 },
		{ 0x1.4BD3EC078A3C8p+0, -0x1.09AA518DB1000p-2 },
		{ 0x1.4A27FC3E0258Ap+0, -0x1.047E65263B800p-2 },
		{ 0x1.4880524D48434p+0, -0x1.FEB224586F000p-3 },
		{ 0x1.46DCE1B192D0Bp+0, -0x1.F474A7517B000p-3 },
		{ 0x1.453D9D3391854p+0, -0x1.EA4443D103000p-3 },
		{ 0x1.43A2744B4845Ap+0, -0x1.E020D44E9B000p-3 },
		{ 0x1.420B54115F8FBp+0, -0x1.D60A22977F000p-3 },
		{ 0x1.40782DA3EF4B1p+0, -0x1.CC00104959000p-3 },
		{ 0x1.3EE8F5D57FE8Fp+0, -0x1.C202956891000p-3 },
		{ 0x1.3D5D9A00B4CE9p+0, -0x1.B81178D811000p-3 },
		{ 0x1.3BD60C010C12Bp+0, -0x1.AE2C9CCD3D000p-3 },
		{ 0x1.3A5242B75DAB8p+0, -0x1.A45402E129000p-3 },
		{ 0x1.38D22CD9FD002p+0, -0x1.9A877681DF000p-3 },
		{ 0x1.3755BC5847A1Cp+0, -0x1.90C6D69483000p-3 },
		{ 0x1.35DCE49AD36E2p+0, -0x1.87120A645C000p-3 },
		{ 0x1.34679984DD440p+0, -0x1.7D68FB4143000p-3 },
		{ 0x1.32F5CCEFFCB24p+0, -0x1.73CB83C627000p-3 },
		{ 0x1.3187775A10D49p+0, -0x1.6A39A9B376000p-3 },
		{ 0x1.301C8373E3990p+0, -0x1.60B3154B7A000p-3 },
		{ 0x1.2EB4EBB95F841p+0, -0x1.5737D76243000p-3 },
		{ 0x1.2D50A0219A9D1p+0, -0x1.4DC7B8FC23000p-3 },
		{ 0x1.2BEF9A8B7FD2Ap+0, -0x1.4462C51D20000p-3 },
		{ 0x1.2A91C7A0C1BABp+0, -0x1.3B08ABC830000p-3 },
		{ 0x1.293726014B530p+0, -0x1.31B996B490000p-3 },
		{ 0x1.27DFA5757A1F5p+0, -0x1.2875490A44000p-3 },
		{ 0x1.268B39B1D3BBFp+0, -0x1.1F3B9F879A000p-3 },
		{ 0x1.2539D838FF5BDp+0, -0x1.160C8252CA000p-3 },
		{ 0x1.23EB7AAC9083Bp+0, -0x1.0CE7F57F72000p-3 },
		{ 0x1.22A012BA940B6p+0, -0x1.03CDC49FEA000p-3 },
		{ 0x1.2157996CC4132p+0, -0x1.F57BDBC4B8000p-4 },
		{ 0x1.201201DD2FC9Bp+0, -0x1.E370896404000p-4 },
		{ 0x1.1ECF4494D480Bp+0, -0x1.D17983EF94000p-4 },
		{ 0x1.1D8F5528F6569p+0, -0x1.BF9674ED8A000p-4 },
		{ 0x1.1C52311577E7Cp+0, -0x1.ADC79202F6000p-4 },
		{ 0x1.1B17C74CB26E9p+0, -0x1.9C0C3E7288000p-4 },
		{ 0x1.19E010C2C1AB6p+0, -0x1.8A646B372C000p-4 },
		{ 0x1.18AB07BB670BDp+0, -0x1.78D01B3AC0000p-4 },
		{ 0x1.1778A25EFBCB6p+0, -0x1.674F145380000p-4 },
		{ 0x1.1648D354C31DAp+0, -0x1.55E0E6D878000p-4 },
		{ 0x1.151B990275FDDp+0, -0x1.4485CDEA1E000p-4 },
		{ 0x1.13F0EA432D24Cp+0, -0x1.333D94D6AA000p-4 },
		{ 0x1.12C8B7210F9DAp+0, -0x1.22079F8C56000p-4 },
		{ 0x1.11A3028ECB531p+0, -0x1.10E4698622000p-4 },
		{ 0x1.107FBDA8434AFp+0, -0x1.FFA6C6AD20000p-5 },
		{ 0x1.0F5EE0F4E6BB3p+0, -0x1.DDA8D4A774000p-5 },
		{ 0x1.0E4065D2A9FCEp+0, -0x1.BBCECE4850000p-5 },
		{ 0x1.0D244632CA521p+0, -0x1.9A1894012C000p-5 },
		{ 0x1.0C0A77CE2981Ap+0, -0x1.788583302C000p-5 },
		{ 0x1.0AF2F83C636D1p+0, -0x1.5715E67D68000p-5 },
		{ 0x1.09DDB98A01339p+0, -0x1.35C8A49658000p-5 },
		{ 0x1.08CABAF52E7DFp+0, -0x1.149E364154000p-5 },
		{ 0x1.07B9F2F4E28FBp+0, -0x1.E72C082EB8000p-6 },
		{ 0x1.06AB58C358F19p+0, -0x1.A55F152528000p-6 },
		{ 0x1.059EEA5ECF92Cp+0, -0x1.63D62CF818000p-6 },
		{ 0x1.04949CDD12C90p+0, -0x1.228FB8CAA0000p-6 },
		{ 0x1.038C6C6F0ADA9p+0, -0x1.C317B20F90000p-7 },
		{ 0x1.02865137932A9p+0, -0x1.419355DAA0000p-7 },
		{ 0x1.0182427EA7348p+0, -0x1.81203C2EC0000p-8 },
		{ 0x1.008040614B195p+0, -0x1.0040979240000p-9 },
		{ 0x1.FE01FF726FA1Ap-1, 0x1.FEFF384900000p-9 },
		{ 0x1.FA11CC261EA74p-1, 0x1.7DC41353D0000p-7 },
		{ 0x1.F6310B081992Ep-1, 0x1.3CEA3C4C28000p-6 },
		{ 0x1.F25F63CEEADCDp-1, 0x1.B9FC114890000p-6 },
		{ 0x1.EE9C8039113E7p-1, 0x1.1B0D8CE110000p-5 },
		{ 0x1.EAE8078CBB1ABp-1, 0x1.58A5BD001C000p-5 },
		{ 0x1.E741AA29D0C9Bp-1, 0x1.95C8340D88000p-5 },
		{ 0x1.E3A91830A99B5p-1, 0x1.D276AEF578000p-5 },
		{ 0x1.E01E009609A56p-1, 0x1.07598E598C000p-4 },
		{ 0x1.DCA01E577BB98p-1, 0x1.253F5E30D2000p-4 },
		{ 0x1.D92F20B7C9103p-1, 0x1.42EDD8B380000p-4 },
		{ 0x1.D5CAC66FB5CCEp-1, 0x1.606598757C000p-4 },
		{ 0x1.D272CAA5EDE9Dp-1, 0x1.7DA76356A0000p-4 },
		{ 0x1.CF26E3E6B2CCDp-1, 0x1.9AB434E1C6000p-4 },
		{ 0x1.CBE6DA2A77902p-1, 0x1.B78C7BB0D6000p-4 },
		{ 0x1.C8B266D37086Dp-1, 0x1.D431332E72000p-4 },
		{ 0x1.C5894BD5D5804p-1, 0x1.F0A3171DE6000p-4 },
		{ 0x1.C26B533BB9F8Cp-1, 0x1.067152B914000p-3 },
		{ 0x1.BF583EEECE73Fp-1, 0x1.147858292B000p-3 },
		{ 0x1.BC4FD75DB96C1p-1, 0x1.2266ECDCA3000p-3 },
		{ 0x1.B951E0C864A28p-1, 0x1.303D7A6C55000p-3 },
		{ 0x1.B65E2C5EF3E2Cp-1, 0x1.3DFC33C331000p-3 },
		{ 0x1.B374867C9888Bp-1, 0x1.4BA366B7A8000p-3 },
		{ 0x1.B094B211D304Ap-1, 0x1.5933928D1F000p-3 },
		{ 0x1.ADBE885F2EF7Ep-1, 0x1.66ACD2418F000p-3 },
		{ 0x1.AAF1D31603DA2p-1, 0x1.740F8EC669000p-3 },
		{ 0x1.A82E63FD358A7p-1, 0x1.815C0F51AF000p-3 },
		{ 0x1.A5740EF09738Bp-1, 0x1.8E92954F68000p-3 },
		{ 0x1.A2C2A90AB4B27p-1, 0x1.9BB3602F84000p-3 },
		{ 0x1.A01A01393F2D1p-1, 0x1.A8BED1C2C0000p-3 },
		{ 0x1.9D79F24DB3C1Bp-1, 0x1.B5B515C01D000p-3 },
		{ 0x1.9AE2505C7B190p-1, 0x1.C2967CCBCC000p-3 },
		{ 0x1.9852EF297CE2Fp-1, 0x1.CF635D5486000p-3 },
		{ 0x1.95CBAEEA44B75p-1, 0x1.DC1BD3446C000p-3 },
		{ 0x1.934C69DE74838p-1, 0x1.E8C01B8CFE000p-3 },
		{ 0x1.90D4F2F6752E6p-1, 0x1.F5509C0179000p-3 },
		{ 0x1.8E6528EFFD79Dp-1, 0x1.00E6C121FB800p-2 },
		{ 0x1.8BFCE9FCC007Cp-1, 0x1.071B80E93D000p-2 },
		{ 0x1.899C0DABEC30Ep-1, 0x1.0D46B9E867000p-2 },
		{ 0x1.87427AA2317FBp-1, 0x1.13687334BD000p-2 },
		{ 0x1.84F00ACB39A08p-1, 0x1.1980D67234800p-2 },
		{ 0x1.82A49E8653E55p-1, 0x1.1F8FFE0CC8000p-2 },
		{ 0x1.8060195F40260p-1, 0x1.2595FD7636800p-2 },
		{ 0x1.7E22563E0A329p-1, 0x1.2B9300914A800p-2 },
		{ 0x1.7BEB377DCB5ADp-1, 0x1.3187210436000p-2 },
		{ 0x1.79BAA679725C2p-1, 0x1.377266DEC1800p-2 },
		{ 0x1.77907F2170657p-1, 0x1.3D54FFBAF3000p-2 },
		{ 0x1.756CADBD6130Cp-1, 0x1.432EEE32FE000p-2 },
	},
	.tab2 =
	{
		{ 0x1.61000014FB66Bp-1,  0x1.E026C91425B3Cp-56 },
		{ 0x1.63000034DB495p-1,  0x1.DBFEA48005D41p-55 },
		{ 0x1.650000D94D478p-1,  0x1.E7FA786D6A5B7p-55 },
		{ 0x1.67000074E6FADp-1,  0x1.1FCEA6B54254Cp-57 },
		{ 0x1.68FFFFEDF0FAEp-1, -0x1.C7E274C590EFDp-56 },
		{ 0x1.6B0000763C5BCp-1, -0x1.AC16848DCDA01p-55 },
		{ 0x1.6D0001E5CC1F6p-1,  0x1.33F1C9D499311p-55 },
		{ 0x1.6EFFFEB05F63Ep-1, -0x1.E80041AE22D53p-56 },
		{ 0x1.710000E86978p-1,   0x1.BFF6671097952p-56 },
		{ 0x1.72FFFFC67E912p-1,  0x1.C00E226BD8724p-55 },
		{ 0x1.74FFFDF81116Ap-1, -0x1.E02916EF101D2p-57 },
		{ 0x1.770000F679C9p-1,  -0x1.7FC71CD549C74p-57 },
		{ 0x1.78FFFFA7EC835p-1,  0x1.1BEC19EF50483p-55 },
		{ 0x1.7AFFFFE20C2E6p-1, -0x1.07E1729CC6465p-56 },
		{ 0x1.7CFFFED3FC9p-1,   -0x1.08072087B8B1Cp-55 },
		{ 0x1.7EFFFE9261A76p-1,  0x1.DC0286D9DF9AEp-55 },
		{ 0x1.81000049CA3E8p-1,  0x1.97FD251E54C33p-55 },
		{ 0x1.8300017932C8Fp-1, -0x1.AFEE9B630F381p-55 },
		{ 0x1.850000633739Cp-1,  0x1.9BFBF6B6535BCp-55 },
		{ 0x1.87000204289C6p-1, -0x1.BBF65F3117B75p-55 },
		{ 0x1.88FFFEBF57904p-1, -0x1.9006EA23DCB57p-55 },
		{ 0x1.8B00022BC04DFp-1, -0x1.D00DF38E04B0Ap-56 },
		{ 0x1.8CFFFE50C1B8Ap-1, -0x1.8007146FF9F05p-55 },
		{ 0x1.8EFFFFC918E43p-1,  0x1.3817BD07A7038p-55 },
		{ 0x1.910001EFA5FC7p-1,  0x1.93E9176DFB403p-55 },
		{ 0x1.9300013467BB9p-1,  0x1.F804E4B980276p-56 },
		{ 0x1.94FFFE6EE076Fp-1, -0x1.F7EF0D9FF622Ep-55 },
		{ 0x1.96FFFDE3C12D1p-1, -0x1.082AA962638BAp-56 },
		{ 0x1.98FFFF4458A0Dp-1, -0x1.7801B9164A8EFp-55 },
		{ 0x1.9AFFFDD982E3Ep-1, -0x1.740E08A5A9337p-55 },
		{ 0x1.9CFFFED49FB66p-1,  0x1.FCE08C19BEp-60    },
		{ 0x1.9F00020F19C51p-1, -0x1.A3FAA27885B0Ap-55 },
		{ 0x1.A10001145B006p-1,  0x1.4FF489958DA56p-56 },
		{ 0x1.A300007BBF6FAp-1,  0x1.CBEAB8A2B6D18p-55 },
		{ 0x1.A500010971D79p-1,  0x1.8FECADD78793p-55  },
		{ 0x1.A70001DF52E48p-1, -0x1.F41763DD8ABDBp-55 },
		{ 0x1.A90001C593352p-1, -0x1.EBF0284C27612p-55 },
		{ 0x1.AB0002A4F3E4Bp-1, -0x1.9FD043CFF3F5Fp-57 },
		{ 0x1.ACFFFD7AE1ED1p-1, -0x1.23EE7129070B4p-55 },
		{ 0x1.AEFFFEE510478p-1,  0x1.A063EE00EDEA3p-57 },
		{ 0x1.B0FFFDB650D5Bp-1,  0x1.A06C8381F0AB9p-58 },
		{ 0x1.B2FFFFEAACA57p-1, -0x1.9011E74233C1Dp-56 },
		{ 0x1.B4FFFD995BADCp-1, -0x1.9FF1068862A9Fp-56 },
		{ 0x1.B7000249E659Cp-1,  0x1.AFF45D0864F3Ep-55 },
		{ 0x1.B8FFFF987164p-1,   0x1.CFE7796C2C3F9p-56 },
		{ 0x1.BAFFFD204CB4Fp-1, -0x1.3FF27EEF22BC4p-57 },
		{ 0x1.BCFFFD2415C45p-1, -0x1.CFFB7EE3BEA21p-57 },
		{ 0x1.BEFFFF86309DFp-1, -0x1.14103972E0B5Cp-55 },
		{ 0x1.C0FFFE1B57653p-1,  0x1.BC16494B76A19p-55 },
		{ 0x1.C2FFFF1FA57E3p-1, -0x1.4FEEF8D30C6EDp-57 },
		{ 0x1.C4FFFDCBFE424p-1, -0x1.43F68BCEC4775p-55 },
		{ 0x1.C6FFFED54B9F7p-1,  0x1.47EA3F053E0ECp-55 },
		{ 0x1.C8FFFEB998FD5p-1,  0x1.383068DF992F1p-56 },
		{ 0x1.CB0002125219Ap-1, -0x1.8FD8E64180E04p-57 },
		{ 0x1.CCFFFDD94469Cp-1,  0x1.E7EBE1CC7EA72p-55 },
		{ 0x1.CEFFFEAFDC476p-1,  0x1.EBE39AD9F88FEp-55 },
		{ 0x1.D1000169AF82Bp-1,  0x1.57D91A8B95A71p-56 },
		{ 0x1.D30000D0FF71Dp-1,  0x1.9C1906970C7DAp-55 },
		{ 0x1.D4FFFEA790FC4p-1, -0x1.80E37C558FE0Cp-58 },
		{ 0x1.D70002EDC87E5p-1, -0x1.F80D64DC10F44p-56 },
		{ 0x1.D900021DC82AAp-1, -0x1.47C8F94FD5C5Cp-56 },
		{ 0x1.DAFFFD86B0283p-1,  0x1.C7F1DC521617Ep-55 },
		{ 0x1.DD000296C4739p-1,  0x1.8019EB2FFB153p-55 },
		{ 0x1.DEFFFE54490F5p-1,  0x1.E00D2C652CC89p-57 },
		{ 0x1.E0FFFCDABF694p-1, -0x1.F8340202D69D2p-56 },
		{ 0x1.E2FFFDB52C8DDp-1,  0x1.B00C1CA1B0864p-56 },
		{ 0x1.E4FFFF24216EFp-1,  0x1.2FFA8B094AB51p-56 },
		{ 0x1.E6FFFE88A5E11p-1, -0x1.7F673B1EFBE59p-58 },
		{ 0x1.E9000119EFF0Dp-1, -0x1.4808D5E0BC801p-55 },
		{ 0x1.EAFFFDFA51744p-1,  0x1.80006D54320B5p-56 },
		{ 0x1.ED0001A127FA1p-1, -0x1.002F860565C92p-58 },
		{ 0x1.EF00007BABCC4p-1, -0x1.540445D35E611p-55 },
		{ 0x1.F0FFFF57A8D02p-1, -0x1.FFB3139EF9105p-59 },
		{ 0x1.F30001EE58AC7p-1,  0x1.A81ACF2731155p-55 },
		{ 0x1.F4FFFF5823494p-1,  0x1.A3F41D4D7C743p-55 },
		{ 0x1.F6FFFFCA94C6Bp-1, -0x1.202F41C987875p-57 },
		{ 0x1.F8FFFE1F9C441p-1,  0x1.77DD1F477E74Bp-56 },
		{ 0x1.FAFFFD2E0E37Ep-1, -0x1.F01199A7CA331p-57 },
		{ 0x1.FD0001C77E49Ep-1,  0x1.181EE4BCEACB1p-56 },
		{ 0x1.FEFFFF7E0C331p-1, -0x1.E05370170875Ap-57 },
		{ 0x1.00FFFF465606Ep+0, -0x1.A7EAD491C0ADAp-55 },
		{ 0x1.02FFFF3867A58p+0, -0x1.77F69C3FCB2Ep-54  },
		{ 0x1.04FFFFDFC0D17p+0,  0x1.7BFFE34CB945Bp-54 },
		{ 0x1.0700003CD4D82p+0,  0x1.20083C0E456CBp-55 },
		{ 0x1.08FFFF9F2CBE8p+0, -0x1.DFFDFBE37751Ap-57 },
		{ 0x1.0B000010CDA65p+0, -0x1.13F7FAEE626EBp-54 },
		{ 0x1.0D00001A4D338p+0,  0x1.07DFA79489FF7p-55 },
		{ 0x1.0EFFFFADAFDFDp+0, -0x1.7040570D66BCp-56  },
		{ 0x1.110000BBAFD96p+0,  0x1.E80D4846D0B62p-55 },
		{ 0x1.12FFFFAE5F45Dp+0,  0x1.DBFFA64FD36EFp-54 },
		{ 0x1.150000DD59AD9p+0,  0x1.A0077701250AEp-54 },
		{ 0x1.170000F21559Ap+0,  0x1.DFDF9E2E3DEEEp-55 },
		{ 0x1.18FFFFC275426p+0,  0x1.10030DC3B7273p-54 },
		{ 0x1.1B000123D3C59p+0,  0x1.97F7980030188p-54 },
		{ 0x1.1CFFFF8299EB7p+0, -0x1.5F932AB9F8C67p-57 },
		{ 0x1.1EFFFF48AD4p+0,    0x1.37FBF9DA75BEBp-54 },
		{ 0x1.210000C8B86A4p+0,  0x1.F806B91FD5B22p-54 },
		{ 0x1.2300003854303p+0,  0x1.3FFC2EB9FBF33p-54 },
		{ 0x1.24FFFFFBCF684p+0,  0x1.601E77E2E2E72p-56 },
		{ 0x1.26FFFF52921D9p+0,  0x1.FFCBB767F0C61p-56 },
		{ 0x1.2900014933A3Cp+0, -0x1.202CA3C02412Bp-56 },
		{ 0x1.2B00014556313p+0, -0x1.2808233F21F02p-54 },
		{ 0x1.2CFFFEBFE523Bp+0, -0x1.8FF7E384FDCF2p-55 },
		{ 0x1.2F0000BB8AD96p+0, -0x1.5FF51503041C5p-55 },
		{ 0x1.30FFFFB7AE2AFp+0, -0x1.10071885E289Dp-55 },
		{ 0x1.32FFFFEAC5F7Fp+0, -0x1.1FF5D3FB7B715p-54 },
		{ 0x1.350000CA66756p+0,  0x1.57F82228B82BDp-54 },
		{ 0x1.3700011FBF721p+0,  0x1.000BAC40DD5CCp-55 },
		{ 0x1.38FFFF9592FB9p+0, -0x1.43F9D2DB2A751p-54 },
		{ 0x1.3B00004DDD242p+0,  0x1.57F6B707638E1p-55 },
		{ 0x1.3CFFFF5B2C957p+0,  0x1.A023A10BF1231p-56 },
		{ 0x1.3EFFFEAB0B418p+0,  0x1.87F6D66B152Bp-54  },
		{ 0x1.410001532AFF4p+0,  0x1.7F8375F198524p-57 },
		{ 0x1.4300017478B29p+0,  0x1.301E672DC5143p-55 },
		{ 0x1.44FFFE795B463p+0,  0x1.9FF69B8B2895Ap-55 },
		{ 0x1.46FFFE80475Ep+0,  -0x1.5C0B19BC2F254p-54 },
		{ 0x1.48FFFEF6FC1E7p+0,  0x1.B4009F23A2A72p-54 },
		{ 0x1.4AFFFE5BEA704p+0, -0x1.4FFB7BF0D7D45p-54 },
		{ 0x1.4D000171027DEp+0, -0x1.9C06471DC6A3Dp-54 },
		{ 0x1.4F0000FF03EE2p+0,  0x1.77F890B85531Cp-54 },
		{ 0x1.5100012DC4BD1p+0,  0x1.004657166A436p-57 },
		{ 0x1.530001605277Ap+0, -0x1.6BFCECE233209p-54 },
		{ 0x1.54FFFECDB704Cp+0, -0x1.902720505A1D7p-55 },
		{ 0x1.56FFFEF5F54A9p+0,  0x1.BBFE60EC96412p-54 },
		{ 0x1.5900017E61012p+0,  0x1.87EC581AFEF9p-55  },
		{ 0x1.5B00003C93E92p+0, -0x1.F41080ABF0CCp-54  },
		{ 0x1.5D0001D4919BCp+0, -0x1.8812AFB254729p-54 },
		{ 0x1.5EFFFE7B87A89p+0, -0x1.47EB780ED6904p-54 },
	},
};

#define log_T __log_data.tab
#define log_T2 __log_data.tab2
#define log_B __log_data.poly1
#define log_A __log_data.poly
#define log_Ln2hi __log_data.ln2hi
#define log_Ln2lo __log_data.ln2lo
#define log_N (1 << LOG_TABLE_BITS)
#define log_OFF 0x3fe6000000000000

// +--------------------------------------------------------------+
// |                   log2 and log2f constants                   |
// +--------------------------------------------------------------+
#define LOG2F_TABLE_BITS 4
#define LOG2F_POLY_ORDER 4
const struct log2f_data
{
	struct
	{
		double invc, logc;
	} tab[1 << LOG2F_TABLE_BITS];
	double poly[LOG2F_POLY_ORDER];
} __log2f_data =
{
  .tab =
  {
	  { 0x1.661EC79F8F3BEp+0, -0x1.EFEC65B963019p-2 },
	  { 0x1.571ED4AAF883Dp+0, -0x1.B0B6832D4FCA4p-2 },
	  { 0x1.49539F0F010Bp+0,  -0x1.7418B0A1FB77Bp-2 },
	  { 0x1.3C995B0B80385p+0, -0x1.39DE91A6DCF7Bp-2 },
	  { 0x1.30D190C8864A5p+0, -0x1.01D9BF3F2B631p-2 },
	  { 0x1.25E227B0B8EAp+0,  -0x1.97C1D1B3B7AFp-3  },
	  { 0x1.1BB4A4A1A343Fp+0, -0x1.2F9E393AF3C9Fp-3 },
	  { 0x1.12358F08AE5BAp+0, -0x1.960CBBF788D5Cp-4 },
	  { 0x1.0953F419900A7p+0, -0x1.A6F9DB6475FCEp-5 },
	  { 0x1p+0,                0x0p+0 },
	  { 0x1.E608CFD9A47ACp-1,  0x1.338CA9F24F53Dp-4 },
	  { 0x1.CA4B31F026AAp-1,   0x1.476A9543891BAp-3 },
	  { 0x1.B2036576AFCE6p-1,  0x1.E840B4AC4E4D2p-3 },
	  { 0x1.9C2D163A1AA2Dp-1,  0x1.40645F0C6651Cp-2 },
	  { 0x1.886E6037841EDp-1,  0x1.88E9C2C1B9FF8p-2 },
	  { 0x1.767DCF5534862p-1,  0x1.CE0A44EB17BCCp-2 },
  },
  .poly =
  {
	  -0x1.712B6F70A7E4Dp-2, 0x1.ECABF496832Ep-2, -0x1.715479FFAE3DEp-1, 0x1.715475F35C8B8p0,
  }
};

#define log2f_N    (1 << LOG2F_TABLE_BITS)
#define log2f_T    __log2f_data.tab
#define log2f_A    __log2f_data.poly
#define log2f_OFF  0x3f330000

#define LOG2_TABLE_BITS 6
#define LOG2_POLY_ORDER 7
#define LOG2_POLY1_ORDER 11
const struct log2_data
{
	double invln2hi;
	double invln2lo;
	double poly[LOG2_POLY_ORDER - 1];
	double poly1[LOG2_POLY1_ORDER - 1];
	struct
	{
		double invc, logc;
	} tab[1 << LOG2_TABLE_BITS];
	struct
	{
		double chi, clo;
	} tab2[1 << LOG2_TABLE_BITS];
} __log2_data =
{
	// First coefficient: 0x1.71547652b82fe1777d0ffda0d24p0
	.invln2hi = 0x1.7154765200000p+0,
	.invln2lo = 0x1.705fc2eefa200p-33,
	.poly1 =
	{
		// relative error: 0x1.2fad8188p-63
		// in -0x1.5b51p-5 0x1.6ab2p-5
		-0x1.71547652b82fep-1,
		0x1.ec709dc3a03f7p-2,
		-0x1.71547652b7c3fp-2,
		0x1.2776c50f05be4p-2,
		-0x1.ec709dd768fe5p-3,
		0x1.a61761ec4e736p-3,
		-0x1.7153fbc64a79bp-3,
		0x1.484d154f01b4ap-3,
		-0x1.289e4a72c383cp-3,
		0x1.0b32f285aee66p-3,
	},
	.poly =
	{
		// relative error: 0x1.a72c2bf8p-58
		// abs error: 0x1.67a552c8p-66
		// in -0x1.f45p-8 0x1.f45p-8
		-0x1.71547652b8339p-1,
		0x1.ec709dc3a04bep-2,
		-0x1.7154764702ffbp-2,
		0x1.2776c50034c48p-2,
		-0x1.ec7b328ea92bcp-3,
		0x1.a6225e117f92ep-3,
	},
	/*
	Algorithm:
			x = 2^k z
			log2(x) = k + log2(c) + log2(z/c)
			log2(z/c) = poly(z/c - 1)
		where z is in [1.6p-1; 1.6p0] which is split into N subintervals and z falls
		into the ith one, then table entries are computed as
			tab[i].invc = 1/c
			tab[i].logc = (double)log2(c)
			tab2[i].chi = (double)c
			tab2[i].clo = (double)(c - (double)c)
		where c is near the center of the subinterval and is chosen by trying +-2^29
		floating point invc candidates around 1/center and selecting one for which
			1) the rounding error in 0x1.8p10 + logc is 0,
			2) the rounding error in z - chi - clo is < 0x1p-64 and
			3) the rounding error in (double)log2(c) is minimized (< 0x1p-68).
		Note: 1) ensures that k + logc can be computed without rounding error, 2)
		ensures that z/c - 1 can be computed as (z - chi - clo)*invc with close to a
		single rounding error when there is no fast fma for z*invc - 1, 3) ensures
		that logc + poly(z/c - 1) has small error, however near x == 1 when
		|log2(x)| < 0x1p-4, this is not enough so that is special cased.
	*/
	.tab =
	{
		{0x1.724286bb1acf8p+0, -0x1.1095feecdb000p-1},
		{0x1.6e1f766d2cca1p+0, -0x1.08494bd76d000p-1},
		{0x1.6a13d0e30d48ap+0, -0x1.00143aee8f800p-1},
		{0x1.661ec32d06c85p+0, -0x1.efec5360b4000p-2},
		{0x1.623fa951198f8p+0, -0x1.dfdd91ab7e000p-2},
		{0x1.5e75ba4cf026cp+0, -0x1.cffae0cc79000p-2},
		{0x1.5ac055a214fb8p+0, -0x1.c043811fda000p-2},
		{0x1.571ed0f166e1ep+0, -0x1.b0b67323ae000p-2},
		{0x1.53909590bf835p+0, -0x1.a152f5a2db000p-2},
		{0x1.5014fed61adddp+0, -0x1.9217f5af86000p-2},
		{0x1.4cab88e487bd0p+0, -0x1.8304db0719000p-2},
		{0x1.49539b4334feep+0, -0x1.74189f9a9e000p-2},
		{0x1.460cbdfafd569p+0, -0x1.6552bb5199000p-2},
		{0x1.42d664ee4b953p+0, -0x1.56b23a29b1000p-2},
		{0x1.3fb01111dd8a6p+0, -0x1.483650f5fa000p-2},
		{0x1.3c995b70c5836p+0, -0x1.39de937f6a000p-2},
		{0x1.3991c4ab6fd4ap+0, -0x1.2baa1538d6000p-2},
		{0x1.3698e0ce099b5p+0, -0x1.1d98340ca4000p-2},
		{0x1.33ae48213e7b2p+0, -0x1.0fa853a40e000p-2},
		{0x1.30d191985bdb1p+0, -0x1.01d9c32e73000p-2},
		{0x1.2e025cab271d7p+0, -0x1.e857da2fa6000p-3},
		{0x1.2b404cf13cd82p+0, -0x1.cd3c8633d8000p-3},
		{0x1.288b02c7ccb50p+0, -0x1.b26034c14a000p-3},
		{0x1.25e2263944de5p+0, -0x1.97c1c2f4fe000p-3},
		{0x1.234563d8615b1p+0, -0x1.7d6023f800000p-3},
		{0x1.20b46e33eaf38p+0, -0x1.633a71a05e000p-3},
		{0x1.1e2eefdcda3ddp+0, -0x1.494f5e9570000p-3},
		{0x1.1bb4a580b3930p+0, -0x1.2f9e424e0a000p-3},
		{0x1.19453847f2200p+0, -0x1.162595afdc000p-3},
		{0x1.16e06c0d5d73cp+0, -0x1.f9c9a75bd8000p-4},
		{0x1.1485f47b7e4c2p+0, -0x1.c7b575bf9c000p-4},
		{0x1.12358ad0085d1p+0, -0x1.960c60ff48000p-4},
		{0x1.0fef00f532227p+0, -0x1.64ce247b60000p-4},
		{0x1.0db2077d03a8fp+0, -0x1.33f78b2014000p-4},
		{0x1.0b7e6d65980d9p+0, -0x1.0387d1a42c000p-4},
		{0x1.0953efe7b408dp+0, -0x1.a6f9208b50000p-5},
		{0x1.07325cac53b83p+0, -0x1.47a954f770000p-5},
		{0x1.05197e40d1b5cp+0, -0x1.d23a8c50c0000p-6},
		{0x1.03091c1208ea2p+0, -0x1.16a2629780000p-6},
		{0x1.0101025b37e21p+0, -0x1.720f8d8e80000p-8},
		{0x1.fc07ef9caa76bp-1, 0x1.6fe53b1500000p-7},
		{0x1.f4465d3f6f184p-1, 0x1.11ccce10f8000p-5},
		{0x1.ecc079f84107fp-1, 0x1.c4dfc8c8b8000p-5},
		{0x1.e573a99975ae8p-1, 0x1.3aa321e574000p-4},
		{0x1.de5d6f0bd3de6p-1, 0x1.918a0d08b8000p-4},
		{0x1.d77b681ff38b3p-1, 0x1.e72e9da044000p-4},
		{0x1.d0cb5724de943p-1, 0x1.1dcd2507f6000p-3},
		{0x1.ca4b2dc0e7563p-1, 0x1.476ab03dea000p-3},
		{0x1.c3f8ee8d6cb51p-1, 0x1.7074377e22000p-3},
		{0x1.bdd2b4f020c4cp-1, 0x1.98ede8ba94000p-3},
		{0x1.b7d6c006015cap-1, 0x1.c0db86ad2e000p-3},
		{0x1.b20366e2e338fp-1, 0x1.e840aafcee000p-3},
		{0x1.ac57026295039p-1, 0x1.0790ab4678000p-2},
		{0x1.a6d01bc2731ddp-1, 0x1.1ac056801c000p-2},
		{0x1.a16d3bc3ff18bp-1, 0x1.2db11d4fee000p-2},
		{0x1.9c2d14967feadp-1, 0x1.406464ec58000p-2},
		{0x1.970e4f47c9902p-1, 0x1.52dbe093af000p-2},
		{0x1.920fb3982bcf2p-1, 0x1.651902050d000p-2},
		{0x1.8d30187f759f1p-1, 0x1.771d2cdeaf000p-2},
		{0x1.886e5ebb9f66dp-1, 0x1.88e9c857d9000p-2},
		{0x1.83c97b658b994p-1, 0x1.9a80155e16000p-2},
		{0x1.7f405ffc61022p-1, 0x1.abe186ed3d000p-2},
		{0x1.7ad22181415cap-1, 0x1.bd0f2aea0e000p-2},
		{0x1.767dcf99eff8cp-1, 0x1.ce0a43dbf4000p-2},
		},
	.tab2 =
	{
		{0x1.6200012b90a8ep-1, 0x1.904ab0644b605p-55},
		{0x1.66000045734a6p-1, 0x1.1ff9bea62f7a9p-57},
		{0x1.69fffc325f2c5p-1, 0x1.27ecfcb3c90bap-55},
		{0x1.6e00038b95a04p-1, 0x1.8ff8856739326p-55},
		{0x1.71fffe09994e3p-1, 0x1.afd40275f82b1p-55},
		{0x1.7600015590e1p-1, -0x1.2fd75b4238341p-56},
		{0x1.7a00012655bd5p-1, 0x1.808e67c242b76p-56},
		{0x1.7e0003259e9a6p-1, -0x1.208e426f622b7p-57},
		{0x1.81fffedb4b2d2p-1, -0x1.402461ea5c92fp-55},
		{0x1.860002dfafcc3p-1, 0x1.df7f4a2f29a1fp-57},
		{0x1.89ffff78c6b5p-1, -0x1.e0453094995fdp-55},
		{0x1.8e00039671566p-1, -0x1.a04f3bec77b45p-55},
		{0x1.91fffe2bf1745p-1, -0x1.7fa34400e203cp-56},
		{0x1.95fffcc5c9fd1p-1, -0x1.6ff8005a0695dp-56},
		{0x1.9a0003bba4767p-1, 0x1.0f8c4c4ec7e03p-56},
		{0x1.9dfffe7b92da5p-1, 0x1.e7fd9478c4602p-55},
		{0x1.a1fffd72efdafp-1, -0x1.a0c554dcdae7ep-57},
		{0x1.a5fffde04ff95p-1, 0x1.67da98ce9b26bp-55},
		{0x1.a9fffca5e8d2bp-1, -0x1.284c9b54c13dep-55},
		{0x1.adfffddad03eap-1, 0x1.812c8ea602e3cp-58},
		{0x1.b1ffff10d3d4dp-1, -0x1.efaddad27789cp-55},
		{0x1.b5fffce21165ap-1, 0x1.3cb1719c61237p-58},
		{0x1.b9fffd950e674p-1, 0x1.3f7d94194cep-56},
		{0x1.be000139ca8afp-1, 0x1.50ac4215d9bcp-56},
		{0x1.c20005b46df99p-1, 0x1.beea653e9c1c9p-57},
		{0x1.c600040b9f7aep-1, -0x1.c079f274a70d6p-56},
		{0x1.ca0006255fd8ap-1, -0x1.a0b4076e84c1fp-56},
		{0x1.cdfffd94c095dp-1, 0x1.8f933f99ab5d7p-55},
		{0x1.d1ffff975d6cfp-1, -0x1.82c08665fe1bep-58},
		{0x1.d5fffa2561c93p-1, -0x1.b04289bd295f3p-56},
		{0x1.d9fff9d228b0cp-1, 0x1.70251340fa236p-55},
		{0x1.de00065bc7e16p-1, -0x1.5011e16a4d80cp-56},
		{0x1.e200002f64791p-1, 0x1.9802f09ef62ep-55},
		{0x1.e600057d7a6d8p-1, -0x1.e0b75580cf7fap-56},
		{0x1.ea00027edc00cp-1, -0x1.c848309459811p-55},
		{0x1.ee0006cf5cb7cp-1, -0x1.f8027951576f4p-55},
		{0x1.f2000782b7dccp-1, -0x1.f81d97274538fp-55},
		{0x1.f6000260c450ap-1, -0x1.071002727ffdcp-59},
		{0x1.f9fffe88cd533p-1, -0x1.81bdce1fda8bp-58},
		{0x1.fdfffd50f8689p-1, 0x1.7f91acb918e6ep-55},
		{0x1.0200004292367p+0, 0x1.b7ff365324681p-54},
		{0x1.05fffe3e3d668p+0, 0x1.6fa08ddae957bp-55},
		{0x1.0a0000a85a757p+0, -0x1.7e2de80d3fb91p-58},
		{0x1.0e0001a5f3fccp+0, -0x1.1823305c5f014p-54},
		{0x1.11ffff8afbaf5p+0, -0x1.bfabb6680bac2p-55},
		{0x1.15fffe54d91adp+0, -0x1.d7f121737e7efp-54},
		{0x1.1a00011ac36e1p+0, 0x1.c000a0516f5ffp-54},
		{0x1.1e00019c84248p+0, -0x1.082fbe4da5dap-54},
		{0x1.220000ffe5e6ep+0, -0x1.8fdd04c9cfb43p-55},
		{0x1.26000269fd891p+0, 0x1.cfe2a7994d182p-55},
		{0x1.2a00029a6e6dap+0, -0x1.00273715e8bc5p-56},
		{0x1.2dfffe0293e39p+0, 0x1.b7c39dab2a6f9p-54},
		{0x1.31ffff7dcf082p+0, 0x1.df1336edc5254p-56},
		{0x1.35ffff05a8b6p+0, -0x1.e03564ccd31ebp-54},
		{0x1.3a0002e0eaeccp+0, 0x1.5f0e74bd3a477p-56},
		{0x1.3e000043bb236p+0, 0x1.c7dcb149d8833p-54},
		{0x1.4200002d187ffp+0, 0x1.e08afcf2d3d28p-56},
		{0x1.460000d387cb1p+0, 0x1.20837856599a6p-55},
		{0x1.4a00004569f89p+0, -0x1.9fa5c904fbcd2p-55},
		{0x1.4e000043543f3p+0, -0x1.81125ed175329p-56},
		{0x1.51fffcc027f0fp+0, 0x1.883d8847754dcp-54},
		{0x1.55ffffd87b36fp+0, -0x1.709e731d02807p-55},
		{0x1.59ffff21df7bap+0, 0x1.7f79f68727b02p-55},
		{0x1.5dfffebfc3481p+0, -0x1.180902e30e93ep-54},
	},
};

#define log2_T        __log2_data.tab
#define log2_T2       __log2_data.tab2
#define log2_B        __log2_data.poly1
#define log2_A        __log2_data.poly
#define log2_InvLn2hi __log2_data.invln2hi
#define log2_InvLn2lo __log2_data.invln2lo
#define log2_N        (1 << LOG2_TABLE_BITS)
#define log2_OFF      0x3fe6000000000000

// +--------------------------------------------------------------+
// |                  log10 and log10f constants                  |
// +--------------------------------------------------------------+

static const float
	ivln10hi  =  4.3432617188e-01, // 0x3ede6000
	ivln10lo  = -3.1689971365e-05, // 0xb804ead9
	log10_2hi =  3.0102920532e-01, // 0x3e9a2080
	log10_2lo =  7.9034151668e-07, // 0x355427db
	// |(log(1+s)-log(1-s))/s - Lg(s)| < 2**-34.24 (~[-4.95e-11, 4.97e-11]).
	Lg1 = 0xAAAAAA.0p-24, // 0.66666662693
	Lg2 = 0xCCCE13.0p-25, // 0.40000972152
	Lg3 = 0x91E9EE.0p-25, // 0.28498786688
	Lg4 = 0xF89E26.0p-26; // 0.24279078841

static const double
	ivln10hid  = 4.34294481878168880939e-01, // 0x3fdbcb7b, 0x15200000
	ivln10lod  = 2.50829467116452752298e-11, // 0x3dbb9438, 0xca9aadd5
	log10_2hid = 3.01029995663611771306e-01, // 0x3FD34413, 0x509F6000
	log10_2lod = 3.69423907715893078616e-13, // 0x3D59FEF3, 0x11F12B36
	Lg1d = 6.666666666666735130e-01,  // 3FE55555 55555593
	Lg2d = 3.999999999940941908e-01,  // 3FD99999 9997FA04
	Lg3d = 2.857142874366239149e-01,  // 3FD24924 94229359
	Lg4d = 2.222219843214978396e-01,  // 3FCC71C5 1D8E78AF
	Lg5d = 1.818357216161805012e-01,  // 3FC74664 96CB03DE
	Lg6d = 1.531383769920937332e-01,  // 3FC39A09 D078C69F
	Lg7d = 1.479819860511658591e-01;  // 3FC2F112 DF3E5244

// +--------------------------------------------------------------+
// |                  log2_inline and Constants                   |
// +--------------------------------------------------------------+
#define POWF_LOG2_TABLE_BITS 4
#define POWF_LOG2_POLY_ORDER 5
#define log2inline_N (1 << POWF_LOG2_TABLE_BITS)
#define POWF_SCALE_BITS 0
#define POWF_SCALE ((double)(1 << POWF_SCALE_BITS))

const struct powf_log2_data
{
	struct
	{
		double invc, logc;
	} tab[1 << POWF_LOG2_TABLE_BITS];
	double poly[POWF_LOG2_POLY_ORDER];
} __powf_log2_data =
{
	.tab =
	{
		{ 0x1.661EC79F8F3BEp+0, -0x1.EFEC65B963019p-2 * POWF_SCALE },
		{ 0x1.571ED4AAF883Dp+0, -0x1.B0B6832D4FCA4p-2 * POWF_SCALE },
		{ 0x1.49539F0F010Bp+0,  -0x1.7418B0A1FB77Bp-2 * POWF_SCALE },
		{ 0x1.3C995B0B80385p+0, -0x1.39DE91A6DCF7Bp-2 * POWF_SCALE },
		{ 0x1.30D190C8864A5p+0, -0x1.01D9BF3F2B631p-2 * POWF_SCALE },
		{ 0x1.25E227B0B8EAp+0,  -0x1.97C1D1B3B7AFp-3  * POWF_SCALE },
		{ 0x1.1BB4A4A1A343Fp+0, -0x1.2F9E393AF3C9Fp-3 * POWF_SCALE },
		{ 0x1.12358F08AE5BAp+0, -0x1.960CBBF788D5Cp-4 * POWF_SCALE },
		{ 0x1.0953F419900A7p+0, -0x1.A6F9DB6475FCEp-5 * POWF_SCALE },
		{               0x1p+0,                0x0p+0 * POWF_SCALE },
		{ 0x1.E608CFD9A47ACp-1,  0x1.338CA9F24F53Dp-4 * POWF_SCALE },
		{ 0x1.CA4B31F026AAp-1,   0x1.476A9543891BAp-3 * POWF_SCALE },
		{ 0x1.B2036576AFCE6p-1,  0x1.E840B4AC4E4D2p-3 * POWF_SCALE },
		{ 0x1.9C2D163A1AA2Dp-1,  0x1.40645F0C6651Cp-2 * POWF_SCALE },
		{ 0x1.886E6037841EDp-1,  0x1.88E9C2C1B9FF8p-2 * POWF_SCALE },
		{ 0x1.767DCF5534862p-1,  0x1.CE0A44EB17BCCp-2 * POWF_SCALE },
	},
	.poly =
	{
		0x1.27616C9496E0Bp-2 * POWF_SCALE, -0x1.71969A075C67Ap-2 * POWF_SCALE,
		0x1.EC70A6CA7BADDp-2 * POWF_SCALE, -0x1.7154748BEF6C8p-1 * POWF_SCALE,
		0x1.71547652AB82Bp0 * POWF_SCALE,
	}
};

#define log2inline_T __powf_log2_data.tab
#define log2inline_A __powf_log2_data.poly
#define log2inline_OFF 0x3F330000

// Subnormal input is normalized so ix has negative biased exponent.
// Output is multiplied by log2inline_N (POWF_SCALE) if TOINT_INTRINICS is set.
double_t log2_inline(uint32_t floatInt)
{
	double_t zVar, rVar, rVarSquared, rVarQuad, polynomial, qVar, result, yVar, cInverse, logc;
	uint32_t zVarInt, topBits, temp;
	int vVar, index;
	
	// x = 2^vVar zVar; where zVar is in range [log2inline_OFF,2*log2inline_OFF] and exact.
	// The range is split into log2inline_N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	temp = floatInt - log2inline_OFF;
	index = ((temp >> (23 - POWF_LOG2_TABLE_BITS)) % log2inline_N);
	topBits = (temp & 0xFF800000);
	zVarInt = floatInt - topBits;
	vVar = ((int32_t)topBits >> (23 - POWF_SCALE_BITS)); /* arithmetic shift */
	cInverse = log2inline_T[index].invc;
	logc = log2inline_T[index].logc;
	zVar = (double_t)asfloat(zVarInt);
	
	// log2(x) = log1p(zVar/c-1)/ln2 + log2(c) + vVar
	rVar = (zVar * cInverse) - 1;
	yVar = logc + (double_t)vVar;
	
	// Pipelined polynomial evaluation to approximate log1p(rVar)/ln2.
	rVarSquared = rVar * rVar;
	result = (log2inline_A[0] * rVar) + log2inline_A[1];
	polynomial = (log2inline_A[2] * rVar) + log2inline_A[3];
	rVarQuad = rVarSquared * rVarSquared;
	qVar = (log2inline_A[4] * rVar) + yVar;
	qVar = (polynomial * rVarSquared) + qVar;
	result = (result * rVarQuad) + qVar;
	return result;
}

// +--------------------------------------------------------------+
// |                  exp2_inline and Constants                   |
// +--------------------------------------------------------------+
#define exp2inline_T __exp2f_data.tab
#define exp2inline_C __exp2f_data.poly
#define exp2inline_SHIFT __exp2f_data.shift_scaled
#define exp2inline_SIGN_BIAS (1 << (EXP2F_TABLE_BITS + 11))

// The output of log2 and thus the input of exp2 is either scaled by N
// (in case of fast toint intrinsics) or not.  The unscaled xd must be
// in [-1021,1023], signBias sets the sign of the result.
float exp2_inline(double_t value, uint32_t signBias)
{
	uint64_t ki, ski, tVarInt;
	double_t kd, zVar, rVar, rVarSquared, result, tVar;
	
	// x = k/N + rVar with rVar in [-1/(2N), 1/(2N)]
	kd = eval_as_double(value + exp2inline_SHIFT);
	ki = asuint64(kd);
	kd -= exp2inline_SHIFT; // k/N
	rVar = value - kd;
	
	// exp2(x) = 2^(k/N) * 2^rVar ~= s * (C0*rVar^3 + C1*rVar^2 + C2*rVar + 1)
	tVarInt = exp2inline_T[ki % EXP2F_N];
	ski = ki + signBias;
	tVarInt += (ski << (52 - EXP2F_TABLE_BITS));
	tVar = asdouble(tVarInt);
	zVar = (exp2inline_C[0] * rVar) + exp2inline_C[1];
	rVarSquared = rVar * rVar;
	result = (exp2inline_C[2] * rVar) + 1;
	result = (zVar * rVarSquared) + result;
	result = result * tVar;
	return eval_as_float(result);
}

// +--------------------------------------------------------------+
// |                   log_inline and Constants                   |
// +--------------------------------------------------------------+
#define POW_LOG_TABLE_BITS 7
#define POW_LOG_POLY_ORDER 8
#define powlog_N (1 << POW_LOG_TABLE_BITS)

const struct pow_log_data
{
	double ln2hi;
	double ln2lo;
	double poly[POW_LOG_POLY_ORDER - 1]; /* First coefficient is 1.  */
	/* Note: the pad field is unused, but allows slightly faster indexing.  */
	struct
	{
		double invc, pad, logc, logctail;
	} tab[1 << POW_LOG_TABLE_BITS];
} __pow_log_data =
{
	.ln2hi = 0x1.62E42FEFA3800p-1,
	.ln2lo = 0x1.EF35793C76730p-45,
	.poly =
	{
		// relative error: 0x1.11922ap-70
		// in -0x1.6bp-8 0x1.6bp-8
		// Coefficients are scaled to match the scaling during evaluation.
		-0x1p-1,
		 0x1.555555555556p-2  * -2,
		-0x1.0000000000006p-2 * -2,
		 0x1.999999959554Ep-3 *  4,
		-0x1.555555529A47Ap-3 *  4,
		 0x1.2495B9B4845E9p-3 * -8,
		-0x1.0002B8B263FC3p-3 * -8,
	},
	/*
	Algorithm:
			x = 2^k z
			log(x) = k ln2 + log(c) + log(z/c)
			log(z/c) = poly(z/c - 1)
		where z is in [0x1.69555p-1; 0x1.69555p0] which is split into N subintervals
		and z falls into the ith one, then table entries are computed as
			tab[i].invc = 1/c
			tab[i].logc = round(0x1p43*log(c))/0x1p43
			tab[i].logctail = (double)(log(c) - logc)
		where c is chosen near the center of the subinterval such that 1/c has only a
		few precision bits so z/c - 1 is exactly representible as double:
			1/c = center < 1 ? round(N/center)/N : round(2*N/center)/N/2
		Note: |z/c - 1| < 1/N for the chosen c, |log(c) - logc - logctail| < 0x1p-97,
		the last few bits of logc are rounded away so k*ln2hi + logc has no rounding
		error and the interval for z is selected such that near x == 1, where log(x)
		is tiny, large cancellation error is avoided in logc + poly(z/c - 1).
	*/
	.tab =
	{
		{ 0x1.6A00000000000p+0, 0, -0x1.62C82F2B9C800p-2,  0x1.AB42428375680p-48 },
		{ 0x1.6800000000000p+0, 0, -0x1.5D1BDBF580800p-2, -0x1.CA508D8E0F720p-46 },
		{ 0x1.6600000000000p+0, 0, -0x1.5767717455800p-2, -0x1.362A4D5B6506Dp-45 },
		{ 0x1.6400000000000p+0, 0, -0x1.51AAD872DF800p-2, -0x1.684E49EB067D5p-49 },
		{ 0x1.6200000000000p+0, 0, -0x1.4BE5F95777800p-2, -0x1.41B6993293EE0p-47 },
		{ 0x1.6000000000000p+0, 0, -0x1.4618BC21C6000p-2,  0x1.3D82F484C84CCp-46 },
		{ 0x1.5E00000000000p+0, 0, -0x1.404308686A800p-2,  0x1.C42F3ED820B3Ap-50 },
		{ 0x1.5C00000000000p+0, 0, -0x1.3A64C55694800p-2,  0x1.0B1C686519460p-45 },
		{ 0x1.5A00000000000p+0, 0, -0x1.347DD9A988000p-2,  0x1.5594DD4C58092p-45 },
		{ 0x1.5800000000000p+0, 0, -0x1.2E8E2BAE12000p-2,  0x1.67B1E99B72BD8p-45 },
		{ 0x1.5600000000000p+0, 0, -0x1.2895A13DE8800p-2,  0x1.5CA14B6CFB03Fp-46 },
		{ 0x1.5600000000000p+0, 0, -0x1.2895A13DE8800p-2,  0x1.5CA14B6CFB03Fp-46 },
		{ 0x1.5400000000000p+0, 0, -0x1.22941FBCF7800p-2, -0x1.65A242853DA76p-46 },
		{ 0x1.5200000000000p+0, 0, -0x1.1C898C1699800p-2, -0x1.FAFBC68E75404p-46 },
		{ 0x1.5000000000000p+0, 0, -0x1.1675CABABA800p-2,  0x1.F1FC63382A8F0p-46 },
		{ 0x1.4E00000000000p+0, 0, -0x1.1058BF9AE4800p-2, -0x1.6A8C4FD055A66p-45 },
		{ 0x1.4C00000000000p+0, 0, -0x1.0A324E2739000p-2, -0x1.C6BEE7EF4030Ep-47 },
		{ 0x1.4A00000000000p+0, 0, -0x1.0402594B4D000p-2, -0x1.036B89EF42D7Fp-48 },
		{ 0x1.4A00000000000p+0, 0, -0x1.0402594B4D000p-2, -0x1.036B89EF42D7Fp-48 },
		{ 0x1.4800000000000p+0, 0, -0x1.FB9186D5E4000p-3,  0x1.D572AAB993C87p-47 },
		{ 0x1.4600000000000p+0, 0, -0x1.EF0ADCBDC6000p-3,  0x1.B26B79C86AF24p-45 },
		{ 0x1.4400000000000p+0, 0, -0x1.E27076E2AF000p-3, -0x1.72F4F543FFF10p-46 },
		{ 0x1.4200000000000p+0, 0, -0x1.D5C216B4FC000p-3,  0x1.1BA91BBCA681Bp-45 },
		{ 0x1.4000000000000p+0, 0, -0x1.C8FF7C79AA000p-3,  0x1.7794F689F8434p-45 },
		{ 0x1.4000000000000p+0, 0, -0x1.C8FF7C79AA000p-3,  0x1.7794F689F8434p-45 },
		{ 0x1.3E00000000000p+0, 0, -0x1.BC286742D9000p-3,  0x1.94EB0318BB78Fp-46 },
		{ 0x1.3C00000000000p+0, 0, -0x1.AF3C94E80C000p-3,  0x1.A4E633FCD9066p-52 },
		{ 0x1.3A00000000000p+0, 0, -0x1.A23BC1FE2B000p-3, -0x1.58C64DC46C1EAp-45 },
		{ 0x1.3A00000000000p+0, 0, -0x1.A23BC1FE2B000p-3, -0x1.58C64DC46C1EAp-45 },
		{ 0x1.3800000000000p+0, 0, -0x1.9525A9CF45000p-3, -0x1.AD1D904C1D4E3p-45 },
		{ 0x1.3600000000000p+0, 0, -0x1.87FA06520D000p-3,  0x1.BBDBF7FDBFA09p-45 },
		{ 0x1.3400000000000p+0, 0, -0x1.7AB890210E000p-3,  0x1.BDB9072534A58p-45 },
		{ 0x1.3400000000000p+0, 0, -0x1.7AB890210E000p-3,  0x1.BDB9072534A58p-45 },
		{ 0x1.3200000000000p+0, 0, -0x1.6D60FE719D000p-3, -0x1.0E46AA3B2E266p-46 },
		{ 0x1.3000000000000p+0, 0, -0x1.5FF3070A79000p-3, -0x1.E9E439F105039p-46 },
		{ 0x1.3000000000000p+0, 0, -0x1.5FF3070A79000p-3, -0x1.E9E439F105039p-46 },
		{ 0x1.2E00000000000p+0, 0, -0x1.526E5E3A1B000p-3, -0x1.0DE8B90075B8Fp-45 },
		{ 0x1.2C00000000000p+0, 0, -0x1.44D2B6CCB8000p-3,  0x1.70CC16135783Cp-46 },
		{ 0x1.2C00000000000p+0, 0, -0x1.44D2B6CCB8000p-3,  0x1.70CC16135783Cp-46 },
		{ 0x1.2A00000000000p+0, 0, -0x1.371FC201E9000p-3,  0x1.178864D27543Ap-48 },
		{ 0x1.2800000000000p+0, 0, -0x1.29552F81FF000p-3, -0x1.48D301771C408p-45 },
		{ 0x1.2600000000000p+0, 0, -0x1.1B72AD52F6000p-3, -0x1.E80A41811A396p-45 },
		{ 0x1.2600000000000p+0, 0, -0x1.1B72AD52F6000p-3, -0x1.E80A41811A396p-45 },
		{ 0x1.2400000000000p+0, 0, -0x1.0D77E7CD09000p-3,  0x1.A699688E85BF4p-47 },
		{ 0x1.2400000000000p+0, 0, -0x1.0D77E7CD09000p-3,  0x1.A699688E85BF4p-47 },
		{ 0x1.2200000000000p+0, 0, -0x1.FEC9131DBE000p-4, -0x1.575545CA333F2p-45 },
		{ 0x1.2000000000000p+0, 0, -0x1.E27076E2B0000p-4,  0x1.A342C2AF0003Cp-45 },
		{ 0x1.2000000000000p+0, 0, -0x1.E27076E2B0000p-4,  0x1.A342C2AF0003Cp-45 },
		{ 0x1.1E00000000000p+0, 0, -0x1.C5E548F5BC000p-4, -0x1.D0C57585FBE06p-46 },
		{ 0x1.1C00000000000p+0, 0, -0x1.A926D3A4AE000p-4,  0x1.53935E85BAAC8p-45 },
		{ 0x1.1C00000000000p+0, 0, -0x1.A926D3A4AE000p-4,  0x1.53935E85BAAC8p-45 },
		{ 0x1.1A00000000000p+0, 0, -0x1.8C345D631A000p-4,  0x1.37C294D2F5668p-46 },
		{ 0x1.1A00000000000p+0, 0, -0x1.8C345D631A000p-4,  0x1.37C294D2F5668p-46 },
		{ 0x1.1800000000000p+0, 0, -0x1.6F0D28AE56000p-4, -0x1.69737C93373DAp-45 },
		{ 0x1.1600000000000p+0, 0, -0x1.51B073F062000p-4,  0x1.F025B61C65E57p-46 },
		{ 0x1.1600000000000p+0, 0, -0x1.51B073F062000p-4,  0x1.F025B61C65E57p-46 },
		{ 0x1.1400000000000p+0, 0, -0x1.341D7961BE000p-4,  0x1.C5EDACCF913DFp-45 },
		{ 0x1.1400000000000p+0, 0, -0x1.341D7961BE000p-4,  0x1.C5EDACCF913DFp-45 },
		{ 0x1.1200000000000p+0, 0, -0x1.16536EEA38000p-4,  0x1.47C5E768FA309p-46 },
		{ 0x1.1000000000000p+0, 0, -0x1.F0A30C0118000p-5,  0x1.D599E83368E91p-45 },
		{ 0x1.1000000000000p+0, 0, -0x1.F0A30C0118000p-5,  0x1.D599E83368E91p-45 },
		{ 0x1.0E00000000000p+0, 0, -0x1.B42DD71198000p-5,  0x1.C827AE5D6704Cp-46 },
		{ 0x1.0E00000000000p+0, 0, -0x1.B42DD71198000p-5,  0x1.C827AE5D6704Cp-46 },
		{ 0x1.0C00000000000p+0, 0, -0x1.77458F632C000p-5, -0x1.CFC4634F2A1EEp-45 },
		{ 0x1.0C00000000000p+0, 0, -0x1.77458F632C000p-5, -0x1.CFC4634F2A1EEp-45 },
		{ 0x1.0A00000000000p+0, 0, -0x1.39E87B9FEC000p-5,  0x1.502B7F526FEAAp-48 },
		{ 0x1.0A00000000000p+0, 0, -0x1.39E87B9FEC000p-5,  0x1.502B7F526FEAAp-48 },
		{ 0x1.0800000000000p+0, 0, -0x1.F829B0E780000p-6, -0x1.980267C7E09E4p-45 },
		{ 0x1.0800000000000p+0, 0, -0x1.F829B0E780000p-6, -0x1.980267C7E09E4p-45 },
		{ 0x1.0600000000000p+0, 0, -0x1.7B91B07D58000p-6, -0x1.88D5493FAA639p-45 },
		{ 0x1.0400000000000p+0, 0, -0x1.FC0A8B0FC0000p-7, -0x1.F1E7CF6D3A69Cp-50 },
		{ 0x1.0400000000000p+0, 0, -0x1.FC0A8B0FC0000p-7, -0x1.F1E7CF6D3A69Cp-50 },
		{ 0x1.0200000000000p+0, 0, -0x1.FE02A6B100000p-8, -0x1.9E23F0DDA40E4p-46 },
		{ 0x1.0200000000000p+0, 0, -0x1.FE02A6B100000p-8, -0x1.9E23F0DDA40E4p-46 },
		{ 0x1.0000000000000p+0, 0,  0x0.0000000000000p+0,  0x0.0000000000000p+0  },
		{ 0x1.0000000000000p+0, 0,  0x0.0000000000000p+0,  0x0.0000000000000p+0  },
		{ 0x1.FC00000000000p-1, 0,  0x1.0101575890000p-7, -0x1.0C76B999D2BE8p-46 },
		{ 0x1.F800000000000p-1, 0,  0x1.0205658938000p-6, -0x1.3DC5B06E2F7D2p-45 },
		{ 0x1.F400000000000p-1, 0,  0x1.8492528C90000p-6, -0x1.AA0BA325A0C34p-45 },
		{ 0x1.F000000000000p-1, 0,  0x1.0415D89E74000p-5,  0x1.111C05CF1D753p-47 },
		{ 0x1.EC00000000000p-1, 0,  0x1.466AED42E0000p-5, -0x1.C167375BDFD28p-45 },
		{ 0x1.E800000000000p-1, 0,  0x1.894AA149FC000p-5, -0x1.97995D05A267Dp-46 },
		{ 0x1.E400000000000p-1, 0,  0x1.CCB73CDDDC000p-5, -0x1.A68F247D82807p-46 },
		{ 0x1.E200000000000p-1, 0,  0x1.EEA31C006C000p-5, -0x1.E113E4FC93B7Bp-47 },
		{ 0x1.DE00000000000p-1, 0,  0x1.1973BD1466000p-4, -0x1.5325D560D9E9Bp-45 },
		{ 0x1.DA00000000000p-1, 0,  0x1.3BDF5A7D1E000p-4,  0x1.CC85EA5DB4ED7p-45 },
		{ 0x1.D600000000000p-1, 0,  0x1.5E95A4D97A000p-4, -0x1.C69063C5D1D1Ep-45 },
		{ 0x1.D400000000000p-1, 0,  0x1.700D30AEAC000p-4,  0x1.C1E8DA99DED32p-49 },
		{ 0x1.D000000000000p-1, 0,  0x1.9335E5D594000p-4,  0x1.3115C3ABD47DAp-45 },
		{ 0x1.CC00000000000p-1, 0,  0x1.B6AC88DAD6000p-4, -0x1.390802BF768E5p-46 },
		{ 0x1.CA00000000000p-1, 0,  0x1.C885801BC4000p-4,  0x1.646D1C65AACD3p-45 },
		{ 0x1.C600000000000p-1, 0,  0x1.EC739830A2000p-4, -0x1.DC068AFE645E0p-45 },
		{ 0x1.C400000000000p-1, 0,  0x1.FE89139DBE000p-4, -0x1.534D64FA10AFDp-45 },
		{ 0x1.C000000000000p-1, 0,  0x1.1178E8227E000p-3,  0x1.1EF78CE2D07F2p-45 },
		{ 0x1.BE00000000000p-1, 0,  0x1.1AA2B7E23F000p-3,  0x1.CA78E44389934p-45 },
		{ 0x1.BA00000000000p-1, 0,  0x1.2D1610C868000p-3,  0x1.39D6CCB81B4A1p-47 },
		{ 0x1.B800000000000p-1, 0,  0x1.365FCB0159000p-3,  0x1.62FA8234B7289p-51 },
		{ 0x1.B400000000000p-1, 0,  0x1.4913D8333B000p-3,  0x1.5837954FDB678p-45 },
		{ 0x1.B200000000000p-1, 0,  0x1.527E5E4A1B000p-3,  0x1.633E8E5697DC7p-45 },
		{ 0x1.AE00000000000p-1, 0,  0x1.6574EBE8C1000p-3,  0x1.9CF8B2C3C2E78p-46 },
		{ 0x1.AC00000000000p-1, 0,  0x1.6F0128B757000p-3, -0x1.5118DE59C21E1p-45 },
		{ 0x1.AA00000000000p-1, 0,  0x1.7898D85445000p-3, -0x1.C661070914305p-46 },
		{ 0x1.A600000000000p-1, 0,  0x1.8BEAFEB390000p-3, -0x1.73D54AAE92CD1p-47 },
		{ 0x1.A400000000000p-1, 0,  0x1.95A5ADCF70000p-3,  0x1.7F22858A0FF6Fp-47 },
		{ 0x1.A000000000000p-1, 0,  0x1.A93ED3C8AE000p-3, -0x1.8724350562169p-45 },
		{ 0x1.9E00000000000p-1, 0,  0x1.B31D8575BD000p-3, -0x1.C358D4EACE1AAp-47 },
		{ 0x1.9C00000000000p-1, 0,  0x1.BD087383BE000p-3, -0x1.D4BC4595412B6p-45 },
		{ 0x1.9A00000000000p-1, 0,  0x1.C6FFBC6F01000p-3, -0x1.1EC72C5962BD2p-48 },
		{ 0x1.9600000000000p-1, 0,  0x1.DB13DB0D49000p-3, -0x1.AFF2AF715B035p-45 },
		{ 0x1.9400000000000p-1, 0,  0x1.E530EFFE71000p-3,  0x1.212276041F430p-51 },
		{ 0x1.9200000000000p-1, 0,  0x1.EF5ADE4DD0000p-3, -0x1.A211565BB8E11p-51 },
		{ 0x1.9000000000000p-1, 0,  0x1.F991C6CB3B000p-3,  0x1.BCBECCA0CDF30p-46 },
		{ 0x1.8C00000000000p-1, 0,  0x1.07138604D5800p-2,  0x1.89CDB16ED4E91p-48 },
		{ 0x1.8A00000000000p-1, 0,  0x1.0C42D67616000p-2,  0x1.7188B163CEAE9p-45 },
		{ 0x1.8800000000000p-1, 0,  0x1.1178E8227E800p-2, -0x1.C210E63A5F01Cp-45 },
		{ 0x1.8600000000000p-1, 0,  0x1.16B5CCBACF800p-2,  0x1.B9ACDF7A51681p-45 },
		{ 0x1.8400000000000p-1, 0,  0x1.1BF99635A6800p-2,  0x1.CA6ED5147BDB7p-45 },
		{ 0x1.8200000000000p-1, 0,  0x1.214456D0EB800p-2,  0x1.A87DEBA46BAEAp-47 },
		{ 0x1.7E00000000000p-1, 0,  0x1.2BEF07CDC9000p-2,  0x1.A9CFA4A5004F4p-45 },
		{ 0x1.7C00000000000p-1, 0,  0x1.314F1E1D36000p-2, -0x1.8E27AD3213CB8p-45 },
		{ 0x1.7A00000000000p-1, 0,  0x1.36B6776BE1000p-2,  0x1.16ECDB0F177C8p-46 },
		{ 0x1.7800000000000p-1, 0,  0x1.3C25277333000p-2,  0x1.83B54B606BD5Cp-46 },
		{ 0x1.7600000000000p-1, 0,  0x1.419B423D5E800p-2,  0x1.8E436EC90E09Dp-47 },
		{ 0x1.7400000000000p-1, 0,  0x1.4718DC271C800p-2, -0x1.F27CE0967D675p-45 },
		{ 0x1.7200000000000p-1, 0,  0x1.4C9E09E173000p-2, -0x1.E20891B0AD8A4p-45 },
		{ 0x1.7000000000000p-1, 0,  0x1.522AE0738A000p-2,  0x1.EBE708164C759p-45 },
		{ 0x1.6E00000000000p-1, 0,  0x1.57BF753C8D000p-2,  0x1.FADEDEE5D40EFp-46 },
		{ 0x1.6C00000000000p-1, 0,  0x1.5D5BDDF596000p-2, -0x1.A0B2A08A465DCp-47 },
	},
};

#define powlog_T __pow_log_data.tab
#define powlog_A __pow_log_data.poly
#define powlog_Ln2hi __pow_log_data.ln2hi
#define powlog_Ln2lo __pow_log_data.ln2lo
#define powlog_OFF 0x3fe6955500000000

// Compute y+TAIL = log(x) where the rounded result is y and TAIL has about
// additional 15 bits precision.  IX is the bit representation of x, but
// normalized in the subnormal range using the sign bit for the exponent.
double_t log_inline(uint64_t doubleInt, double_t* tail)
{
	// double_t for better performance on targets with FLT_EVAL_METHOD==2.
	double_t zVar, result, invc, logc, logctail, vVar, resultHigh, tVar1, tVar2, resultLow, p;
	double_t lowPart1, lowPart2, lowPart3, lowPart4;
	double_t rVar, arVar, arVarSquared, arVarCubed;
	uint64_t zVarInt, temp;
	int vVarInt, index;
	
	// x = 2^vVar zVar; where zVar is in range [OFF,2*OFF) and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	temp = doubleInt - powlog_OFF;
	index = ((temp >> (52 - POW_LOG_TABLE_BITS)) % powlog_N);
	vVarInt = (int64_t)temp >> 52; // arithmetic shift
	zVarInt = doubleInt - (temp & 0xfffULL << 52);
	zVar = asdouble(zVarInt);
	vVar = (double_t)vVarInt;
	
	// log(x) = vVar*Ln2 + log(c) + log1p(zVar/c-1).
	invc = powlog_T[index].invc;
	logc = powlog_T[index].logc;
	logctail = powlog_T[index].logctail;
	
	// Note: 1/c is j/N or j/N/2 where j is an integer in [N,2N) and
    // |zVar/c - 1| < 1/N, so rVar = zVar/c - 1 is exactly representible.
	// Split zVar such that rhi, rlo and rhi*rhi are exact and |rlo| <= |rVar|.
	double_t zhi = asdouble((zVarInt + (1ULL << 31)) & (-1ULL << 32));
	double_t zlo = zVar - zhi;
	double_t rhi = zhi * invc - 1.0;
	double_t rlo = zlo * invc;
	rVar = rhi + rlo;
	
	// vVar*Ln2 + log(c) + rVar.
	tVar1 = (vVar * powlog_Ln2hi) + logc;
	tVar2 = tVar1 + rVar;
	lowPart1 = (vVar * powlog_Ln2lo) + logctail;
	lowPart2 = tVar1 - tVar2 + rVar;
	
	// Evaluation is optimized assuming superscalar pipelined execution.
	arVar = powlog_A[0] * rVar; // A[0] = -0.5.
	arVarSquared = rVar * arVar;
	arVarCubed = rVar * arVarSquared;
	// vVar*Ln2 + log(c) + rVar + A[0]*rVar*rVar.
	double_t arhi = powlog_A[0] * rhi;
	double_t arhi2 = rhi * arhi;
	resultHigh = tVar2 + arhi2;
	lowPart3 = rlo * (arVar + arhi);
	lowPart4 = tVar2 - resultHigh + arhi2;
	// p = log1p(rVar) - rVar - A[0]*rVar*rVar.
	p = (arVarCubed * (powlog_A[1] + rVar * powlog_A[2] + arVarSquared * (powlog_A[3] + rVar * powlog_A[4] + arVarSquared * (powlog_A[5] + rVar * powlog_A[6]))));
	resultLow = lowPart1 + lowPart2 + lowPart3 + lowPart4 + p;
	result = resultHigh + resultLow;
	*tail = resultHigh - result + resultLow;
	return result;
}

//TODO: Collapse these down to exp_N and other macros wich are already defined in math_exp_helpers.c?
#define expinline_N (1 << EXP_TABLE_BITS)
#define expinline_InvLn2N   __exp_data.invln2N
#define expinline_NegLn2hiN __exp_data.negln2hiN
#define expinline_NegLn2loN __exp_data.negln2loN
#define expinline_Shift     __exp_data.shift
#define expinline_T         __exp_data.tab
#define expinline_C2        __exp_data.poly[5 - EXP_POLY_ORDER]
#define expinline_C3        __exp_data.poly[6 - EXP_POLY_ORDER]
#define expinline_C4        __exp_data.poly[7 - EXP_POLY_ORDER]
#define expinline_C5        __exp_data.poly[8 - EXP_POLY_ORDER]
#define expinline_C6        __exp_data.poly[9 - EXP_POLY_ORDER] //TODO: Do we even use this?

// Handle cases that may overflow or underflow when computing the result that
// is scale*(1+TMP) without intermediate rounding.  The bit representation of
// scale is in SBITS, however it has a computed exponent that may have
// overflown into the sign bit so that needs to be adjusted before using it as
// a double.  (int32_t)KI is the k used in the argument reduction and exponent
// adjustment of scale, positive k here means the result may overflow and
// negative k means the result may underflow.
double specialcase(double_t tmp, uint64_t sbits, uint64_t ki)
{
	double_t scale, result;

	if ((ki & 0x80000000) == 0)
	{
		// k > 0, the exponent of scale might have overflowed by <= 460.
		sbits -= (1009ull << 52);
		scale = asdouble(sbits);
		result = 0x1p1009 * (scale + scale * tmp);
		return eval_as_double(result);
	}
	// k < 0, need special care in the subnormal range.
	sbits += (1022ull << 52);
	// Note: sbits is signed scale.
	scale = asdouble(sbits);
	result = scale + scale * tmp;
	if (fabs(result) < 1.0)
	{
		// Round result to the right precision before scaling it into the subnormal
		// range to avoid double rounding that can cause 0.5+E/2 ulp error where
		// E is the worst-case ulp error outside the subnormal range.  So this
		// is only useful if the goal is better than 1 ulp worst-case error.
		double_t highDouble, lowDouble, signedOne = 1.0;
		if (result < 0.0) { signedOne = -1.0; }
		lowDouble = scale - result + (scale * tmp);
		highDouble = signedOne + result;
		lowDouble = signedOne - highDouble + result + lowDouble;
		result = eval_as_double(highDouble + lowDouble) - signedOne;
		// Fix the sign of 0.
		if (result == 0.0) { result = asdouble(sbits & 0x8000000000000000); }
		// The underflow exception needs to be signaled explicitly.
		fp_force_eval(fp_barrier(0x1p-1022) * 0x1p-1022);
	}
	result = 0x1p-1022 * result;
	return eval_as_double(result);
}

#define expinline_SIGN_BIAS (0x800 << EXP_TABLE_BITS)

// Computes sign*exp(value+valueTail) where |valueTail| < 2^-8/N and |valueTail| <= |value|.
// The sign_bias argument is SIGN_BIAS or 0 and sets the sign to -1 or 1.
double exp_inline(double_t value, double_t valueTail, uint32_t sign_bias)
{
	uint32_t valueTopBitsAbs;
	uint64_t zVarShiftedInt, index, topBits, sbits;
	// double_t for better performance on targets with FLT_EVAL_METHOD==2.
	double_t zVarShifted, zVar, rVar, rVarSquared, scale, tail, temp;
	
	valueTopBitsAbs = (top12(value) & 0x7FF);
	if (predict_false(valueTopBitsAbs - top12(0x1p-54) >= top12(512.0) - top12(0x1p-54)))
	{
		if (valueTopBitsAbs - top12(0x1p-54) >= 0x80000000)
		{
			// Avoid spurious underflow for tiny value.
			// Note: 0 is common input.
			double_t one = 1.0;
			return sign_bias ? -one : one;
		}
		if (valueTopBitsAbs >= top12(1024.0))
		{
			// Note: inf and nan are already handled.
			if (asuint64(value) >> 63) { return __math_uflow(sign_bias); }
			else { return __math_oflow(sign_bias); }
		}
		// Large value is special cased below.
		valueTopBitsAbs = 0;
	}
	
	// exp(value) = 2^(k/N) * exp(rVar), with exp(rVar) in [2^(-1/2N),2^(1/2N)].
	// value = ln2/N*k + rVar, with int k and rVar in [-ln2/2N, ln2/2N].
	zVar = expinline_InvLn2N * value;
	// zVar - zVarShifted is in [-1, 1] in non-nearest rounding modes.
	zVarShifted = eval_as_double(zVar + expinline_Shift);
	zVarShiftedInt = asuint64(zVarShifted);
	zVarShifted -= expinline_Shift;
	rVar = value + (zVarShifted * expinline_NegLn2hiN) + (zVarShifted * expinline_NegLn2loN);
	// The code assumes 2^-200 < |valueTail| < 2^-8/N.
	rVar += valueTail;
	// 2^(k/N) ~= scale * (1 + tail).
	index = 2 * (zVarShiftedInt % expinline_N);
	topBits = ((zVarShiftedInt + sign_bias) << (52 - EXP_TABLE_BITS));
	tail = asdouble(expinline_T[index]);
	// This is only a valid scale when -1023*N < k < 1024*N.
	sbits = expinline_T[index + 1] + topBits;
	// exp(value) = 2^(k/N) * exp(rVar) ~= scale + scale * (tail + exp(rVar) - 1).
	// Evaluation is optimized assuming superscalar pipelined execution.
	rVarSquared = rVar * rVar;
	// Without fma the worst case error is 0.25/N ulp larger.
	// Worst case error is less than 0.5+1.11/N+(abs poly error * 2^53) ulp.
	temp = tail + rVar + rVarSquared * (expinline_C2 + rVar * expinline_C3) + rVarSquared * rVarSquared * (expinline_C4 + rVar * expinline_C5);
	if (predict_false(valueTopBitsAbs == 0)) { return specialcase(temp, sbits, zVarShiftedInt); }
	scale = asdouble(sbits);
	// Note: temp == 0 or |temp| > 2^-200 and scale > 2^-739, so there
	// is no spurious underflow here even without fma.
	return eval_as_double(scale + (scale * temp));
}
