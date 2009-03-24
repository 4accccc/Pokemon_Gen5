//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_light.c
 *	@brief		フィールドライトシステム
 *	@author		tomoya takahashi
 *	@data		2009.03.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include  "field_light.h"

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

//-------------------------------------
///	ライトフェード
//=====================================
#define LIGHT_FADE_COUNT_MAX	( 30 )

//-------------------------------------
///	アーカイブデータ
//=====================================
#define LIGHT_ARC_ID			( 0 )
#define LIGHT_ARC_SEASON_NUM	( 4 )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	ライト１データ
//=====================================
typedef struct {
	u32			endtime;
	u8			light_flag[4];
	GXRgb		light_color[4];
	VecFx16		light_vec[4];

	GXRgb		diffuse;
	GXRgb		ambient;
	GXRgb		specular;
	GXRgb		emission;
	GXRgb		fog_color;
} LIGHT_DATA;

//-------------------------------------
///	RGBフェード
//=====================================
typedef struct {
	u16	r_start;
	s16	r_dist;
	u16	g_start;
	s16	g_dist;
	u16	b_start;
	s16	b_dist;
} RGB_FADE;


//-------------------------------------
///	方向ベクトルフェード
//=====================================
typedef struct {
	VecFx32	start;	// 開始ベクトル
	VecFx32	normal;	// 回転軸
	u16	way_dist;	// 回転量
} VEC_FADE;


//-------------------------------------
///	ライトフェード
//=====================================
typedef struct {

	// フェードしない情報
	u32			endtime;
	u8			light_flag[4];

	RGB_FADE		light_color[4];
	VEC_FADE		light_vec[4];
	
	RGB_FADE		diffuse;
	RGB_FADE		ambient;
	RGB_FADE		specular;
	RGB_FADE		emission;
	RGB_FADE		fog_color;

	u16				count;
	u16				count_max;
} LIGHT_FADE;


//-------------------------------------
///	フィールドライトシステム
//=====================================
struct _FIELD_LIGHT {
	// FOGシステム
	FIELD_FOG_WORK* p_fog;
	
	// データバッファ
	u32			data_num;			// データ数
	LIGHT_DATA* p_data;				// データ
	u16			now_index;			// 今の反映インデックス

	// 反映情報
	LIGHT_DATA	reflect_data;		// 反映データ
	u16			reflect_flag;		// 反映フラグ
	BOOL		change;				// データ設定フラグ

	// 反映フェード情報
	LIGHT_FADE	fade;

	// 反映時間
	s32			time_second;

};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	システム
//=====================================
static void FIELD_LIGHT_Reflect( const FIELD_LIGHT* cp_sys, FIELD_FOG_WORK* p_fog );
static void FIELD_LIGHT_LoadData( FIELD_LIGHT* p_sys, u32 light_no, u32 season, u32 heapID );
static void FIELD_LIGHT_LoadDataEx( FIELD_LIGHT* p_sys, u32 arcid, u32 dataid, u32 heapID );
static void FIELD_LIGHT_ReleaseData( FIELD_LIGHT* p_sys );
static s32	FIELD_LIGHT_SearchNowIndex( const FIELD_LIGHT* cp_sys, int rtc_second );


//-------------------------------------
///	RGBフェード
//=====================================
static void RGB_FADE_Init( RGB_FADE* p_wk, GXRgb start, GXRgb end );
static GXRgb RGB_FADE_Calc( const RGB_FADE* cp_wk, u16 count, u16 count_max );

//-------------------------------------
///	方向ベクトルフェード
//=====================================
static void VEC_FADE_Init( VEC_FADE* p_wk, const VecFx16* cp_start, const VecFx16* cp_end );
static void VEC_FADE_Calc( const VEC_FADE* cp_wk, u16 count, u16 count_max, VecFx16* p_ans );

//-------------------------------------
///	ライトフェード
//=====================================
static void LIGHT_FADE_Init( LIGHT_FADE* p_wk, const LIGHT_DATA* cp_start, const LIGHT_DATA* cp_end );
static void LIGHT_FADE_Main( LIGHT_FADE* p_wk );
static BOOL LIGHT_FADE_IsFade( const LIGHT_FADE* cp_wk );
static void LIGHT_FADE_GetData( const LIGHT_FADE* cp_wk, LIGHT_DATA* p_data );








//----------------------------------------------------------------------------
/**
 *	@brief	フィールドライト	システム作成
 *
 *	@param	light_no		ライトナンバー
 *	@param	season			季節
 *	@param	rtc_second		秒数
 *	@param	p_fog			フォグシステム
 *	@param	heapID			ヒープ
 *	
 *	@return	システムワーク
 */
//-----------------------------------------------------------------------------
FIELD_LIGHT* FIELD_LIGHT_Create( u32 light_no, u32 season, int rtc_second, FIELD_FOG_WORK* p_fog, u32 heapID )
{
	FIELD_LIGHT* p_sys;

	p_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_LIGHT) );


	// ライト情報読み込み
	FIELD_LIGHT_LoadData( p_sys, light_no, season, heapID );

	// データ反映
	p_sys->reflect_flag = TRUE;

	// 初期情報を設定
	p_sys->now_index = FIELD_LIGHT_SearchNowIndex( p_sys, rtc_second );

	// データ反映
	GFL_STD_MemCopy( &p_sys->p_data[p_sys->now_index], &p_sys->reflect_data, sizeof(LIGHT_DATA) );
	p_sys->change = TRUE;

	// フォグシステムを保存
	p_sys->p_fog = p_fog;

	return p_sys;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フィールドライト	システム破棄
 *
 *	@param	p_sys			システムワーク
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_Delete( FIELD_LIGHT* p_sys )
{
	// データ破棄
	FIELD_LIGHT_ReleaseData( p_sys );

	GFL_HEAP_FreeMemory( p_sys );
}


//----------------------------------------------------------------------------
/**
 *	@brief	フィールドライト	システムメイン
 *
 *	@param	p_sys			システムワーク
 *	@param	rtc_second		今のRTC時間（秒単位）
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_Main( FIELD_LIGHT* p_sys, int rtc_second )
{
	int starttime;
	
	// ライトデータの有無チェック
	if( p_sys->data_num == 0 ){
		return ;
	}

	// ライトデータ変更チェック
	{
		if( (p_sys->now_index - 1) < 0 ){
			starttime = 0;
		}else{
			starttime = p_sys->p_data[ p_sys->now_index-1 ].endtime;
		}
		
		// 今のテーブルの範囲内じゃないかチェック
		if( (starttime > rtc_second) ||
			(p_sys->p_data[ p_sys->now_index ].endtime <= rtc_second) ){

			// 変更
			p_sys->now_index  = (p_sys->now_index + 1) % p_sys->data_num;

			// フェード設定
			LIGHT_FADE_Init( &p_sys->fade, &p_sys->reflect_data, &p_sys->p_data[ p_sys->now_index ] );
		}
	}

	// ライトフェード
	if( LIGHT_FADE_IsFade( &p_sys->fade ) ){
		LIGHT_FADE_Main( &p_sys->fade );
		LIGHT_FADE_GetData( &p_sys->fade, &p_sys->reflect_data );
		p_sys->change = TRUE;
	}

	// データ設定処理へ
	if( p_sys->change ){
		FIELD_LIGHT_Reflect( p_sys, p_sys->p_fog );
		p_sys->change = FALSE;
	}

	// rtc時間を保存
	p_sys->time_second = rtc_second;

}


//----------------------------------------------------------------------------
/**
 *	@brief	フィールドライト	ライト情報の変更
 *
 *	@param	light_no		ライトナンバー
 *	@param	season			季節
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_Change( FIELD_LIGHT* p_sys, u32 light_no, u32 season, u32 heapID )
{
	// ライト情報を再読み込み
	FIELD_LIGHT_LoadData( p_sys, light_no, season, heapID );

	// 初期情報を設定
	p_sys->now_index = FIELD_LIGHT_SearchNowIndex( p_sys, p_sys->time_second );

	// フェード開始
	LIGHT_FADE_Init( &p_sys->fade, &p_sys->reflect_data, &p_sys->p_data[ p_sys->now_index ] );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フィールドライト	ライト情報の変更　外部情報
 *
 *	@param	arcid			アークID
 *	@param	dataid			データID
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_ChangeEx( FIELD_LIGHT* p_sys, u32 arcid, u32 dataid, u32 heapID )
{
	// ライト情報を再読み込み
	FIELD_LIGHT_LoadDataEx( p_sys, arcid, dataid, heapID );

	// 初期情報を設定
	p_sys->now_index = FIELD_LIGHT_SearchNowIndex( p_sys, p_sys->time_second );

	// フェード開始
	LIGHT_FADE_Init( &p_sys->fade, &p_sys->reflect_data, &p_sys->p_data[ p_sys->now_index ] );
}


//----------------------------------------------------------------------------
/**
 *	@brief	フィールドライト	反映フラグ設定
 *
 *	@param	p_sys		ワーク
 *	@param	flag		フラグ
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_SetReflect( FIELD_LIGHT* p_sys, BOOL flag )
{
	p_sys->reflect_flag = flag;
	if( p_sys->reflect_flag ){
		// データ反映
		p_sys->change = TRUE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フィールドライト	反映フラグを取得
 *
 *	@param	cp_sys		ワーク
 *
 *	@retval	TRUE	反映
 *	@retval	FALSE	反映しない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_LIGHT_GetReflect( const FIELD_LIGHT* cp_sys )
{
	return cp_sys->reflect_flag;
}



//----------------------------------------------------------------------------
/**
 *	@brief	今が夜か取得
 *
 *	@param	cp_sys		システムワーク
 *
 *	@retval	TRUE	夜
 *	@retval	FALSE	それ以外
 */
//-----------------------------------------------------------------------------
BOOL FIELD_LIGHT_GetNight( const FIELD_LIGHT* cp_sys )
{
	if( (cp_sys->time_second >= 34200) || (cp_sys->time_second < 7200) ){
		return TRUE;
	}
	return FALSE;
}








//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief	ライト情報の設定
 *
 *	@param	cp_sys	システムワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_LIGHT_Reflect( const FIELD_LIGHT* cp_sys, FIELD_FOG_WORK* p_fog )
{
	int i;
	
	if( cp_sys->reflect_flag ){

		for( i=0; i<4; i++ ){
			if( cp_sys->reflect_data.light_flag[i] ){
				NNS_G3dGlbLightVector( i, 
						cp_sys->reflect_data.light_vec[i].x, 
						cp_sys->reflect_data.light_vec[i].y,
						cp_sys->reflect_data.light_vec[i].z );

				NNS_G3dGlbLightColor( i, 
						cp_sys->reflect_data.light_color[i] );
			}else{
				
				NNS_G3dGlbLightVector( i, 0, 0, 0 );
				NNS_G3dGlbLightColor( i, 0 );
			}
		}

		NNS_G3dGlbMaterialColorDiffAmb( cp_sys->reflect_data.diffuse,
				cp_sys->reflect_data.ambient, TRUE );

		NNS_G3dGlbMaterialColorSpecEmi( cp_sys->reflect_data.specular,
				cp_sys->reflect_data.emission, FALSE );

		FIELD_FOG_SetColorRgb( p_fog, cp_sys->reflect_data.fog_color );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライト情報の読み込み処理
 *	
 *	@param	p_sys		システムワーク
 *	@param	light_no	ライトナンバー
 *	@param	season		季節
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void FIELD_LIGHT_LoadData( FIELD_LIGHT* p_sys, u32 light_no, u32 season, u32 heapID )
{
	FIELD_LIGHT_LoadDataEx( p_sys, LIGHT_ARC_ID, (light_no*LIGHT_ARC_SEASON_NUM)*season, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライト情報の読み込み処理
 *
 *	@param	p_sys			システムワーク
 *	@param	arcid			アーカイブID
 *	@param	dataid			データID
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
static void FIELD_LIGHT_LoadDataEx( FIELD_LIGHT* p_sys, u32 arcid, u32 dataid, u32 heapID )
{
	u32 size;
	GF_ASSERT( !p_sys->p_data );

	p_sys->p_data	= GFL_ARC_UTIL_LoadEx( arcid, dataid, FALSE, heapID, &size );
	p_sys->data_num	= size / sizeof(LIGHT_DATA);
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライト情報の破棄　処理
 *
 *	@param	p_sys		システムワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_LIGHT_ReleaseData( FIELD_LIGHT* p_sys )
{
	if( p_sys->p_data ){
		GFL_HEAP_FreeMemory( p_sys->p_data );
		p_sys->p_data = NULL;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	rtc秒のときのデータインデックスを取得
 *
 *	@param	cp_sys			ワーク
 *	@param	rtc_second		rtc秒
 *	
 *	@return	データインデックス
 */
//-----------------------------------------------------------------------------
static s32	FIELD_LIGHT_SearchNowIndex( const FIELD_LIGHT* cp_sys, int rtc_second )
{
	int i;

	for( i=0; i<cp_sys->data_num; i++ ){

		if( cp_sys->p_data[i].endtime > rtc_second ){
			return i;
		}
	}

	// ありえない
	GF_ASSERT( 0 );
	return 0;
}


//----------------------------------------------------------------------------
/**
 *	@brief	RGBフェード	初期化
 *
 *	@param	p_wk		ワーク
 *	@param	start		開始色
 *	@param	end			終了色
 */
//-----------------------------------------------------------------------------
static void RGB_FADE_Init( RGB_FADE* p_wk, GXRgb start, GXRgb end )
{
	p_wk->r_start		= (start & GX_RGB_R_MASK )>> GX_RGB_R_SHIFT;
	p_wk->r_dist		= ((end & GX_RGB_R_MASK )>> GX_RGB_R_SHIFT) - p_wk->r_start;
	p_wk->g_start		= (start & GX_RGB_G_MASK )>> GX_RGB_G_SHIFT;
	p_wk->g_dist		= ((end & GX_RGB_G_MASK )>> GX_RGB_G_SHIFT) - p_wk->g_start;
	p_wk->b_start		= (start & GX_RGB_B_MASK )>> GX_RGB_B_SHIFT;
	p_wk->b_dist		= ((end & GX_RGB_B_MASK )>> GX_RGB_B_SHIFT) - p_wk->b_start;
}

//----------------------------------------------------------------------------
/**
 *	@brief	RGBフェード	計算処理
 *
 *	@param	cp_wk			ワーク
 *	@param	count			カウント値
 *	@param	count_max		カウント最大値
 *
 *	@return	RGBカラー
 */
//-----------------------------------------------------------------------------
static GXRgb RGB_FADE_Calc( const RGB_FADE* cp_wk, u16 count, u16 count_max )
{
	u8 r, g, b;

	r = (cp_wk->r_dist * count) / count_max;
	r += cp_wk->r_start;
	g = (cp_wk->g_dist * count) / count_max;
	g += cp_wk->g_start;
	b = (cp_wk->b_dist * count) / count_max;
	b += cp_wk->b_start;

	return GX_RGB( r, g, b );
}

//----------------------------------------------------------------------------
/**
 *	@brief	方向ベクトルフェード	初期化
 *
 *	@param	p_wk			ワーク
 *	@param	cp_start		開始
 *	@param	cp_end			終了
 */
//-----------------------------------------------------------------------------
static void VEC_FADE_Init( VEC_FADE* p_wk, const VecFx16* cp_start, const VecFx16* cp_end )
{
	fx32 cos;
	VecFx16 normal;

	VEC_Set( &p_wk->start, cp_start->x, cp_start->y, cp_start->z );
	VEC_Fx16CrossProduct( cp_start, cp_end, &normal ); 
	VEC_Set( &p_wk->normal, normal.x, normal.y, normal.z );
	cos				= VEC_Fx16DotProduct( cp_start, cp_end ); 
	p_wk->way_dist	= FX_AcosIdx( cos );
}

//----------------------------------------------------------------------------
/**
 *	@brief	方向ベクトルのフェードを計算
 *
 *	@param	cp_wk			ワーク
 *	@param	count			カウント値
 *	@param	count_max		カウント最大値
 *	@param	p_ans			計算結果
 */	
//-----------------------------------------------------------------------------
static void VEC_FADE_Calc( const VEC_FADE* cp_wk, u16 count, u16 count_max, VecFx16* p_ans )
{
	u16	rotate;
	MtxFx43	mtx;
	VecFx32 way;

	rotate = (cp_wk->way_dist * count) / count_max;
	MTX_RotAxis43( &mtx, &cp_wk->normal, FX_SinIdx( rotate ), FX_CosIdx( rotate ) );

	MTX_MultVec43( &cp_wk->start, &mtx, &way );
	VEC_Normalize( &way, &way );

	VEC_Fx16Set( p_ans, way.x, way.y, way.z );
}


//----------------------------------------------------------------------------
/**
 *	@brief	ライトフェード処理	初期化
 *
 *	@param	p_wk			ワーク
 *	@param	cp_start		開始ライト情報
 *	@param	cp_end			終了ライト情報
 */
//-----------------------------------------------------------------------------
static void LIGHT_FADE_Init( LIGHT_FADE* p_wk, const LIGHT_DATA* cp_start, const LIGHT_DATA* cp_end )
{
	int i;

	p_wk->count			= 0;
	p_wk->count_max		= LIGHT_FADE_COUNT_MAX;


	// フェードしない情報も保存
	p_wk->endtime = cp_end->endtime;
	
	for( i=0; i<4; i++ ){

		RGB_FADE_Init( &p_wk->light_color[i], cp_start->light_color[i], cp_end->light_color[i] );
		VEC_FADE_Init( &p_wk->light_vec[i], &cp_start->light_vec[i], &cp_end->light_vec[i] );

		p_wk->light_flag[i] = cp_end->light_flag[i];
	}

	RGB_FADE_Init( &p_wk->diffuse, cp_start->diffuse, cp_end->diffuse );
	RGB_FADE_Init( &p_wk->ambient, cp_start->ambient, cp_end->ambient );
	RGB_FADE_Init( &p_wk->specular, cp_start->specular, cp_end->specular );
	RGB_FADE_Init( &p_wk->emission, cp_start->emission, cp_end->emission );
	RGB_FADE_Init( &p_wk->fog_color, cp_start->fog_color, cp_end->fog_color );

}

//----------------------------------------------------------------------------
/**
 *	@brief	ライトフェード処理	メイン
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void LIGHT_FADE_Main( LIGHT_FADE* p_wk )
{
	if( p_wk->count < p_wk->count_max ){
		p_wk->count ++;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライトフェード処理	フェード中チェック
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	フェード中
 *	@retval	FALSE	フェードしてない
 */
//-----------------------------------------------------------------------------
static BOOL LIGHT_FADE_IsFade( const LIGHT_FADE* cp_wk )
{
	if( cp_wk->count <= cp_wk->count_max ){
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライトフェード処理	ライトデータ取得
 *
 *	@param	cp_wk		ワーク	
 *	@param	p_data		ライトデータ格納先
 */
//-----------------------------------------------------------------------------
static void LIGHT_FADE_GetData( const LIGHT_FADE* cp_wk, LIGHT_DATA* p_data )
{
	int i;

	p_data->endtime		= cp_wk->endtime;
	
	for( i=0; i<4; i++ ){
		p_data->light_color[i]	= RGB_FADE_Calc( &cp_wk->light_color[i], cp_wk->count, cp_wk->count_max );
		VEC_FADE_Calc( &cp_wk->light_vec[i], cp_wk->count, cp_wk->count_max, &p_data->light_vec[i] );

		p_data->light_flag[i]	= cp_wk->light_flag[i];
	}

	p_data->diffuse		= RGB_FADE_Calc( &cp_wk->diffuse, cp_wk->count, cp_wk->count_max );
	p_data->ambient		= RGB_FADE_Calc( &cp_wk->ambient, cp_wk->count, cp_wk->count_max );
	p_data->specular	= RGB_FADE_Calc( &cp_wk->specular, cp_wk->count, cp_wk->count_max );
	p_data->emission	= RGB_FADE_Calc( &cp_wk->emission, cp_wk->count, cp_wk->count_max );
	p_data->fog_color	= RGB_FADE_Calc( &cp_wk->fog_color, cp_wk->count, cp_wk->count_max );

}





