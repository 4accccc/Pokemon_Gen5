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

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	メダル構造体
//=====================================
typedef struct {
  u16   get_year :7;        // 取得年
  u16   get_month :4;       // 取得月
  u16   get_day :5;         // 取得日

  u16   medal_state : 3;    // メダル状態
  u16   is_new :1;          // 新着フラグ
  u16   padding :12;
}MEDAL_ST;


//-------------------------------------
///	メダルボックスデータ構造体
//=====================================
struct _MEDALBOX{
  MEDAL_ST  medal[MDLID_MAX]; // メダルデータ
  u8        push_medal;       // イチオシメダル番号
  u8        box_rank;         // メダルボックスランク
  u16       padding;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static MEDAL_ST* GetMedalST( MEDALBOX *p_mdlbox , MedalID medal_id );


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

//=============================================================================
/**
 *					公開関数
*/
//=============================================================================

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
u8 MEDALBOX_GetMedalState( MEDALBOX *p_mdlbox, MedalID medal_id )
{
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

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
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

  SetMedalState( p_medal_st , MEDAL_STATE_TITLE_ACHIVE );

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
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

  SetMedalState( p_medal_st , MEDAL_STATE_HINT_ACHIVE );
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

  GF_ASSERT( year <= 99 );
  GF_ASSERT( month <= 12 );
  GF_ASSERT( day <= 31 );

  GF_ASSERT( p_medal_st->medal_state == MEDAL_STATE_TITLE_ACHIVE );
  SetMedalState( p_medal_st , MEDAL_STATE_TITLE_GET );

  p_medal_st->get_year      = year;
  p_medal_st->get_month     = month;
  p_medal_st->get_day       = day;
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
void MEDALBOX_SetHintMedalGetFlg( MEDALBOX *p_mdlbox, MedalID medal_id )
{
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

  GF_ASSERT( p_medal_st->medal_state == MEDAL_STATE_HINT_ACHIVE );
  
  SetMedalState( p_medal_st , MEDAL_STATE_HINT_GET );

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
  GF_ASSERT( medal_id < 255 );
  GF_ASSERT( medal_id > 0 );
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

  GF_ASSERT( p_mdlbox->box_rank < MEDALBOX_RANK_BW);
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
MedalID MEDALBOX_GetPushMedalID( MEDALBOX *p_mdlbox )
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
u8 MEDALBOX_GetMedalBoxRank( MEDALBOX *p_mdlbox )
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
BOOL MEDALBOX_GetNewMedalFlg( MEDALBOX *p_mdlbox, MedalID medal_id )
{
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

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
void MEDALBOX_GetDate( MEDALBOX *p_mdlbox, MedalID medal_id , u8 *p_year , u8 *p_month , u8 *p_day )
{
  MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );
  
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
u16 MEDALBOX_GetMedalBoxMax( MEDALBOX *p_mdlbox )
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
u16 MEDALBOX_GetMedalCount( MEDALBOX *p_mdlbox , BOOL is_hint_count )
{
  MedalID medal_id;
  u16 count = 0;

  GF_ASSERT( p_mdlbox );

  for( medal_id = 0; medal_id < MDLID_MAX ; medal_id++ )
  {
    MEDAL_ST* p_medal_st = GetMedalST( p_mdlbox , medal_id );

    if( is_hint_count )
    {
      if( (p_medal_st->medal_state == MEDAL_STATE_TITLE_GET) ||
          (p_medal_st->medal_state == MEDAL_STATE_HINT_GET ) )
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
 *	@brief  メダルデータのポインタを取得
 *
 *	@param	* p_sv 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
MEDALBOX* SaveData_GetMedalBox( SAVE_CONTROL_WORK * p_sv )
{
  static MEDALBOX tentative_medal_box_save;
  return &tentative_medal_box_save;
}

