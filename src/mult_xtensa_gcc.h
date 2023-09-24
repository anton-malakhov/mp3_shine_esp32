#include <stdint.h>

#if 0
// steals a lot of performance
#define mul(x,y) ({ \
    register int32_t result; \
    asm ("mulsh %0, %2, %1" : "=r" (result) : "r" (x), "r" (y)); \
    result ;\
})

//Not sure about this
#define muls(x,y) ({ \
    register int32_t result; \
    asm ( \
    	"mulsh %0, %2, %1\n\t" \
    	"add %0, %0, %0" \
    	: "=r" (result) : "r" (x), "r" (y)); \
    result ;\
})
#endif

//no rounding shortcut
#define mulr(x,y) ({ \
    register int32_t result; \
    asm ("mulsh %0, %2, %1" : "=r" (result) : "r" (x), "r" (y)); \
    result ;\
})

//no rounding shortcut
#define mulsr(x,y) ({ \
    register int32_t result; \
    asm ( \
        "mulsh %0, %2, %1\n\t" \
        "add %0, %0, %0" \
        : "=r" (result) : "r" (x), "r" (y)); \
    result ;\
})

#if 0
// steals a lot of performance
#define mul0(hi,lo,a,b)     (((void)lo),(hi)  = mul((a), (b)))

// This didn't seem to help either
#define muladd(hi, lo, x, y) ({ \
    asm ( \
    		"mulsh a7, %2, %1\n\t" \
    		"add %0, %0, a7\n\t" \
    		: "+r" (hi) : "r" (x), "r" (y) \
    		: "a7");\
})

#define mulsub(hi, lo, x, y) ({ \
    asm ( \
            "mulsh a8, %2, %1\n\t" \
            "sub %0, %0, a8\n\t" \
            : "+r" (hi) : "r" (x), "r" (y) \
            : "a8");\
})

#define mulz(hi,lo)
#endif