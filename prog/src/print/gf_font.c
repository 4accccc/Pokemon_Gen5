//=============================================================================================
/**
 * @file  gf_font.c
 * @brief フォントデータマネージャ
 * @author  taya
 *
 * @date  2005.09.14  作成
 * @date  2008.10.11  従来の独自フォーマットからNitroFont(2bit)フォーマットに変更
 * @date  2009.08.26  NitroFontから再び独自フォーマットへ（常駐テーブル圧縮のため）
 */
//=============================================================================================
#include  <assert.h>
#include  <heap.h>
#include  <heapsys.h>
#include  <arc_tool.h>

#include  "print/gf_font.h"

enum {
  DEBUG_PRINT_FLAG = 0,
};

typedef u8 (*pWidthGetFunc)( const GFL_FONT*, u32 );
typedef void (*pGetBitmapFunc)(const GFL_FONT*, u32, void*, GFL_FONT_SIZE* );
typedef void (*pDotExpandFunc)( const u8* glyphSrc, u16 glyph2ndCharBits, u8* dst );

enum {
  SRC_CHAR_SIZE = 0x10,
  SRC_CHAR_MAX = 4,

  SRC_CHAR_MAXSIZE = SRC_CHAR_SIZE*SRC_CHAR_MAX,
};


/// デフォルトカラー設定
enum {
  COLOR_DEFAULT_LETTER    = 0x01,
  COLOR_DEFAULT_SHADOW    = 0x02,
  COLOR_DEFAULT_BACKGROUND  = 0x00,
};

typedef struct {

  ARCHANDLE*    file_handle;
  u8            read_buffer[SRC_CHAR_MAXSIZE];


}FILEREAD_TYPE_WORK;



/**
 *  CodeMap
 */
typedef struct {
  u16  codeBegin;
  u16  codeEnd;
  u16  mappingMethod;
  u16  reserved;
  u32  nextOfs;
  u16  mapInfo[0];
}NNSCodeMap;

typedef struct {
  u16   indexBegin;
  u16   indexEnd;
  u32   nextOfs;
}NNSFontWidth;

typedef struct {
  u8    cellWidth;
  u8    cellHeight;
  u16   cellSize;
  s8    baselinePos;
  u8    maxCharWidth;
  u8    bpp;
  u8    flags;
}NNSGlyphInfo;

typedef struct {
  u8    fontType;
  u8    linefeed;
  u16   alterCharIndex;

  s8    leftWidth;
  u8    glyphWidth;
  s8    totalWidth;
  u8    encoding;

  u32   ofsGlyph;
  u32   ofsWidth;
  u32   ofsMap;

}NNSFontInfo;

typedef struct {
  const u8* src;
  u32     rem_bits;
  u8      read_bit;
}BIT_READER;


typedef struct {
  u8    lotsWidth[ 4 ];   ///< 多い幅ベスト３（4番目はパディング用で常にゼロ）
  u32   bitTableOfs;      ///< ビットテーブル部までのオフセット
  u32   hashTableOfs;     ///< ハッシュテーブル部までのオフセット
  u32   hashChainOfs;     ///< 重複ハッシュチェイン部までのオフセット
}GFL_FONT_WIDTH_HEADER;

//------------------------------------------------------------------
/**
 * マネージャワーク
 */
//------------------------------------------------------------------
struct  _GFL_FONT {
  GFL_FONT_LOADTYPE        loadType;
  pGetBitmapFunc           GetBitmapFunc;
  pDotExpandFunc           DotExpandFunc;

  u8*                      fontBitData;
  u8                       readBuffer[SRC_CHAR_MAXSIZE];

  ARCHANDLE*               fileHandle;
  u32                      arcDatID;

  NNSFontInfo              fontHeader;
  BOOL                     fixedFontFlag;
  pWidthGetFunc            WidthGetFunc;

  NNSGlyphInfo  glyphInfo;
  u16           glyphRemBits;
  u8            glyphCharW;
  u8            glyphCharH;
  u32           fontDataImgOfs;

  GFL_FONT_WIDTH_HEADER*   widthTblTop;
  NNSCodeMap*       codeMapTop;
  u32               ofsGlyphTop;
  u8*               glyphBuf;
};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void load_font_header( GFL_FONT* wk, u32 datID, BOOL fixedFontFlag, HEAPID heapID );
static void unload_font_header( GFL_FONT* wk );
static void setup_font_datas( GFL_FONT* wk, GFL_FONT_LOADTYPE loadType, HEAPID heapID );
static void setup_type_on_memory( GFL_FONT* wk, u8 cellW, u8 cellH, HEAPID heapID );
static void setup_type_read_file( GFL_FONT* wk, u8 cellW, u8 cellH, HEAPID heapID );
static void cleanup_font_datas( GFL_FONT* wk );
static void cleanup_type_on_memory( GFL_FONT* wk );
static void cleanup_type_read_file( GFL_FONT* wk );
static u32 get_glyph_index( const GFL_FONT* wk, u32 code );
static void GetBitmapOnMemory( const GFL_FONT* wk, u32 index, void* dst, GFL_FONT_SIZE* size );
static void GetBitmapFileRead( const GFL_FONT* wk, u32 index, void* dst, GFL_FONT_SIZE* size );
static void getBitmapCommon( const GFL_FONT* wk, const u8* glyphBuf, u32 glyphRemBits, void* dst, GFL_FONT_SIZE* dstSize );
static inline void expand_ntr_glyph_block( const u8* srcData, u16 blockPos, u16 cellWidth, BOOL maskFlag, u8* dst );
static u8 GetWidthProportionalFont( const GFL_FONT* wk, u32 glyphIndex );
static u8 GetWidthFixedFont( const GFL_FONT* wk, u32 index );
static inline void ExpandFontData( const void* src_p, void* dst_p );
static inline void BitReader_Init( BIT_READER* br, const u8* src );
static inline void BitReader_SetNextBit( BIT_READER* br );
static inline u8 BitReader_Read( BIT_READER* br, u8 bits );
static inline void dotExpand_1x1( const u8* glyphSrc, u16 remBits, u8* dst );
static inline void dotExpand_2x2( const u8* glyphSrc, u16 remBits, u8* dst );



//=============================================================================================
/**
 * フォントデータハンドラ作成
 *
 * @param   arcID     フォントデータが含まれるアーカイブID
 * @param   datID     フォントデータのアーカイブ内ID
 * @param   loadType    フォントデータ読み出し方式
 * @param   fixedFontFlag 等幅フォントとして扱うためのフラグ（TRUEなら等幅）
 * @param   heapID      ハンドラ生成用ヒープID
 *
 * @retval  GFL_FONT*   フォントデータハンドラ
 */
//=============================================================================================
GFL_FONT* GFL_FONT_Create( u32 arcID, u32 datID, GFL_FONT_LOADTYPE loadType, BOOL fixedFontFlag, HEAPID heapID )
{
  GFL_FONT* wk = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_FONT) );
  if( wk )
  {
    wk->fileHandle = GFL_ARC_OpenDataHandle( arcID, heapID );
    wk->arcDatID = datID;
    load_font_header( wk, datID, fixedFontFlag, heapID );
    setup_font_datas( wk, loadType, heapID );
  }
  return wk;
}

//=============================================================================================
/**
 * フォントデータハンドラ削除
 *
 * @param   wk    フォントデータハンドラ
 *
 */
//=============================================================================================
void GFL_FONT_Delete( GFL_FONT* wk )
{
  cleanup_font_datas( wk );
  unload_font_header( wk );
  GFL_HEAP_FreeMemory( wk );
}

//------------------------------------------------------------------
/**
 * 両タイプで共有するヘッダデータを読み込み・構築
 *
 * @param   wk        ワークポインタ
 * @param   datID     フォントファイルのデータID
 * @param   fixedFontFlag 固定フォントフラグ
 * @param   heapID      ヒープID
 *
 */
//------------------------------------------------------------------
static void load_font_header( GFL_FONT* wk, u32 datID, BOOL fixedFontFlag, HEAPID heapID )
{
  if( wk->fileHandle )
  {
    GFL_ARC_LoadDataOfsByHandle( wk->fileHandle, datID, 24, sizeof(wk->fontHeader), &(wk->fontHeader) );
    GFL_ARC_LoadDataImgofsByHandle( wk->fileHandle, datID, &wk->fontDataImgOfs );

    wk->fixedFontFlag = fixedFontFlag;

    // WidthTable
    if( fixedFontFlag )
    {
      wk->widthTblTop = NULL;
      wk->WidthGetFunc = GetWidthFixedFont;
    }
    else
    {
      u32 widthTblSize = wk->fontHeader.ofsMap - wk->fontHeader.ofsWidth;

      wk->widthTblTop = GFL_HEAP_AllocMemory( heapID, widthTblSize );
      wk->WidthGetFunc = GetWidthProportionalFont;

      GFL_ARC_LoadDataOfsByHandle( wk->fileHandle, datID, wk->fontHeader.ofsWidth,
            widthTblSize, (void*)(wk->widthTblTop) );

      #if DEBUG_PRINT_FLAG
      OS_TPrintf("[GF_FONT] WidthTableSize=%08x bytes\n", widthTblSize);
      #endif
    }

    // GlyphInfo
    GFL_ARC_LoadDataOfsByHandle( wk->fileHandle, datID, wk->fontHeader.ofsGlyph,
          sizeof(wk->glyphInfo), (void*)(&wk->glyphInfo) );
    wk->glyphCharW = wk->glyphInfo.cellWidth  /8  + (wk->glyphInfo.cellWidth  % 8 != 0);
    wk->glyphCharH = wk->glyphInfo.cellHeight /8  + (wk->glyphInfo.cellHeight % 8 != 0);
    wk->glyphRemBits = (wk->glyphInfo.cellWidth * 2) % 8;
    if( wk->glyphRemBits == 0 ){ wk->glyphRemBits = 8; }
    OS_TPrintf("[GF_FONT] CellW=%d, RemBits=%d\n", wk->glyphInfo.cellWidth, wk->glyphRemBits);

    wk->ofsGlyphTop = wk->fontHeader.ofsGlyph + sizeof(NNSGlyphInfo);
    wk->glyphBuf = GFL_HEAP_AllocMemory( heapID, wk->glyphInfo.cellSize );

    // Glyph index table
    {
      u32  mapTblSize = GFL_ARC_GetDataSizeByHandle( wk->fileHandle, datID ) - wk->fontHeader.ofsMap;
      wk->codeMapTop = GFL_HEAP_AllocMemory( heapID, mapTblSize );
      GFL_ARC_LoadDataOfsByHandle( wk->fileHandle, datID, wk->fontHeader.ofsMap,
            mapTblSize, (void*)(wk->codeMapTop) );

      #if DEBUG_PRINT_FLAG
      OS_TPrintf("[GF_FONT] MapIdxTableSize=%08x bytes\n", mapTblSize);
      OS_TPrintf("[GF_FONT] WidthTableOfs  = %08x\n", wk->fontHeader.ofsWidth);
      OS_TPrintf("[GF_FONT] MapIdxTableOfs = %08x\n", wk->fontHeader.ofsMap);
      #endif
    }
  }
}
//------------------------------------------------------------------
/**
 * 構築したヘッダデータの解放
 *
 * @param   wk    ワークポインタ
 *
 */
//------------------------------------------------------------------
static void unload_font_header( GFL_FONT* wk )
{
  if( wk->glyphBuf )
  {
    GFL_HEAP_FreeMemory( wk->glyphBuf );
  }
  if( wk->widthTblTop )
  {
    GFL_HEAP_FreeMemory( wk->widthTblTop );
  }
  if( wk->codeMapTop )
  {
    GFL_HEAP_FreeMemory( wk->codeMapTop );
  }
  if( wk->fileHandle )
  {
    GFL_ARC_CloseDataHandle( wk->fileHandle );
  }
}



//------------------------------------------------------------------
/**
 * 管理するフォントデータの読み込み処理
 *
 * @param   wk        マネージャワークポインタ
 * @param   loadType    読み込みタイプ
 * @param   heapID      常駐タイプの場合、フォントビットデータ確保用ヒープの指定。
 *              ファイルリードタイプの場合、使用しない
 *
 */
//------------------------------------------------------------------
static void setup_font_datas( GFL_FONT* wk, GFL_FONT_LOADTYPE loadType, HEAPID heapID )
{
  static void (* const setup_func[])( GFL_FONT*, u8, u8, HEAPID ) = {
    setup_type_read_file,
    setup_type_on_memory,
  };

//  GF_ASSERT(loadType == GFL_FONT_LOADTYPE_FILE);

  wk->loadType = loadType;

  setup_func[loadType]( wk, wk->glyphCharW, wk->glyphCharH, heapID );
}
//--------------------------------------------------------------------------
/**
 * 管理するフォントデータの読み込み処理（ビットデータ常駐タイプ）
 *
 * @param   wk      マネージャワークポインタ
 * @param   cellW   読み出すビットデータの横キャラ数
 * @param   cellH   読み出すビットデータの縦キャラ数
 * @param   heapID    ヒープID
 *
 */
//--------------------------------------------------------------------------
static void setup_type_on_memory( GFL_FONT* wk, u8 cellW, u8 cellH, HEAPID heapID )
{
  #if 1
//  void* fontData = ArcUtil_Load( arcID, datID, FALSE, heapID, ALLOC_TOP );
  u32  fileSize, bitDataSize;

  fileSize = GFL_ARC_GetDataSizeByHandle( wk->fileHandle, wk->arcDatID );
  bitDataSize = fileSize - wk->ofsGlyphTop;

  #if DEBUG_PRINT_FLAG
  OS_TPrintf("[メモリ常駐型フォント] FileSize=0x%08x, Bitデータサイズ=0x%08xbyte\n", fileSize, bitDataSize);
  #endif

  wk->fontBitData = GFL_HEAP_AllocMemory( heapID, bitDataSize );

  GFL_ARC_LoadDataOfsByHandle( wk->fileHandle, wk->arcDatID, wk->ofsGlyphTop,
            bitDataSize, wk->fontBitData );

  #if DEBUG_PRINT_FLAG
  OS_TPrintf("[メモリ常駐型フォント] Bitデータ読み込み完了\n");
  #endif


  wk->GetBitmapFunc = GetBitmapOnMemory;
  if( cellW==1 && cellH==1 )
  {
    wk->DotExpandFunc = dotExpand_1x1;
  }
  else if( cellW==2 && cellH==2 )
  {
    wk->DotExpandFunc = dotExpand_2x2;
  }

  #else
  // WBではオンメモリ処理をしないと思われるため未実装。
  // とりあえずファイル読み込みタイプと同じ処理をしておく
  setup_type_read_file( wk, cellW, cellH, heapID );
  #endif
}
//--------------------------------------------------------------------------
/**
 * 管理するフォントデータの読み込み処理（ビットデータ逐次読み出しタイプ）
 *
 * @param   wk      マネージャワークポインタ
 * @param   cellW   読み出すビットデータの横キャラ数
 * @param   cellH   読み出すビットデータの縦キャラ数
 * @param   heapID    ヒープID
 *
 */
//--------------------------------------------------------------------------
static void setup_type_read_file( GFL_FONT* wk, u8 cellW, u8 cellH, HEAPID heapID )
{
  wk->fontBitData = NULL;
  wk->GetBitmapFunc = GetBitmapFileRead;

  if( cellW==1 && cellH==1 )
  {
    wk->DotExpandFunc = dotExpand_1x1;
  }
  else if( cellW==2 && cellH==2 )
  {
    wk->DotExpandFunc = dotExpand_2x2;
  }
}

//------------------------------------------------------------------
/**
 * 読み込んだフォントデータの破棄
 *
 * @param   wk    フォントデータマネージャポインタ
 *
 */
//------------------------------------------------------------------
static void cleanup_font_datas( GFL_FONT* wk )
{
  static void (* const cleanup_func[])( GFL_FONT* ) = {
    cleanup_type_read_file,
    cleanup_type_on_memory,
  };

  cleanup_func[ wk->loadType ]( wk );
}
//------------------------------------------------------------------
/**
 * 読み込んだフォントデータの破棄（ビットデータ常駐タイプ）
 *
 * @param   wk    フォントデータマネージャポインタ
 *
 */
//------------------------------------------------------------------
static void cleanup_type_on_memory( GFL_FONT* wk )
{
  GFL_HEAP_FreeMemory( wk->fontBitData );
  wk->fontBitData = NULL;
}
//------------------------------------------------------------------
/**
 * 読み込んだフォントデータの破棄（ビットデータ逐次読み込みタイプ）
 *
 * @param   wk    フォントデータマネージャポインタ
 *
 */
//------------------------------------------------------------------
static void cleanup_type_read_file( GFL_FONT* wk )
{

}



//==============================================================================================
//==============================================================================================


//==============================================================================================
/*
 *  文字ビットマップデータ取得
 *
 * @param wk      [in]  フォントデータマネージャ
 * @param code    [in]  文字コード
 * @param dst     [out] ビットマップデータ取得バッファ
 * @param size    [out] 文字ドットサイズ情報取得用構造体アドレス
 *
 */
//==============================================================================================
void GFL_FONT_GetBitMap( const GFL_FONT* wk, STRCODE code, void* dst, GFL_FONT_SIZE* size )
{
  u32 index;

  index = get_glyph_index( wk, code );
  wk->GetBitmapFunc( wk, index, dst, size );
}


//--------------------------------------------------------------------------
/**
 * 文字コード→グリフインデックス
 *
 * @param   wk      [in] フォントデータマネージャ
 * @param   code    [in] 文字コード（UTF-16）
 *
 * @retval  u32     グリフインデックス
 */
//--------------------------------------------------------------------------
static u32 get_glyph_index( const GFL_FONT* wk, u32 code )
{
  enum {
    PRINTFLAG = FALSE,
  };

  const NNSCodeMap* pMap = wk->codeMapTop;


  while( 1 )
  {
    if( (pMap->codeBegin <= code) && (pMap->codeEnd >= code) )
    {
      switch( pMap->mappingMethod ){
      case NNS_G2D_MAPMETHOD_DIRECT:
        return ( code - pMap->codeBegin ) + pMap->mapInfo[0];

      case NNS_G2D_MAPMETHOD_TABLE:
      {
        u16 aidx = code - pMap->codeBegin;
        return pMap->mapInfo[aidx];
      }

      case NNS_G2D_MAPMETHOD_SCAN:
      {
        u16 numEntries;
        int min, max, mid, midIdx;

        numEntries = pMap->mapInfo[0];
        min = 1;
        max = numEntries;

        while( min <= max )
        {
          mid = (min + (max - min) / 2);
          midIdx = 1 + mid * 2;

          if( pMap->mapInfo[midIdx] < code )
          {
            min = mid + 1;
            continue;
          }
          if( code < pMap->mapInfo[midIdx] )
          {
            max = mid - 1;
            continue;
          }
          return pMap->mapInfo[midIdx+1];
        }
      }
      break;

      default:
        GF_ASSERT(0);
        break;
      }

      return 31;
    }
    else if( pMap->nextOfs )
    {
      u32 ofs = pMap->nextOfs - wk->fontHeader.ofsMap;
      pMap = (const NNSCodeMap*)((u32)(wk->codeMapTop) + ofs);
    }
    else
    {
      return 0;
    }
  }
}



//--------------------------------------------------------------------------
/**
 * 文字ビットマップデータ取得処理（ビットデータ常駐タイプ）
 *
 * @param   wk      [in]  ワークポインタ
 * @param   index   [in]  グリフインデックス
 * @param   dst     [out] ビットデータ読み込み先バッファ
 * @param   size    [out] ビットマップサイズデータ読み込み構造体アドレス
 *
 */
//--------------------------------------------------------------------------
static void GetBitmapOnMemory( const GFL_FONT* wk, u32 index, void* dst, GFL_FONT_SIZE* size )
{
  u32 ofs = index * wk->glyphInfo.cellSize;

  GFL_STD_MemCopy( &(wk->fontBitData[ ofs ]), (void*)(wk->glyphBuf), wk->glyphInfo.cellSize );
//  GFL_ARC_SeekDataByHandle( wk->fileHandle, wk->fontDataImgOfs+ofsTop );
//  GFL_ARC_LoadDataByHandleContinue( wk->fileHandle, wk->glyphInfo.cellSize, (void*)(wk->glyphBuf) );

  getBitmapCommon( wk, wk->glyphBuf, wk->glyphRemBits, dst, size );
}

//--------------------------------------------------------------------------
/**
 * 文字ビットマップデータ取得処理（ビットデータ逐次読み込みタイプ）
 *
 * @param   wk      [in]  ワークポインタ
 * @param   index   [in]  グリフインデックス
 * @param   dst     [out] ビットデータ読み込み先バッファ
 * @param   size    [out] ビットマップサイズデータ読み込み構造体アドレス
 *
 */
//--------------------------------------------------------------------------
static void GetBitmapFileRead( const GFL_FONT* wk, u32 index, void* dst, GFL_FONT_SIZE* size )
{
  u32 ofsTop = wk->ofsGlyphTop + index * wk->glyphInfo.cellSize;

  GFL_ARC_SeekDataByHandle( wk->fileHandle, wk->fontDataImgOfs+ofsTop );
  GFL_ARC_LoadDataByHandleContinue( wk->fileHandle, wk->glyphInfo.cellSize, (void*)(wk->glyphBuf) );

  getBitmapCommon( wk, wk->glyphBuf, wk->glyphRemBits, dst, size );

#if 0
  wk->DotExpandFunc( &wk->glyphBuf[3], wk->glyphRemBits, dst );

  size->left_width  = wk->glyphBuf[0];
  size->glyph_width = wk->glyphBuf[1];
  size->width       = wk->glyphBuf[2];
  size->height      = wk->fontHeader.linefeed;
#endif
}

static void getBitmapCommon( const GFL_FONT* wk, const u8* glyphBuf, u32 glyphRemBits, void* dst, GFL_FONT_SIZE* dstSize )
{
  wk->DotExpandFunc( &glyphBuf[3], glyphRemBits, dst );

  dstSize->left_width  = glyphBuf[0];
  dstSize->glyph_width = glyphBuf[1];
  dstSize->width       = glyphBuf[2];
  dstSize->height      = wk->fontHeader.linefeed;
}

//--------------------------------------------------------------------------
/**
 *
 *
 * @param   srcData
 * @param   startBit
 * @param   dst
 *
 */
//--------------------------------------------------------------------------
static inline void expand_ntr_glyph_block( const u8* srcData, u16 blockPos, u16 cellWidth, BOOL maskFlag, u8* dst )
{
  #ifndef GLOBAL_PRINT_FLAG
  enum {
    PRINTFLG = FALSE,
  };
  #endif

  static const struct {
    u8  mask1;
    u8  shift1;
    u8  mask2;
    u8  shift2;
  }ptbl[] = {
    { 0xff, 0, 0x00, 1 }, // 0
    { 0x7f, 1, 0x80, 0 }, // 1
    { 0x3f, 2, 0xc0, 0 }, // 2
    { 0x1f, 3, 0xe0, 0 }, // 3
    { 0x0f, 4, 0xf0, 0 }, // 4
    { 0x07, 5, 0xf8, 0 }, // 5
    { 0x03, 6, 0xfc, 0 }, // 6
    { 0x01, 7, 0xfe, 0 }, // 7
  };
  u32 byte, bit, i;
  u32 startBit;

  startBit = (cellWidth * (blockPos / 2) * 8) + ((blockPos&1) * 8);


  for(i=0; i<8; i++)
  {
    byte = startBit / 8;
    bit  = startBit & 7;
    dst[i] = srcData[byte] & ptbl[bit].mask1;
    dst[i] <<= ptbl[bit].shift1;
    dst[i] |= ((srcData[byte+1] & ptbl[bit].mask2) >> (8 - ptbl[bit].shift1));
    startBit += cellWidth;
  }

  if( maskFlag )
  {
    u8 mask = 0xff;
    int shift = 8 - (cellWidth & 7);
    mask = (mask >> shift) << shift;
    for(i=0; i<8; i++)
    {
      dst[i] &= mask;
    }
  }

}

//==============================================================================================
/**
 * 文字幅（ドット単位）取得
 *
 * @param   wk    フォントデータハンドラ
 * @param   code  文字コード
 *
 * @retval  u16
 */
//==============================================================================================
u16 GFL_FONT_GetWidth( const GFL_FONT* wk, STRCODE code )
{
  GFL_FONT_SIZE  size;
  u32 index = get_glyph_index( wk, code );

  return wk->WidthGetFunc( wk, index );
}

//==============================================================================================
/**
 * 行の高さ（ドット単位）取得
 *
 * @param   wk    フォントデータハンドラ
 *
 * @retval  u16
 */
//==============================================================================================
u16 GFL_FONT_GetLineHeight( const GFL_FONT* wk )
{
  return wk->fontHeader.linefeed;
}



//----------------------------------------------------------------------------------
/**
 * １文字幅取得関数（プロポーショナル）
 *
 * @param   wk      フォントデータハンドラ
 * @param   index   文字インデックス
 * @param   size    [out] 幅データ取得用構造体
 */
//----------------------------------------------------------------------------------
static u8 GetWidthProportionalFont( const GFL_FONT* wk, u32 glyphIndex )
{
  const GFL_FONT_WIDTH_HEADER*  wtbl = wk->widthTblTop;

  {
    u32 bitIdx, byteIdx, shift;
    u8  widthIdx;
    const u8* bitTable;

    bitTable = (const u8*)( (u32)wtbl + wtbl->bitTableOfs );

    bitIdx = glyphIndex * 2;
    byteIdx = bitIdx / 8;
    bitIdx %= 8;
    shift = 6 - bitIdx;

    widthIdx = (bitTable[ byteIdx ] >> shift) & 0x03;

    if( widthIdx < 3 ){
      return wtbl->lotsWidth[ widthIdx ];
    }
  }

  {
    enum {
      HASH_SIZE = 512,
    };
    const s8* hashTable;
    u16 hashCode, keyBit, dupIdx;

    hashTable = (const s8*)( (u32)wtbl + wtbl->hashTableOfs );

    keyBit = ((glyphIndex>>12)&1) ^ ((glyphIndex>>11)&1) ^ ((glyphIndex>>10)&1) ^ ((glyphIndex>>9)&1);
    hashCode = ((glyphIndex & 0x1ff) ^ (keyBit << 3)) % HASH_SIZE;

    if( hashTable[hashCode] >= 0 ){
      return hashTable[ hashCode ];
    }else{
      dupIdx = hashTable[ hashCode ] * -1;
      --dupIdx;
    }

    {
      const u8* chainTable = (const u8*)( (u32)wtbl + wtbl->hashChainOfs);
      u16 index;
      u8  max, i;
      while( dupIdx ){
        chainTable += (1 + ((*chainTable) * 3));
        dupIdx--;
      }

      max = *chainTable++;
      for(i=0; i<max; ++i)
      {
        index = (*chainTable << 8) | *(chainTable+1);
        if( glyphIndex == index ){
          u8 width = *(chainTable + 2);
          return width;
        }
        else{
          chainTable += 3;
        }
      }
    }
  }

  // 正常ならここには来ない
  GF_ASSERT_MSG(0, "GlyphIdx=%d", glyphIndex);
  return wk->fontHeader.totalWidth;

#if 0
  while( 1 )
  {
    if( wtbl->indexBegin <= index && index <= wtbl->indexEnd )
    {
      u8* adrs = (u8*)((u32)wtbl + sizeof(NNSFontWidth) + (index - wtbl->indexBegin) * 3);

      size->left_width  = adrs[0];
      size->glyph_width = adrs[1];
      size->width     = adrs[2];
      size->height    = wk->fontHeader.linefeed;
      return;
    }
    else if( wtbl->nextOfs )
    {
      wtbl = (const NNSFontWidth*)
          (u32)(wk->widthTblTop) + (u32)(wtbl->nextOfs - (u32)(wk->widthTblTop));
    }
    else
    {
      GF_ASSERT(0);
      GetWidthFixedFont( wk, index, size );
      break;
    }
  }
#endif

}
//------------------------------------------------------------------
/**
 * １文字幅取得関数（等幅）
 *
 * @param   wk
 * @param   bcode
 *
 * @retval  u8
 */
//------------------------------------------------------------------
static u8 GetWidthFixedFont( const GFL_FONT* wk, u32 index )
{
  return wk->fontHeader.totalWidth;
}







//==============================================================================================
// フォントデータ展開処理
//==============================================================================================

static u16 DotTbl[256];

static struct {
  u8  letter;
  u8  shadow;
  u8  back;
}CurrentColor;


void GFL_FONTSYS_Init( void )
{
  GFL_FONTSYS_SetDefaultColor();
}

void GFL_FONTSYS_SetDefaultColor( void )
{
  GFL_FONTSYS_SetColor( COLOR_DEFAULT_LETTER, COLOR_DEFAULT_SHADOW, COLOR_DEFAULT_BACKGROUND );
}


void GFL_FONTSYS_SetColor( u8 letterColor, u8 shadowColor, u8 backColor )
{
  int d1,d2,d3,d4,n;
  u32 col[4];

  col[0] = 0;
  col[1] = letterColor;
  col[2] = shadowColor;
  col[3] = backColor;

  CurrentColor.letter = letterColor;
  CurrentColor.shadow = shadowColor;
  CurrentColor.back   = backColor;

  n = 0;
  for(d1=0; d1<4; d1++)
  {
    for(d2=0; d2<4; d2++)
    {
      for(d3=0; d3<4; d3++)
      {
        for(d4=0; d4<4; d4++)
        {
          DotTbl[n++] = (col[d4]<<12) |
                  (col[d3]<<8) |
                  (col[d2]<<4) |
                  (col[d1]);
        }
      }
    }
  }
}

void GFL_FONTSYS_GetColor( u8* letter, u8* shadow, u8* back )
{
  *letter = CurrentColor.letter;
  *shadow = CurrentColor.shadow;
  *back = CurrentColor.back;
}

BOOL GFL_FONTSYS_IsDifferentColor( u8 letter, u8 shadow, u8 back )
{
  return      (letter != CurrentColor.letter)
          ||  (shadow != CurrentColor.shadow)
          ||  (back != CurrentColor.back);
}

static inline void ExpandFontData( const void* src_p, void* dst_p )
{
  const u8* src;
  u32* dst;

  src = src_p;
  dst = dst_p;

  *dst++ = DotTbl[ *src++ ];
  *dst++ = DotTbl[ *src++ ];
  *dst++ = DotTbl[ *src++ ];
  *dst++ = DotTbl[ *src++ ];
  *dst++ = DotTbl[ *src++ ];
  *dst++ = DotTbl[ *src++ ];
  *dst++ = DotTbl[ *src++ ];
  *dst++ = DotTbl[ *src++ ];
}

static inline void BitReader_Init( BIT_READER* br, const u8* src )
{
  br->src = src;
  br->read_bit = *src;
  br->rem_bits = 8;
}

static inline void BitReader_SetNextBit( BIT_READER* br )
{
  br->src++;
  br->read_bit = *(br->src);
  br->rem_bits = 8;
}

static inline u8 BitReader_Read( BIT_READER* br, u8 bits )
{
  static const u8 bitmask[] = {
    0x00,
    0x80,
    0xc0,
    0xe0,
    0xf0,
    0xf8,
    0xfc,
    0xfe,
    0xff
  };

#if 1
  u8 ret_u, ret_d, ret, shift;

  if( br->rem_bits < bits )
  {
    ret_u = br->read_bit & bitmask[br->rem_bits];
    shift = br->rem_bits;
    bits -= br->rem_bits;
    BitReader_SetNextBit( br );
  }
  else
  {
    ret_u = 0;
    shift = 0;
  }

  ret_d = br->read_bit & bitmask[bits];
  br->rem_bits -= bits;
  if( br->rem_bits == 0 )
  {
    BitReader_SetNextBit( br );
  }
  else
  {
    br->read_bit <<= bits;
  }

  ret = ret_u | (ret_d >> shift);
  return ret;

#else
  // こっちは再帰なので inline にできない
  u8 ret;
  if( br->rem_bits >= bits )
  {
    ret = br->read_bit & bitmask[bits];
    br->rem_bits -= bits;
    if( br->rem_bits == 0 )
    {
      BitReader_SetNextBit( br );
    }
    else
    {
      br->read_bit <<= bits;
    }
  }
  else
  {
    u32 rem_bits = br->rem_bits;
    u32 bits2nd = bits - rem_bits;
    ret = br->read_bit & bitmask[rem_bits];
    BitReader_SetNextBit( br );
    ret |= (BitReader_Read( br, bits2nd ) >> rem_bits);
  }
  return ret;
#endif

}


static BIT_READER BitReader = {0};

// 1x1char (幅5ドット以上）ドット読み取り
static inline void dotExpand_1x1( const u8* glyphSrc, u16 remBits, u8* dst )
{
  u16* dst16 = (u16*)dst;
  u8 dots;

  BitReader_Init( &BitReader, glyphSrc );

  dots = BitReader_Read( &BitReader, 8 );
  *dst16++ = DotTbl[dots];
  dots = BitReader_Read( &BitReader, remBits );
  *dst16++ = DotTbl[dots];

  dots = BitReader_Read( &BitReader, 8 );
  *dst16++ = DotTbl[dots];
  dots = BitReader_Read( &BitReader, remBits );
  *dst16++ = DotTbl[dots];

  dots = BitReader_Read( &BitReader, 8 );
  *dst16++ = DotTbl[dots];
  dots = BitReader_Read( &BitReader, remBits );
  *dst16++ = DotTbl[dots];

  dots = BitReader_Read( &BitReader, 8 );
  *dst16++ = DotTbl[dots];
  dots = BitReader_Read( &BitReader, remBits );
  *dst16++ = DotTbl[dots];

  dots = BitReader_Read( &BitReader, 8 );
  *dst16++ = DotTbl[dots];
  dots = BitReader_Read( &BitReader, remBits );
  *dst16++ = DotTbl[dots];

  dots = BitReader_Read( &BitReader, 8 );
  *dst16++ = DotTbl[dots];
  dots = BitReader_Read( &BitReader, remBits );
  *dst16++ = DotTbl[dots];

  dots = BitReader_Read( &BitReader, 8 );
  *dst16++ = DotTbl[dots];
  dots = BitReader_Read( &BitReader, remBits );
  *dst16++ = DotTbl[dots];

  dots = BitReader_Read( &BitReader, 8 );
  *dst16++ = DotTbl[dots];
  dots = BitReader_Read( &BitReader, remBits );
  *dst16++ = DotTbl[dots];

}

// 2x2char (幅9～12ドット）ドット読み取り
static inline void dotExpand_2x2( const u8* glyphSrc, u16 remBits, u8* dst )
{
  u16* dst1st = (u16*)dst;
  u16* dst2nd = (u16*)( dst + 0x20 );
  u8 dots;

  BitReader_Init( &BitReader, glyphSrc );

  // この１ブロックで横１ライン分のドットを読み取っている
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

  dst1st += 16;
  dst2nd += 16;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;

    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, 8 );
    *dst1st++ = DotTbl[dots];
    dots = BitReader_Read( &BitReader, remBits );
    *dst2nd++ = DotTbl[dots];
    dst2nd++;
}


