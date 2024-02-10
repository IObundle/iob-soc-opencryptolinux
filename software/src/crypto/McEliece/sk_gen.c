/*
  This file is for secret-key generation
*/

#include "sk_gen.h"

#include "controlbits.h"
#include "gf.h"
#include "params.h"
#include "util.h"

#ifndef PC
#include "printf.h"
#else
#include <stdio.h>
#endif
#include "arena.h"

/* input: f, element in GF((2^m)^t) */
/* output: out, minimal polynomial of f */
/* return: 0 for success and -1 for failure */
int PQCLEAN_MCELIECE348864_CLEAN_genpoly_gen(gf *out, gf *f) {
    int i, j, k, c;

    int mark = MarkArena();
    
    gf *mat = (gf*) PushBytes(((SYS_T + 1)*(SYS_T))*sizeof(gf));
    gf mask, inv, t;

    // fill matrix
    mat[0*SYS_T+0] = 1;

    for (i = 1; i < SYS_T; i++) {
        mat[0*SYS_T+i] = 0;
    }

    for (i = 0; i < SYS_T; i++) {
        mat[1*SYS_T+i] = f[i];
    }

    for (j = 2; j <= SYS_T; j++) {
        // printf("\t\trow %d\n", j);
        PQCLEAN_MCELIECE348864_CLEAN_GF_mul(&(mat[j*SYS_T]), &(mat[(j - 1)*SYS_T]), f);
    }

    // gaussian
    for (j = 0; j < SYS_T; j++) {
        for (k = j + 1; k < SYS_T; k++) {
            mask = PQCLEAN_MCELIECE348864_CLEAN_gf_iszero(mat[ j*SYS_T + j ]);

            for (c = j; c < SYS_T + 1; c++) {
                mat[ c*SYS_T + j ] ^= mat[ c*SYS_T + k ] & mask;
            }

        }

        if ( mat[ j*SYS_T + j ] == 0 ) { // return if not systematic
            PopArena(mark);
            return -1;
        }

        inv = PQCLEAN_MCELIECE348864_CLEAN_gf_inv(mat[j*SYS_T+j]);

        for (c = j; c < SYS_T + 1; c++) {
            mat[ c*SYS_T + j ] = PQCLEAN_MCELIECE348864_CLEAN_gf_mul(mat[ c*SYS_T + j ], inv) ;
        }

        for (k = 0; k < SYS_T; k++) {
            if (k != j) {
                t = mat[ j*SYS_T + k ];

                for (c = j; c < SYS_T + 1; c++) {
                    mat[ c*SYS_T + k ] ^= PQCLEAN_MCELIECE348864_CLEAN_gf_mul(mat[ c*SYS_T + j ], t);
                }
            }
        }
    }

    for (i = 0; i < SYS_T; i++) {
        out[i] = mat[ SYS_T*SYS_T + i ];
    }

    PopArena(mark);
    return 0;
}

/* input: permutation p represented as a list of 32-bit intergers */
/* output: -1 if some interger repeats in p */
/*          0 otherwise */
int PQCLEAN_MCELIECE348864_CLEAN_perm_check(const uint32_t *p) {
    int i;
    int mark = MarkArena();

    uint64_t *list = (uint64_t*) PushBytes((1 << GFBITS)*sizeof(uint64_t));

    for (i = 0; i < (1 << GFBITS); i++) {
        list[i] = p[i];
    }

    PQCLEAN_MCELIECE348864_CLEAN_sort_63b(1 << GFBITS, list);

    for (i = 1; i < (1 << GFBITS); i++) {
        if (list[i - 1] == list[i]) {
            return -1;
        }
    }

    PopArena(mark);
    return 0;
}

