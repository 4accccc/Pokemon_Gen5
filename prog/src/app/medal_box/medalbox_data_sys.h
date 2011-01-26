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

#define PAGE_MEDAL_MAX (5) // 1ページあたりのメダル数

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
  MDL_SORT_TYPE,
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
  u8  graphic_type;
  MDL_NEWSTATE new_state; // 新着状態( 0=新着でない,1=新着,2=新着書き込み)
} MY_MEDAL_ST;


//-------------------------------------
///	取得済みメダルデータ構造体
//=====================================
typedef struct {
  MDL_SORTTYPE  sort_type;
  MedalID       sort_medal_id[MDLID_MAX];
  MedalID       push_medalid;
  MY_MEDAL_ST   *p_medal_st;
  u8            boxrank;
  u8            medal_cnt;
  u8            page_max;
}MY_MEDALDATA;

//-------------------------------------
///	取得済みメダルデータ構造体
//=====================================
typedef struct {
  MY_MEDAL_ST   *p_medal_st[PAGE_MEDAL_MAX];
  u8  medal_max;
  u8  page_num;   // ページ番号
}PAGEDATA;
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern void MEDALDATASYS_InitMedalData( MY_MEDALDATA* p_my_medal , GAMEDATA* p_gamedata , HEAPID heapID);
extern void MEDALDATASYS_DeleteMedalData( MY_MEDALDATA* p_my_medal );
extern void MEDALDATASYS_GetPageMedal( MY_MEDALDATA *p_my_medal , PAGEDATA* p_page , u16 page_num );



