/* Copyright (c) 2014, Cisco Systems, INC
   Written by XiangMingZhu WeiZhou MinPeng YanWang

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cpu_support.h"
#include "macros.h"
#include "main.h"
#include "pitch.h"
#include "x86_cpu.h"

#ifdef _WIN32

#include <intrin.h>
#define cpuid(info,x) __cpuid(info,x)
#else

#if defined(CPU_INFO_BY_C)
#include <cpuid.h>
#endif

static void cpuid(unsigned int CPUInfo[4], unsigned int InfoType)
{
#if defined(CPU_INFO_BY_ASM)
    __asm__ __volatile__ (
        "cpuid":
        "=a" (CPUInfo[0]),
        "=b" (CPUInfo[1]),
        "=c" (CPUInfo[2]),
        "=d" (CPUInfo[3]) :
        "a" (InfoType), "c" (0)
    );
#elif defined(CPU_INFO_BY_C)
    __get_cpuid(InfoType, &(CPUInfo[0]), &(CPUInfo[1]), &(CPUInfo[2]), &(CPUInfo[3]));
#endif
}

#endif

#include "SigProc_FIX.h"
#include "celt_lpc.h"

typedef struct CPU_Feature{
    /*  SIMD: 128-bit */
    int HW_SSE2;
    int HW_SSE41;
} CPU_Feature;

#define OPUS_DEFAULT 0
#define OPUS_SSE2    1
#define OPUS_SSE4_1  2

static void opus_cpu_feature_check(CPU_Feature *cpu_feature)
{
    unsigned int info[ 4 ] = {0, 0, 0, 0};
    unsigned int nIds = 0;
    unsigned int nExIds = 0;

    cpuid( info, 0 );
    nIds = info[ 0 ];
    cpuid( info, 0x80000000 );
    nExIds = info[ 0 ];

    if ( nIds >= 0x00000001 ){
        cpuid( info, 0x00000001 );
        cpu_feature->HW_SSE2   = ( info[ 3 ] & ( (int)1 << 26 ) ) != 0;
        cpu_feature->HW_SSE41  = ( info[ 2 ] & ( (int)1 << 19 ) ) != 0;
    }
}

int opus_select_arch(void)
{
    CPU_Feature cpu_feature = { 0, 0 };

    opus_cpu_feature_check(&cpu_feature);

    if ( cpu_feature.HW_SSE41)
    {
        return OPUS_SSE4_1;
    }
    else if (cpu_feature.HW_SSE2)
    {
       return OPUS_SSE2;
    }

    return OPUS_DEFAULT;
}

