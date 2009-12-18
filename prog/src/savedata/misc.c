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
//-------------------------------------
///	未分類セーブデータ
//	・どこに分類するべきか判断に迷うもの。
//	・規模が小さくそのためにブロック確保をするのがもったいないもの
//=====================================
struct _MISC
{	
	u16 favorite_monsno;		//お気に入りポケモン
	u8  favorite_form_no:7;		//お気に入りポケモンのフォルム番号
	u8  favorite_egg_flag:1;	//お気に入りポケモンのタマゴフラグ
	u8	namein_mode[NAMEIN_MAX];	//5つ

	PMS_DATA gds_self_introduction;		// GDSプロフィールの自己紹介メッセージ
	
	//パルパーク
	u32 palpark_highscore:28;
	u32 palpark_finish_state:4;
};

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
 * @brief   GDS自己紹介メッセージを取得
 *
 * @param   misc		
 * @param   pms			代入先
 */
//--------------------------------------------------------------
void MISC_GetGdsSelfIntroduction(const MISC *misc, PMS_DATA *pms)
{
	*pms = misc->gds_self_introduction;
}

//--------------------------------------------------------------
/**
 * @brief   GDS自己紹介メッセージをセットする
 *
 * @param   misc		
 * @param   pms			セットするメッセージ
 */
//--------------------------------------------------------------
void MISC_SetGdsSelfIntroduction(MISC *misc, const PMS_DATA *pms)
{
	misc->gds_self_introduction = *pms;
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
