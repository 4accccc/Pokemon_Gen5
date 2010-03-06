/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_COMMON_NONPORT_H_
#define DWC_COMMON_NONPORT_H_

#ifdef __cplusplus
extern "C"
{
#endif

    void DWCi_Np_CpuCopy8( register const void *srcp, register void *dstp, register u32 size );
    void DWCi_Np_CpuCopy16( register const void *srcp, register void *dstp, register u32 size );
    void DWCi_Np_CpuCopy32( register const void *srcp, register void *dstp, register u32 size );
    void DWCi_Np_CpuClear8(void *dest, u32 size);
    void DWCi_Np_CpuClear16(void *dest, u32 size);
    void DWCi_Np_CpuClear32(void *dest, u32 size);

    void DWCi_Np_GetMacAddress(u8 *macAddress);

// ライブラリの初期化状態をチェックするマクロ
    void DWCi_Np_SetInitFlag(BOOL initialzied);
    BOOL DWCi_Np_GetInitFlag();

#ifdef RVLDWC_DEBUG
#define DWC_ASSERT_IF_NOT_INITIALIZED() DWC_ASSERTMSG(DWCi_Np_GetInitFlag(), "DWC hasn't initialized yet.");
#else
#define DWC_ASSERT_IF_NOT_INITIALIZED()
#endif

    /* コンパイル時チェック */
#define DWC_COMPILER_ASSERT(expr) extern void compiler_assert ( char is[(expr) ? +1 : -1] )


    static DWC_INLINE void DWCi_Np_ToLE( const void *srcp, void *dstp, u32 size )
    {
        u32 i;

        for ( i=0; i<size/4; i++ )
        {

            ((u32*)dstp)[i] = DWCi_HtoLEl( ((u32*)srcp)[i] );

        }

    }

#ifdef __cplusplus
}
#endif



#endif // DWC_COMMON_NONPORT_H_
