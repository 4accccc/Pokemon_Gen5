//======================================================================
/**
 * @file  mus_mcss_debug.c
 * @brief MCSを使ってファイルを読む
 * @author  ariizumi
 * @data  09/04/17
 */
//======================================================================
#if PM_DEBUG

#include <gflib.h>
#include "test/ariizumi/ari_debug.h"
#include "test/ariizumi/mus_mcss_debug.h"
#include "debug/mcs_readfile.h"

#include "../../../../resource/musical/pokegra_mus/pokegura_mus_path.cdat"
#define FILE_LEN (1024)
void* MUS_MCSS_DebugLoadFile( const u32 fileId , u8 type , const BOOL isComp , void *dataWork , HEAPID heapId )
{
  if ( fileId >= NELEMS( pogeguraMusFileName ) )
  {
    ARI_TPrintf("MUS_MCSS FileIdError[%d]\n",fileId);
    return NULL;
  }
  {
    void *buf = NULL;
    char *fileName = GFL_HEAP_AllocMemory( GetHeapLowID(heapId) , FILE_LEN );
    
    GFL_STD_MemClear( fileName, FILE_LEN );
    GFL_STD_MemCopy( pogeguraMusPathName, fileName, sizeof(pogeguraMusPathName) );
    GFL_STD_StrCat( fileName, pogeguraMusFileName[fileId], GFL_STD_StrLen(pogeguraMusFileName[fileId]) );

    
    if( isComp == TRUE )
    {
      void *tempBuf = GF_MCS_FILE_ReadAlloc( fileName , GetHeapLowID(heapId) , 0 );
      buf = GFL_HEAP_AllocMemory( heapId, MI_GetUncompressedSize( tempBuf ) );

      MI_UncompressLZ8( tempBuf, buf );
      GFL_HEAP_FreeMemory( tempBuf );
    }
    else
    {
      buf = GF_MCS_FILE_ReadAlloc( fileName , heapId , 0 );
    }
    //ファイル別変換
    switch( type )
    {
    case MMFT_NCBR:
      {
        NNSG2dCharacterData **data = dataWork;
        if( NNS_G2dGetUnpackedBGCharacterData( buf, data ) == FALSE)
        {
          // 失敗したらNULL
          GFL_HEAP_FreeMemory( buf );
          buf = NULL;
        }
      }
      break;
    case MMFT_NCLR:
      {
        NNSG2dPaletteData **data = dataWork;
        if( NNS_G2dGetUnpackedPaletteData( buf, data ) == FALSE )
        {
          GFL_HEAP_FreeMemory( buf );
          buf = NULL;
        }
      }
      break;
    case MMFT_NCER:
      {
        NNSG2dCellDataBank **data = dataWork;
        if( NNS_G2dGetUnpackedCellBank( buf, data ) == FALSE )
        {
          GFL_HEAP_FreeMemory( buf );
          buf = NULL;
        }
      }
      break;
    case MMFT_NANR:
      {
        NNSG2dCellAnimBankData **data = dataWork;
        if( NNS_G2dGetUnpackedAnimBank( buf, data ) == FALSE )
        {
          GFL_HEAP_FreeMemory( buf );
          buf = NULL;
        }
      }
      break;
    case MMFT_NMCR:
      {
        NNSG2dMultiCellDataBank **data = dataWork;
        if( NNS_G2dGetUnpackedMultiCellBank( buf, data ) == FALSE )
        {
          GFL_HEAP_FreeMemory( buf );
          buf = NULL;
        }
      }
      break;
    case MMFT_NMAR:
      {
        NNSG2dMultiCellAnimBankData **data = dataWork;
        if( NNS_G2dGetUnpackedMCAnimBank( buf, data ) == FALSE )
        {
          GFL_HEAP_FreeMemory( buf );
          buf = NULL;
        }
      }
      break;
    case MMFT_NCEC:
      //変換無し
      break;
    case MMFT_DATA:
      //変換無し
      break;
    }

    GFL_HEAP_FreeMemory( fileName );
    return buf;
  }
}
#endif
