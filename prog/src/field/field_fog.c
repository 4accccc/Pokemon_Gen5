//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_fog.c
 *	@brief		フォグ管理システム
 *	@author		tomoya takahashi
 *	@data		2009.03.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "field_fog.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

// フェードチェックアサート
#define FOG_FADE_ASSERT(x)		GF_ASSERT( FADE_WORK_IsFade( &(x)->fade ) == FALSE )


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	フォグフェードシステム
//=====================================
typedef struct {
	u16 count;
	u16 count_max;
	
	s32	offset_start;
	s32	offset_dist;

	u32	slope_start;
	s32	slope_dist;

} FADE_WORK;


//-------------------------------------
///	システムワーク
//=====================================
struct _FIELD_FOG_WORK {
	u8		flag;			// on/off (TRUE/FALSE)
	u8		blendmode;		// フォグモード
	u8		slope;			// かかり具合
	u8		alpha;			// フォグカラーアルファ
	s32		offset;			// オフセットデプス値
	u16		rgb;			// フォグカラー
  u16   pading;

	u8		fog_tbl[FIELD_FOG_TBL_MAX];	// 32段階のフォグテーブル

	// フォグフェードシステム
	FADE_WORK fade;


	BOOL	change;		// データが変更された
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	フェードシステム
//=====================================
static BOOL FADE_WORK_IsFade( const FADE_WORK* cp_wk );
static void FADE_WORK_Init( FADE_WORK* p_wk, s32 offset_start, s32 offset_end, u16 slope_start, u16 slope_end, u16 count_max );
static void FADE_WORK_Main( FADE_WORK* p_wk );
static s32 FADE_WORK_GetOffset( const FADE_WORK* cp_wk );
static u16 FADE_WORK_GetSlope( const FADE_WORK* cp_wk );






//----------------------------------------------------------------------------
/**
 *	@brief	フォグシステム　作成
 *
 *	@param	heapID		ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
FIELD_FOG_WORK* FIELD_FOG_Create( u32 heapID )
{
	FIELD_FOG_WORK* p_wk;

	p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_FOG_WORK) );

	p_wk->blendmode = FIELD_FOG_BLEND_COLOR_ALPHA;
	p_wk->alpha		= 31;

	FIELD_FOG_TBL_SetUpDefault( p_wk );

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグシステムの破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_Delete( FIELD_FOG_WORK* p_wk )
{
	GFL_HEAP_FreeMemory( p_wk );
}



//----------------------------------------------------------------------------
/**
 *	@brief	フォグシステムのメイン
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_Main( FIELD_FOG_WORK* p_wk )
{
	GF_ASSERT( p_wk );
	
	// フェードシステムメイン
	if( FADE_WORK_IsFade( &p_wk->fade ) ){
		
		//  
		FADE_WORK_Main( &p_wk->fade );


		// フェード情報を設定
		p_wk->offset	= FADE_WORK_GetOffset( &p_wk->fade );
		p_wk->slope		= FADE_WORK_GetSlope( &p_wk->fade );
		p_wk->change	= TRUE;	// データ設定
	}

}

//----------------------------------------------------------------------------
/**
 *	@brief	データ反映関数
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_Reflect( FIELD_FOG_WORK* p_wk )
{
	// データが変更されたので、SDKに反映
	if( p_wk->change ){

		OS_TPrintf( "offset = 0x%x\n", p_wk->offset );
		OS_TPrintf( "slope = %d\n", p_wk->slope );
		G3X_SetFog( p_wk->flag, p_wk->blendmode,
				p_wk->slope, p_wk->offset );
	
		G3X_SetFogColor( p_wk->rgb, p_wk->alpha );
	
		G3X_SetFogTable( (u32*)p_wk->fog_tbl );	

		p_wk->change = FALSE;
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	フォグ	ON/OFF設定
 *
 *	@param	p_wk		ワーク
 *	@param	flag		フラグ
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_SetFlag( FIELD_FOG_WORK* p_wk, BOOL flag )
{
	GF_ASSERT( p_wk );
	p_wk->flag		= flag;
	p_wk->change	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグ　ブレンドモード設定
 *
 *	@param	p_wk		ワーク
 *	@param	blendmode	ブレンドモード
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_SetBlendMode( FIELD_FOG_WORK* p_wk, FIELD_FOG_BLEND blendmode )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( blendmode < FIELD_FOG_BLEND_MAX );
	p_wk->blendmode = blendmode;
	p_wk->change	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグ　かかり具合設定
 *
 *	@param	p_wk		ワーク
 *	@param	slope		かかり具合
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_SetSlope( FIELD_FOG_WORK* p_wk, FIELD_FOG_SLOPE slope )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( slope < FIELD_FOG_SLOPE_MAX );
	FOG_FADE_ASSERT(p_wk);
	p_wk->slope		= slope;
	p_wk->change	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグオフセット　設定
 *
 *	@param	p_wk			ワーク
 *	@param	depth_offset	オフセット
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_SetOffset( FIELD_FOG_WORK* p_wk, s32 depth_offset )
{
	GF_ASSERT( p_wk );
	FOG_FADE_ASSERT(p_wk);
	p_wk->offset	= depth_offset;
	p_wk->change	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグカラー　設定
 *
 *	@param	p_wk		ワーク
 *	@param	rgb			カラー
 32*/
//-----------------------------------------------------------------------------
void FIELD_FOG_SetColorRgb( FIELD_FOG_WORK* p_wk, GXRgb rgb )
{
	GF_ASSERT( p_wk );
	p_wk->rgb		= rgb;
	p_wk->change	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグアルファ	設定
 *
 *	@param	p_wk	ワーク
 *	@param	a		アルファ値
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_SetColorA( FIELD_FOG_WORK* p_wk, u8 a )
{
	GF_ASSERT( p_wk );
	p_wk->alpha		= a;
	p_wk->change	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグテーブル	設定
 *
 *	@param	p_wk		ワーク
 *	@param	index		インデックス
 *	@param	data		データ
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_SetTbl( FIELD_FOG_WORK* p_wk, u8 index, u8 data )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( index < FIELD_FOG_TBL_MAX );
	GF_ASSERT( data < 127 );
	p_wk->fog_tbl[index]	= data;
	p_wk->change			= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグテーブル	バッファごと設定
 *
 *	@param	p_wk		ワーク
 *	@param	cp_tbl		テーブルワーク	（u8*32）分
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_SetTblAll( FIELD_FOG_WORK* p_wk, const u8* cp_tbl )
{
	GF_ASSERT( p_wk );
	GF_ASSERT( cp_tbl );
	GFL_STD_MemCopy32( cp_tbl, p_wk->fog_tbl, sizeof(u8)*FIELD_FOG_TBL_MAX );
	p_wk->change	= TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief	フォグ ON/OFF	フラグ取得
 *
 *	@param	cp_wk	ワーク
 *	
 *	@retval	TRUE	ON
 *	@retval	FALSE	OFF
 */
//-----------------------------------------------------------------------------
BOOL FIELD_FOG_GetFlag( const FIELD_FOG_WORK* cp_wk )
{
	GF_ASSERT( cp_wk );
	return cp_wk->flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグ　モード　取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@return	フォグモード
 */
//-----------------------------------------------------------------------------
FIELD_FOG_BLEND FIELD_FOG_GetBlendMode( const FIELD_FOG_WORK* cp_wk )
{
	GF_ASSERT( cp_wk );
	return cp_wk->blendmode;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグ	スロープ（かかり具合）取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	かかり具合
 */
//-----------------------------------------------------------------------------
FIELD_FOG_SLOPE FIELD_FOG_GetSlope( const FIELD_FOG_WORK* cp_wk )
{
	GF_ASSERT( cp_wk );
	return cp_wk->slope;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグ　オフセット	取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	オフセット
 */
//-----------------------------------------------------------------------------
s32 FIELD_FOG_GetOffset( const FIELD_FOG_WORK* cp_wk )
{
	GF_ASSERT( cp_wk );
	return cp_wk->offset;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグ　カラー　RGB　取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	カラー
 */
//-----------------------------------------------------------------------------
GXRgb FIELD_FOG_GetColorRgb( const FIELD_FOG_WORK* cp_wk )
{
	GF_ASSERT( cp_wk );
	return cp_wk->rgb;
}


//----------------------------------------------------------------------------
/**
 *	@brief	フォグ　カラー　アルファ　取得
 *
 *	@param	cp_wk	ワーク
 *	
 *	@return	アルファ値
 */
//-----------------------------------------------------------------------------
u8 FIELD_FOG_GetColorA( const FIELD_FOG_WORK* cp_wk )
{
	GF_ASSERT( cp_wk );
	return cp_wk->alpha;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグテーブル　値取得
 *
 *	@param	cp_wk	ワーク
 *	@param	index	インデックス
 *
 *	@return	値
 */
//-----------------------------------------------------------------------------
u8 FIELD_FOG_GetTbl( const FIELD_FOG_WORK* cp_wk, u8 index )
{
	GF_ASSERT( cp_wk );
	GF_ASSERT( index < FIELD_FOG_TBL_MAX );
	return cp_wk->fog_tbl[index];
}


//----------------------------------------------------------------------------
/**
 *	@brief	基本的なフォグテーブルを作成
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_TBL_SetUpDefault( FIELD_FOG_WORK* p_wk )
{
	int i;

	GF_ASSERT( p_wk );

	for( i=0; i<FIELD_FOG_TBL_MAX; i++ ){
		p_wk->fog_tbl[i] = i*4;
		p_wk->fog_tbl[i] += (i*4) / FIELD_FOG_TBL_MAX;
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	フォグフェード	初期化
 *
 *	@param	p_wk			ワーク
 *	@param	offset_end		オフセット終了
 *	@param	slope_end		かかり具合終了
 *	@param	count_max		フェードシンク数
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_FADE_Init( FIELD_FOG_WORK* p_wk, s32 offset_end, FIELD_FOG_SLOPE slope_end, u32 count_max )
{
	GF_ASSERT( p_wk );
	FADE_WORK_Init( &p_wk->fade, p_wk->offset, offset_end, p_wk->slope, slope_end, count_max );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグフェード	詳細初期化
 *
 *	@param	p_wk			ワーク
 *	@param	offset_start	オフセット開始値
 *	@param	offset_end		オフセット終了値
 *	@param	slope_start		スロープ開始値
 *	@param	slope_end		スロープ終了値
 *	@param	count_max		フェードに使用するシンク数
 */
//-----------------------------------------------------------------------------
void FIELD_FOG_FADE_InitEx( FIELD_FOG_WORK* p_wk, s32 offset_start, s32 offset_end, FIELD_FOG_SLOPE slope_start, FIELD_FOG_SLOPE slope_end, u16 count_max  )
{
	GF_ASSERT( p_wk );
	FADE_WORK_Init( &p_wk->fade, offset_start, offset_end, slope_start, slope_end, count_max );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグ　フェード中かチェック
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	フェード中
 *	@retval	FALSE	フェードしてない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_FOG_FADE_IsFade( const FIELD_FOG_WORK* cp_wk )
{
	GF_ASSERT( cp_wk );

	return FADE_WORK_IsFade( &cp_wk->fade );
}







//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief	フェード中かチェック
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	フェード中
 *	@retval	FALSE	フェードしてない
 */
//-----------------------------------------------------------------------------
static BOOL FADE_WORK_IsFade( const FADE_WORK* cp_wk )
{
	if( cp_wk->count >= cp_wk->count_max ){
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグフェード処理	初期化
 *
 *	@param	p_wk			ワーク
 *	@param	offset_start	オフセット開始値
 *	@param	offset_end		オフセット終了値
 *	@param	slope_start		かかり具合開始値
 *	@param	slope_end		かかり具合終了値
 *	@param	count_max		フェードに使用するシンク数
 */
//-----------------------------------------------------------------------------
static void FADE_WORK_Init( FADE_WORK* p_wk, s32 offset_start, s32 offset_end, u16 slope_start, u16 slope_end, u16 count_max )
{
	p_wk->count			= 0;
	p_wk->count_max		= count_max;

	p_wk->offset_start	= offset_start;
	p_wk->offset_dist	= offset_end - offset_start;

	p_wk->slope_start	= slope_start;
	p_wk->slope_dist	= slope_end - slope_start;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェード処理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void FADE_WORK_Main( FADE_WORK* p_wk )
{
	if( p_wk->count < p_wk->count_max  ){
		p_wk->count++;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	オフセットの値を取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	オフセットの値
 */
//-----------------------------------------------------------------------------
static s32 FADE_WORK_GetOffset( const FADE_WORK* cp_wk )
{
	s32 ans;

	ans = (cp_wk->offset_dist * cp_wk->count) / cp_wk->count_max;
	ans += cp_wk->offset_start;

	return ans;
}

//----------------------------------------------------------------------------
/**
 *	@brief	かかり具合の取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static u16 FADE_WORK_GetSlope( const FADE_WORK* cp_wk )
{
	u32 ans;

	ans = (cp_wk->slope_dist * cp_wk->count) / cp_wk->count_max;
	ans += cp_wk->slope_start;

	return ans;
}


