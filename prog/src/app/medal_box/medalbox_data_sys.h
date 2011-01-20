//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medalbox_data_sys.h
 *	@brief  メダルボックス用データ操作
 *	@author	Masayuki Onoue
 *	@data		2011.01.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "system/medaldata.h"
#include "savedata/medalbox.h"
#include "gamesystem/gamesystem.h"
#include "system/main.h"            // HEAPID

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
/// 新着状態
//=====================================
typedef enum
{
  MDL_NO_NEW,   // 新着でない
  MDL_IS_NEW,   // 新着状態
  MDL_WRITE_NEW,// 新着書き込み
} MDL_NEWSTATE;

//-------------------------------------
/// ソートタイプ
//=====================================
typedef enum
{
  MDL_SORT_ABC,
} MDL_SORTTYPE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
/// メダルデータ	
//=====================================
typedef struct {
  MedalID medal_id;
  u8  medal_state;        // メダル状態
  u8  year;
  u8  month;
  u8  day;
  MDL_NEWSTATE new_state; // 新着状態( 0=新着でない,1=新着,2=新着書き込み)
} MY_MEDAL_ST;


//-------------------------------------
///	取得済みメダルデータ構造体
//=====================================
typedef struct {
  MDL_SORTTYPE  sort_type;
  MedalID       *p_medal_id;
  MedalID       push_medalid;
  MY_MEDAL_ST   *p_medal_st;
  u8            boxrank;
  u8            medal_cnt;
}MY_MEDALDATA;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern void MEDALDATASYS_InitMedalData( MY_MEDALDATA* p_my_medal , GAMEDATA* p_gamedata , HEAPID heapID);
extern void MEDALDATASYS_DeleteMedalData( MY_MEDALDATA* p_my_medal );



