//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_view.c
 *	@brief	ランダムマッチ表示関係
 *	@author	Toru=Nagihashi
 *	@data		2009.11.04
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	lib
#include <gflib.h>

//	SYSTEM
#include "system/gf_date.h"

//	module
#include "pokeicon/pokeicon.h"
#include "ui/print_tool.h"
#include "system/pms_draw.h"
#include "system/nsbtx_to_clwk.h"

//	archive
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "wifimatch_gra.naix"
#include "msg/msg_wifi_match.h"
#include "app/app_menu_common.h"

//	print
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"

// mine
#include "wifibattlematch_view.h"

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					プレイヤー情報表示
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
enum
{	
	PLAYERINFO_BMPWIN_MAX	= 30,
};
enum
{	
	PLAYERINFO_CLWK_POKEICON_TOP,
	PLAYERINFO_CLWK_POKEICON_END	= PLAYERINFO_CLWK_POKEICON_TOP + TEMOTI_POKEMAX,
	PLAYERINFO_CLWK_TRAINER,
	PLAYERINFO_CLWK_MAX,
};
enum
{	
	RESID_POKEICON_CGR_TOP,
	RESID_POKEICON_CGR_END	= RESID_POKEICON_CGR_TOP+TEMOTI_POKEMAX,
	RESID_POKEICON_CELL_TOP,
	RESID_POKEICON_CELL_END	= RESID_POKEICON_CELL_TOP+TEMOTI_POKEMAX,
	RESID_POKEICON_PLT,
	RESID_TRAINER_PLT,
	RESID_TRAINER_CGR,
	RESID_TRAINER_CELL,
	RESID_MAX,
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	プレイヤー情報表示ワーク
//=====================================
struct _PLAYERINFO_WORK
{	
	u32					bmpwin_max;
	GFL_BMPWIN	*p_bmpwin[ PLAYERINFO_BMPWIN_MAX ];
	PRINT_UTIL	print_util[ PLAYERINFO_BMPWIN_MAX ];
	GFL_CLWK		*p_clwk[ PLAYERINFO_CLWK_MAX ];
	u32					res[RESID_MAX];
	u32					clwk_max;
	WIFIBATTLEMATCH_MODE mode;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BMPWIN作成
//=====================================
static void PlayerInfo_Bmpwin_Rnd_Create( PLAYERINFO_WORK * p_wk, WIFIBATTLEMATCH_MODE mode, const PLAYERINFO_RANDOMMATCH_DATA *cp_data, const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID );
static void PlayerInfo_Bmpwin_Wifi_Create( PLAYERINFO_WORK * p_wk, WIFIBATTLEMATCH_MODE mode, const PLAYERINFO_WIFICUP_DATA *cp_data, const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID );
static void PlayerInfo_Bmpwin_Live_Create( PLAYERINFO_WORK * p_wk, WIFIBATTLEMATCH_MODE mode, const PLAYERINFO_LIVECUP_DATA *cp_data,const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID );
static BOOL PlayerInfo_Bmpwin_PrintMain( PLAYERINFO_WORK * p_wk, PRINT_QUE *p_que );
static void PlayerInfo_Bmpwin_Delete( PLAYERINFO_WORK * p_wk );
//-------------------------------------
///	CLWK作成
//=====================================
static void PlayerInfo_POKEICON_Create( PLAYERINFO_WORK * p_wk, WIFIBATTLEMATCH_MODE mode, const u16 *cp_poke, const u16 *cp_form, GFL_CLUNIT *p_unit, HEAPID heapID );
static void PlayerInfo_POKEICON_Delete( PLAYERINFO_WORK * p_wk );
static void PlayerInfo_TRAINER_Cleate( PLAYERINFO_WORK * p_wk, u32 trainerID, GFL_CLUNIT *p_unit, HEAPID heapID );
static void PlayerInfo_TRAINER_Delete( PLAYERINFO_WORK * p_wk );
//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤー情報表示	初期化
 *
 *	@param	WIFIBATTLEMATCH_MODE mode	モード
 *	@param	void *cp_data							モード別の情報構造体
 *	@param	*p_unit										セル作成のためのユニット
 *	@param	PRINT_QUE *p_que					文字表示用キュー
 *	@param	*p_msg										文字表示用メッセージデータ
 *	@param	*p_word										文字表示用単語登録
 *	@param	heapID										ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
PLAYERINFO_WORK	* PLAYERINFO_Init( WIFIBATTLEMATCH_MODE mode, const void *cp_data_adrs, const MYSTATUS* p_my, GFL_CLUNIT *p_unit, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID )
{	
	PLAYERINFO_WORK	*	p_wk;
	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(PLAYERINFO_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(PLAYERINFO_WORK) );
	p_wk->mode	= mode;

	switch( mode )
	{	
	case WIFIBATTLEMATCH_MODE_RANDOM_FREE:
		/* fallthrough */
	case WIFIBATTLEMATCH_MODE_RANDOM_RATE:
		{
			const PLAYERINFO_RANDOMMATCH_DATA *cp_data	= cp_data_adrs;
			PlayerInfo_Bmpwin_Rnd_Create( p_wk, mode, cp_data_adrs, p_my, p_font, p_que, p_msg, p_word, heapID );
			PlayerInfo_TRAINER_Cleate( p_wk, cp_data->trainerID, p_unit, heapID );
		}
		break;

	case WIFIBATTLEMATCH_MODE_WIFI_LIMIT:
		/* fallthrough */
	case WIFIBATTLEMATCH_MODE_WIFI_NOLIMIT:
		{	
			const PLAYERINFO_WIFICUP_DATA *cp_data	= cp_data_adrs;
			PlayerInfo_Bmpwin_Wifi_Create( p_wk, mode, cp_data, p_my, p_font, p_que, p_msg, p_word, heapID );
			PlayerInfo_POKEICON_Create( p_wk, mode, cp_data->poke, cp_data->form, p_unit, heapID );
			PlayerInfo_TRAINER_Cleate( p_wk, cp_data->trainerID, p_unit, heapID );
		}
		break;

	case WIFIBATTLEMATCH_MODE_LIVE:
		{
			const PLAYERINFO_LIVECUP_DATA *cp_data	= cp_data_adrs;
			PlayerInfo_Bmpwin_Live_Create( p_wk, mode, cp_data, p_my, p_font, p_que, p_msg, p_word, heapID );
			PlayerInfo_POKEICON_Create( p_wk, mode, cp_data->poke, cp_data->form, p_unit, heapID );
			PlayerInfo_TRAINER_Cleate( p_wk, cp_data->trainerID, p_unit, heapID );
		}
		break;
	}



	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤー情報表示	破棄
 *
 *	@param	PLAYERINFO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void PLAYERINFO_Exit( PLAYERINFO_WORK *p_wk )
{
	PlayerInfo_TRAINER_Delete( p_wk );

	switch( p_wk->mode )
	{	
	case WIFIBATTLEMATCH_MODE_WIFI_LIMIT:
		/* fallthrough */
	case WIFIBATTLEMATCH_MODE_WIFI_NOLIMIT:
		/* fallthrough */
	case WIFIBATTLEMATCH_MODE_LIVE:
		PlayerInfo_POKEICON_Delete( p_wk );
		break;
	}

	PlayerInfo_Bmpwin_Delete( p_wk );

	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プリントメイン
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	PRINT_QUE	*p_que				プリントキュー
 *
 *	@return	TRUEで全てのプリント終了	FALSEでまだ
 */
//-----------------------------------------------------------------------------
BOOL PLAYERINFO_PrintMain( PLAYERINFO_WORK * p_wk, PRINT_QUE *p_que )
{	
 	return PlayerInfo_Bmpwin_PrintMain( p_wk, p_que );
}
//=============================================================================
/**
 *					BMPWIN
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINを作成し、文字表示	ランダムマッチ版
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	mode										モード
 *	@param	PLAYERINFO_RANDOMMATCH_DATA *cp_data	表示するためのデータ
 *	@param	MYSTATUS* p_my												プレイヤーのデータ
 *	@param	*p_font																フォント
 *	@param	*p_que																キュー
 *	@param	*p_msg																メッセージ
 *	@param	*p_word																単語登録
 *	@param	heapID																ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_Bmpwin_Rnd_Create( PLAYERINFO_WORK * p_wk, WIFIBATTLEMATCH_MODE mode, const PLAYERINFO_RANDOMMATCH_DATA *cp_data, const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID )
{	
	enum
	{	
		PLAYERINFO_RND_BMPWIN_MODE,
		PLAYERINFO_RND_BMPWIN_RULE,
		PLAYERINFO_RND_BMPWIN_PLAYERSCORE,
		PLAYERINFO_RND_BMPWIN_RATE_LABEL,
		PLAYERINFO_RND_BMPWIN_RATE_NUM,
		PLAYERINFO_RND_BMPWIN_BTLCNT_LABEL,
		PLAYERINFO_RND_BMPWIN_BTLCNT_NUM,
		PLAYERINFO_RND_BMPWIN_WIN_LABEL,
		PLAYERINFO_RND_BMPWIN_WIN_NUM,
		PLAYERINFO_RND_BMPWIN_LOSE_LABEL,
		PLAYERINFO_RND_BMPWIN_LOSE_NUM,
		PLAYERINFO_RND_BMPWIN_MAX,
	};
	
	static const struct
	{	
		u8 x;
		u8 y;
		u8 w;
		u8 h;
	}sc_bmpwin_range[PLAYERINFO_RND_BMPWIN_MAX]	=
	{	
		//モード表示
		{	
			2,2,26,2,
		},
		//バトルルール
		{	
			8,6,20,2,
		},
		//○○の成績
		{	
			8,8,20,2,
		},
		//レーティング
		{	
			2,12,20,2,
		},
		//レーティングの数値
		{	
			25, 12, 5, 2,
		},
		//対戦回数
		{	
			2,15,20,2,
		},
		//対戦回数の数値
		{	
			25, 15, 5, 2,
		},
		//勝った数
		{	
			2,18,20,2,
		},
		//勝った数の数値
		{	
			25, 18, 5, 2,
		},
		//負けた数
		{	
			2,21,20,2,
		},
		//負けた数の数値
		{	
			25, 21, 5, 2,
		},
	};
	//BMPWIN登録個数
	p_wk->bmpwin_max	= PLAYERINFO_RND_BMPWIN_MAX;

	//BMPWIN作成
	{	
		int i;
		for( i = 0; i < p_wk->bmpwin_max; i++ )
		{	
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( PLAYERINFO_BG_FRAME, 
					sc_bmpwin_range[i].x, sc_bmpwin_range[i].y, sc_bmpwin_range[i].w, sc_bmpwin_range[i].h,
					PLAYERINFO_PLT_BG_FONT, GFL_BMP_CHRAREA_GET_B );

			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0xE );

			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			if( p_wk->p_bmpwin[i] )
			{	
				PRINT_UTIL_Setup( &p_wk->print_util[i], p_wk->p_bmpwin[i] );
			}
		}
	}

	//文字表示
	{	
		int i;
		STRBUF	*p_src;
		STRBUF	*p_str;
		BOOL is_print;

		p_src	= GFL_STR_CreateBuffer( 128, heapID );
		p_str	= GFL_STR_CreateBuffer( 128, heapID );

		for( i = 0; i < PLAYERINFO_RND_BMPWIN_MAX; i++ )
		{	
			is_print	= TRUE;
			switch( i )
			{	
			case PLAYERINFO_RND_BMPWIN_MODE:
				GF_ASSERT( mode <= WIFIBATTLEMATCH_MODE_RANDOM_RATE );
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_000 + mode, p_str );
				break;
			case PLAYERINFO_RND_BMPWIN_RULE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_002 + cp_data->btl_rule, p_str );
				break;
			case PLAYERINFO_RND_BMPWIN_PLAYERSCORE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_007, p_src );
		//		WORDSET_RegisterPlayerName( p_word, 0, p_my );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_RND_BMPWIN_RATE_LABEL:
				if( mode == WIFIBATTLEMATCH_MODE_RANDOM_RATE )
				{	
					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_008, p_str );
				}
				else
				{	
					is_print	= FALSE;
				}
				break;
			case PLAYERINFO_RND_BMPWIN_RATE_NUM:
				if( mode == WIFIBATTLEMATCH_MODE_RANDOM_RATE )
				{	
					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_009, p_src );
					WORDSET_RegisterNumber( p_word, 0, cp_data->rate, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
					WORDSET_ExpandStr( p_word, p_str, p_src );
				}
				else
				{	
					is_print	= FALSE;
				}
				break;
			case PLAYERINFO_RND_BMPWIN_BTLCNT_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_010, p_str );
				break;
			case PLAYERINFO_RND_BMPWIN_BTLCNT_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_011, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->btl_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_RND_BMPWIN_WIN_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_012, p_str );
				break;
			case PLAYERINFO_RND_BMPWIN_WIN_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_013, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->win_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_RND_BMPWIN_LOSE_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_014, p_str );
				break;
			case PLAYERINFO_RND_BMPWIN_LOSE_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_015, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->lose_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			}
			if( is_print )
			{	
				PRINT_UTIL_Print( &p_wk->print_util[i], p_que, 0, 0, p_str,  p_font );
			}

		}

		GFL_STR_DeleteBuffer( p_src );
		GFL_STR_DeleteBuffer( p_str );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINを作成し、文字表示	WIFI大会版
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	mode										モード
 *	@param	PLAYERINFO_RANDOMMATCH_DATA *cp_data	表示するためのデータ
 *	@param	MYSTATUS* p_my												プレイヤーのデータ
 *	@param	*p_font																フォント
 *	@param	*p_que																キュー
 *	@param	*p_msg																メッセージ
 *	@param	*p_word																単語登録
 *	@param	heapID																ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_Bmpwin_Wifi_Create( PLAYERINFO_WORK * p_wk, WIFIBATTLEMATCH_MODE mode, const PLAYERINFO_WIFICUP_DATA *cp_data, const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID )
{	
	enum
	{	
		PLAYERINFO_CUP_BMPWIN_CUPNAME,
		PLAYERINFO_CUP_BMPWIN_TIME,
		PLAYERINFO_CUP_BMPWIN_PLAYERNAME,
		PLAYERINFO_CUP_BMPWIN_RATE_LABEL,
		PLAYERINFO_CUP_BMPWIN_RATE_NUM,
		PLAYERINFO_CUP_BMPWIN_BTLCNT_LABEL,
		PLAYERINFO_CUP_BMPWIN_BTLCNT_NUM,
		PLAYERINFO_CUP_BMPWIN_REGPOKE,
		PLAYERINFO_CUP_BMPWIN_MAX,
	};
	
	static const struct
	{	
		u8 x;
		u8 y;
		u8 w;
		u8 h;
	}sc_bmpwin_range[PLAYERINFO_CUP_BMPWIN_MAX]	=
	{	
		//大会名表示
		{	
			1,1,30,4,
		},
		//開催日時
		{	
			1,6,30,2,
		},
		//プレイヤー名
		{	
			1,9,9,2,
		},
		//レーティング
		{	
			10,11,15,2,
		},
		//レーティングの数値
		{	
			25, 11, 5, 2,
		},
		//対戦回数
		{	
			10,14,15,2,
		},
		//対戦回数の数値
		{	
			25, 14, 5, 2,
		},
		//登録ポケモン
		{	
			2,18,28,2,
		},
	};
	//BMPWIN登録個数
	p_wk->bmpwin_max	= PLAYERINFO_CUP_BMPWIN_MAX;

	//BMPWIN作成
	{	
		int i;
		for( i = 0; i < p_wk->bmpwin_max; i++ )
		{	
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( PLAYERINFO_BG_FRAME, 
					sc_bmpwin_range[i].x, sc_bmpwin_range[i].y, sc_bmpwin_range[i].w, sc_bmpwin_range[i].h,
					PLAYERINFO_PLT_BG_FONT, GFL_BMP_CHRAREA_GET_B );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0xE );
			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			if( p_wk->p_bmpwin[i] )
			{	
				PRINT_UTIL_Setup( &p_wk->print_util[i], p_wk->p_bmpwin[i] );
			}
		}
	}

	//文字表示
	{	
		int i;
		STRBUF	*p_src;
		STRBUF	*p_str;

		BOOL	is_print;

		p_src	= GFL_STR_CreateBuffer( 128, heapID );
		p_str	= GFL_STR_CreateBuffer( 128, heapID );

		for( i = 0; i < PLAYERINFO_CUP_BMPWIN_MAX; i++ )
		{	
			is_print	= TRUE;
			switch( i )
			{	
			case PLAYERINFO_CUP_BMPWIN_CUPNAME:
				GFL_STR_SetStringCode( p_str, cp_data->cup_name );
				break;
			case PLAYERINFO_CUP_BMPWIN_TIME:
				{	
					u16 s_y;
					u8 s_m, s_d;
					u16 e_y;
					u8 e_m, e_d;

					s_y	= 2000 + GFDATE_GetYear( cp_data->start_date );
					s_m	= GFDATE_GetMonth( cp_data->start_date );
					s_d	= GFDATE_GetDay( cp_data->start_date );
					e_y	= 2000 + GFDATE_GetYear( cp_data->end_date );
					e_m	= GFDATE_GetMonth( cp_data->end_date );
					e_d	= GFDATE_GetDay( cp_data->end_date );

					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_016, p_src );
					WORDSET_RegisterNumber( p_word, 0, s_y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 1, s_m, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 2, s_d, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 3, e_y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 4, e_m, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 5, e_d, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_ExpandStr( p_word, p_str, p_src );
				}
				break;
			case PLAYERINFO_CUP_BMPWIN_PLAYERNAME:
				//MyStatus_CopyNameString( p_my, p_str );
				break;
			case PLAYERINFO_CUP_BMPWIN_RATE_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_008, p_str );
				break;
			case PLAYERINFO_CUP_BMPWIN_RATE_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_009, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->rate, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_CUP_BMPWIN_BTLCNT_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_010, p_str );
				break;
			case PLAYERINFO_CUP_BMPWIN_BTLCNT_NUM:
				if( mode == WIFIBATTLEMATCH_MODE_WIFI_LIMIT )
				{	
					PRINTTOOL_PrintFraction(
					&p_wk->print_util[i], p_que, p_font,
					sc_bmpwin_range[i].w*4, 0, cp_data->btl_cnt, cp_data->btl_max, heapID );
					is_print =FALSE;
				}
				else if( mode == WIFIBATTLEMATCH_MODE_WIFI_NOLIMIT )
				{	
					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_011, p_src );
					WORDSET_RegisterNumber( p_word, 0, cp_data->btl_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
					WORDSET_ExpandStr( p_word, p_str, p_src );
				}
				break;
			case PLAYERINFO_CUP_BMPWIN_REGPOKE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_017, p_str );
				break;
			}
			if( is_print )
			{	
				PRINT_UTIL_Print( &p_wk->print_util[i], p_que, 0, 0, p_str,  p_font );
			}
		}

		GFL_STR_DeleteBuffer( p_src );
		GFL_STR_DeleteBuffer( p_str );
	}

}

//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINを作成し、文字表示	ライブ大会版
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	mode										モード
 *	@param	PLAYERINFO_RANDOMMATCH_DATA *cp_data	表示するためのデータ
 *	@param	MYSTATUS* p_my												プレイヤーのデータ
 *	@param	*p_font																フォント
 *	@param	*p_que																キュー
 *	@param	*p_msg																メッセージ
 *	@param	*p_word																単語登録
 *	@param	heapID																ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_Bmpwin_Live_Create( PLAYERINFO_WORK * p_wk, WIFIBATTLEMATCH_MODE mode, const PLAYERINFO_LIVECUP_DATA *cp_data,const MYSTATUS* p_my, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID )
{	
	enum
	{	
		PLAYERINFO_LIVE_BMPWIN_CUPNAME,
		PLAYERINFO_LIVE_BMPWIN_TIME,
		PLAYERINFO_LIVE_BMPWIN_PLAYERNAME,
		PLAYERINFO_LIVE_BMPWIN_WIN_LABEL,
		PLAYERINFO_LIVE_BMPWIN_WIN_NUM,
		PLAYERINFO_LIVE_BMPWIN_LOSE_LABEL,
		PLAYERINFO_LIVE_BMPWIN_LOSE_NUM,
		PLAYERINFO_LIVE_BMPWIN_BTLCNT_LABEL,
		PLAYERINFO_LIVE_BMPWIN_BTLCNT_NUM,
		PLAYERINFO_LIVE_BMPWIN_REGPOKE,
		PLAYERINFO_LIVE_BMPWIN_MAX,
	};
	
	static const struct
	{	
		u8 x;
		u8 y;
		u8 w;
		u8 h;
		u32 mode;	//表示モード
	}sc_bmpwin_range[PLAYERINFO_LIVE_BMPWIN_MAX]	=
	{	
		//大会名表示
		{	
			1,1,30,4,
		},
		//開催日時
		{	
			1,6,30,2,
		},
		//プレイヤー名
		{
			1,9,9,2,
		},
		//勝った数
		{
			11,9,12,2,
		},
		//勝った数の数値
		{	
			25,9, 6, 2,
		},
		//負けた数
		{	
			11,12,12,2,
		},
		//負けた数の数値
		{	
			25,12, 6, 2,
		},
		//対戦回数
		{	
			11,15,12,2,
		},
		//対戦回数の数値
		{	
			25,15,6, 2,
		},
		//登録ポケモン
		{	
			1,18,30,2,
		},
	};
	//BMPWIN登録個数
	p_wk->bmpwin_max	= PLAYERINFO_LIVE_BMPWIN_MAX;

	//BMPWIN作成
	{	
		int i;
		for( i = 0; i < p_wk->bmpwin_max; i++ )
		{	
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( PLAYERINFO_BG_FRAME, 
					sc_bmpwin_range[i].x, sc_bmpwin_range[i].y, sc_bmpwin_range[i].w, sc_bmpwin_range[i].h,
					PLAYERINFO_PLT_BG_FONT, GFL_BMP_CHRAREA_GET_B );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0xE );
			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			if( p_wk->p_bmpwin[i] )
			{	
				PRINT_UTIL_Setup( &p_wk->print_util[i], p_wk->p_bmpwin[i] );
			}
		}
	}

	//文字表示
	{	
		int i;
		STRBUF	*p_src;
		STRBUF	*p_str;

		BOOL	is_print;

		p_src	= GFL_STR_CreateBuffer( 128, heapID );
		p_str	= GFL_STR_CreateBuffer( 128, heapID );

		for( i = 0; i < PLAYERINFO_LIVE_BMPWIN_MAX; i++ )
		{	
			is_print	= TRUE;
			switch( i )
			{	
			case PLAYERINFO_LIVE_BMPWIN_CUPNAME:
				GFL_STR_SetStringCode( p_str, cp_data->cup_name );
				break;
			case PLAYERINFO_LIVE_BMPWIN_TIME:
				{	
					u16 s_y;
					u8 s_m, s_d;
					u16 e_y;
					u8 e_m, e_d;

					s_y	= 2000 + GFDATE_GetYear( cp_data->start_date );
					s_m	= GFDATE_GetMonth( cp_data->start_date );
					s_d	= GFDATE_GetDay( cp_data->start_date );
					e_y	= 2000 + GFDATE_GetYear( cp_data->end_date );
					e_m	= GFDATE_GetMonth( cp_data->end_date );
					e_d	= GFDATE_GetDay( cp_data->end_date );

					GFL_MSG_GetString( p_msg, WIFIMATCH_STR_016, p_src );
					WORDSET_RegisterNumber( p_word, 0, s_y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 1, s_m, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 2, s_d, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 3, e_y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 4, e_m, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_RegisterNumber( p_word, 5, e_d, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
					WORDSET_ExpandStr( p_word, p_str, p_src );
				}
				break;
			case PLAYERINFO_LIVE_BMPWIN_PLAYERNAME:
				//MyStatus_CopyNameString( p_my, p_str );
				break;
			case PLAYERINFO_LIVE_BMPWIN_WIN_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_012, p_str );
				break;
			case PLAYERINFO_LIVE_BMPWIN_WIN_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_013, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->win_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_LIVE_BMPWIN_LOSE_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_014, p_str );
				break;
			case PLAYERINFO_LIVE_BMPWIN_LOSE_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_015, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->lose_cnt, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case PLAYERINFO_LIVE_BMPWIN_BTLCNT_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_010, p_str );
				break;
			case PLAYERINFO_LIVE_BMPWIN_BTLCNT_NUM:
				PRINTTOOL_PrintFraction(
						&p_wk->print_util[i], p_que, p_font,
						sc_bmpwin_range[i].w*4, 0, cp_data->btl_cnt, cp_data->btl_max, heapID );
				is_print =FALSE;
				break;
			case PLAYERINFO_LIVE_BMPWIN_REGPOKE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_017, p_str );
				break;
			}
			if( is_print )
			{	
			PRINT_UTIL_Print( &p_wk->print_util[i], p_que, 0, 0, p_str,  p_font );
			}
		}

		GFL_STR_DeleteBuffer( p_src );
		GFL_STR_DeleteBuffer( p_str );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	プリントメイン
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	PRINT_QUE	*p_que				プリントキュー
 *
 *	@return	TRUEで全てのプリント終了	FALSEでまだ
 */
//-----------------------------------------------------------------------------
static BOOL PlayerInfo_Bmpwin_PrintMain( PLAYERINFO_WORK * p_wk, PRINT_QUE *p_que )
{	
	int i;
	BOOL ret;
	ret	= TRUE;
	for( i = 0; i < p_wk->bmpwin_max; i++ )
	{	
		if( p_wk->p_bmpwin[i] )
		{	
			ret	&= PRINT_UTIL_Trans( &p_wk->print_util[i], p_que );
		}
	}

	return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWIN破棄
 *
 *	@param	PLAYERINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_Bmpwin_Delete( PLAYERINFO_WORK * p_wk )
{	
	int i;
	for( i = 0; i < p_wk->bmpwin_max; i++ )
	{	
		if( p_wk->p_bmpwin[i] )
		{	
			GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
		}
	}
}
//=============================================================================
/**
 *					CLWK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ポケアイコン読みこみ
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	mode					モード
 *	@param	u16 *cp_poke	モンスター番号が入った配列6個分
 *	@param	u16 *cp_form	フォルム配列6個分
 *	@param	*p_unit				CLWK読みこみ用
 *	@param	heapID				ヒープＩＤ
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_POKEICON_Create( PLAYERINFO_WORK * p_wk, WIFIBATTLEMATCH_MODE mode, const u16 *cp_poke, const u16 *cp_form, GFL_CLUNIT *p_unit, HEAPID heapID )
{	
	//リソース読みこみ
	{	
		int i;
		ARCHANDLE	*p_handle;

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_POKEICON, heapID );

		p_wk->res[ RESID_POKEICON_PLT ]	= GFL_CLGRP_PLTT_RegisterComp( p_handle,
        POKEICON_GetPalArcIndex(), CLSYS_DRAW_SUB, PLAYERINFO_PLT_OBJ_POKEICON*0x20, heapID );

		p_wk->clwk_max	= 0;
		for( i = PLAYERINFO_CLWK_POKEICON_TOP; i < PLAYERINFO_CLWK_POKEICON_END; i++ )
		{
			if( cp_poke[i] != 0 )
			{	
				p_wk->res[ RESID_POKEICON_CGR_TOP + i ]		= GFL_CLGRP_CGR_Register( 
						p_handle, POKEICON_GetCgxArcIndexByMonsNumber( cp_poke[i], cp_form[i], FALSE ),
						FALSE, CLSYS_DRAW_SUB, heapID ); 
				p_wk->res[ RESID_POKEICON_CELL_TOP + i ]	= GFL_CLGRP_CELLANIM_Register( 
						p_handle, POKEICON_GetCellArcIndex(), POKEICON_GetAnmArcIndex(), heapID ); 
				p_wk->clwk_max++;
			}
		}
		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK作成
	{	
		GFL_CLWK_DATA	clwk_data;
		int i;

		GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );
		clwk_data.pos_y		= PLAYERINFO_POKEICON_Y;
		clwk_data.anmseq	= POKEICON_ANM_HPMAX;
		for( i = 0; i < p_wk->clwk_max; i++ )
		{	
			clwk_data.pos_x	= PLAYERINFO_POKEICON_START_X + PLAYERINFO_POKEICON_DIFF_X*i;
			p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_unit,
					p_wk->res[ RESID_POKEICON_CGR_TOP + i ],
					p_wk->res[ RESID_POKEICON_PLT ],
					p_wk->res[ RESID_POKEICON_CELL_TOP + i ],
					&clwk_data,
					CLSYS_DRAW_SUB,
					heapID );

			GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk[i],
					POKEICON_GetPalNum( cp_poke[i], cp_form[i], FALSE ), 
					CLWK_PLTTOFFS_MODE_OAM_COLOR );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンアイコン破棄
 *
 *	@param	PLAYERINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_POKEICON_Delete( PLAYERINFO_WORK * p_wk )
{	
	
	//CLWK破棄
	{	
		int i;
		for( i = 0; i < p_wk->clwk_max; i++ )
		{	
			GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
		}
	}

	//リソース破棄
	{	
		int i;


		for( i = 0; i < p_wk->clwk_max; i++ )
		{	
			GFL_CLGRP_CGR_Release( p_wk->res[ RESID_POKEICON_CGR_TOP + i ] );
			GFL_CLGRP_CELLANIM_Release( p_wk->res[ RESID_POKEICON_CELL_TOP + i ] );
		}	

		GFL_CLGRP_PLTT_Release( p_wk->res[ RESID_POKEICON_PLT ] );
	
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分の見た目を作成
 *
 *	@param	PLAYERINFO_WORK * p_wk	ワーク
 *	@param	trainerID		見た目
 *	@param	*p_unit			ユニット
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_TRAINER_Cleate( PLAYERINFO_WORK * p_wk, u32 trainerID, GFL_CLUNIT *p_unit, HEAPID heapID )
{	
	//リソース読みこみ
	{	
		ARCHANDLE	*p_handle;

		p_handle	= GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), heapID );

		p_wk->res[ RESID_TRAINER_PLT ]	= GFL_CLGRP_PLTT_Register( p_handle,
        APP_COMMON_GetNull4x4PltArcIdx(), CLSYS_DRAW_SUB, PLAYERINFO_PLT_OBJ_TRAINER*0x20, heapID );

		p_wk->res[ RESID_TRAINER_CGR ]		= GFL_CLGRP_CGR_Register( 
				p_handle, APP_COMMON_GetNull4x4CharArcIdx(),
				FALSE, CLSYS_DRAW_SUB, heapID ); 

		p_wk->res[ RESID_TRAINER_CELL ]	= GFL_CLGRP_CELLANIM_Register( 
				p_handle, APP_COMMON_GetStatusIconCellArcIdx(APP_COMMON_MAPPING_128K), 
				APP_COMMON_GetStatusIconAnimeArcIdx(APP_COMMON_MAPPING_128K), heapID ); 

		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK
	{	
		GFL_CLWK_DATA	clwk_data;

		GFL_STD_MemClear( &clwk_data, sizeof(GFL_CLWK_DATA) );
		clwk_data.pos_y		= 0;
		clwk_data.pos_x		= 0;
		p_wk->p_clwk[PLAYERINFO_CLWK_TRAINER]	= GFL_CLACT_WK_Create( p_unit,
				p_wk->res[ RESID_TRAINER_CGR ],
				p_wk->res[ RESID_TRAINER_PLT ],
				p_wk->res[ RESID_TRAINER_CELL ],
				&clwk_data,
				CLSYS_DRAW_SUB,
				heapID );


	 	CLWK_TransNSBTX( p_wk->p_clwk[PLAYERINFO_CLWK_TRAINER], 
				ARCID_MMDL_RES, trainerID, 3, NSBTX_DEF_SX, NSBTX_DEF_SY, 0, CLSYS_DRAW_SUB, heapID );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分の見た目を破棄
 *
 *	@param	PLAYERINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void PlayerInfo_TRAINER_Delete( PLAYERINFO_WORK * p_wk )
{	
	//CLWK破棄
	{	
		GFL_CLACT_WK_Remove( p_wk->p_clwk[PLAYERINFO_CLWK_TRAINER] );
	}

	//リソース破棄
	{	
		GFL_CLGRP_CGR_Release( p_wk->res[ RESID_TRAINER_CGR ] );
		GFL_CLGRP_CELLANIM_Release( p_wk->res[ RESID_TRAINER_CELL ] );
		GFL_CLGRP_PLTT_Release( p_wk->res[ RESID_POKEICON_PLT ] );
	}
}

#ifdef DEBUG_DATA_CREATE
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用データを作成
 *
 *	@param	WIFIBATTLEMATCH_MODE mode	モード
 *	@param	*p_data										受け取りワーク
 */
//-----------------------------------------------------------------------------
void PLAYERINFO_DEBUG_CreateData( WIFIBATTLEMATCH_MODE mode, void *p_data_adrs )
{	
	switch( mode )
	{	
	case WIFIBATTLEMATCH_MODE_RANDOM_FREE:
		/* fallthrough */
	case WIFIBATTLEMATCH_MODE_RANDOM_RATE:
		{	
			PLAYERINFO_RANDOMMATCH_DATA	*p_data	= p_data_adrs;
			GFL_STD_MemClear( p_data, sizeof(PLAYERINFO_RANDOMMATCH_DATA) );
			p_data->btl_rule	= 0;
			p_data->rate			= 1234;
			p_data->btl_cnt		= 99;
			p_data->win_cnt		= 70;
			p_data->lose_cnt	= 29;	
		}
		break;

	case WIFIBATTLEMATCH_MODE_WIFI_LIMIT:
		/* fallthrough */
	case WIFIBATTLEMATCH_MODE_WIFI_NOLIMIT:
		{	
			PLAYERINFO_WIFICUP_DATA	*p_data	= p_data_adrs;
			GFL_STD_MemClear( p_data, sizeof(PLAYERINFO_WIFICUP_DATA) );
			p_data->cup_name[0]	= L'デ';
			p_data->cup_name[1]	= L'バ';
			p_data->cup_name[2]	= L'ッ';
			p_data->cup_name[3]	= L'グ';
			p_data->cup_name[4]	= L'W';
			p_data->cup_name[5]	= L'i';
			p_data->cup_name[6]	= L'-';
			p_data->cup_name[7]	= L'F';
			p_data->cup_name[8]	= L'i';
			p_data->cup_name[9]	= L'た';
			p_data->cup_name[10]	= L'い';
			p_data->cup_name[11]	= L'か';
			p_data->cup_name[12]	= L'い';
			p_data->cup_name[13]	= GFL_STR_GetEOMCode();
			p_data->start_date	= GFDATE_Set( 10, 11, 12, 0);
			p_data->end_date		= GFDATE_Set( 10, 12, 24, 0);
			p_data->rate				= 53;
			p_data->btl_cnt			= 2;
			p_data->btl_max			= 15;
			p_data->poke[0]			= 1;
			p_data->poke[1]			= 2;
			p_data->poke[2]			= 3;
			p_data->poke[3]			= 4;
			p_data->poke[4]			= 5;
			p_data->poke[5]			= 6;
		}
		break;

	case WIFIBATTLEMATCH_MODE_LIVE:
		{	
			PLAYERINFO_LIVECUP_DATA	*p_data	= p_data_adrs;
			GFL_STD_MemClear( p_data, sizeof(PLAYERINFO_LIVECUP_DATA) );
			p_data->cup_name[0]	= L'デ';
			p_data->cup_name[1]	= L'バ';
			p_data->cup_name[2]	= L'ッ';
			p_data->cup_name[3]	= L'グ';
			p_data->cup_name[4]	= L'ラ';
			p_data->cup_name[5]	= L'イ';
			p_data->cup_name[6]	= L'ブ';
			p_data->cup_name[7]	= L'た';
			p_data->cup_name[8]	= L'い';
			p_data->cup_name[9]	= L'か';
			p_data->cup_name[10]	= L'い';
			p_data->cup_name[11]	= GFL_STR_GetEOMCode();
			p_data->start_date	= GFDATE_Set( 11, 11, 12, 0);
			p_data->end_date		= GFDATE_Set( 11, 12, 24, 0);
			p_data->win_cnt			= 20;
			p_data->lose_cnt		= 17;
			p_data->btl_cnt			= 2;
			p_data->btl_max			= 15;
			p_data->poke[0]			= 1;
			p_data->poke[1]			= 2;
			p_data->poke[2]			= 3;
			p_data->poke[3]			= 4;
			p_data->poke[4]			= 5;
			p_data->poke[5]			= 6;
		}
		break;
	}
}
#endif


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					対戦者情報表示
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
enum
{	
	MATCHINFO_BMPWIN_PLAYER,
	MATCHINFO_BMPWIN_RATE_LABEL,
	MATCHINFO_BMPWIN_RATE_NUM,
	MATCHINFO_BMPWIN_COMEFROM,
	MATCHINFO_BMPWIN_CONTRY,
	MATCHINFO_BMPWIN_PLACE,
	MATCHINFO_BMPWIN_GREET,
	MATCHINFO_BMPWIN_PMS,
	MATCHINFO_BMPWIN_MAX,
};
enum
{	
	MATCHINFO_CLWK_MAX	= 6,
};
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	対戦者情報表示ワーク
//=====================================
struct _MATCHINFO_WORK
{	
	GFL_BMPWIN		*p_bmpwin[ MATCHINFO_BMPWIN_MAX ];
	PRINT_UTIL		print_util[ MATCHINFO_BMPWIN_MAX ];
	GFL_CLWK			*p_clwk[ MATCHINFO_CLWK_MAX ];
	PMS_DRAW_WORK	*p_pms;
};
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BMPWIN
//=====================================
static void MatchInfo_Bmpwin_Create( MATCHINFO_WORK * p_wk, const MATCHINFO_DATA *cp_data, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID );
static BOOL MatchInfo_Bmpwin_PrintMain( MATCHINFO_WORK * p_wk, PRINT_QUE *p_que );
static void MatchInfo_Bmpwin_Delete( MATCHINFO_WORK * p_wk );
//=============================================================================
/**
 *					データ
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	対戦者情報	作成
 *
 *	@param	const MATCHINFO_DATA *cp_data	データ
 *	@param	PRINT_QUE *p_que							キュー
 *	@param	*p_msg												メッセージ
 *	@param	*p_word												単語登録
 *	@param	heapID												ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
MATCHINFO_WORK	* MATCHINFO_Init( const MATCHINFO_DATA *cp_data, GFL_CLUNIT *p_unit, GFL_FONT	*p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID )
{	
	MATCHINFO_WORK	*	p_wk;
	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(MATCHINFO_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(MATCHINFO_WORK) );

	p_wk->p_pms	= PMS_DRAW_Init( p_unit, CLSYS_DRAW_MAIN, p_que, p_font, MATCHINFO_PLT_OBJ_PMS, 1, heapID );
	

	MatchInfo_Bmpwin_Create( p_wk, cp_data, p_font, p_que, p_msg, p_word, heapID );

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	対戦者情報破棄
 *
 *	@param	MATCHINFO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MATCHINFO_Exit( MATCHINFO_WORK *p_wk )
{	
	MatchInfo_Bmpwin_Delete( p_wk );

	PMS_DRAW_Exit( p_wk->p_pms );

	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	対戦者情報	プリント
 *
 *	@param	MATCHINFO_WORK *p_wk	ワーク
 *	@param	*p_que								キュー
 *
 *	@return	TRUEでプリント終了	FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL MATCHINFO_PrintMain( MATCHINFO_WORK *p_wk, PRINT_QUE *p_que )
{	
	return MatchInfo_Bmpwin_PrintMain( p_wk, p_que );
}

#ifdef DEBUG_DATA_CREATE
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用データ作成
 *
 *	@param	MATCHINFO_DATA *p_data_adrs データ受け取り
 */
//-----------------------------------------------------------------------------
void MATCHINFO_DEBUG_CreateData( MATCHINFO_DATA *p_data )
{	
	GFL_STD_MemClear( p_data, sizeof(MATCHINFO_DATA) );
	p_data->name[0]	= L'た';
	p_data->name[1]	= L'い';
	p_data->name[2]	= L'せ';
	p_data->name[3]	= L'ん';
	p_data->name[4]	= L'し';
	p_data->name[5]	= L'ゃ';
	p_data->name[6]	= GFL_STR_GetEOMCode();
	p_data->rate		= 999;
	p_data->area		= 0;
	p_data->country	= 0;
	p_data->pms.sentence_type	= 0;
	p_data->pms.sentence_id		= 0;
	p_data->pms.word[0]				= 1;
	p_data->pms.word[1]				= 2;
}
#endif //DEBUG_DATA_CREATE

//=============================================================================
/**
 *				BMPWIN
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWIN作成
 *
 *	@param	MATCHINFO_WORK * p_wk		ワーク
 *	@param	MATCHINFO_DATA *cp_data	データ
 *	@param	*p_font	フォント
 *	@param	*p_que	キュー
 *	@param	*p_msg	メッセージ
 *	@param	*p_word	単語登録
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void MatchInfo_Bmpwin_Create( MATCHINFO_WORK * p_wk, const MATCHINFO_DATA *cp_data, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID )
{	
	static const struct
	{	
		u8 x;
		u8 y;
		u8 w;
		u8 h;
	}sc_bmpwin_range[MATCHINFO_BMPWIN_MAX]	=
	{	
		//プレイヤー名
		{	
			6,2,14,2,
		},
		//レーティング
		{	
			17,2,8,2,
		},
		//レーティングの数値
		{	
			25, 2, 5, 2,
		},
		//住んでいるところ
		{	
			2,5,28,2,
		},
		//国
		{	
			2, 7, 28, 2,
		},
		//場所
		{	
			2, 9, 28, 2,
		},
		//挨拶
		{	
			2, 13, 28, 2,
		},
		//PMS
		{	
			2, 15, 28, 4,
		},
	};

	//BMPWIN作成
	{	
		int i;
		for( i = 0; i < MATCHINFO_BMPWIN_MAX; i++ )
		{	
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( MATCHINFO_BG_FRAME, 
					sc_bmpwin_range[i].x, sc_bmpwin_range[i].y, sc_bmpwin_range[i].w, sc_bmpwin_range[i].h,
					MATCHINFO_PLT_BG_FONT, GFL_BMP_CHRAREA_GET_B );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0xE );
			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			if( i != MATCHINFO_BMPWIN_PMS )
			{	
				PRINT_UTIL_Setup( &p_wk->print_util[i], p_wk->p_bmpwin[i] );
			}
		}
	}

	//文字表示
	{	
		int i;
		STRBUF	*p_src;
		STRBUF	*p_str;

		BOOL	is_print;

		p_src	= GFL_STR_CreateBuffer( 128, heapID );
		p_str	= GFL_STR_CreateBuffer( 128, heapID );

		for( i = 0; i < MATCHINFO_BMPWIN_MAX; i++ )
		{	
			is_print	= TRUE;		
			switch( i )
			{	
			case MATCHINFO_BMPWIN_PLAYER:
				GFL_STR_SetStringCode( p_str, cp_data->name );
				break;
			case MATCHINFO_BMPWIN_RATE_LABEL:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_008, p_str );
				break;
			case MATCHINFO_BMPWIN_RATE_NUM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_009, p_src );
				WORDSET_RegisterNumber( p_word, 0, cp_data->rate, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case MATCHINFO_BMPWIN_COMEFROM:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_018, p_str );
				break;
			case MATCHINFO_BMPWIN_CONTRY:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_020, p_src );
				WORDSET_RegisterCountryName( p_word, 0, cp_data->country );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case MATCHINFO_BMPWIN_PLACE:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_021, p_src );
				WORDSET_RegisterLocalPlaceName( p_word, 0, cp_data->country, cp_data->area );
				WORDSET_ExpandStr( p_word, p_str, p_src );
				break;
			case MATCHINFO_BMPWIN_GREET:
				GFL_MSG_GetString( p_msg, WIFIMATCH_STR_019, p_str );
				break;
			case MATCHINFO_BMPWIN_PMS:
				PMS_DRAW_Print( p_wk->p_pms, p_wk->p_bmpwin[i], (PMS_DATA*)&cp_data->pms, 0 );
				is_print	= FALSE;
				break;
			}
			if( is_print )
			{	
				PRINT_UTIL_Print( &p_wk->print_util[i], p_que, 0, 0, p_str,  p_font );
			}

		}

		GFL_STR_DeleteBuffer( p_src );
		GFL_STR_DeleteBuffer( p_str );
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINプリントチェック
 *
 *	@param	MATCHINFO_WORK * p_wk	ワーク
 *	@param	*p_que									キュー
 *
 *	@return	TRIEでプリント終了	FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL MatchInfo_Bmpwin_PrintMain( MATCHINFO_WORK * p_wk, PRINT_QUE *p_que )
{	
	int i;
	BOOL ret;
	ret	= TRUE;
	for( i = 0; i < MATCHINFO_BMPWIN_MAX; i++ )
	{	
		if( i != MATCHINFO_BMPWIN_PMS )
		{	
			ret	&= PRINT_UTIL_Trans( &p_wk->print_util[i], p_que );
		}
		else
		{	
			PMS_DRAW_Main( p_wk->p_pms );
			ret &= PMS_DRAW_IsPrintEnd( p_wk->p_pms );
		}
	}

	return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief	BMPWIｎ破棄
 *
 *	@param	MATCHINFO_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void MatchInfo_Bmpwin_Delete( MATCHINFO_WORK * p_wk )
{	
	int i;
	for( i = 0; i < MATCHINFO_BMPWIN_MAX; i++ )
	{	
		if( p_wk->p_bmpwin[i] )
		{	
			GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
		}
	}
}
