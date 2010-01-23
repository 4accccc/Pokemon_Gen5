//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		misc.c
 *	@brief	未分類セーブデータアクセス
 *	@author	Toru=Nagihashi
 *	@data		2009.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "savedata/misc.h"
#include "savedata/save_tbl.h"

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
#include "misc_local.h"

//=============================================================================
/**
 *	プロトタイプ
 */
//=============================================================================
//-------------------------------------
///	名前入力
//=====================================
static void MISC_InitNameIn( MISC *p_misc );

//=============================================================================
/**
 *	セーブデータシステムに依存する関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	MISC構造体のサイズ取得
 *
 *	@return	MISC構造体のサイズ
 */
//-----------------------------------------------------------------------------
int MISC_GetWorkSize( void )
{	
	return sizeof(MISC);
}
//----------------------------------------------------------------------------
/**
 *	@brief	MISC構造体初期化
 *
 *	@param	MISC *p_msc ワーク
 */
//-----------------------------------------------------------------------------
void MISC_Init( MISC *p_msc )
{	
	GFL_STD_MemClear( p_msc, sizeof(MISC) );

	MISC_InitNameIn( p_msc );
	
	p_msc->palpark_highscore = 100; //仮
	
	p_msc->self_introduction[0] = GFL_STR_GetEOMCode();
}

//=============================================================================
/**
 *	セーブデータ取得のための関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	未分類データのCONST版ポインタ取得
 *
 *	@param	SAVE_CONTROL_WORK * cp_sv	セーブデータ保持ワークへのポインタ
 *
 *	@return	MISC
 */
//-----------------------------------------------------------------------------
const MISC * SaveData_GetMiscConst( const SAVE_CONTROL_WORK * cp_sv)
{	
	return (const MISC *)SaveData_GetMisc( (SAVE_CONTROL_WORK *)cp_sv);
}
//----------------------------------------------------------------------------
/**
 *	@brief	未分類データのポインタ取得
 *
 *	@param	SAVE_CONTROL_WORK * p_sv	セーブデータ保持ワークへのポインタ
 *
 *	@return	MISC
 */
//-----------------------------------------------------------------------------
MISC * SaveData_GetMisc( SAVE_CONTROL_WORK * p_sv)
{	
	MISC	*p_msc;
	p_msc = SaveControl_DataPtrGet(p_sv, GMDATA_ID_MISC);
	return p_msc;
}

//=============================================================================
/**
 *	名前入力の関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	名前入力のモード設定
 *
 *	@param	MISC *p_misc	MISC
 *	@param	mode					モード
 *	@param	input_type		入力タイプ
 */
//-----------------------------------------------------------------------------
void MISC_SetNameInMode( MISC *p_misc, NAMEIN_MODE mode, u8 input_type )
{	
	p_misc->namein_mode[ mode ]	= input_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief	名前入力のモード取得
 *
 *	@param	const MISC *cp_misc		MISC
 *	@param	mode									モード
 *
 *	@return	入力タイプ
 */
//-----------------------------------------------------------------------------
u8 MISC_GetNameInMode( const MISC *cp_misc, NAMEIN_MODE mode )
{	
	return cp_misc->namein_mode[ mode ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	名前入力の初期化
 *
 *	@param	MISC *p_misc ワーク
 */
//-----------------------------------------------------------------------------
static void MISC_InitNameIn( MISC *p_misc )
{
	p_misc->namein_mode[ NAMEIN_POKEMON ] = 1;
}

//==============================================================================
//  GDS
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   お気に入りポケモンのセット
 *
 * @param   misc		
 * @param   monsno		ポケモン番号
 * @param   form_no		フォルム番号
 * @param   egg_flag	タマゴフラグ
 */
//--------------------------------------------------------------
void MISC_SetFavoriteMonsno(MISC * misc, int monsno, int form_no, int egg_flag)
{
	misc->favorite_monsno = monsno;
	misc->favorite_form_no = form_no;
	misc->favorite_egg_flag = egg_flag;
}

//--------------------------------------------------------------
/**
 * @brief   お気に入りポケモン取得
 *
 * @param   misc		
 * @param   monsno		ポケモン番号(お気に入りを設定していない場合は0)
 * @param   form_no		フォルム番号
 * @param   egg_flag	タマゴフラグ
 */
//--------------------------------------------------------------
void MISC_GetFavoriteMonsno(const MISC * misc, int *monsno, int *form_no, int *egg_flag)
{
	*monsno = misc->favorite_monsno;
	*form_no = misc->favorite_form_no;
	*egg_flag = misc->favorite_egg_flag;
}

//--------------------------------------------------------------
/**
 * @brief   パルパーク：ハイスコア取得/設定
 *
 * @param   misc		
 */
//--------------------------------------------------------------
const u32  MISC_GetPalparkHighscore(const MISC *misc)
{
	return misc->palpark_highscore;
}
void  MISC_SetPalparkHighscore(MISC *misc , u32 score)
{
	misc->palpark_highscore = score;
}

//--------------------------------------------------------------
/**
 * @brief   パルパーク：終了ステート取得/設定
 *          script/palpark_scr_local.h に定義
 * @param   misc		
 */
//--------------------------------------------------------------
const u8  MISC_GetPalparkFinishState(const MISC *misc)
{
	return misc->palpark_finish_state;
}
void  MISC_SetPalparkFinishState(MISC *misc , u8 state)
{
	misc->palpark_finish_state = state;
}


//==============================================================================
//  すれ違い通信
//==============================================================================
//==================================================================
/**
 * すれ違い通信：お礼を言われた回数を取得
 *
 * @param   misc		
 *
 * @retval  u32		お礼を言われた回数
 */
//==================================================================
u32 MISC_CrossComm_GetThanksRecvCount(const MISC *misc)
{
  return misc->thanks_recv_count;
}

//==================================================================
/**
 * すれ違い通信：お礼を言われた回数をインクリメント
 *
 * @param   misc		
 *
 * @retval  u32		インクリメント後のお礼を言われた回数
 */
//==================================================================
u32 MISC_CrossComm_IncThanksRecvCount(MISC *misc)
{
  if(misc->thanks_recv_count < CROSS_COMM_THANKS_RECV_COUNT_MAX){
    misc->thanks_recv_count++;
  }
  return misc->thanks_recv_count;
}

//==================================================================
/**
 * すれ違いした人数を取得
 *
 * @param   misc		
 *
 * @retval  u32		すれ違いをした人数
 */
//==================================================================
u32 MISC_CrossComm_GetSuretigaiCount(const MISC *misc)
{
  return misc->suretigai_count;
}

//==================================================================
/**
 * すれ違いをした人数をインクリメント
 *
 * @param   misc		
 *
 * @retval  u32		インクリメント後のすれ違いをした人数
 */
//==================================================================
u32 MISC_CrossComm_IncSuretigaiCount(MISC *misc)
{
  if(misc->suretigai_count < CROSS_COMM_SURETIGAI_COUNT_MAX){
    misc->suretigai_count++;
  }
  return misc->suretigai_count;
}

//==================================================================
/**
 * すれ違い通信の自己紹介文を取得
 *
 * @param   misc		
 *
 * @retval  const STRCODE *		
 */
//==================================================================
const STRCODE * MISC_CrossComm_GetSelfIntroduction(const MISC *misc)
{
  return misc->self_introduction;
}

//==================================================================
/**
 * すれ違い通信の自己紹介文をセット
 *
 * @param   misc		
 * @param   srcbuf		
 */
//==================================================================
void MISC_CrossComm_SetSelfIntroduction(MISC *misc, const STRBUF *srcbuf)
{
  GFL_STR_GetStringCode( srcbuf, misc->self_introduction, SAVE_SURETIGAI_SELFINTRODUCTION_LEN );
}

//==================================================================
/**
 * すれ違い通信のお礼メッセージを取得
 *
 * @param   misc		
 *
 * @retval  const STRCODE *		
 */
//==================================================================
const STRCODE * MISC_CrossComm_GetThankyouMessage(const MISC *misc)
{
  return misc->thankyou_message;
}

//==================================================================
/**
 * すれ違い通信のお礼メッセージをセット
 *
 * @param   misc		
 * @param   srcbuf		
 */
//==================================================================
void MISC_CrossComm_SetThankyouMessage(MISC *misc, const STRBUF *srcbuf)
{
  GFL_STR_GetStringCode( srcbuf, misc->thankyou_message, SAVE_SURETIGAI_THANKYOU_LEN );
}

//----------------------------------------------------------
/**
 * @brief バッジ保持状態の取得
 * @param my      自分状態保持ワークへのポインタ
 * @param badge_id  バッジの指定
 * @return  BOOL  バッジを持っているかどうか
 */
//----------------------------------------------------------
BOOL MISC_GetBadgeFlag(const MISC * misc, int badge_id)
{
  if (misc->badge & (1 << badge_id)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

//----------------------------------------------------------
/**
 * @brief バッジ取得のセット
 * @param misc      自分状態保持ワークへのポインタ
 * @param badge_id  バッジの指定
 */
//----------------------------------------------------------
void MISC_SetBadgeFlag(MISC * misc, int badge_id)
{
  misc->badge |= (1 << badge_id);
}

//----------------------------------------------------------
/**
 * @brief 持っているバッジの数を取得する
 * @param misc      自分状態保持ワークへのポインタ
 * @return  int   持っているバッジの数（０～３２）
 */
//----------------------------------------------------------
int MISC_GetBadgeCount(const MISC * misc)
{
  int count = 0;
  u32 badge;

  for (badge = misc->badge; badge != 0; badge >>= 1) {
    if (badge & 1) {
      count ++;
    }
  }
  return count;
}
//----------------------------------------------------------
/**
 * @brief 持っている金額を取得する
 * @param my    自分状態保持ワークへのポインタ
 * @return  u32   持っている金額
 */
//----------------------------------------------------------
u32 MISC_GetGold(const MISC * misc)
{
  return misc->gold;
}

//----------------------------------------------------------
/**
 * @brief   お金を設定する
 * @param misc    自分状態保持ワークへのポインタ
 * @param gold  セットする金額
 * @return  u32   現在の手持ち金額
 */
//----------------------------------------------------------
u32 MISC_SetGold(MISC * misc, u32 gold)
{
  if (gold > MY_GOLD_MAX) {
    gold = MY_GOLD_MAX;
  }
  misc->gold = gold;
  return misc->gold;
}

//----------------------------------------------------------
/**
 * @brief  お金を増やす
 * @param misc    自分状態保持ワークへのポインタ
 * @param add   加える金額
 * @return  u32   現在の手持ち金額
 */
//----------------------------------------------------------
u32 MISC_AddGold(MISC * misc, u32 add)
{
  if (add > MY_GOLD_MAX) {
    misc->gold = MY_GOLD_MAX;
  } else {
    misc->gold += add;
  }
  if (misc->gold > MY_GOLD_MAX) {
    misc->gold = MY_GOLD_MAX;
  }
  return misc->gold;
}

//----------------------------------------------------------
/**
 * @brief    お金を減らす
 * @param misc    自分状態保持ワークへのポインタ
 * @param sub   引き出す金額
 * @return  u32   現在の手持ち金額
 */
//----------------------------------------------------------
u32 MISC_SubGold(MISC * misc, u32 sub)
{
  if (misc->gold < sub) {
    misc->gold = 0;
  } else {
    misc->gold -= sub;
  }
  return misc->gold;
}

//==================================================================
//
//
//      サブイベント関連
//
//==================================================================
//----------------------------------------------------------
//----------------------------------------------------------
void MISC_SetGymVictoryInfo( int gym_id, u16 * monsnos )
{
  GF_ASSERT( gym_id < GYM_MAX );
}
