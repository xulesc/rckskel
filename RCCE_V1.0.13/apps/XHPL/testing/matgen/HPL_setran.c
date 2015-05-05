/* 
 * -- High Performance Computing Linpack Benchmark (HPL)                
 *    HPL - 2.0 - September 10, 2008                          
 *    Antoine P. Petitet                                                
 *    University of Tennessee, Knoxville                                
 *    Innovative Computing Laboratory                                 
 *    (C) Copyright 2000-2008 All Rights Reserved                       
 *                                                                      
 * -- Copyright notice and Licensing terms:                             
 *                                                                      
 * Redistribution  and  use in  source and binary forms, with or without
 * modification, are  permitted provided  that the following  conditions
 * are met:                                                             
 *                                                                      
 * 1. Redistributions  of  source  code  must retain the above copyright
 * notice, this list of conditions and the following disclaimer.        
 *                                                                      
 * 2. Redistributions in binary form must reproduce  the above copyright
 * notice, this list of conditions,  and the following disclaimer in the
 * documentation and/or other materials provided with the distribution. 
 *                                                                      
 * 3. All  advertising  materials  mentioning  features  or  use of this
 * software must display the following acknowledgement:                 
 * This  product  includes  software  developed  at  the  University  of
 * Tennessee, Knoxville, Innovative Computing Laboratory.             
 *                                                                      
 * 4. The name of the  University,  the name of the  Laboratory,  or the
 * names  of  its  contributors  may  not  be used to endorse or promote
 * products  derived   from   this  software  without  specific  written
 * permission.                                                          
 *                                                                      
 * -- Disclaimer:                                                       
 *                                                                      
 * THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
 * OR  CONTRIBUTORS  BE  LIABLE FOR ANY  DIRECT,  INDIRECT,  INCIDENTAL,
 * SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES  (INCLUDING,  BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA OR PROFITS; OR BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 * ---------------------------------------------------------------------
 */ 
/*
 * Include files
 */
#include "hpl.h"
/*
 * ---------------------------------------------------------------------
 * Static variables
 * ---------------------------------------------------------------------
 */
static int       ias[2], ics[2], irand[2];
#ifdef _OPENMP
#pragma omp threadprivate(ias,ics,irand)
#endif

#ifdef STDC_HEADERS
void HPL_setran
(
   const int                        OPTION,
   int *                            IRAN
)
#else
void HPL_setran
( OPTION, IRAN )
   const int                        OPTION;
   int *                            IRAN;
#endif
{
/* 
 * Purpose
 * =======
 *
 * HPL_setran initializes  the random generator with the encoding of the
 * first number X(0) in the sequence,  and the constants a and c used to
 * compute the next element in the sequence: X(n+1) = a*X(n) + c.  X(0),
 * a and c are stored in the static variables  irand, ias and ics.  When
 * OPTION is 0 (resp. 1 and 2),  irand  (resp. ia and ic)  is set to the
 * values of the input array IRAN.  When OPTION is 3, IRAN is set to the
 * current value of irand, and irand is then incremented.
 *
 * Arguments
 * =========
 *
 * OPTION  (local input)                 const int
 *         On entry, OPTION  is an integer that specifies the operations
 *         to be performed on the random generator as specified above.
 *
 * IRAN    (local input/output)          int *
 *         On entry,  IRAN is an array of dimension 2, that contains the
 *         16-lower and 15-higher bits of a random number.
 *
 * ---------------------------------------------------------------------
 */ 
/*
 * .. Local Variables ..
 */
   int                        j[2];
/* ..
 * .. Executable Statements ..
 */
   if(      OPTION == 3 )
   {                                       /* return current value */
      IRAN[0] = irand[0]; IRAN[1] = irand[1];
      HPL_lmul( irand, ias, j );         /* j     = irand * ias;   */
      HPL_ladd( j, ics, irand );         /* irand = j     + ics;   */
   } 
   else if( OPTION == 0 ) { irand[0] = IRAN[0]; irand[1] = IRAN[1]; }
   else if( OPTION == 1 ) { ias  [0] = IRAN[0]; ias  [1] = IRAN[1]; }
   else if( OPTION == 2 ) { ics  [0] = IRAN[0]; ics  [1] = IRAN[1]; }
/*
 * End of HPL_setran
 */
}
