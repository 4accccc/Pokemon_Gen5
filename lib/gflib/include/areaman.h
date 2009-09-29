//==============================================================================
/**
 *
 *@file   areaman.h
 *@brief  VRAMキャラデータ領域マネージャ
 *@author taya
 *@data   2006.10.27
 *
 */
//==============================================================================
#ifndef __AREAMAN_H__
#define __AREAMAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AREAMAN_DEBUG
//--------------------------------------------
// 不完全型定義
//--------------------------------------------
typedef struct _GFL_AREAMAN   GFL_AREAMAN;

//--------------------------------------------
//
//--------------------------------------------
typedef u32   GFL_AREAMAN_POS;

#define AREAMAN_POS_NOTFOUND  (0xffffffff)

//------------------------------------------------------------------
/**
 * 領域マネージャ作成
 *
 * @param   maxBlock    [in] このマネージャーが管理するブロック数
 * @param   heapID      [in] 使用ヒープＩＤ
 *
 * @retval  GFL_AREAMAN*  領域マネージャポインタ
 */
//------------------------------------------------------------------
extern GFL_AREAMAN*
  GFL_AREAMAN_Create
    ( u32 maxBlock, u16 heapID );
//------------------------------------------------------------------
/**
 * 領域マネージャ破棄
 *
 * @param   man     [in] 領域マネージャポインタ
 */
//------------------------------------------------------------------
extern void
  GFL_AREAMAN_Delete
    ( GFL_AREAMAN* man );
//------------------------------------------------------------------
/**
 * 領域先頭から末尾まで、空いている所を探して確保
 *
 * @param   man       [in] マネージャ
 * @param   blockNum    [in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS   確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
extern u32
  GFL_AREAMAN_ReserveAuto
    ( GFL_AREAMAN* man, u32 blockNum );


//------------------------------------------------------------------
/**
 * 領域末尾から先頭まで、空いている所を探して確保
 *
 * @param   man       [in] マネージャ
 * @param   blockNum    [in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS   確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
extern u32
  GFL_AREAMAN_ReserveAutoLo
    ( GFL_AREAMAN* man, u32 blockNum );

//------------------------------------------------------------------
/**
 * 領域の指定範囲内から空いている所を探して確保（前方から探索）
 *
 * @param   man       [in] マネージャ
 * @param   startBlock    [in] 探索開始ブロック
 * @param   numBlockArea  [in] 探索ブロック範囲
 * @param   numBlockReserve [in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS   確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
extern u32
  GFL_AREAMAN_ReserveAssignArea
    ( GFL_AREAMAN* man, u32 startBlock, u32 numBlockArea, u32 numBlockReserve );


//------------------------------------------------------------------
/**
 * 領域の指定範囲内から空いている所を探して確保（後方から探索）
 *
 * @param   man       [in] マネージャ
 * @param   startBlock    [in] 探索開始ブロック
 * @param   numBlockArea  [in] 探索ブロック範囲
 * @param   numBlockReserve [in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS   確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
extern u32
  GFL_AREAMAN_ReserveAssignAreaLo
    ( GFL_AREAMAN* man, u32 startBlock, u32 numBlockArea, u32 numBlockReserve );


//------------------------------------------------------------------
/**
 * 領域の指定位置から確保
 *
 * @param   man       [in] マネージャ
 * @param   blockNum    [in] 確保したいブロック数
 *
 * @retval  BOOL      TRUEで成功
 */
//------------------------------------------------------------------
extern BOOL
  GFL_AREAMAN_ReserveAssignPos
    ( GFL_AREAMAN* man, GFL_AREAMAN_POS pos, u32 blockNum );
//------------------------------------------------------------------
/**
 * 領域を開放
 *
 * @param   man     [in] マネージャ
 * @param   pos     [in] 確保している位置
 * @param   blockNum  [in] 確保しているブロック数
 */
//------------------------------------------------------------------
extern void
  GFL_AREAMAN_Release
    ( GFL_AREAMAN* man, GFL_AREAMAN_POS pos, u32 blockNum );




#ifdef AREAMAN_DEBUG
//------------------------------------------------------------------
/**
 * デバッグ出力開始／停止
 *
 * @param   man
 * @param   flag  TRUEで開始／FALSEで停止
 */
//------------------------------------------------------------------
extern void
  GFL_AREAMAN_SetPrintDebug
    ( GFL_AREAMAN* man, BOOL flag );

//------------------------------------------------------------------
/**
 * デバッグ出力実行
 *
 * @param   man
 */
//------------------------------------------------------------------
extern void GFL_AREAMAN_PrintDebug( GFL_AREAMAN* man );

#else
#define GFL_AREAMAN_SetPrintDebug( m, f )  /* */
#endif

#ifdef __cplusplus
}/* extern "C" */
#endif


#endif  // __AREAMAN_H__
