//======================================================================
/**
 * @file  mus_mcss_debug.c
 * @brief MCSを使ってファイルを読む
 * @author  ariizumi
 * @data  09/04/17
 */
//======================================================================
#pragma once
#if PM_DEBUG

enum
{
  MMFT_NCBR,
  MMFT_NCLR,
  MMFT_NCER,
  MMFT_NANR,
  MMFT_NMCR,
  MMFT_NMAR,
  MMFT_NCEC,

  MMFT_DATA,  //変換無し
};

extern void* MUS_MCSS_DebugLoadFile( const u32 fileId , u8 type , const BOOL isComp , void *dataWork , HEAPID heapId );
#endif //PM_DEBUG
