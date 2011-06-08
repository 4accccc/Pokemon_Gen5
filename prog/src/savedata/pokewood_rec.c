//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		pokewood_rec.c
 *	@brief  ポケウッド用戦闘録画セーブデータ
 *	@author	Toru=Nagihashi
 *	@data		2011.06.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/main.h"
#include "savedata/save_tbl.h"

#include "pokewood_rec_local.h"
#include "savedata/pokewood_rec.h"


//=============================================================================
/**
 *					デバッグ
*/
//=============================================================================
#ifdef PM_DEBUG
#define DEBUG_POKEWOOD_REC_PRINT
#endif //PM_DEBUG


#ifdef DEBUG_POKEWOOD_REC_PRINT
#define POKEWOOD_REC_Printf(...) OS_TPrintf(__VA_ARGS__)
#endif //DEBUG_POKEWOOD_REC_PRINT

#ifndef POKEWOOD_REC_Printf
#define POKEWOOD_REC_Printf(...) /* none */
#endif //POKEWOOD_REC_Printf


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ポケウッド戦闘録画セーブデータ
//=====================================
typedef struct
{
  ///後々battle_recのようにGDSプロフィールやヘッダが必要になる可能性を
  //考えて切り分けておく
  POKEWOOD_REC_WORK rec;  ///<戦闘録画本体 この構造体は最後尾にすること
} POKEWOOD_REC_SAVEDATA;

static POKEWOOD_REC_SAVEDATA * sp_pokewood_rec_sv = NULL;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static SAVE_EXTRA_ID PokeWood_Rec_LoadDataNumToSaveExtraID( POKEWOOD_LOADDATA_NUM num );

//----------------------------------------------------------------------------
/**
 *	@brief　ポケウッドセーブデータを外部から展開用に初期化
 *
 *	@param	HEAPID heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void POKEWOOD_REC_Init( HEAPID heapID )
{
  GF_ASSERT( sp_pokewood_rec_sv == NULL );
  sp_pokewood_rec_sv = GFL_HEAP_AllocClearMemory( heapID, SAVESIZE_EXTRA_POKEWOOD);
}
//----------------------------------------------------------------------------
/**
 *	@brief　ポケウッドセーブデータを破棄
 */
//-----------------------------------------------------------------------------
void POKEWOOD_REC_Exit( void )
{
  GF_ASSERT( sp_pokewood_rec_sv != NULL );
  GFL_HEAP_FreeMemory( sp_pokewood_rec_sv );
  sp_pokewood_rec_sv = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  読み込み
 *
 *	@param	SAVE_CONTROL_WORK *p_wk　セーブデータ
 *	@param	heapID    ヒープID
 *	@param	num       ロードするセーブデータ番号
 *
 *	@return　ロード結果
 */
//-----------------------------------------------------------------------------
LOAD_RESULT POKEWOOD_REC_Load( SAVE_CONTROL_WORK *p_sv, HEAPID heapID, POKEWOOD_LOADDATA_NUM num )
{
  LOAD_RESULT result;
  const SAVE_EXTRA_ID extraID = PokeWood_Rec_LoadDataNumToSaveExtraID( num );

  GF_ASSERT( sp_pokewood_rec_sv != NULL );

  GFL_STD_MemClear( sp_pokewood_rec_sv, SAVESIZE_EXTRA_POKEWOOD );

  //読み込み
  result = SaveControl_Extra_LoadWork(
    p_sv, extraID, heapID, sp_pokewood_rec_sv, SAVESIZE_EXTRA_POKEWOOD);

  SaveControl_Extra_Unload( p_sv, extraID );

  //ロード結果チェック
  if( result == LOAD_RESULT_NULL )
  {
    POKEWOOD_REC_Printf("ポケウッド録画データが存在しません\n");
    return RECLOAD_RESULT_NULL;  //初期化データの為、データなし
  }
  else if( result != LOAD_RESULT_OK )
  {
    POKEWOOD_REC_Printf("ポケウッド録画データの読み込みエラーです\h");
    return RECLOAD_RESULT_ERROR;
  }

  //整合性チェック  TODO
  if( 0 )
  {
    POKEWOOD_REC_Printf("ポケウッド録画データが不正です\n");
    return RECLOAD_RESULT_NG;
  }
  else
  {
    return RECLOAD_RESULT_OK;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  セーブ
 *
 *	@param	GAMEDATA *p_gamedata  ゲームデータ
 *	@param	heapID　              ヒープID
 *	@param	num                   読み込むセーブデータ番号
 *	@param	*p_work0　            セーブ用ワーク１（０クリアしてから渡すこと）
 *
 *	@return　セーブ結果
 */
//-----------------------------------------------------------------------------
SAVE_RESULT POKEWOOD_REC_Save( GAMEDATA *p_gamedata, HEAPID heapID, POKEWOOD_LOADDATA_NUM num, u16 *p_work )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
  };

  SAVE_RESULT result;

  SAVE_CONTROL_WORK *p_sv     = GAMEDATA_GetSaveControlWork(p_gamedata);
  const SAVE_EXTRA_ID extraID = PokeWood_Rec_LoadDataNumToSaveExtraID( num );
  u16 *p_seq = p_work;

  GF_ASSERT( sp_pokewood_rec_sv != NULL );

  switch(*p_seq){
  case SEQ_START:
    //セーブ対象の外部セーブ領域のセーブシステムを作成
    SaveControl_Extra_SystemSetup(p_sv, extraID, heapID, sp_pokewood_rec_sv, SAVESIZE_EXTRA_POKEWOOD);

    GAMEDATA_ExtraSaveAsyncStart(p_gamedata, extraID);
    (*p_seq)++;
    break;
  case SEQ_WAIT:
    result = GAMEDATA_ExtraSaveAsyncMain(p_gamedata, extraID);

    if(result == SAVE_RESULT_OK || result == SAVE_RESULT_NG)
    {
      //外部セーブ完了。セーブシステムを破棄
      SaveControl_Extra_UnloadWork(p_sv, extraID);
      return result;
    }
    break;
  }
  return SAVE_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief 存在しているかチェック 何らかのデータがあるかどうか
 *	@return　TRUE存在　FALSEなし
 */
//-----------------------------------------------------------------------------
BOOL POKEWOOD_REC_IsExist( void )
{
  GF_ASSERT( sp_pokewood_rec_sv != NULL );
  return GFL_STD_CODED_CheckSum(sp_pokewood_rec_sv, SAVESIZE_EXTRA_POKEWOOD) != 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief ポケウッド録画データ本体取得
 *
 *	@return 録画データ本体
 */
//-----------------------------------------------------------------------------
POKEWOOD_REC_WORK* POKEWOOD_REC_GetWork( void )
{
  GF_ASSERT( sp_pokewood_rec_sv != NULL );
  return &sp_pokewood_rec_sv->rec;
}


//----------------------------------------------------------------------------
/**
 *	@brief　ロードデータ番号を外部セーブ番号にする
 *
 *	@param	int num LOADDATA_MYREC～LOADDATA_DOWNLOAD2
 *
 *	@return SAVE_EXTRA_ID
 */
//-----------------------------------------------------------------------------
static SAVE_EXTRA_ID PokeWood_Rec_LoadDataNumToSaveExtraID( POKEWOOD_LOADDATA_NUM num )
{
  switch( num )
  {
  default:
    GF_ASSERT(0);
  case LOADDATA_POKEWOOD0:
    return SAVE_EXTRA_ID_POKEWOOD_00;
  case LOADDATA_POKEWOOD1:
    return SAVE_EXTRA_ID_POKEWOOD_01;
  }
}
