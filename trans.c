/* Authors: Theodore Bieber (tjbieber), James Honicker (jlhonicker)
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, rowBlock, colBlock;
    int diag = 0;
    int temp = 0;

    /*
    * We use blocking to maximize speed by using the L1 cache as much as possible.
    * We do this by handling n positions in the array at a time 
    * (n = 8 for N==32, n=4 for N==64, and n=16 for N==?)
    * The inner loops do the actual transposition, 
    * the outer loops increment through the blocks
    */

    if (N == 32) { // Handle the first test case 
        //                                         |-block size is 8
        for (colBlock = 0; colBlock < N; colBlock+=8) {
            for (rowBlock = 0; rowBlock < N; rowBlock+=8) {
                // ^iterate over the blocks
                for (i = rowBlock; i < rowBlock + 8; i++) {
                    for (j = colBlock; j < colBlock + 8; j++) {
                        // ^iterate over each element in the blocks,
                        // transposition time
                        if (i != j) {
                            B[j][i] = A[i][j];
                        } else {
                            temp = A[i][j];
                            diag = i;
                        }
                    }
                    
                    // we know its a square, so we don't need to move the diagonal elements
                    if (rowBlock == colBlock) {
                        B[diag][diag] = temp;
                    }
                }   
            }
        }
    } else if (N == 64) { // Second test case, block size is 4
        for (colBlock = 0; colBlock < N; colBlock+=4) {
            for (rowBlock = 0; rowBlock < N; rowBlock+=4) {
                // ^iterate over the blocks        
                for (i = rowBlock; i < rowBlock + 4; i++) {
                    for (j = colBlock; j < colBlock + 4; j++) {
                        // ^iterate over each element in the blocks,
                        // transposition time
                        if (i != j) {
                            B[j][i] = A[i][j];
                        } else {
                            temp = A[i][j];
                            diag = i;
                        }
                    }

                    // we know its a square, so we don't need to move the diagonal elements
                    if (rowBlock == colBlock) {
                        B[diag][diag] = temp;
                    }
                }   
            }
        }
    } else { // Third test case is a mystery to us
        //                                          |-block size = 16
        for (colBlock = 0; colBlock < M; colBlock+=16) {
            for (rowBlock = 0; rowBlock < N; rowBlock+=16) {       
                // Since our sizes are prime, not all blocks will be square
                // potentially, (rowBlock + 16 > N)
                // check for i, j < n, m
                for (i = rowBlock; (i < rowBlock + 16) && (i < N); i++) {
                    for (j = colBlock; (j < colBlock + 16) && (j < M); j++) {
                        if (i != j) {
                            B[j][i] = A[i][j];
                        } else {
                            temp = A[i][j];
                            diag = i;
                        }
                    }
                    // diagonals still don't need to be swapped
                    if (rowBlock == colBlock) {
                        B[diag][diag] = temp;
                    }
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
// COMMENTING THIS OUT BECAUSE I DON'T WANT TO SEE IT EVERY TIME I TEST
// char trans_desc[] = "Simple row-wise scan transpose";
// void trans(int M, int N, int A[N][M], int B[M][N]) {
//     int i, j, tmp;

//     for (i = 0; i < N; i++) {
//         for (j = 0; j < M; j++) {
//             tmp = A[i][j];
//             B[j][i] = tmp;
//         }
//     }    

// }

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

