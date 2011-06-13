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
  POKEWOOD_REC_WORK rec;  ///<戦闘録画本体
  u32 crc;                ///<最後尾にすること
} POKEWOOD_REC_SAVEDATA;

//セーブ領域より小さくないとダメ
SDK_COMPILER_ASSERT( sizeof(POKEWOOD_REC_SAVEDATA) < SAVESIZE_EXTRA_POKEWOOD );

static POKEWOOD_REC_SAVEDATA * sp_pokewood_rec_sv = NULL;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static SAVE_EXTRA_ID PokeWood_Rec_LoadDataNumToSaveExtraID( POKEWOOD_LOADDATA_NUM num );

//----------------------------------------------------------------------------
/**
 *	@brief  ワークを初期化
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
void POKEWOOD_REC_WorkInit( void *p_wk_adrs )
{
  GFL_STD_MemClear( p_wk_adrs, sizeof(POKEWOOD_REC_SAVEDATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief ワークサイズを取得
 */
//-----------------------------------------------------------------------------
u32 POKEWOOD_REC_GetWorkSize( void )
{
  return sizeof( POKEWOOD_REC_SAVEDATA );
}

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
 *	@return　ロード結果 LOAD_RESULT_BREAKはこない。破壊の場合はNG
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
    return LOAD_RESULT_NULL;  //初期化データの為、データなし
  }
  else if( result != LOAD_RESULT_OK )
  {
    POKEWOOD_REC_Printf("ポケウッド録画データの読み込みエラーです\h");
    return LOAD_RESULT_ERROR;
  }

  //整合性チェック
  if( sp_pokewood_rec_sv->crc == GFL_STD_CrcCalc(sp_pokewood_rec_sv,sizeof(POKEWOOD_REC_SAVEDATA) - sizeof(sp_pokewood_rec_sv->crc) ) )
  {
    return LOAD_RESULT_OK;
  }
  else
  {
    POKEWOOD_REC_Printf("ポケウッド録画データが不正です\n");
    return LOAD_RESULT_NG;
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
  
    //CRC作成
    sp_pokewood_rec_sv->crc = GFL_STD_CrcCalc(sp_pokewood_rec_sv,sizeof(POKEWOOD_REC_SAVEDATA) - sizeof(sp_pokewood_rec_sv->crc) );


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
 *	@brief 存在しているかチェック 読み出されているかつ壊れていないならばTRUE
 *	@return　TRUE存在　FALSEなしor壊れている
 */
//-----------------------------------------------------------------------------
BOOL POKEWOOD_REC_IsExist( void )
{
  GF_ASSERT( sp_pokewood_rec_sv != NULL );

  return (GFL_STD_CODED_CheckSum(sp_pokewood_rec_sv, SAVESIZE_EXTRA_POKEWOOD) != 0)
    && (sp_pokewood_rec_sv->crc == GFL_STD_CrcCalc(sp_pokewood_rec_sv,sizeof(POKEWOOD_REC_SAVEDATA) - sizeof(sp_pokewood_rec_sv->crc) ));
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
 *	@brief  録画データから詳細データ取得
 *
 *	@param	POKEWOOD_REC_PARAM param  何のデータを取得するか
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM param )
{
  GF_ASSERT( sp_pokewood_rec_sv != NULL );
  switch( param )
  {
  case POKEWOOD_REC_PARAM_SENARIO:
    return sp_pokewood_rec_sv->rec.senario.senario_no;
  default:
    GF_ASSERT(0);
    return 0;
  }
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
  static const SAVE_EXTRA_ID sc_loaddata_to_extra_id[]  =
  {
    SAVE_EXTRA_ID_POKEWOOD_00,
    SAVE_EXTRA_ID_POKEWOOD_01,
    SAVE_EXTRA_ID_POKEWOOD_02,
    SAVE_EXTRA_ID_POKEWOOD_03,
    SAVE_EXTRA_ID_POKEWOOD_04,
  };

  num -= LOADDATA_MAX;

  if( num >= NELEMS(sc_loaddata_to_extra_id)
      || num < 0 )
  {
    GF_ASSERT(0);
    num = 0;
  }

  return sc_loaddata_to_extra_id[ num ];
}
