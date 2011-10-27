//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medal_common.c
 *	@brief  メダル共通処理
 *	@author	Masayuki Onoue
 *	@date		2011.10.27
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
// ライブラリ
#include  <gflib.h>

//システム
#include "gamesystem/game_data.h"

#include "system/medaldata.h"   // fro MEDALDATA
#include "arc_def.h"            // for ARCID_MEDALDATA
#include "savedata/medalbox.h"  // メダルセーブデータ

#include "medal_common.h"       

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

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void InitHintMedal( MEDALBOX *p_medal_box , HEAPID heapID );


//----------------------------------------------------------------------------
/**
 *	@brief  ヒント取得条件が最初からのメダルをヒント取得状態にする
 *
 *	@param	*p_sv 
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void InitHintMedal( MEDALBOX *p_medal_box , HEAPID heapID )
{
  MedalID     medal_id;
  ARCHANDLE   *p_handle;
  MEDALDATA   medal_data[MDLID_MAX];
  RTCDate     rtc;
  u8          medal_state;

  GF_ASSERT_HEAVY( p_medal_box );

  // 獲得条件アーカイブデータハンドル取得
  p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , heapID );

  // 達成条件データを取得
  GFL_ARC_LoadDataOfsByHandle( p_handle , 0 , 0 , sizeof(MEDALDATA)*MDLID_MAX ,medal_data );

  // 現在の時間を取得する
  GFL_RTC_GetDate(&rtc);

  // 最初から持っているヒントメダルを初期化する
  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );
    if( medal_state < MEDAL_STATE_HINT_ACHIVE )
    {
      if( medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_FIRST )
      {
        MEDALBOX_SetHintMedalAchive( p_medal_box , medal_id );
        MEDALBOX_SetHintMedalGetFlg( p_medal_box , medal_id ,rtc.year ,rtc.month ,rtc.day);
      }
    }
  }

  // アーカイブクローズ
  GFL_ARC_CloseDataHandle(p_handle);
}

//----------------------------------------------------------------------------
/**
 *	@brief  デフォルトメダルデータの設定
 *
 *	@param	*p_sv 
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDAL_COMMON_SetDefaultData( MEDALBOX *p_sv , HEAPID heapID )
{
  // ヒントメダルの初期化
  InitHintMedal( p_sv , heapID );
}

