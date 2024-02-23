/*
  This file is for public-key generation
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "iob-uart16550.h"

#include "controlbits.h"
#include "benes.h"
#include "crypto_declassify.h"
#include "crypto_uint64.h"
#include "params.h"
#include "pk_gen.h"
#include "root.h"
#include "uint64_sort.h"
#include "util.h"

#include "versat_accel.h"
#include "unitConfiguration.h"
#include "iob-uart.h"
#include "printf.h"
#include "arena.h"

static VectorLikeOperationConfig* vec;
static void* matAddr;

// Matrix Allocate
#define MA(Y,X,TYPE) (TYPE*) PushBytes(sizeof(TYPE) * (X) * (Y))

// Matrix Index
#define MI(Y,X,ROWSIZE) ((Y) * (ROWSIZE) + (X)) 
#define MR(Y,ROWSIZE) ((Y) * (ROWSIZE))

#define ALIGN_4(VAL) (((VAL) + 3) & (~3))
#define ALIGN_16(VAL) (((VAL) + 15) & (~15))

// Need to align
#define ASYS (SYS_N / 8) // 436 -> 448
#define SBYTE (SYS_N / 8) // 436 -> 448

//#define ASYS ALIGN_16(SYS_N / 8) // 436 -> 448
//#define SBYTE ALIGN_4(SYS_N / 8) // 436 -> 448
#define SINT (ASYS / 4) // 109 -> 112

void PrintRow(unsigned char* view){
#if 0
    for(int i = 0; i < SBYTE; i++){
        printf("%02x",view[i]);
        if(i % 16 == 0 && i != 0){
            printf("\n");
        }
    }
#else
    for(int i = 0; i < 16; i++){
        printf("%02x",view[i]);
    }
    printf("\n");
    for(int i = SBYTE - 16; i < SBYTE; i++){
        printf("%02x",view[i]);
    }
    printf("\n");
#endif
}

void PrintFullRow(unsigned char* view){
    for(int i = 0; i < SBYTE; i++){
        printf("%02x",view[i]);
        if(i % 16 == 0 && i != 0){
            printf("\n");
        }
    }
}

void ReadRow(uint32_t* row){
    for (int i = 0; i < SINT; i++){
        row[i] = VersatUnitRead((iptr) matAddr,i);
    }
}

void VersatLoadRow(uint32_t* row){
    VersatMemoryCopy(matAddr,(int*) row,SINT * sizeof(int));
}

void VersatPrintRow(){
    uint32_t values[SINT];
    ReadRow(values);
    PrintRow((uint8_t*) values);
}

uint8_t* GetVersatRowForTests(){  
    static uint8_t buffer[ASYS];
    ReadRow((uint32_t*) buffer);
    return buffer;
}

void PrintSimpleMat(unsigned char** mat,int centerRow){
    PrintRow(mat[0]);
    PrintRow(mat[centerRow - 1]);
    PrintRow(mat[centerRow]);
    PrintRow(mat[centerRow + 1]);
    PrintRow(mat[PK_NROWS - 1]);
}

void PrintFullMat(unsigned char** mat){
    printf("Printing full mat:\n");
    for(int i = 0; i < PK_NROWS; i++){
        PrintFullRow(mat[i]);
    }
}

void VersatMcElieceLoop1(uint8_t *row, uint8_t mask,bool first){
    static uint8_t savedMask = 0;
    uint32_t *row_int = (uint32_t*) row;

    ConfigureSimpleVReadShallow(&vec->row, SINT, (int*) row_int);
    if(first){
        vec->mat.in0_wr = 0;
    } else {
        uint32_t mask_int = (savedMask) | (savedMask << 8) | (savedMask << 8*2) | (savedMask << 8*3);
        vec->mask.constant = mask_int;
        vec->mat.in0_wr = 1;
    }

    EndAccelerator();
    StartAccelerator();

    savedMask = mask;
}

// Can only be called if k != row. Care
void VersatMcElieceLoop2(unsigned char** mat,int timesCalled,int k,int row,uint8_t mask){
    static uint8_t savedMask = 0;

    int toRead =    k;
    int toCompute = ((toRead - 1    == row) ? toRead - 2    : toRead - 1);
    int toWrite =   ((toCompute - 1 == row) ? toCompute - 2 : toCompute - 1);

    if(toRead < PK_NROWS){
        int *toRead_int = (int*) mat[toRead];

        vec->mat.in0_wr = 0;

        ConfigureSimpleVReadShallow(&vec->row, SINT,toRead_int);        
    } else {
        vec->row.enableRead = 0;
        toRead = -9;
    }
    
    if(timesCalled >= 1 && toCompute >= 0 && toCompute < PK_NROWS){
        uint32_t mask_int = (savedMask) | (savedMask << 8) | (savedMask << 8*2) | (savedMask << 8*3);

        vec->mask.constant = mask_int;
    } else {
        ConfigureSimpleVWrite(&vec->writer, SINT, (int*) NULL);
        vec->writer.enableWrite = 0;
        toCompute = -9;
    }
    
    if(timesCalled >= 2 && toWrite >= 0){
        int* toWrite_int = (int*) mat[toWrite];
        ConfigureSimpleVWrite(&vec->writer, SINT,toWrite_int);
    } else {
        toWrite = -9;
        vec->writer.enableWrite = 0;
    }

    EndAccelerator();
    StartAccelerator();

    savedMask = mask;
}

static crypto_uint64 uint64_is_equal_declassify(uint64_t t, uint64_t u) {
    crypto_uint64 mask = crypto_uint64_equal_mask(t, u);
    crypto_declassify(&mask, sizeof mask);
    return mask;
}

static crypto_uint64 uint64_is_zero_declassify(uint64_t t) {
    crypto_uint64 mask = crypto_uint64_zero_mask(t);
    crypto_declassify(&mask, sizeof mask);
    return mask;
}

/* input: secret key sk */
/* output: public key pk */
int pk_gen(unsigned char *pk, unsigned char *sk, const uint32_t *perm, int16_t *pi) {
    int i, j, k;
    int row, c;

    int mark = MarkArena();

    // Init needed values for versat later on.  
    vec = (VectorLikeOperationConfig*) accelConfig;
    matAddr = TOP_mat_addr;

    ActivateMergedAccelerator(VectorLikeOperation);

    ConfigureSimpleVReadBare(&vec->row);

    vec->mat.iterA = 1;
    vec->mat.incrA = 1;
    vec->mat.iterB = 1;
    vec->mat.incrB = 1;
    vec->mat.perA = SINT + 1;
    vec->mat.dutyA = SINT + 1;
    vec->mat.perB = SINT + 1;
    vec->mat.dutyB = SINT + 1;

    uint64_t buf[ 1 << GFBITS ];

    volatile unsigned char** mat = PushArray(PK_NROWS,volatile unsigned char*);
    for(int i = 0; i < PK_NROWS; i++){
        mat[i] = PushArray(SYS_N / 8,volatile unsigned char);
    }

    unsigned char mask;
    unsigned char b;

    gf* g = PushArray(SYS_T + 1,gf);
    gf* L = PushArray(SYS_N,gf); // support
    gf* inv = PushArray(SYS_N,gf);

    //

    g[ SYS_T ] = 1;

    for (i = 0; i < SYS_T; i++) {
        g[i] = load_gf(sk);
        sk += 2;
    }

    for (i = 0; i < (1 << GFBITS); i++) {
        buf[i] = perm[i];
        buf[i] <<= 31;
        buf[i] |= i;
    }

    uint64_sort(buf, 1 << GFBITS);

    for (i = 1; i < (1 << GFBITS); i++) {
        if (uint64_is_equal_declassify(buf[i - 1] >> 31, buf[i] >> 31)) {
            PopArena(mark);
            return -1;
        }
    }

    for (i = 0; i < (1 << GFBITS); i++) {
        pi[i] = buf[i] & GFMASK;
    }
    for (i = 0; i < SYS_N;         i++) {
        L[i] = bitrev(pi[i]);
    }

    // filling the matrix

    root(inv, g, L);

    for (i = 0; i < SYS_N; i++) {
        inv[i] = gf_inv(inv[i]);
    }

    for (i = 0; i < PK_NROWS; i++) {
        for (j = 0; j < SYS_N / 8; j++) {
            mat[i][j] = 0;
        }
    }

    for (i = 0; i < SYS_T; i++) {
        for (j = 0; j < SYS_N; j += 8) {
            for (k = 0; k < GFBITS;  k++) {
                b  = (inv[j + 7] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 6] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 5] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 4] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 3] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 2] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 1] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 0] >> k) & 1;

                mat[ i * GFBITS + k ][ j / 8 ] = b;
            }
        }

        for (j = 0; j < SYS_N; j++) {
            inv[j] = gf_mul(inv[j], L[j]);
        }
    }

    // gaussian elimination
    for (i = 0; i < (PK_NROWS + 7) / 8; i++) {
        for (j = 0; j < 8; j++) {
            row = i * 8 + j;

            if (row >= PK_NROWS) {
                break;
            }

#if 1
            uint32_t *out_int = (uint32_t*) mat[row];
            EndAccelerator();

            VersatLoadRow(out_int);
            bool first = true;
            for (k = row + 1; k < PK_NROWS; k++) {
                mask = mat[ row ][ i ] ^ mat[ k ][ i ];
                mask >>= j;
                mask &= 1;
                mask = -mask;

                VersatMcElieceLoop1(mat[k],mask,first);

                // We could fetch the value from Versat, but it's easier to calculate it CPU side.
                mat[row][i] ^= mat[k][i] & mask;
                first = false;
            }

            // Last run, use valid data to compute last operation
            VersatMcElieceLoop1(mat[PK_NROWS - 1],0,false); // TODO: Have a proper function instead of sending a "fake" adress

            EndAccelerator();

#else
            for (k = row + 1; k < PK_NROWS; k++) {
                mask = mat[ row ][ i ] ^ mat[ k ][ i ];
                mask >>= j;
                mask &= 1;
                mask = -mask;

                for (c = 0; c < SYS_N / 8; c++) {
                    mat[ row ][ c ] ^= mat[ k ][ c ] & mask;
                }
            }
#endif
            if ( uint64_is_zero_declassify((mat[ row ][ i ] >> j) & 1) ) { // return if not systematic
                printf("Early finish row:%d\n",row);
                PopArena(mark);
                return -1;
            }

            ReadRow(out_int);

#if 1
            int index = 0;
            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    mask = mat[k][i] >> j;
                    mask &= 1;
                    mask = -mask;

                    VersatMcElieceLoop2(mat,index,k,row,mask);
                    index += 1;
                }
            }

            VersatMcElieceLoop2(mat,index++,PK_NROWS,row,0);
            VersatMcElieceLoop2(mat,index++,PK_NROWS + 1,row,0);
            vec->writer.enableWrite = 0;

            clear_cache();
#else
            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    mask = mat[ k ][ i ] >> j;
                    mask &= 1;
                    mask = -mask;

                    for (c = 0; c < SYS_N / 8; c++) {
                        mat[ k ][ c ] ^= mat[ row ][ c ] & mask;
                    }
                }
            }
#endif
        }
    }

    for (i = 0; i < PK_NROWS; i++) {
        memcpy(pk + i * PK_ROW_BYTES, mat[i] + PK_NROWS / 8, PK_ROW_BYTES);
    }

    PopArena(mark);
    return 0;
}
