//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_compatible_savedata.c
 *	@brief	赤外線相性チェック用セーブデータ
 *	@author	Toru=Nagihashi
 *	@data		2009.07.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/main.h"
#include "print/str_tool.h"
#include "buflen.h"
#include "savedata/save_tbl.h"

#include "savedata/irc_compatible_savedata.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define IRC_COMPATIBLE_SV_EXIST_NONE		(0xFF)		//ランクにいない

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	相性チェックランキング
//=====================================
typedef struct 
{
	STRCODE name[IRC_COMPATIBLE_SV_DATA_NAME_LEN];		// 16
	u32			score			:7;													//	4	
	u32			play_cnt	:10;
  u32     sex       : 1;
  u32     birth_month:  4;
  u32     birth_day :  4;
	u32			dummy			:6;
	u32			trainerID;														// 4
}IRC_COMPATIBLE_RANKING_DATA;

//-------------------------------------
///	相性チェック用セーブデータ
//=====================================
struct _IRC_COMPATIBLE_SAVEDATA
{	
	IRC_COMPATIBLE_RANKING_DATA	rank[IRC_COMPATIBLE_SV_RANKING_MAX];
	u32	new_rank		:8;
	u32 dummy				:24;
};
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static u8 Irc_Compatible_SV_InsertRanking( IRC_COMPATIBLE_RANKING_DATA *p_rank, u16 len, const IRC_COMPATIBLE_RANKING_DATA *cp_new );
static BOOL Irc_Compatible_SV_IsExits( const IRC_COMPATIBLE_RANKING_DATA *cp_rank, u16 len, const IRC_COMPATIBLE_RANKING_DATA *cp_new );
static void Irc_Compatible_SV_SetData( IRC_COMPATIBLE_RANKING_DATA *p_data, const STRCODE *cp_name, u8 score, u16 play_cnt, u8 sex, u8 birth_month, u8 birth_day, u32 trainerID );
static u16 Irc_Compatible_SV_GetPlayCount( const IRC_COMPATIBLE_RANKING_DATA *cp_rank, u16 len, u32 trainerID );
static void Irc_Compatible_SV_AddPlayCount( IRC_COMPATIBLE_RANKING_DATA *p_rank, u16 len, const IRC_COMPATIBLE_RANKING_DATA *cp_new );
//=============================================================================
/**
 *	セーブデータ取得取得
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	セーブデータ取得
 *
 *	@param	SAVE_CONTROL_WORK* p_sv_ctrl	セーブControl
 *
 *	@return	相性チェック用せーぶでーた
 */
//-----------------------------------------------------------------------------
IRC_COMPATIBLE_SAVEDATA * IRC_COMPATIBLE_SV_GetSavedata( SAVE_CONTROL_WORK* p_sv_ctrl )
{	
	return SaveControl_DataPtrGet( p_sv_ctrl, GMDATA_ID_IRCCOMPATIBLE );
}

//=============================================================================
/**
 *	データ取得関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ランクインしている人の数を取得
 *
 *	@param	const IRC_COMPATIBLE_SAVEDATA *cp_sv	セーブ
 *
 *	@return	ランクイン数
 */
//-----------------------------------------------------------------------------
u32 IRC_COMPATIBLE_SV_GetRankNum( const IRC_COMPATIBLE_SAVEDATA *cp_sv )
{	
	int i;

	//ランクインの数はセーブしておらず、play_cntを見て計算する
	for( i = 0; i < IRC_COMPATIBLE_SV_RANKING_MAX; i++ )
	{	
		if( cp_sv->rank[i].play_cnt == 0 )
		{	
			break;
		}
	}

	return i;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤー名を取得
 *
 *	@param	const IRC_COMPATIBLE_SAVEDATA *cp_sv	セーブ
 *	@param	rank																	順位
 *	@retval	STRCODE
 */
//-----------------------------------------------------------------------------
const STRCODE* IRC_COMPATIBLE_SV_GetPlayerName( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank )
{	
	GF_ASSERT( rank < IRC_COMPATIBLE_SV_RANKING_MAX );
  return cp_sv->rank[ rank ].name;
}
//----------------------------------------------------------------------------
/**
 *	@brief	スコアを取得
 *
 *	@param	const IRC_COMPATIBLE_SAVEDATA *cp_sv	セーブ
 *	@param	rank																	順位
 *
 *	@return	スコア
 */
//-----------------------------------------------------------------------------
u8	IRC_COMPATIBLE_SV_GetScore( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank )
{	
	GF_ASSERT( rank < IRC_COMPATIBLE_SV_RANKING_MAX );
	return cp_sv->rank[ rank ].score;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プレイ回数を取得
 *
 *	@param	const IRC_COMPATIBLE_SAVEDATA *cp_sv	セーブ
 *	@param	rank																	順位
 *
 *	@return	プレイ回数
 */
//-----------------------------------------------------------------------------
u16 IRC_COMPATIBLE_SV_GetPlayCount( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank )
{	
	GF_ASSERT( rank < IRC_COMPATIBLE_SV_RANKING_MAX );
	return cp_sv->rank[ rank ].play_cnt;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランキングに入っている男性の人数を取得
 *
 *	@param	const IRC_COMPATIBLE_SAVEDATA *cp_sv  セーブ
 *
 *	@return 男性の人数
 */
//-----------------------------------------------------------------------------
u32 IRC_COMPATIBLE_SV_GetSex( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank )
{ 
	GF_ASSERT( rank < IRC_COMPATIBLE_SV_RANKING_MAX );
	return cp_sv->rank[ rank ].sex;
}
//----------------------------------------------------------------------------
/**
 *	@brief	新規を取得
 *
 *	@param	const IRC_COMPATIBLE_SAVEDATA *cp_sv	セーブ
 *
 *	@return	新規順位
 */
//-----------------------------------------------------------------------------
u8	IRC_COMPATIBLE_SV_GetNewRank( const IRC_COMPATIBLE_SAVEDATA *cp_sv )
{	
	return cp_sv->new_rank;
}


//=============================================================================
/**
 *	データ設定関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ランキングにデータをセット
 *
 *	@param	IRC_COMPATIBLE_SAVEDATA *p_sv	セーブ
 *	@param	STRCODE *cp_name							名前
 *	@param	score													点数（０～１００）
 *	@param	trainerID											ID
 *
 *	@return	順位	or ランキングに入らなかった場合IRC_COMPATIBLE_SV_ADD_NONE
 */
//-----------------------------------------------------------------------------
u8 IRC_COMPATIBLE_SV_AddRanking( IRC_COMPATIBLE_SAVEDATA *p_sv, const STRCODE *cp_name, u8 score, u8 sex, u8 birth_month, u8 birth_day, u32 trainerID )
{	
	IRC_COMPATIBLE_RANKING_DATA	new_data;
	u16 play_cnt;

	//回数取得
	play_cnt	= Irc_Compatible_SV_GetPlayCount( p_sv->rank, IRC_COMPATIBLE_SV_RANKING_MAX, trainerID );

	//データ作成
	Irc_Compatible_SV_SetData( &new_data, cp_name, score, play_cnt, sex, birth_month, birth_day, trainerID );

	//データ挿入
	p_sv->new_rank	= Irc_Compatible_SV_InsertRanking( p_sv->rank, IRC_COMPATIBLE_SV_RANKING_MAX, &new_data );

	//データ内にnewと同じものがあったら、カウントアップ
	//（上記でいれたデータもカウントアップされます）
	if( Irc_Compatible_SV_IsExits( p_sv->rank, IRC_COMPATIBLE_SV_RANKING_MAX, &new_data ) )
	{	
		Irc_Compatible_SV_AddPlayCount( p_sv->rank, IRC_COMPATIBLE_SV_RANKING_MAX, &new_data );
	}

	return p_sv->new_rank;
}

//=============================================================================
/**
 *	セーブデータシステムで使用する関数
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	セーブデータ領域のサイズ
 *
 *	@param	void 
 *
 *	@return	サイズ
 */
//-----------------------------------------------------------------------------
int IRC_COMPATIBLE_SV_GetWorkSize( void )
{	
	return sizeof(IRC_COMPATIBLE_SAVEDATA);
}
//----------------------------------------------------------------------------
/**
 *	@brief	セーブデータ初期化
 *
 *	@param	IRC_COMPATIBLE_SAVEDATA *p_sv		ワーク
 *
 */
//-----------------------------------------------------------------------------
void IRC_COMPATIBLE_SV_Init( IRC_COMPATIBLE_SAVEDATA *p_sv )
{	
	GFL_STD_MemClear( p_sv, sizeof(IRC_COMPATIBLE_SAVEDATA) );
	p_sv->new_rank	= IRC_COMPATIBLE_SV_NEW_NONE;
}
//=============================================================================
/**
 *	PRIVATE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ランキングデータを挿入する
 *
 *	@param	IRC_COMPATIBLE_RANKING_DATA *p_rank		挿入するランキング配列
 *	@param	len																要素数
 *	@param	IRC_COMPATIBLE_RANKING_DATA *cp_new　	挿入するデータ
 *
 *	@return	挿入した場所インデックス
 */
//-----------------------------------------------------------------------------
static u8 Irc_Compatible_SV_InsertRanking( IRC_COMPATIBLE_RANKING_DATA *p_rank, u16 len, const IRC_COMPATIBLE_RANKING_DATA *cp_new )
{
	int i, j;
	int insert	= -1;
	IRC_COMPATIBLE_SAVEDATA temp;

	//挿入箇所をチェック
	for( i = 0; i < len; i++ )
	{	
		if( p_rank[i].score < cp_new->score )
		{	
			insert	= i;
			break;
		}
	}

	//挿入できないチェック
	if( insert == -1 )
	{	
		return IRC_COMPATIBLE_SV_ADD_NONE;
	}

	//後ろから1個ずつずらす
	for( j = len - 1; j > insert ; j-- )
	{	
		//処理軽減のために両方の移動先がplay_cntならば（データなしとみなして、行わない）
		if( p_rank[j].play_cnt != 0 ||  p_rank[j-1].play_cnt != 0 )
		{	
			GFL_STD_MemCopy( &p_rank[j-1], &p_rank[j], sizeof(IRC_COMPATIBLE_RANKING_DATA) );
		}
	}

	//挿入
	GFL_STD_MemCopy( cp_new, &p_rank[insert], sizeof(IRC_COMPATIBLE_RANKING_DATA) );
	return insert;
}

//----------------------------------------------------------------------------
/**
 *	@brief	同じ人がもう一度やったかチェックする
 *
 *	@param	const IRC_COMPATIBLE_SAVEDATA *cp_rank	チェックするランキング配列
 *	@param	len																			要素数
 *	@param	IRC_COMPATIBLE_SAVEDATA *cp_new					チェックするデータ
 *
 *	@return	順位インデックス　または いないならばIRC_COMPATIBLE_SV_EXIST_NONE
 */
//-----------------------------------------------------------------------------
static BOOL Irc_Compatible_SV_IsExits( const IRC_COMPATIBLE_RANKING_DATA *cp_rank, u16 len, const IRC_COMPATIBLE_RANKING_DATA *cp_new )
{	
	int i;

	for( i = 0; i < len; i++ )
	{	
		if( cp_rank[i].trainerID == cp_new->trainerID )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	データを設定
 *
 *	@param	IRC_COMPATIBLE_SAVEDATA *p_data	ワーク
 *	@param	STRCODE *cp_name								名前
 *	@param	score														得点
 *	@param  sex                             性別  1が男 0が女
 *	@param	trainerID												ID
 *
 */
//-----------------------------------------------------------------------------
static void Irc_Compatible_SV_SetData( IRC_COMPATIBLE_RANKING_DATA *p_data, const STRCODE *cp_name, u8 score, u16 play_cnt, u8 sex, u8 birth_month, u8 birth_day, u32 trainerID )
{	
	GF_ASSERT( score <= IRC_COMPATIBLE_SV_DATA_SCORE_MAX );

	//クリア
	GFL_STD_MemClear( p_data, sizeof(IRC_COMPATIBLE_RANKING_DATA) );
	//初期化
  STRTOOL_Copy( cp_name, p_data->name, IRC_COMPATIBLE_SV_DATA_NAME_LEN );
	p_data->score			  = score;
	p_data->trainerID	  = trainerID;
  p_data->sex         = sex;
  p_data->birth_month = birth_month;
  p_data->birth_day   = birth_day;
	p_data->play_cnt	  = play_cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレイ回数を取得
 *
 *	@param	const IRC_COMPATIBLE_RANKING_DATA *cp_rank	ワーク
 *	@param	len																					長さ
 *	@param	trainerID																		ID
 *
 *	@retval	プレイ回数
 *
 */
//-----------------------------------------------------------------------------
static u16 Irc_Compatible_SV_GetPlayCount( const IRC_COMPATIBLE_RANKING_DATA *cp_rank, u16 len, u32 trainerID )
{	
	int i;

	for( i = 0; i < len; i++ )
	{	
		if( cp_rank[i].trainerID == trainerID )
		{
			return cp_rank[i].play_cnt;
		}
	}

	return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	NEWと同じIDの人のランキングを増やす
 *
 *	@param	IRC_COMPATIBLE_SAVEDATA *p_rank		ランキング配列
 *	@param	len																要素数
 *	@param	IRC_COMPATIBLE_SAVEDATA *cp_new		増やすデータ
 *
 */
//-----------------------------------------------------------------------------
static void Irc_Compatible_SV_AddPlayCount( IRC_COMPATIBLE_RANKING_DATA *p_rank, u16 len, const IRC_COMPATIBLE_RANKING_DATA *cp_new )
{	
	int i;

	for( i = 0; i < len; i++ )
	{	
		if( p_rank[i].trainerID == cp_new->trainerID )
		{
			p_rank[i].play_cnt++;
			p_rank[i].play_cnt	= MATH_CLAMP( p_rank[i].play_cnt, 0, IRC_COMPATIBLE_SV_DATA_PLAYCNT_MAX );
		}
	}
}
