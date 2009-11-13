//============================================================================================
/**
 * @file  palanm.c
 * @brief パレットフェード処理
 * @author  Hiroyuki Nakamura
 * @date  2004.11.24
 */
//============================================================================================
#define PALANM_H_GLOBAL

#include <gflib.h>
#include "system/palanm.h"


//============================================================================================
//  定数定義
//============================================================================================
#define DEF_FADE_VAL  ( 2 )     // 基本速度

///フェード計算を行うローカルタスクのTCBプライオリティ
/// ※必ずTCBの最後に実行されるようにプライオリティを下にしている
#define TCBPRI_CALC_FADETASK    (0xffffffff - 1)

#define NORMAL_COLOR_NUM  ( 16 )    // 通常パレットのカラー数
#define EXTRA_COLOR_NUM   ( 256 )   // 拡張パレットのカラー数


//==============================================================================
//  構造体定義
//==============================================================================
/// パレットフェードデータ
typedef struct _PALETTE_FADE_DATA{
  FADE_REQ_EX2  dat[ ALL_PALETTE_SIZE ];

  u16 req_flg:2;    // 0=未登録, 1=動作中, 2=一時停止
  u16 req_bit:14;   // リクエストビット（カラー計算用）
  u16 trans_bit:14; // トランスビット（カラー転送用）
  u16 tcb_flg:1;    // TCB登録フラグ　0=未登録, 1=登録済
  u16 auto_trans:1;     // トランスビットに関係なく、毎フレーム全パレットを転送

  u8 force_stop;    //TRUE:パレットフェード強制終了

  u8 dummy[3];
}PALETTE_FADE_DATA;

/// カラーパレット構造体
typedef struct {
    u16 Red:5;              // 赤
    u16 Green:5;            // 緑
    u16 Blue:5;             // 青
    u16 Dummy_15:1;
} PLTT_DATA;

//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static u8 ReqBitCheck( u16 flg, u16 req );
static void TransBitSet( PALETTE_FADE_PTR pfd, u16 id );
static void FadeBitCheck( FADEREQ req, FADE_REQ_EX2 * wk, u16 * fade_bit );
static void FadeReqSet(
        FADE_REQ_PARAM * wk, u16 fade_bit,
        s8 wait, u8 start_evy, u8 end_evy, u16 next_rgb );
static void PaletteFadeTask( GFL_TCB *tcb, void * work );
static void NormalPaletteFade(PALETTE_FADE_PTR pfd);
static void ExtraPaletteFade(PALETTE_FADE_PTR pfd);
static void PaletteFade( PALETTE_FADE_PTR pfd, u16 id, u16 siz );
static void PaletteFadeMain( PALETTE_FADE_PTR pfd, u16 id, u16 siz );
static void FadeWorkSet( u16 * def, u16 * trans, FADE_REQ_PARAM * wk, u32 siz );
static void FadeParamCheck(PALETTE_FADE_PTR pfd, u8 id, FADE_REQ_PARAM * wk );



//--------------------------------------------------------------
/**
 * @brief   パレットフェードシステムワーク作成
 *
 * @param   heap_id   ヒープID
 *
 * @retval  作成したパレットフェードシステムワークのポインタ
 *
 * システムを使わなくなったらPaletteFadeFreeで解放処理を行ってください
 */
//--------------------------------------------------------------
PALETTE_FADE_PTR PaletteFadeInit(int heap_id)
{
  PALETTE_FADE_PTR pfd;

  pfd = GFL_HEAP_AllocMemory(heap_id, sizeof(PALETTE_FADE_DATA));
  MI_CpuClear8(pfd, sizeof(PALETTE_FADE_DATA));
  return pfd;
}

//--------------------------------------------------------------
/**
 * @brief   パレットフェードシステム解放
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 */
//--------------------------------------------------------------
void PaletteFadeFree(PALETTE_FADE_PTR pfd)
{
  GFL_HEAP_FreeMemory(pfd);
}

//--------------------------------------------------------------------------------------------
/**
 * リクエストデータセット
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param req     リクエストデータ番号
 * @param def_wk    初期データ
 * @param trans_wk  転送用データ
 * @param siz     サイズ（バイト単位）
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void PaletteFadeWorkAdrSet(PALETTE_FADE_PTR pfd, FADEREQ req, void * def_wk, void * trans_wk, u32 siz )
{
  pfd->dat[req].def_wk   = (u16 *)def_wk;
  pfd->dat[req].trans_wk = (u16 *)trans_wk;
  pfd->dat[req].siz      = siz;
}

//--------------------------------------------------------------------------------------------
/**
 * リクエストデータをmallocしてセット
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param req   リクエストデータ番号
 * @param siz   データサイズ（バイト単位）
 * @param heap  ヒープID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void PaletteFadeWorkAllocSet(PALETTE_FADE_PTR pfd, FADEREQ req, u32 siz, HEAPID heap )
{
  void * def_wk;
  void * trans_wk;

  def_wk   = GFL_HEAP_AllocMemory( heap, siz );
  trans_wk = GFL_HEAP_AllocMemory( heap, siz );

#ifdef  OSP_ERR_PALANMWK_GET  // パレットフェードのワーク取得失敗
  if( def_wk == NULL ){
    OS_Printf( "ERROR : PaletteFadeWorkAlloc ( %d ) - def_wk\n", req );
  }
  if( trans_wk == NULL ){
    OS_Printf( "ERROR : PaletteFadeWorkAlloc ( %d ) - trans_wk\n", req );
  }
#endif  // OSP_ERR_PALANMWK_GET

  PaletteFadeWorkAdrSet(pfd, req, def_wk, trans_wk, siz );
}

//--------------------------------------------------------------------------------------------
/**
 * リクエストデータ開放
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param req   リクエストデータ番号
 *
 * @return  none
 *
 * @li  PaletteFadeWorkAllocSet()で取得した場合に使用
 */
//--------------------------------------------------------------------------------------------
void PaletteFadeWorkAllocFree(PALETTE_FADE_PTR pfd, FADEREQ req)
{
  GFL_HEAP_FreeMemory(pfd->dat[req].def_wk );
  GFL_HEAP_FreeMemory(pfd->dat[req].trans_wk );
}

//--------------------------------------------------------------------------------------------
/**
 * パレットの初期データをセット
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param dat   パレットデータ
 * @param req   リクエストデータ番号
 * @param pos   開始位置
 * @param siz   データサイズ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void PaletteWorkSet(PALETTE_FADE_PTR pfd, const void * dat, FADEREQ req, u16 pos, u16 siz )
{
#ifdef  OSP_ERR_PALANM_SIZ_OVER   // サイズオーバー
  if( (pos*2+siz) > pfd->dat[req].siz ){
    OS_Printf( "ERROR : PaletteWorkSet - SizeOver ( %d )\n", req );
  }
#endif  // OSP_ERR_PALANM_SIZ_OVER
  MI_CpuCopy16( dat, (void *)&pfd->dat[req].def_wk[pos], (u32)siz );
  MI_CpuCopy16( dat, (void *)&pfd->dat[req].trans_wk[pos], (u32)siz );
}

//--------------------------------------------------------------
/**
 * @brief   アーカイブされているパレットデータをロードしてワークに展開します(拡張版)
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param   fileIdx     アーカイブファイルインデックス
 * @param   dataIdx     アーカイブデータインデックス
 * @param   heap      データ読み込みテンポラリとして使うヒープID
 * @param   req       リクエストデータ番号
 * @param   trans_size    転送サイズ(バイト単位　※2バイトアライメントされていること)
 * @param   pos       パレット転送開始位置(カラー単位)
 * @param   read_pos    ロードしたパレットの読み込み開始位置(カラー単位)
 */
//--------------------------------------------------------------
void PaletteWorkSetEx_Arc(PALETTE_FADE_PTR pfd, u32 fileIdx, u32 dataIdx, HEAPID heap,
  FADEREQ req, u32 trans_size, u16 pos, u16 read_pos)
{
  NNSG2dPaletteData *pal_data;
  void *arc_data;

  arc_data = GFL_ARC_UTIL_LoadPalette(fileIdx, dataIdx, &pal_data, GFL_HEAP_LOWID(heap));
  GF_ASSERT(arc_data != NULL);

  if(trans_size == 0){
    trans_size = pal_data->szByte;
  }

  GF_ASSERT(pos * sizeof(pos) + trans_size <= pfd->dat[req].siz);
  PaletteWorkSet(pfd, &(((u16*)(pal_data->pRawData))[read_pos]), req, pos, trans_size);

  GFL_HEAP_FreeMemory(arc_data);
}

//--------------------------------------------------------------
/**
 * @brief   アーカイブされているパレットデータをロードしてワークに展開します
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param   fileIdx     アーカイブファイルインデックス
 * @param   dataIdx     アーカイブデータインデックス
 * @param   heap      データ読み込みテンポラリとして使うヒープID
 * @param   req       リクエストデータ番号
 * @param   trans_size    転送サイズ(バイト単位　※2バイトアライメントされていること)
 * @param   pos       パレット転送開始位置(カラー単位)
 */
//--------------------------------------------------------------
void PaletteWorkSet_Arc(PALETTE_FADE_PTR pfd, u32 fileIdx, u32 dataIdx, HEAPID heap,
  FADEREQ req, u32 trans_size, u16 pos)
{
  PaletteWorkSetEx_Arc(pfd, fileIdx, dataIdx, heap, req, trans_size, pos, 0);
}

//--------------------------------------------------------------
/**
 * @brief   アーカイブされているパレットデータをロードしてワークに展開します(ハンドル拡張版)
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param   fileIdx     アーカイブファイルインデックス
 * @param   dataIdx     アーカイブデータインデックス
 * @param   heap      データ読み込みテンポラリとして使うヒープID
 * @param   req       リクエストデータ番号
 * @param   trans_size    転送サイズ(バイト単位　※2バイトアライメントされていること)
 * @param   pos       パレット転送開始位置(カラー単位)
 * @param   read_pos    ロードしたパレットの読み込み開始位置(カラー単位)
 */
//--------------------------------------------------------------
void PaletteWorkSetEx_ArcHandle(PALETTE_FADE_PTR pfd, ARCHANDLE* handle, ARCDATID dataIdx, HEAPID heap,
  FADEREQ req, u32 trans_size, u16 pos, u16 read_pos)
{
  NNSG2dPaletteData *pal_data;
  void *arc_data;

  arc_data = GFL_ARCHDL_UTIL_LoadPalette( handle, dataIdx, &pal_data, GFL_HEAP_LOWID(heap) );
  GF_ASSERT(arc_data != NULL);

  if(trans_size == 0){
    trans_size = pal_data->szByte;
  }

  GF_ASSERT(pos * sizeof(pos) + trans_size <= pfd->dat[req].siz);
  PaletteWorkSet(pfd, &(((u16*)(pal_data->pRawData))[read_pos]), req, pos, trans_size);

  GFL_HEAP_FreeMemory(arc_data);
}

//--------------------------------------------------------------
/**
 * @brief   アーカイブされているパレットデータをロードしてワークに展開します（ハンドル版）
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param   fileIdx     アーカイブファイルインデックス
 * @param   dataIdx     アーカイブデータインデックス
 * @param   heap      データ読み込みテンポラリとして使うヒープID
 * @param   req       リクエストデータ番号
 * @param   trans_size    転送サイズ(バイト単位　※2バイトアライメントされていること)
 * @param   pos       パレット転送開始位置(カラー単位)
 */
//--------------------------------------------------------------
void PaletteWorkSet_ArcHandle(PALETTE_FADE_PTR pfd, ARCHANDLE* handle, ARCDATID dataIdx, HEAPID heap,
  FADEREQ req, u32 trans_size, u16 pos)
{
  PaletteWorkSetEx_ArcHandle(pfd, handle, dataIdx, heap, req, trans_size, pos, 0);
}

//--------------------------------------------------------------
/**
 * @brief   パレットVRAMからワークへデータコピーをします
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param   req       リクエストデータ番号
 * @param   pos       パレット転送開始位置(カラー単位)
 * @param   trans_size    転送サイズ(バイト単位　※2バイトアライメントされていること)
 *
 * 拡張パレットには未対応です
 */
//--------------------------------------------------------------
void PaletteWorkSet_VramCopy(PALETTE_FADE_PTR pfd, FADEREQ req, u16 pos, u32 trans_size)
{
  u16 *pltt_vram;

  GF_ASSERT(pos * sizeof(pos) + trans_size <= pfd->dat[req].siz);

  switch(req){
  case FADE_MAIN_BG:
    pltt_vram = (u16 *)HW_BG_PLTT;
    break;
  case FADE_SUB_BG:
    pltt_vram = (u16 *)HW_DB_BG_PLTT;
    break;
  case FADE_MAIN_OBJ:
    pltt_vram = (u16 *)HW_OBJ_PLTT;
    break;
  case FADE_SUB_OBJ:
    pltt_vram = (u16 *)HW_DB_OBJ_PLTT;
    break;
  default:
    GF_ASSERT(0 && "未対応のリクエストです");
    return;
  }

  PaletteWorkSet(pfd, &pltt_vram[pos], req, pos, trans_size);
}

//--------------------------------------------------------------
/**
 * @brief   拡張パレットVRAMからワークへデータコピーをします
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param   req       リクエストデータ番号
 * @param   trans_size    転送サイズ(バイト単位　※2バイトアライメントされていること)
 * @param   pos       パレット転送開始位置(カラー単位)
 *
 * メインパレットはPaletteWorkSet_VramCopyを使用してください
 */
//--------------------------------------------------------------
void PaletteWorkSet_VramCopyEx(PALETTE_FADE_PTR pfd, FADEREQ req, u32 trans_size, u16 pos)
{
  GF_ASSERT(0 && "現在未作成です");
}

//--------------------------------------------------------------
/**
 * @brief   アーカイブされているパレットデータを指定したワークにロードします(拡張版)
 *
 * @param   fileIdx     アーカイブファイルインデックス
 * @param   dataIdx     アーカイブデータインデックス
 * @param   heap      データ読み込みテンポラリとして使うヒープID
 * @param   trans_size    転送サイズ(バイト単位　※2バイトアライメントされていること)
 * @param   read_pos    ロードしたパレットの読み込み開始位置(カラー単位)
 * @param   dest      ロードしたパレットの展開先へのポインタ
 */
//--------------------------------------------------------------
void PaletteWorkSetEx_ArcWork(u32 fileIdx, u32 dataIdx, HEAPID heap, u32 trans_size,
  u16 read_pos, void *dest)
{
  NNSG2dPaletteData *pal_data;
  void *arc_data;

  arc_data = GFL_ARC_UTIL_LoadPalette(fileIdx, dataIdx, &pal_data, heap);
  GF_ASSERT(arc_data != NULL);

  if(trans_size == 0){
    trans_size = pal_data->szByte;
  }

  MI_CpuCopy16(&(((u16*)(pal_data->pRawData))[read_pos]), dest, trans_size);

  GFL_HEAP_FreeMemory(arc_data);
}

//--------------------------------------------------------------------------------------------
/**
 * パレットワーク間コピー
 *
 * @param   pfd       パレットフェードシステムワークへのポインタ
 * @param src_req     コピー元リクエストデータ番号
 * @param src_pos     コピー元開始位置(カラー単位)
 * @param dest_req    コピー先リクエストデータ番号
 * @param dest_pos    コピー先開始位置(カラー単位)
 * @param siz       コピーするデータサイズ(バイト単位)
 *
 * コピー元は元データから読み取り、コピー先は元データ、転送先、の両方にコピーを行います
 */
//--------------------------------------------------------------------------------------------
void PaletteWorkCopy(PALETTE_FADE_PTR pfd, FADEREQ src_req, u16 src_pos,
  FADEREQ dest_req, u16 dest_pos, u16 siz )
{
  MI_CpuCopy16((void *)&pfd->dat[src_req].def_wk[src_pos],
    (void *)&pfd->dat[dest_req].def_wk[dest_pos], siz);
  MI_CpuCopy16((void *)&pfd->dat[src_req].def_wk[src_pos],
    (void *)&pfd->dat[dest_req].trans_wk[dest_pos], siz);
}

//--------------------------------------------------------------------------------------------
/**
 * パレットワークにセットされているパレットデータを取得
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param tpye  取得するパレットの種類
 *
 * @return  セットされているパレットデータ
 */
//--------------------------------------------------------------------------------------------
u16 * PaletteWorkDefaultWorkGet( PALETTE_FADE_PTR pfd, FADEREQ type )
{
  return pfd->dat[type].def_wk;
}

//--------------------------------------------------------------------------------------------
/**
 * パレットワークにセットされている転送用パレットデータを取得
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param tpye  取得するパレットの種類
 *
 * @return  セットされているパレットデータ
 */
//--------------------------------------------------------------------------------------------
u16 * PaletteWorkTransWorkGet( PALETTE_FADE_PTR pfd, FADEREQ type )
{
  return pfd->dat[type].trans_wk;
}

//--------------------------------------------------------------------------------------------
/**
 * パレットフェードリクエスト（上書き不可）
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param req_bit   パレットの種類（ビット指定）
 * @param fade_bit  パレット指定（ビット指定）bit0 = pal0
 * @param wait    計算待ち時間（マイナスあり）
 * @param start_evy 初期濃度
 * @param end_evy   最終濃度
 * @param next_rgb  変更後の色
 *
 * @retval  "0 = 正常"
 * @retval  "1 = エラー"
 *
 * @li  濃度：0 ～ 16 ( 0(元の色) ～ 2,3..(中間色) ～ 16(指定した色) )
 */
//--------------------------------------------------------------------------------------------
u8 PaletteFadeReq(PALETTE_FADE_PTR pfd, u16 req_bit, u16 fade_bit, s8 wait,
  u8 start_evy, u8 end_evy, u16 next_rgb, GFL_TCBSYS *tcbsys )
{
  u16 cpy_bit;
  u8  tmp;
  u8  i;

  cpy_bit = fade_bit;
  tmp = 0;
  for( i=0; i<ALL_PALETTE_SIZE; i++ ){
    if( ReqBitCheck( req_bit, i ) == TRUE &&
      ReqBitCheck( pfd->req_bit, i ) == FALSE ){

      FadeBitCheck( i, &pfd->dat[i], &fade_bit );
      FadeReqSet( &pfd->dat[i].prm, fade_bit, wait, start_evy, end_evy, next_rgb );

      TransBitSet( pfd, i );
      if( i >= FADE_MAIN_BG_EX0 ){
        PaletteFadeMain( pfd, i, EXTRA_COLOR_NUM );
      }else{
        PaletteFadeMain( pfd, i, NORMAL_COLOR_NUM );
      }

      fade_bit = cpy_bit;
      tmp = 1;
    }
  }

  if( tmp == 1 ){
    pfd->req_bit |= req_bit;
    if( pfd->tcb_flg == 0 ){
      pfd->tcb_flg = 1;
      pfd->req_flg = 1;
      pfd->force_stop = FALSE;
      GFL_TCB_AddTask( tcbsys, PaletteFadeTask, pfd, TCBPRI_CALC_FADETASK );
    }
  }

  return tmp;
}

//--------------------------------------------------------------------------------------------
/**
 * パレットフェードリクエスト（上書き可）
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param req_bit   パレットの種類（ビット指定）
 * @param fade_bit  パレット指定（ビット指定）bit0 = pal0
 * @param wait    計算待ち時間（マイナスあり）
 * @param start_evy 初期濃度
 * @param end_evy   最終濃度
 * @param next_rgb  変更後の色
 *
 * @retval  "0 = 正常"
 * @retval  "1 = エラー"
 *
 * @li  濃度：0 ～ 16 ( 0(元の色) ～ 2,3..(中間色) ～ 16(指定した色) )
 */
//--------------------------------------------------------------------------------------------
u8 PaletteFadeReqWrite(PALETTE_FADE_PTR pfd, u16 req_bit, u16 fade_bit, s8 wait,
  u8 start_evy, u8 end_evy, u16 next_rgb, GFL_TCBSYS *tcbsys )
{
  u16 cpy_bit;
  u8  tmp;
  u8  i;

  cpy_bit = fade_bit;
  tmp = 0;
  for( i=0; i<ALL_PALETTE_SIZE; i++ ){
    if( ReqBitCheck( req_bit, i ) == TRUE ){
      FadeBitCheck( i, &pfd->dat[i], &fade_bit );
      FadeReqSet( &pfd->dat[i].prm, fade_bit, wait, start_evy, end_evy, next_rgb );

      TransBitSet( pfd, i );
      if( i >= FADE_MAIN_BG_EX0 ){
        PaletteFadeMain( pfd, i, EXTRA_COLOR_NUM );
      }else{
        PaletteFadeMain( pfd, i, NORMAL_COLOR_NUM );
      }

      fade_bit = cpy_bit;
      tmp = 1;
    }
  }

  if( tmp == 1 ){
//    pfd->req_bit |= req_bit;
    pfd->req_bit = req_bit;
    if( pfd->tcb_flg == 0 ){
      pfd->tcb_flg = 1;
      pfd->req_flg = 1;
      pfd->force_stop = FALSE;
      GFL_TCB_AddTask( tcbsys, PaletteFadeTask, pfd, TCBPRI_CALC_FADETASK );
    }
  }

  return tmp;
}

//--------------------------------------------------------------------------------------------
/**
 * リクエストの状態を調べる
 *
 * @param flg
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static u8 ReqBitCheck( u16 flg, u16 req )
{
  if( ( flg & ( 1 << req ) ) != 0 ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 転送フラグセット
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param id    パレットID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void TransBitSet( PALETTE_FADE_PTR pfd, u16 id )
{
  if( ReqBitCheck( pfd->trans_bit, id ) == TRUE ){
    return;
  }
  pfd->trans_bit |= ( 1 << id );
}

//--------------------------------------------------------------------------------------------
/**
 * フェードビットチェック
 *
 * @param req     リクエストデータ番号
 * @param wk      リクエストデータ
 * @param fade_bit  パレット指定（ビット指定）bit0 = pal0
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void FadeBitCheck( FADEREQ req, FADE_REQ_EX2 * wk, u16 * fade_bit )
{
  u16 siz_bit;
  u8  i, j;

  if( req < NORMAL_PALETTE_SIZE ){
    j = wk->siz / 32;
  }else{
    j = wk->siz / 512;
  }

  siz_bit = 0;
  for( i=0; i<j; i++ ){
    siz_bit += ( 1 << i );
  }

  *fade_bit &= siz_bit;
}


//--------------------------------------------------------------------------------------------
/**
 * パラメータセット
 *
 * @param wk      データ
 * @param fade_bit  パレット指定（ビット指定）bit0 = pal0
 * @param wait    計算待ち時間（マイナスあり）
 * @param start_evy 初期濃度
 * @param end_evy   最終濃度
 * @param next_rgb  変更後の色
 *
 * @return  none
 *
 * @li  濃度：0 ～ 16 ( 0(元の色) ～ 2,3..(中間色) ～ 16(指定した色) )
 */
//--------------------------------------------------------------------------------------------
static void FadeReqSet(
        FADE_REQ_PARAM * wk, u16 fade_bit,
        s8 wait, u8 start_evy, u8 end_evy, u16 next_rgb )
{
  // waitがﾏｲﾅｽの時はvalueを大きくしてﾌｪｰﾄﾞを早くする
  if( wait < 0 ){
    wk->fade_value = DEF_FADE_VAL + MATH_ABS( wait );
    wk->wait = 0;
  }else{
    wk->fade_value = DEF_FADE_VAL;
    wk->wait = wait;
  }

  wk->fade_bit = fade_bit;
  wk->now_evy  = start_evy;
  wk->end_evy  = end_evy;
  wk->next_rgb = next_rgb;
  wk->wait_cnt = wk->wait;

  if( start_evy < end_evy ){
    wk->direction = 0;
  }else{
    wk->direction = 1;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * パレットフェードメインタスク
 *
 * @param tcb   TCBのポインタ
 * @param work  TCBワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PaletteFadeTask( GFL_TCB *tcb, void * work )
{
  PALETTE_FADE_PTR pfd = work;

  if(pfd->force_stop == TRUE){
    pfd->force_stop = FALSE;
    pfd->trans_bit = 0;
    pfd->req_bit = 0;
    pfd->tcb_flg = 0;
    GFL_TCB_DeleteTask( tcb );
    return;
  }

  if( pfd->req_flg != 1 ){ return; }

  pfd->trans_bit = pfd->req_bit;

  NormalPaletteFade(pfd);
  ExtraPaletteFade(pfd);

  if( pfd->req_bit == 0 ){
    pfd->tcb_flg = 0;
    GFL_TCB_DeleteTask( tcb );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * パレットフェード強制停止命令
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void PaletteFadeForceStop(PALETTE_FADE_PTR pfd)
{
  if(pfd->req_bit == 0){
    return;
  }

  pfd->force_stop = TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 通常パレットのフェード計算
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void NormalPaletteFade(PALETTE_FADE_PTR pfd)
{
  u8  i, j;

  for( i=0; i<NORMAL_PALETTE_SIZE; i++ ){
    PaletteFade( pfd, i, NORMAL_COLOR_NUM );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * 拡張パレットのフェード計算
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ExtraPaletteFade(PALETTE_FADE_PTR pfd)
{
  u8  i, j;

  for( i=NORMAL_PALETTE_SIZE; i<ALL_PALETTE_SIZE; i++ ){
    PaletteFade( pfd, i, EXTRA_COLOR_NUM );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * パレットのフェード計算
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param id    パレットID
 * @param siz   カラー個数 ( 通常 = 16, 拡張 = 256 )
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PaletteFade( PALETTE_FADE_PTR pfd, u16 id, u16 siz )
{
  if( ReqBitCheck( pfd->req_bit, id ) == FALSE ){
    return;
  }

  if( pfd->dat[id].prm.wait_cnt < pfd->dat[id].prm.wait ){
    pfd->dat[id].prm.wait_cnt++;
    return;
  }else{
    pfd->dat[id].prm.wait_cnt = 0;
  }

  PaletteFadeMain( pfd, id, siz );
}

//--------------------------------------------------------------------------------------------
/**
 * パレットのフェード計算メイン
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param id    パレットID
 * @param siz   カラー個数 ( 通常 = 16, 拡張 = 256 )
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PaletteFadeMain( PALETTE_FADE_PTR pfd, u16 id, u16 siz )
{
  u32 i;

  for( i=0; i<16; i++ ){
    if( ReqBitCheck( pfd->dat[id].prm.fade_bit, i ) == FALSE ){
      continue;
    }
    FadeWorkSet(
      &pfd->dat[id].def_wk[i*siz],
      &pfd->dat[id].trans_wk[i*siz],
      &pfd->dat[id].prm, siz );
  }
  FadeParamCheck( pfd, id, &pfd->dat[id].prm );
}


//--------------------------------------------------------------------------------------------
/**
 * カラー計算
 *
 * @param def   元パレットデータ
 * @param trans 計算後のパレットデータ
 * @param wk    フェードパラメータ
 * @param siz   計算サイズ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void FadeWorkSet( u16 * def, u16 * trans, FADE_REQ_PARAM * wk, u32 siz )
{
  u32 i;
  u8  red, green, blue;

  for( i=0; i<siz; i++ ){
    red   =
      FADE_CHANGE( (def[i]&0x1f), (wk->next_rgb&0x1f), wk->now_evy );
    green =
      FADE_CHANGE( ((def[i]>>5)&0x1f), ((wk->next_rgb>>5)&0x1f), wk->now_evy );
    blue  =
      FADE_CHANGE( ((def[i]>>10)&0x1f), ((wk->next_rgb>>10)&0x1f), wk->now_evy );

    trans[i] = ( blue << 10 ) | ( green << 5 ) | red;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * フェードの進行状況を監視
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 * @param id    フェードデータ番号
 * @param wk    フェードパラメータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void FadeParamCheck(PALETTE_FADE_PTR pfd, u8 id, FADE_REQ_PARAM * wk )
{
  s16 tmp;

  if( wk->now_evy == wk->end_evy ){
    if( ( pfd->req_bit & ( 1 << id ) ) != 0 ){
      pfd->req_bit ^= ( 1 << id );
    }
  }else if( wk->direction == 0 ){
    tmp = wk->now_evy;
    tmp += wk->fade_value;
    if( tmp > wk->end_evy ){
      tmp = wk->end_evy;
    }
    wk->now_evy = tmp;
  }else{
    tmp = wk->now_evy;
    tmp -= wk->fade_value;
    if( tmp < wk->end_evy ){
      tmp = wk->end_evy;
    }
    wk->now_evy = tmp;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * パレット転送
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 *
 * @return  none
 *
 * @li  VBlank内で呼ぶこと
 */
//--------------------------------------------------------------------------------------------
void PaletteFadeTrans(PALETTE_FADE_PTR pfd)
{
  if( pfd->auto_trans == FALSE && pfd->req_flg != 1 ){ return; }

  {
    int i;

    for( i=0; i<ALL_PALETTE_SIZE; i++ ){
      if(pfd->auto_trans == FALSE){
        if(pfd->dat[i].trans_wk == NULL || ReqBitCheck( pfd->trans_bit, i ) == FALSE){
          continue;
        }
      }

      DC_FlushRange( (void*)pfd->dat[i].trans_wk, pfd->dat[i].siz );
      switch( i ){
      case FADE_MAIN_BG:
        GX_LoadBGPltt(
          (const void *)pfd->dat[i].trans_wk, 0, pfd->dat[i].siz );
        break;
      case FADE_SUB_BG:
        GXS_LoadBGPltt(
          (const void *)pfd->dat[i].trans_wk, 0, pfd->dat[i].siz );
        break;
      case FADE_MAIN_OBJ:
        GX_LoadOBJPltt(
          (const void *)pfd->dat[i].trans_wk, 0, pfd->dat[i].siz );
        break;
      case FADE_SUB_OBJ:
        GXS_LoadOBJPltt(
          (const void *)pfd->dat[i].trans_wk, 0, pfd->dat[i].siz );
        break;
      case FADE_MAIN_BG_EX0:
        GX_BeginLoadBGExtPltt();
        GX_LoadBGExtPltt(
          (const void *)pfd->dat[i].trans_wk,
          EX_PAL_SLOT0_ADRS, pfd->dat[i].siz );
        GX_EndLoadBGExtPltt();
        break;
      case FADE_MAIN_BG_EX1:
        GX_BeginLoadBGExtPltt();
        GX_LoadBGExtPltt(
          (const void *)pfd->dat[i].trans_wk,
          EX_PAL_SLOT1_ADRS, pfd->dat[i].siz );
        GX_EndLoadBGExtPltt();
        break;
      case FADE_MAIN_BG_EX2:
        GX_BeginLoadBGExtPltt();
        GX_LoadBGExtPltt(
          (const void *)pfd->dat[i].trans_wk,
          EX_PAL_SLOT2_ADRS, pfd->dat[i].siz );
        GX_EndLoadBGExtPltt();
        break;
      case FADE_MAIN_BG_EX3:
        GX_BeginLoadBGExtPltt();
        GX_LoadBGExtPltt(
          (const void *)pfd->dat[i].trans_wk,
          EX_PAL_SLOT3_ADRS, pfd->dat[i].siz );
        GX_EndLoadBGExtPltt();
        break;
      case FADE_SUB_BG_EX0:
        GXS_BeginLoadBGExtPltt();
        GXS_LoadBGExtPltt(
          (const void *)pfd->dat[i].trans_wk,
          EX_PAL_SLOT0_ADRS, pfd->dat[i].siz );
        GXS_EndLoadBGExtPltt();
        break;
      case FADE_SUB_BG_EX1:
        GXS_BeginLoadBGExtPltt();
        GXS_LoadBGExtPltt(
          (const void *)pfd->dat[i].trans_wk,
          EX_PAL_SLOT1_ADRS, pfd->dat[i].siz );
        GXS_EndLoadBGExtPltt();
        break;
      case FADE_SUB_BG_EX2:
        GXS_BeginLoadBGExtPltt();
        GXS_LoadBGExtPltt(
          (const void *)pfd->dat[i].trans_wk,
          EX_PAL_SLOT2_ADRS, pfd->dat[i].siz );
        GXS_EndLoadBGExtPltt();
        break;
      case FADE_SUB_BG_EX3:
        GXS_BeginLoadBGExtPltt();
        GXS_LoadBGExtPltt(
          (const void *)pfd->dat[i].trans_wk,
          EX_PAL_SLOT3_ADRS, pfd->dat[i].siz );
        GXS_EndLoadBGExtPltt();
        break;
      case FADE_MAIN_OBJ_EX:
        GX_BeginLoadOBJExtPltt();
        GX_LoadOBJExtPltt(
          (const void *)pfd->dat[i].trans_wk, 0, pfd->dat[i].siz );
        GX_EndLoadOBJExtPltt();
        break;
      case FADE_SUB_OBJ_EX:
        GXS_BeginLoadOBJExtPltt();
        GXS_LoadOBJExtPltt(
          (const void *)pfd->dat[i].trans_wk, 0, pfd->dat[i].siz );
        GXS_EndLoadOBJExtPltt();
      }
    }
  }

  pfd->trans_bit = pfd->req_bit;
  if( pfd->trans_bit == 0 ){
    pfd->req_flg = 0;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * 終了チェック
 *
 * @param   pfd   パレットフェードシステムワークへのポインタ
 *
 * @retval  "0 = 終了"
 * @retval  "0 != 処理中"
 */
//--------------------------------------------------------------------------------------------
u16 PaletteFadeCheck(PALETTE_FADE_PTR pfd)
{
  return pfd->req_bit;
}

//--------------------------------------------------------------
/**
 * @brief   自動転送フラグをセットする
 *
 * @param   pfd     パレットフェードシステムワークへのポインタ
 * @param   on_off    TRUE:自動転送ON。　FALSE:自動転送OFF
 */
//--------------------------------------------------------------
void PaletteTrans_AutoSet(PALETTE_FADE_PTR pfd, int on_off)
{
  pfd->auto_trans = on_off;
}


void PaletteTransSwitch(PALETTE_FADE_PTR pfd, u8 flag)
{
  pfd->req_flg = flag & 0x01;
  pfd->req_bit = 0xFFFFFFFF;
}



//--------------------------------------------------------------------------------------------
/**
 * 指定パレット全体をクリア
 *
 * @param bit   クリアするパレット
 * @param heap  ヒープID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void PaletteAreaClear( u16 bit, HEAPID heap )
{
  void * buf;

  buf = (u8 *)GFL_HEAP_AllocMemory( heap, FADE_PAL_ALL_SIZE );
  GFL_STD_MemClear( buf, FADE_PAL_ALL_SIZE );
  DC_FlushRange( (void*)buf, FADE_PAL_ALL_SIZE );

  if( bit & PF_BIT_MAIN_BG ){
    GX_LoadBGPltt( (const void *)buf, 0, FADE_PAL_ALL_SIZE );
  }
  if( bit & PF_BIT_SUB_BG ){
    GXS_LoadBGPltt( (const void *)buf, 0, FADE_PAL_ALL_SIZE );
  }
  if( bit & PF_BIT_MAIN_OBJ ){
    GX_LoadOBJPltt( (const void *)buf, 0, FADE_PAL_ALL_SIZE );
  }
  if( bit & PF_BIT_SUB_OBJ ){
    GXS_LoadOBJPltt( (const void *)buf, 0, FADE_PAL_ALL_SIZE );
  }

  GFL_HEAP_FreeMemory( buf );

  buf = (u8 *)GFL_HEAP_AllocMemory( heap, FADE_EXPAL_ALL_SIZE );
  GFL_STD_MemClear( buf, FADE_EXPAL_ALL_SIZE );
  DC_FlushRange( (void*)buf, FADE_EXPAL_ALL_SIZE );

  if( bit & PF_BIT_MAIN_BG_EX0 ){
    GX_BeginLoadBGExtPltt();
    GX_LoadBGExtPltt( (const void *)buf, EX_PAL_SLOT0_ADRS, FADE_EXPAL_ALL_SIZE );
    GX_EndLoadBGExtPltt();
  }
  if( bit & PF_BIT_MAIN_BG_EX1 ){
    GX_BeginLoadBGExtPltt();
    GX_LoadBGExtPltt( (const void *)buf, EX_PAL_SLOT1_ADRS, FADE_EXPAL_ALL_SIZE );
    GX_EndLoadBGExtPltt();
  }
  if( bit & PF_BIT_MAIN_BG_EX2 ){
    GX_BeginLoadBGExtPltt();
    GX_LoadBGExtPltt( (const void *)buf, EX_PAL_SLOT2_ADRS, FADE_EXPAL_ALL_SIZE );
    GX_EndLoadBGExtPltt();
  }
  if( bit & PF_BIT_MAIN_BG_EX3 ){
    GX_BeginLoadBGExtPltt();
    GX_LoadBGExtPltt( (const void *)buf, EX_PAL_SLOT3_ADRS, FADE_EXPAL_ALL_SIZE );
    GX_EndLoadBGExtPltt();
  }
  if( bit & PF_BIT_SUB_BG_EX0 ){
    GXS_BeginLoadBGExtPltt();
    GXS_LoadBGExtPltt( (const void *)buf, EX_PAL_SLOT0_ADRS, FADE_EXPAL_ALL_SIZE );
    GXS_EndLoadBGExtPltt();
  }
  if( bit & PF_BIT_SUB_BG_EX1 ){
    GXS_BeginLoadBGExtPltt();
    GXS_LoadBGExtPltt( (const void *)buf, EX_PAL_SLOT1_ADRS, FADE_EXPAL_ALL_SIZE );
    GXS_EndLoadBGExtPltt();
  }
  if( bit & PF_BIT_SUB_BG_EX2 ){
    GXS_BeginLoadBGExtPltt();
    GXS_LoadBGExtPltt( (const void *)buf, EX_PAL_SLOT2_ADRS, FADE_EXPAL_ALL_SIZE );
    GXS_EndLoadBGExtPltt();
  }
  if( bit & PF_BIT_SUB_BG_EX3 ){
    GXS_BeginLoadBGExtPltt();
    GXS_LoadBGExtPltt( (const void *)buf, EX_PAL_SLOT3_ADRS, FADE_EXPAL_ALL_SIZE );
    GXS_EndLoadBGExtPltt();
  }

  if( bit & PF_BIT_MAIN_OBJ_EX ){
    GX_BeginLoadOBJExtPltt();
    GX_LoadOBJExtPltt( (const void *)buf, 0, FADE_EXPAL_ALL_SIZE );
    GX_EndLoadOBJExtPltt();
  }

  if( bit & PF_BIT_SUB_OBJ_EX ){
    GXS_BeginLoadOBJExtPltt();
    GXS_LoadOBJExtPltt( (const void *)buf, 0, FADE_EXPAL_ALL_SIZE );
    GXS_EndLoadOBJExtPltt();
  }

  GFL_HEAP_FreeMemory( buf );
}

//--------------------------------------------------------------
/**
 * @brief   パレットワークを指定コードでクリアする
 *
 * @param   pfd       パレットフェードシステムへのポインタ
 * @param   req       リクエストデータ番号
 * @param   select      バッファ指定
 * @param   clear_code    クリアコード
 * @param   start     開始位置(カラー位置)
 * @param   end       終了位置(カラー位置　※終了位置のカラーは書き換えません。)
 *
 * start=0, end=16 とするとパレット1本まるまるクリア対象になります。
 * start=0, end=15 とすると、パレットの最後のカラーは対象に含まれません。
 */
//--------------------------------------------------------------
void PaletteWork_Clear(PALETTE_FADE_PTR pfd, FADEREQ req, FADEBUF select,
  u16 clear_code, u16 start, u16 end)
{
  GF_ASSERT(end * sizeof(u16) <= pfd->dat[req].siz);

  if(select == FADEBUF_SRC || select == FADEBUF_ALL){
    MI_CpuFill16(&pfd->dat[req].def_wk[start], clear_code, (end - start) * 2);
  }
  if(select == FADEBUF_TRANS || select == FADEBUF_ALL){
    MI_CpuFill16(&pfd->dat[req].trans_wk[start], clear_code, (end - start) * 2);
  }
}

//--------------------------------------------------------------
/**
 * @brief   指定位置のカラーデータをパレットワークから取得する
 *
 * @param   pfd       パレットフェードシステムへのポインタ
 * @param   req       リクエストデータ番号
 * @param   select      バッファ指定
 * @param   color_pos   取得するカラーの位置
 *
 * @retval  カラーデータ
 */
//--------------------------------------------------------------
u16 PaletteWork_ColorGet(PALETTE_FADE_PTR pfd, FADEREQ req, FADEBUF select, u16 color_pos)
{
  if(select == FADEBUF_SRC){
    return pfd->dat[req].def_wk[color_pos];
  }
  if(select == FADEBUF_TRANS){
    return pfd->dat[req].trans_wk[color_pos];
  }

  GF_ASSERT(0 && "バッファ指定が間違っています\n");
  return 0;
}




//==============================================================================
//  個別ツール
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   ソフトフェードイン、アウト
 *
 * @param   src     元パレットデータへのポインタ
 * @param   dest    変更データ代入先
 * @param   col_num   変更するカラー数
 * @param   evy     係数(0～16) (変更の度合い、0(元の色)～2,3..(中間色)～16(指定した色)
 * @param   next_rgb  変更後の色を指定
 */
//--------------------------------------------------------------
void SoftFade(const u16 *src, u16 *dest, u16 col_num, u8 evy, u16 next_rgb)
{
  u16 i;
  int red, green, blue;
  int next_red, next_green, next_blue;

  next_red = ((PLTT_DATA*)&next_rgb)->Red;
  next_green = ((PLTT_DATA*)&next_rgb)->Green;
  next_blue = ((PLTT_DATA*)&next_rgb)->Blue;

  for(i = 0; i < col_num; i++){
    red = ((PLTT_DATA*)&src[i])->Red;
    green = ((PLTT_DATA*)&src[i])->Green;
    blue = ((PLTT_DATA*)&src[i])->Blue;

    dest[i] =
      FADE_CHANGE(red, next_red, evy) |
      (FADE_CHANGE(green, next_green, evy) << 5) |
      (FADE_CHANGE(blue, next_blue, evy) << 10);
  }
}

//--------------------------------------------------------------
/**
 * @brief   PFDを引数としたカラー加減算
 *
 * @param   pfd       パレットフェードシステムへのポインタ
 * @param   req       リクエストデータ番号
 * @param   start_pos   ソフトフェード開始位置(カラー単位)
 * @param   col_num     start_posから何個のカラーを加減算対象とするか(カラー単位)
 * @param   evy       EVY値
 * @param   next_rgb    変更後の色
 */
//--------------------------------------------------------------
void SoftFadePfd(PALETTE_FADE_PTR pfd, FADEREQ req, u16 start_pos, u16 col_num,
  u8 evy, u16 next_rgb)
{
  GF_ASSERT(pfd->dat[req].def_wk != NULL && pfd->dat[req].trans_wk != NULL);

  SoftFade(&pfd->dat[req].def_wk[start_pos], &pfd->dat[req].trans_wk[start_pos],
    col_num, evy, next_rgb);
}

//--------------------------------------------------------------
/**
 * @brief   カラー加減算(ビット指定：パレット単位)
 *
 * @param   src       元パレットデータへのポインタ
 * @param   dest      変更データ代入先
 * @param   fade_bit    加減算対象のビット
 * @param   evy       EVY値
 * @param   next_rgb    変更後の色
 */
//--------------------------------------------------------------
void ColorConceChange(const u16 *src, u16 *dest, u16 fade_bit, u8 evy, u16 next_rgb)
{
  int offset = 0;

  while(fade_bit){
    if(fade_bit & 1){
      SoftFade(&src[offset], &dest[offset], 16, evy, next_rgb);
    }
    fade_bit >>= 1;
    offset += 16;
  }
}

//--------------------------------------------------------------
/**
 * @brief   PFDを引数としたカラー加減算(ビット指定：パレット単位)
 *
 * @param   pfd       パレットフェードシステムへのポインタ
 * @param   req       リクエストデータ番号
 * @param   fade_bit    加減算対象のビット
 * @param   evy       EVY値
 * @param   next_rgb    変更後の色
 */
//--------------------------------------------------------------
void ColorConceChangePfd(PALETTE_FADE_PTR pfd, FADEREQ req, u16 fade_bit, u8 evy, u16 next_rgb)
{
  int offset = 0;

  GF_ASSERT(pfd->dat[req].def_wk != NULL && pfd->dat[req].trans_wk != NULL);

  while(fade_bit){
    if(fade_bit & 1){
      SoftFadePfd(pfd, req, offset, 16, evy, next_rgb);
    }
    fade_bit >>= 1;
    offset += 16;
  }
}

// =============================================================================
//
//
//  ■カラー操作関数郡  add goto
//
//
// =============================================================================
#define RGBtoY(r,g,b) (((r)*76 + (g)*151 + (b)*29) >> 8)
#define COL_FIL(c, p) ((u16)((p)*(c))>>8)

//--------------------------------------------------------------
/**
 * @brief グレースケール化
 *
 * @param pal       変更対象パレットデータ
 * @param pal_size    変更サイズ(何色変更するか)
 *
 * @retval  none
 *
 */
//--------------------------------------------------------------
void PaletteGrayScale(u16* pal, int pal_size)
{
  int i, r, g, b;
  u32 c;

  for(i = 0; i < pal_size; i++){
    r = (*pal) & 0x1f;
    g = ((*pal) >> 5) & 0x1f;
    b = ((*pal) >> 10) & 0x1f;

    c = RGBtoY(r,g,b);

    *pal = (u16)((c<<10)|(c<<5)|c);
    pal++;
  }
}

//--------------------------------------------------------------
/**
 * @brief グレースケール化(白黒調反転)
 *
 * @param pal       変更対象パレットデータ
 * @param pal_size    変更サイズ(何色変更するか)
 *
 * @retval  none
 *
 */
//--------------------------------------------------------------
void PaletteGrayScaleFlip(u16* pal, int pal_size)
{
  int i, r, g, b;
  u32 c;

  for(i = 0; i < pal_size; i++){
    r = 31 - ((*pal) & 0x1f);
    g = 31 - (((*pal) >> 5) & 0x1f);
    b = 31 - (((*pal) >> 10) & 0x1f);

    c = RGBtoY(r,g,b);

    *pal = (u16)((c<<10)|(c<<5)|c);
    pal++;
  }
}


//--------------------------------------------------------------
/**
 * @brief グレースケールから、RGBの比率をかける
 *
 * @param pal
 * @param pal_size
 * @param rp
 * @param gp
 * @param bp
 *
 * @retval  none
 *
 *     r = g = b = 256 でグレースケール関数と同じ動作
 *
 */
//--------------------------------------------------------------
void PaletteColorChange(u16* pal, int pal_size, int rp, int gp, int bp)
{
  int i, r, g, b;
  u32 c;

  for(i = 0; i < pal_size; i++){
    r = ((*pal) & 0x1f);
    g = (((*pal) >> 5) & 0x1f);
    b = (((*pal) >> 10) & 0x1f);

    c = RGBtoY(r,g,b);

    r = COL_FIL(c, rp);
    g = COL_FIL(c, gp);
    b = COL_FIL(c, bp);

    if (r > 31){ r = 31; }
    if (g > 31){ g = 31; }
    if (b > 31){ b = 31; }

    *pal = (u16)((b<<10)|(g<<5)|r);
    pal++;
  }
}


//--------------------------------------------------------------
/**
 * @brief ポケモンのパレットを変換して転送
 *
 * @param pfd
 * @param fileIdx
 * @param dataIdx
 * @param heap
 *
 * @retval  none
 *
 */
//--------------------------------------------------------------
void PokeColorChange(PALETTE_FADE_PTR pfd, u32 fileIdx, u32 dataIdx, HEAPID heap,
           FADEREQ req, u32 trans_size, u16 pos, int r, int g, int b)
{
  NNSG2dPaletteData *pal_data;
  void *arc_data;

  arc_data = GFL_ARC_UTIL_LoadPalette(fileIdx, dataIdx, &pal_data, heap);
  GF_ASSERT(arc_data != NULL);

  if(trans_size == 0){
    trans_size = pal_data->szByte;
  }

  //PaletteGrayScale(pal_data->pRawData, 16);
  PaletteColorChange(pal_data->pRawData, 16, r,g,b);

  PaletteWorkSet(pfd, pal_data->pRawData, req, pos, trans_size);

  GFL_HEAP_FreeMemory(arc_data);
}


