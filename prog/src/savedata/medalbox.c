//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medalbox.c
 *	@brief  メダルボックスデータ
 *	@author	Masayuki Onoue
 *	@data		2011.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "savedata/medalbox.h"
#include "savedata/save_tbl.h"
#include "medalbox_local.h"

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static MEDAL_ST* GetMedalST( MEDALBOX *p_mdlbox , MedalID medal_id );
static const MEDAL_ST* GetMedalSTConst( const MEDALBOX *p_mdlbox , MedalID medal_id );


//=============================================================================
/**
 *					ローカル関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  メダルSTの取得
 *
 *	@param	*p_mdlbox 
 *	@param	medal_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static MEDAL_ST* GetMedalST( MEDALBOX *p_mdlbox , MedalID medal_id )
{
  GF_ASSERT( p_mdlbox );
  GF_ASSERT( medal_id < MDLID_MAX );
  return &p_mdlbox->medal[ medal_id ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルSTの取得(const)
 *
 *	@param	*p_mdlbox 
 *	@param	medal_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const MEDAL_ST* GetMedalSTConst( const MEDALBOX *p_mdlbox , MedalID medal_id )
{
  GF_ASSERT( p_mdlbox );
  GF_ASSERT( medal_id < MDLID_MAX );
  return &p_mdlbox->medal[ medal_id ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルの状態を設定
 *
 *	@param	p_medal_st 
 *	@param	state 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetMedalState( MEDAL_ST* p_medal_st , u8 state )
{
  GF_ASSERT( p_medal_st->medal_state < state );

  p_medal_st->medal_state = state;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルの日付を設定
 *
 *	@param	p_medal_st 
 *	@param	year 
 *	@param	month 
 *	@param	day 
 *
 */
//-----------------------------------------------------------------------------
static void SetMedalDate( MEDAL_ST* p_medal_st, u8 year, u8 month, u8 day )
{
  GF_ASSERT( year <= 99 );
  GF_ASSERT( month >= 1 );
  GF_ASSERT( month <= 12 );
  GF_ASSERT( day >= 1 );
  GF_ASSERT( day <= 31 );

  p_medal_st->get_year      = year;
  p_medal_st->get_month     = month;
  p_medal_st->get_day       = day;
}

//=============================================================================
/**
 *					公開関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	*p_mdlbox
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALBOX_Init(MEDALBOX *p_mdlbox)
{
  GFL_STD_MemFill(p_mdlbox, 0, sizeof(MEDALBOX));
  p_mdlbox->push_medal = MDLID_MAX;

  HABITATLIST_SAVE_Init( MEDALBOX_GetHabitatListSave(p_mdlbox) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックス初期化
 *
 *	@param	none
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
int MEDALBOX_GetWorkSize(void)
{
  return sizeof(MEDALBOX);
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルの状態を取得する
 *
 *	@param	*p_mdlbox
 *	@param	medal_id 
 *
 *	@retval メダル取得状況
 */
//-----------------------------------------------------------------------------
u8 MEDALBOX_GetMedalState( const MEDALBOX *p_mdlbox, MedalID medal_id )
{
  const MEDAL_ST* p_medal_st = GetMedalSTConst( p_mdlbox , medal_id );

  return p_medal_st->medal_state;
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイトルメダル達成状態にする
 *
 *	@param	* p_mdlbox
 *	@param	medal_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALBOX_SetTitleMedalAchive( MEDALBOX * p_mdlbox, MedalID medal_id )
{
  u8  medal_status = MEDALBOX_GetMedalState( p_mdlbox, medal_id );

  if ( medal_status < MEDAL_STATE_TITLE_ACHIVE )
  {
    MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );
    SetMedalState( p_medal_st , MEDAL_STATE_TITLE_ACHIVE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメダル達成フラグを立てる
 *
 *	@param	*p_mdlbox
 *	@param	medal_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALBOX_SetHintMedalAchive( MEDALBOX *p_mdlbox, MedalID medal_id )
{
  // メダル情報取得
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

  // ヒントより上位の状態の場合には設定しない
  if ( p_medal_st->medal_state == MEDAL_STATE_NONE )
  {
    // 新品フラグを立てる
    p_medal_st->is_new = TRUE;
    SetMedalState( p_medal_st , MEDAL_STATE_HINT_ACHIVE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  タイトルメダル取得(取得フラグ、達成フラグ、取得年月日、新着を同時に操作する）
 *
 *	@param	*p_mdlbox
 *	@param	medal_id
 *	@param	year
 *	@param	month
 *	@param	day 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALBOX_SetTitleMedalGetFlg( MEDALBOX *p_mdlbox, MedalID medal_id, u8 year, u8 month, u8 day )
{
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

  SetMedalState( p_medal_st , MEDAL_STATE_TITLE_GET );

  SetMedalDate( p_medal_st, year, month, day );

  // 新品フラグを立てる
  p_medal_st->is_new = TRUE;
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒントメダル取得
 *
 *	@param	*p_mdlbox
 *	@param	medal_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALBOX_SetHintMedalGetFlg( MEDALBOX *p_mdlbox, MedalID medal_id , u8 year, u8 month, u8 day)
{
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

  GF_ASSERT( p_medal_st->medal_state == MEDAL_STATE_HINT_ACHIVE );
  
  SetMedalState( p_medal_st , MEDAL_STATE_HINT_GET );

  SetMedalDate( p_medal_st, year, month, day );
}

//----------------------------------------------------------------------------
/**
 *	@brief  新品メダルフラグを落とす
 *
 *	@param	*p_mdlbox 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALBOX_SetNewMedalFlgOff( MEDALBOX *p_mdlbox, MedalID medal_id )
{
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

  p_medal_st->is_new = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イチオシメダルIDを設定する
 *
 *	@param	*p_mdlbox 
 *	@param	medal_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALBOX_SetPushMedalID( MEDALBOX *p_mdlbox , MedalID medal_id )
{
  // u8制限
  GF_ASSERT( medal_id <= 255 );
  GF_ASSERT( medal_id >= 0 );
  p_mdlbox->push_medal = (u8)medal_id;
}


//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスのランクアップ
 *
 *	@param	*p_mdlbox 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALBOX_RankUpMedalBox( MEDALBOX *p_mdlbox )
{
  if( p_mdlbox->box_rank < MEDALBOX_RANK_BW )
  {
    p_mdlbox->box_rank ++;
  }

  GF_ASSERT( p_mdlbox->box_rank <= MEDALBOX_RANK_BW);
}

//----------------------------------------------------------------------------
/**
 *	@brief  イチオシメダル番号取得
 *
 *	@param	*p_mdlbox 
 *
 *	@retval イチオシメダル番号
 */
//-----------------------------------------------------------------------------
MedalID MEDALBOX_GetPushMedalID( const MEDALBOX *p_mdlbox )
{
  GF_ASSERT( p_mdlbox );

  return p_mdlbox->push_medal;
}


//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスのランクを取得
 *
 *	@param	*p_mdlbox 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 MEDALBOX_GetMedalBoxRank( const MEDALBOX *p_mdlbox )
{
  GF_ASSERT( p_mdlbox );

  return p_mdlbox->box_rank;
}


//----------------------------------------------------------------------------
/**
 *	@brief  新品フラグの取得
 *
 *	@param	*p_mdlbox 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL MEDALBOX_GetNewMedalFlg( const MEDALBOX *p_mdlbox, MedalID medal_id )
{
  const MEDAL_ST* p_medal_st = GetMedalSTConst( p_mdlbox , medal_id );

  return p_medal_st->is_new;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル取得年月日の取得
 *
 *	@param	*p_mdlbox
 *	@param	medal_id 
 *	@param	*p_year 
 *	@param	*p_month 
 *	@param	*p_day 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void MEDALBOX_GetDate( const MEDALBOX *p_mdlbox, MedalID medal_id , u8 *p_year , u8 *p_month , u8 *p_day )
{
  const MEDAL_ST* p_medal_st = GetMedalSTConst( p_mdlbox , medal_id );
  
  GF_ASSERT( p_year );
  GF_ASSERT( p_month );
  GF_ASSERT( p_year );

  *p_year   = p_medal_st->get_year;
  *p_month  = p_medal_st->get_month;
  *p_day    = p_medal_st->get_day;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のランクでメダルボックスに入るメダル最大数を取得
 *
 *	@param	*p_mdlbox 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 MEDALBOX_GetMedalBoxMax( const MEDALBOX *p_mdlbox )
{
  u8 box_rank = MEDALBOX_GetMedalBoxRank( p_mdlbox );

  switch( box_rank )
  {
  case MEDALBOX_RANK_IRON:
    return MEDALBOX_RANK_IRON_MAX;
  case MEDALBOX_RANK_COPPER:
    return MEDALBOX_RANK_COPPER_MAX;
  case MEDALBOX_RANK_SILVER:
    return MEDALBOX_RANK_SILVER_MAX;
  case MEDALBOX_RANK_GOLD:
    return MEDALBOX_RANK_GOLD_MAX;
  case MEDALBOX_RANK_BW:
    return MEDALBOX_RANK_BW_MAX;
  default:
    GF_ASSERT( 0 );
    return 0;
  }

  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在の取得メダル数
 *
 *	@param	*p_mdlbox 
 *	@param	is_hint_count TRUEの時はヒントメダルを数える
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 MEDALBOX_GetMedalCount( const MEDALBOX *p_mdlbox , BOOL is_hint_count )
{
  MedalID medal_id;
  u16 count = 0;

  GF_ASSERT( p_mdlbox );

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    u32 ymd_sub = 0;
    const MEDAL_ST* p_medal_st = GetMedalSTConst( p_mdlbox , medal_id );
    ymd_sub = p_medal_st->get_year + p_medal_st->get_month + p_medal_st->get_day;
    if( is_hint_count )
    {
      // タイトル達成でかつ、日付が入っている場合はヒント取得として数える
      if( (p_medal_st->medal_state == MEDAL_STATE_TITLE_GET) ||
          (p_medal_st->medal_state == MEDAL_STATE_HINT_GET ) ||
          ((p_medal_st->medal_state == MEDAL_STATE_TITLE_ACHIVE) && (ymd_sub != 0) ))
      {
        count++;
      }
    }
    else
    {
      if( p_medal_st->medal_state == MEDAL_STATE_TITLE_GET )
      {
        count++;
      }
    }
  }

  return count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  チュートリアルフラグを取得する
 *
 *	@param	MEDALBOX *p_mdlbox 
 *
 *	@retval TRUEの時チュートリアルクリア
 */
//-----------------------------------------------------------------------------
u8 MEDALBOX_GetTutorialFlag( const MEDALBOX *p_mdlbox )
{
  return p_mdlbox->tutorial_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  チュートリアルフラグを設定する
 *
 *	@param	MEDALBOX *p_mdlbox 
 *
 *	@retval 
 */
//-----------------------------------------------------------------------------
void MEDALBOX_SetTutorialFlag( MEDALBOX *p_mdlbox )
{
  p_mdlbox->tutorial_flag = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルデータのポインタを取得
 *
 *	@param	* p_sv 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
MEDALBOX* SaveData_GetMedalBox( SAVE_CONTROL_WORK * p_sv )
{
	return SaveControl_DataPtrGet(p_sv, GMDATA_ID_MEDALBOX);
}

//----------------------------------------------------------------------------
/**
 *	@brief  せいそくちリストセーブデータを取得
 *
 *	@param	MEDALBOX *p_mdlbox  メダルボックスセーブワーク
 *
 *	@return せいそくちリストセーブデータ
 */
//-----------------------------------------------------------------------------
HABITATLIST_SAVE* MEDALBOX_GetHabitatListSave( MEDALBOX *p_mdlbox )
{
  return (HABITATLIST_SAVE*)p_mdlbox->habitat_list_save_area;
}

//==============================================================================
//  デバッグ用
//==============================================================================
#ifdef PM_DEBUG
void DEBUG_SetMedalState(MEDALBOX * p_mdlbox, MedalID medal_id , u8 state)
{
  RTCDate date;
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );
  p_medal_st->medal_state = state;

  // 現在の日付を設定
  GFL_RTC_GetDate( &date );
  SetMedalDate( p_medal_st, date.year, date.month, date.day );
}
#endif  //PM_DEBUG

