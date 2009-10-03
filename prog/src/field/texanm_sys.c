//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		texanm_sys.c
 *	@brief		ITPアニメデータの必要なところのみを抜き出しテクスチャをアニメーションデータを取得するシステム
 *	@author	 
 *	@date		2005.10.31
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "texanm_sys.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constにはc_を付ける
 *						staticにはs_を付ける
 *						ポインタにはp_を付ける
 *						全て合わさるとcsp_となる
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

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *
 *	@brief	frame数に対応するテクスチャインデックス　パレットインデックスの取得
 *
 *	@param	cp_texanm	テクスチャアニメーションデータテーブル
 *	@param	c_frame		現在フレーム数
 *
 *	@return	フレーム数に対応したテクスチャインデックス　パレットインデックス
 *	
		typedef struct {
			u8	tex_idx;		// テクスチャインデックス	
			u8	pltt_idx;		// パレットインデックス
		} TEXANM_DATA;
 *
 *
 */
//-----------------------------------------------------------------------------
TEXANM_DATA TEXANM_GetFrameData( const TEXANM_DATATBL* cp_texanm, u32 anime_index, u32 frame )
{
	int i;					// ループ用
	u32 anime_data_head;
	u32 anime_data_tail;
	TEXANM_DATA	o_tex_anm;	// 戻り値

	GF_ASSERT( cp_texanm->anime_num > anime_index );

	// アニメーデータのヘッドインデックス取得
	// anime_data_head >= anime_data < anime_data_tail
	anime_data_head = cp_texanm->cp_data_head[ anime_index ];
	if( (anime_index+1) < cp_texanm->anime_num ){
		anime_data_tail	= cp_texanm->cp_data_head[ anime_index+1 ];
	}else{
		anime_data_tail	= cp_texanm->tbl_num;
	}

	// フレーム数に対応するテーブル要素数（ i ）を取得
	for( i = anime_data_head; i < anime_data_tail - 1; i++ ){

		// i + 1の値が要素数が変わるフレーム数です。
		if( cp_texanm->cp_frame[ i + 1 ] > frame ){

			break;
		}
	}
	
	// フレーム数に対応するテクスチャインデックス
	// パレットインデックス代入
	o_tex_anm.tex_idx = cp_texanm->cp_tex_idx[ i ];
	o_tex_anm.pltt_idx = cp_texanm->cp_pltt_idx[ i ];

	return o_tex_anm;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション数の取得
 *
 *	@param	cp_texanm		アニメーションデータ
 *
 *	@return	アニメーション数
 */	
//-----------------------------------------------------------------------------
u32 TEXANM_GetAnimeNum( const TEXANM_DATATBL* cp_texanm )
{
	return cp_texanm->anime_num;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションのフレーム最大数を取得
 *
 *	@param	cp_texanm			アニメーションワーク
 *	@param	anime_index		インデックス
 *
 *	@return	フレーム最大値
 */
//-----------------------------------------------------------------------------
u32 TEXANM_GetFrameMax( const TEXANM_DATATBL* cp_texanm )
{
	return cp_texanm->frame_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションの最終Key設定フレームを取得
 *
 *	@param	cp_texanm				アニメーションワーク
 *	@param	anime_index			アニメーションインデックス
 *
 *	@return	最終Key設定フレーム
 */
//-----------------------------------------------------------------------------
u32 TEXANM_GetLastKeyFrame( const TEXANM_DATATBL* cp_texanm, u32 anime_index )
{
	u32 anime_data_tail;
	if( (anime_index+1) < cp_texanm->anime_num ){
		anime_data_tail	= cp_texanm->cp_data_head[ anime_index+1 ];
	}else{
		anime_data_tail	= cp_texanm->tbl_num;
	}

	return cp_texanm->cp_frame[ anime_data_tail - 1 ];
}

//----------------------------------------------------------------------------
/**
 *
 *	@brief	ROMから読み込んできたデータをテクスチャアニメーションデータテーブル形式にアンパックする
 *
 *	@param	cp_buff			読み込んできたデータ
 *	@param	op_texanm		アンパック先テクスチャアニメーションデータテーブル
 *
 *	@return	none
 *
 * ●op_texanmを使用している期間ではcp_buffのメモリは確保したままにしておいてください
 *
 */
//-----------------------------------------------------------------------------
void TEXANM_UnPackLoadFile( const void* cp_buff, TEXANM_DATATBL* op_texanm )
{	
	const u8* work;

	work = cp_buff;
	
	// テーブル要素数取得
	// 最初の4byteはテーブル要素数
	op_texanm->tbl_num = *((u32*)work);
	work = work + sizeof(u32);		// 4byte移動
	
	// フレームデータテーブル先頭ポインタ設定
	op_texanm->cp_frame = (u16*)work;
	work = work + (sizeof(u16)*op_texanm->tbl_num);			// テーブル分移動
	if( ((sizeof(u16)*op_texanm->tbl_num)%4) ){
		work = work + (4-((sizeof(u16)*op_texanm->tbl_num)%4));	// 4byteアライメント
	}

	// テクスチャインデックスデータテーブル先頭ポインタ設定
	op_texanm->cp_tex_idx = (u8*)work;
	work = work + (sizeof(u8) * op_texanm->tbl_num);	// テーブル分移動
	if( ((sizeof(u8)*op_texanm->tbl_num)%4) ){
		work = work + (4-((sizeof(u8)*op_texanm->tbl_num)%4));	// 4byteアライメント
	}

	// パレットインデックスデータテーブル先頭ポインタ設定
	op_texanm->cp_pltt_idx = (u8*)work;
	work = work + (sizeof(u8) * op_texanm->tbl_num);	// テーブル分移動
	if( ((sizeof(u8)*op_texanm->tbl_num)%4) ){
		work = work + (4-((sizeof(u8)*op_texanm->tbl_num)%4));	// 4byteアライメント
	}

	// アニメーション数
	op_texanm->anime_num = *((u32*)work);
	work = work + sizeof(u32);		// 4byte移動

	// アニメーションヘッド
	op_texanm->cp_data_head = (u8*)work;
	work = work + (sizeof(u8) * op_texanm->anime_num);	// テーブル分移動
	if( ((sizeof(u8)*op_texanm->anime_num)%4) ){
			work = work + (4-((sizeof(u8)*op_texanm->anime_num)%4));	// 4byteアライメント
	}

	// アニメーションフレーム数
	op_texanm->frame_max = *((u32*)work);

#ifdef DEBUG_TEXANM_SYS_PRINT_LOAD_DATA
	TEXANM_DEBUG_PrintData( op_texanm );
#endif
}


#ifdef DEBUG_TEXANM_SYS_PRINT_LOAD_DATA
void TEXANM_DEBUG_PrintData( const TEXANM_DATATBL* cp_texanm )
{
	int i;
	
	OS_TPrintf( "frame_max = %d\n", cp_texanm->frame_max );
	OS_TPrintf( "anime_num = %d\n", cp_texanm->anime_num );
	OS_TPrintf( "anime_start[\n" );
	for( i=0; i<cp_texanm->anime_num; i++ ){
		OS_TPrintf( " %d,", cp_texanm->cp_data_head[i] );
	}
	OS_TPrintf( "]\n\n" );
	
	OS_TPrintf( "tbl_num = %d\n", cp_texanm->tbl_num );
	OS_TPrintf( "frame_tbl[\n" );
	for( i=0; i<cp_texanm->tbl_num; i++ ){
		OS_TPrintf( " %d,", cp_texanm->cp_frame[i] );
	}
	OS_TPrintf( "]\n" );
	OS_TPrintf( "tex_tbl[\n" );
	for( i=0; i<cp_texanm->tbl_num; i++ ){
		OS_TPrintf( " %d,", cp_texanm->cp_tex_idx[i] );
	}
	OS_TPrintf( "]\n" );
	OS_TPrintf( "pltt_tbl[\n" );
	for( i=0; i<cp_texanm->tbl_num; i++ ){
		OS_TPrintf( " %d,", cp_texanm->cp_pltt_idx[i] );
	}
	OS_TPrintf( "]\n" );
	
}
#endif

