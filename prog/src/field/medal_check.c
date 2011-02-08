//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medal_check.c
 *	@brief  メダル達成関数
 *	@author	Masayuki Onoue
 *	@date		2011.02.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "savedata/record.h"
#include "system/medaldata.h" // fro MEDALDATA
#include "arc_def.h"          // for ARCID_MEDALDATA
#include "savedata\medalbox.h"
#include "../../../resource/medal/data/mdlrec_id.h" // for MDLRECID_START etc
#include "field/medal_check.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define   HINT_LEVEL_MAX  (6) // ヒントレベルの種類数

//----------------------------------------------------------------------------
/**
 *	@brief  タイトルメダル、ヒントメダルの達成状態更新関数
 *
 *	@param	*p_sv 
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDAL_AchiveCheck( SAVE_CONTROL_WORK *p_sv , HEAPID heapID )
{
  int i;
  MedalID     medal_id;
  MEDALBOX    *p_medal_box;
  u8          medal_state;
  ARCHANDLE   *p_handle;
  MEDALDATA   medal_data[MDLID_MAX];
  RECORD      *p_rec;
  u8          open_hint_level = 0;
  u8          title_open_cnt[HINT_LEVEL_MAX];
  u8          title_open_max[HINT_LEVEL_MAX];

  GF_ASSERT_HEAVY( p_sv );

  // 獲得条件アーカイブデータハンドル取得
  p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , heapID );

  // 達成条件データを取得
  GFL_ARC_LoadDataOfsByHandle( p_handle , 0 , 0 , sizeof(MEDALDATA)*MDLID_MAX ,medal_data );

  // セーブデータの取得
  p_medal_box = SaveData_GetMedalBox( p_sv );
  p_rec = SaveData_GetRecord(p_sv);

  // 初期化
  for( i = 0; i < HINT_LEVEL_MAX ; i++)
  {
    title_open_cnt[i] = 0;
    title_open_max[i]  = 0;
  }

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );

    // タイトル達成処理
    if( medal_state < MEDAL_STATE_TITLE_ACHIVE )
    {
      if( medal_data[medal_id].cmp_id < MDLRECID_START )
      {
        // レコードデータとの比較
        u32 rec_data = RECORD_Get( p_rec , medal_data[medal_id].cmp_id );

        // タイトル達成を行っていない場合はまず調べる
        if( medal_state < MEDAL_STATE_TITLE_ACHIVE )
        {
          if( medal_data[medal_id].target_count <= rec_data )
          {
            MEDALBOX_SetTitleMedalAchive( p_medal_box , medal_id );
          }
        }
      }
      else
      {
        // レコード意外と比較
        switch( medal_data[medal_id].cmp_id )
        {
        default:
          break;
        }
      }
    }

    // タイトル獲得数とヒントレベル毎の獲得最大数を取得
    // (最初からとヒントレベル式のものを数える)
    medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );
    if( (medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_FIRST) || 
        (medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_LEVEL) )
    {
      if( (medal_state == MEDAL_STATE_TITLE_GET) ||
          (medal_state == MEDAL_STATE_TITLE_ACHIVE))
      {
        title_open_cnt[medal_data[medal_id].hint_level]++;
      }
      title_open_max[ medal_data[medal_id].hint_level ]++;
    }
  }

  // ヒントレベルごとのタイトル取得数が過半数以上を取得している場合は、そのレベル＋１のヒントレベルまで全て開放する
  for( i = HINT_LEVEL_MAX-1 ; i >= 0; i-- )
  {
    if( (title_open_cnt[i]) >= (title_open_max[ i ]/2) )
    {
      for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
      {
        medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );

        // ヒントレベルで開放するものだけ開放
        if( medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_LEVEL )
        {
          if( medal_data[medal_id].hint_level<= i+1 )
          {
            if( medal_state < MEDAL_STATE_HINT_ACHIVE )
            {
              MEDALBOX_SetHintMedalAchive( p_medal_box , medal_id );
            }
          }
        }
      }
      break;
    }
  }

  {
    int hint_level_max = -1;

    // ヒントが開放されているもので一番ヒントレベルが高いものを調べる(ヒントレベル式のもの限定)
    for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
    {
      medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );

      // ヒントレベルで開放するものだけ開放
      if( medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_LEVEL )
      {
        if( (medal_state == MEDAL_STATE_TITLE_GET) ||
            (medal_state == MEDAL_STATE_TITLE_ACHIVE))
        {
          if( hint_level_max < medal_data[medal_id].hint_level )
          {
            hint_level_max = medal_data[medal_id].hint_level;
          }
        }
      }
    }

    // 一番ヒントレベル未満のヒントは全て開放
    if( hint_level_max >= 0 )
    {
      for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
      {
        medal_state = MEDALBOX_GetMedalState( p_medal_box , medal_id );

        // ヒントレベルで開放するものだけ開放
        if( medal_data[medal_id].hint_type == MEDALDATA_HINTTYPE_LEVEL )
        {
          // 取得している最大未満は全て開放
          if( medal_data[medal_id].hint_level < hint_level_max )
          {
            if( medal_state < MEDAL_STATE_HINT_ACHIVE )
            {
              MEDALBOX_SetHintMedalAchive( p_medal_box , medal_id );
            }
          }
        }

      }
    }
  }

  // アーカイブクローズ
  GFL_ARC_CloseDataHandle(p_handle);
}

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
void MEDAL_InitHintMedal( SAVE_CONTROL_WORK *p_sv , HEAPID heapID )
{
  MedalID     medal_id;
  MEDALBOX    *p_medal_box;
  ARCHANDLE   *p_handle;
  MEDALDATA   medal_data[MDLID_MAX];
  RECORD      *p_rec;
  RTCDate     rtc;
  u8          medal_state;

  GF_ASSERT_HEAVY( p_sv );

  // 獲得条件アーカイブデータハンドル取得
  p_handle = GFL_ARC_OpenDataHandle( ARCID_MEDALDATA , heapID );

  // 達成条件データを取得
  GFL_ARC_LoadDataOfsByHandle( p_handle , 0 , 0 , sizeof(MEDALDATA)*MDLID_MAX ,medal_data );

  // セーブデータの取得
  p_medal_box = SaveData_GetMedalBox( p_sv );
  p_rec = SaveData_GetRecord(p_sv);
  
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

