//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_record_proc.c
 *	@brief	レコードプロセス
 *	@author	Toru=Nagihashi
 *	@data		2009.11.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID
#include "pokeicon/pokeicon.h"

//アーカイブ
#include "arc_def.h"
#include "msg/msg_battle_rec.h"

//自分のモジュール
#include "br_btn.h"
#include "br_util.h"

//セーブデータ
#include "savedata/battle_rec.h"

//外部参照
#include "br_record_proc.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ボタンインデックス
//=====================================
typedef enum
{
	BR_RECORD_BTNID_RETURN,
	BR_RECORD_BTNID_SAVE,
	BR_RECORD_BTNID_MAX
} BR_RECORD_BTNID;

//-------------------------------------
///	MSGWIN
//=====================================
typedef enum
{
  BR_RECORD_MSGWINID_S_PROFILE,  //プロフィールをみる／しまう
  BR_RECORD_MSGWINID_S_MAX,
} BR_RECORD_MSGWINID_S;
typedef enum
{
  BR_RECORD_MSGWINID_M_BTL_NAME = 0,  //●●●の記録
  BR_RECORD_MSGWINID_M_BTL_RULE,       //コロシアム　シングル　せいげんなし
  BR_RECORD_MSGWINID_M_BTL_NUMBER,    //ビデオナンバー
  BR_RECORD_MSGWINID_M_BTL_MAX,   

  BR_RECORD_MSGWINID_M_MAX  = BR_RECORD_MSGWINID_M_BTL_MAX,
} BR_RECORD_MSGWINID_M;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	レコードメインワーク
//=====================================
typedef struct 
{
  BMPOAM_SYS_PTR		p_bmpoam;	//BMPOAMシステム
  PRINT_QUE         *p_que;   //プリントキュー

	BR_BTN_WORK	      *p_btn[ BR_RECORD_BTNID_MAX ];
  BR_MSGWIN_WORK    *p_msgwin_s[ BR_RECORD_MSGWINID_S_MAX ];
  BR_MSGWIN_WORK    *p_msgwin_m[ BR_RECORD_MSGWINID_M_MAX ];

  BR_PROFILE_WORK   *p_profile_disp;

  u32               res_icon_plt;
  u32               res_icon_cel;
  u32               res_icon_chr[HEADER_MONSNO_MAX];
  GFL_CLWK          *p_icon[HEADER_MONSNO_MAX];

  BOOL              is_profile;
	HEAPID            heapID;
} BR_RECORD_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	レコードプロセス
//=====================================
static GFL_PROC_RESULT BR_RECORD_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_RECORD_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_RECORD_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	レコードプロセス
//=====================================
const GFL_PROC_DATA BR_RECORD_ProcData =
{	
	BR_RECORD_PROC_Init,
	BR_RECORD_PROC_Main,
	BR_RECORD_PROC_Exit,
};

//=============================================================================
/**
 *					プロトタイプ
 */
//=============================================================================
//画面作成
static void Br_Record_CreateMainDisplaySingle( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param );
static void Br_Record_CreateMainDisplayDouble( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param );
static void Br_Record_AddPokeIcon( BR_RECORD_WORK * p_wk, GFL_CLUNIT *p_clunit, BATTLE_REC_HEADER_PTR p_header, HEAPID heapID );
static void Br_Record_CreateMainDisplayProfile( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param );
static void Br_Record_DeleteMainDisplay( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param );

static void Br_Record_CreateSubDisplay( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param );
static void Br_Record_DeleteSubDisplay( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param );

//その他
static BOOL Br_Record_Check2vs2( BATTLE_REC_HEADER_PTR p_head );
static BOOL Br_Record_GetTrgProfile( BR_RECORD_WORK * p_wk, u32 x, u32 y );
static BOOL Br_Record_GetTrgStart( u32 x, u32 y );

//=============================================================================
/**
 *					レコードプロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	レコードプロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_RECORD_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_RECORD_WORK				*p_wk;
	BR_RECORD_PROC_PARAM	*p_param	= p_param_adrs;

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_RECORD_WORK), BR_PROC_SYS_GetHeapID( p_param->p_procsys ) );
	GFL_STD_MemClear( p_wk, sizeof(BR_RECORD_WORK) );	
	p_wk->heapID	= BR_PROC_SYS_GetHeapID( p_param->p_procsys );
  p_wk->is_profile  = FALSE;

  //モジュール作成
  p_wk->p_bmpoam		= BmpOam_Init( p_wk->heapID, p_param->p_unit);
  p_wk->p_que       = PRINTSYS_QUE_Create( p_wk->heapID );

	//グラフィック初期化
  Br_Record_CreateSubDisplay( p_wk, p_param );
  Br_Record_CreateMainDisplaySingle( p_wk, p_param );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	レコードプロセス	破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_RECORD_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	

	BR_RECORD_WORK				*p_wk	= p_wk_adrs;
	BR_RECORD_PROC_PARAM	*p_param	= p_param_adrs;

  if( p_wk->is_profile )
  { 
    BR_PROFILE_DeleteMainDisplay( p_wk->p_profile_disp );
  }
  else
  { 
    Br_Record_DeleteMainDisplay( p_wk, p_param );
  }

  //画面構築破棄
  Br_Record_DeleteSubDisplay( p_wk, p_param );
  GFL_BG_LoadScreenReq( BG_FRAME_S_FONT );
  GFL_BG_LoadScreenReq( BG_FRAME_M_FONT );

	//モジュール破棄
  PRINTSYS_QUE_Delete( p_wk->p_que );
  BmpOam_Exit( p_wk->p_bmpoam );

	//グラフィック破棄


	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	レコードプロセス	メイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_RECORD_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_MAIN,

    SEQ_CHANGEOUT_START,
    SEQ_CHANGEOUT_WAIT,
    SEQ_CHANGEIN_START,
    SEQ_CHANGEIN_WAIT,

    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_EXIT,
  };

	BR_RECORD_WORK	*p_wk	= p_wk_adrs;
	BR_RECORD_PROC_PARAM	*p_param	= p_param_adrs;


  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
    BR_FADE_StartFade( p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    *p_seq  = SEQ_FADEIN_WAIT;
    break;
  case SEQ_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_param->p_fade ) )
    { 
      *p_seq  = SEQ_MAIN;
    }
    break;
  case SEQ_MAIN:
    {
      u32 x, y;
      if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        //プロフィール切り替えチェック
        if( Br_Record_GetTrgProfile( p_wk, x, y ) )
        { 
          p_wk->is_profile ^= 1;
          *p_seq  = SEQ_CHANGEOUT_START;
        }

        //終了チェック
        if( BR_BTN_GetTrg( p_wk->p_btn[BR_RECORD_BTNID_RETURN], x, y ) )
        {	
          p_param->ret  = BR_RECORD_RETURN_FINISH;
          BR_PROC_SYS_Pop( p_param->p_procsys );
          *p_seq  = SEQ_FADEOUT_START;
        }	

        //再生ボタン
        if( Br_Record_GetTrgStart( x, y ) )
        { 
          p_param->ret  = BR_RECORD_RETURN_BTLREC;
          //フェードをすっとばす
          BR_PROC_SYS_Pop( p_param->p_procsys );
          BR_PROC_SYS_Pop( p_param->p_procsys );
          BR_PROC_SYS_Pop( p_param->p_procsys );
          BR_PROC_SYS_Pop( p_param->p_procsys );
          BR_PROC_SYS_Pop( p_param->p_procsys );
          BR_PROC_SYS_Pop( p_param->p_procsys );
          BR_PROC_SYS_Pop( p_param->p_procsys );
          *p_seq  = SEQ_EXIT;
        }
      }
    }
    break;

  case SEQ_CHANGEOUT_START:
    BR_FADE_StartFade( p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_MAIN, BR_FADE_DIR_OUT );
    (*p_seq)++;
    break;
  case SEQ_CHANGEOUT_WAIT:
    if( BR_FADE_IsEnd( p_param->p_fade ) )
    { 
      (*p_seq)++;
    }
    break;
  case SEQ_CHANGEIN_START:
    { 
      GFL_FONT *p_font;
      GFL_MSGDATA *p_msg;

      p_font  = BR_RES_GetFont( p_param->p_res );
      p_msg   = BR_RES_GetMsgData( p_param->p_res );
      //読み込み変え
      if( p_wk->is_profile )
      { 
        Br_Record_DeleteMainDisplay( p_wk, p_param );
        BR_MSGWIN_PrintColor( p_wk->p_msgwin_s[BR_RECORD_MSGWINID_S_PROFILE], p_msg, msg_719, p_font, BR_PRINT_COL_NORMAL );
        Br_Record_CreateMainDisplayProfile( p_wk, p_param );
      }
      else
      { 
        BR_PROFILE_DeleteMainDisplay( p_wk->p_profile_disp );
        p_wk->p_profile_disp  = NULL;
        BR_MSGWIN_PrintColor( p_wk->p_msgwin_s[BR_RECORD_MSGWINID_S_PROFILE], p_msg, msg_718, p_font, BR_PRINT_COL_NORMAL );
        Br_Record_CreateMainDisplaySingle( p_wk, p_param );
      }
    BR_FADE_StartFade( p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_MAIN, BR_FADE_DIR_IN );
    }
    (*p_seq)++;
    break;
  case SEQ_CHANGEIN_WAIT:
    if( BR_FADE_IsEnd( p_param->p_fade ) )
    { 
      *p_seq  = SEQ_MAIN;
    }
    break;

  case SEQ_FADEOUT_START:
    BR_FADE_StartFade( p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;
  case SEQ_FADEOUT_WAIT:
    if( BR_FADE_IsEnd( p_param->p_fade ) )
    { 
      *p_seq  = SEQ_EXIT;
    }
    break;
  case SEQ_EXIT:
    NAGI_Printf( "RECORD: Exit!\n" );
    return GFL_PROC_RES_FINISH;
  }

  //文字表示
  { 
    int i;
    for( i = 0; i < BR_RECORD_MSGWINID_S_MAX; i++ )
    { 
      if( p_wk->p_msgwin_s[i] )
      { 
        BR_MSGWIN_PrintMain( p_wk->p_msgwin_s[i] );
      }
    }
    for( i = 0; i < BR_RECORD_MSGWINID_M_MAX; i++ )
    { 
      if( p_wk->p_msgwin_m[i] )
      { 
        BR_MSGWIN_PrintMain( p_wk->p_msgwin_m[i] );
      }
    }
  }

  if( p_wk->p_profile_disp )
  { 
    BR_PROFILE_PrintMain( p_wk->p_profile_disp );
  }

  PRINTSYS_QUE_Main( p_wk->p_que );


	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *			プライベート
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	シングル戦用録画ヘッダ画面構築
 *
 *	@param	BR_RECORD_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Record_CreateMainDisplaySingle( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM *p_param )
{	
  static const struct 
  { 
    u8 x;
    u8 y;
    u8 w;
    u8 h;
    u32 msgID;
  } sc_msgwin_data[BR_RECORD_MSGWINID_M_BTL_MAX]  =
  { 
    {
      4,
      4,
      24,
      2,
      msg_09,
    },
    {
      4,
      6,
      24,
      4,
      msg_rule_000,
    },
    {
      4,
      21,
      24,
      2,
      msg_12,
    },
  };

	BR_RES_LoadBG( p_param->p_res, BR_RES_BG_RECORD_M_BTL_SINGLE, p_wk->heapID );
  
  
  //メッセージWIN作成
  {
    int i;
    GFL_FONT *p_font;
    GFL_MSGDATA *p_msg;
    WORDSET *p_word;
    STRBUF  *p_strbuf;
    STRBUF  *p_src;

    p_font  = BR_RES_GetFont( p_param->p_res );
    p_msg   = BR_RES_GetMsgData( p_param->p_res );
    p_word  = BR_RES_GetWordSet( p_param->p_res );

    for( i = 0; i < BR_RECORD_MSGWINID_M_BTL_MAX; i++ )
    { 
      p_wk->p_msgwin_m[i]  = BR_MSGWIN_Init( BG_FRAME_M_FONT, sc_msgwin_data[i].x, sc_msgwin_data[i].y, sc_msgwin_data[i].w, sc_msgwin_data[i].h, PLT_BG_M_FONT, p_wk->p_que, p_wk->heapID );


      switch( i )
      { 
      case BR_RECORD_MSGWINID_M_BTL_NAME:  //●●●の記録
        { 
          STRBUF  *p_name;
          p_strbuf  = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
          p_name    = GDS_Profile_CreateNameString( p_param->p_profile, p_wk->heapID );
          p_src     = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
          WORDSET_RegisterWord( p_word, 0, p_name, GDS_Profile_GetSex(p_param->p_profile), TRUE, PM_LANG );
          WORDSET_ExpandStr( p_word, p_strbuf, p_src );
          GFL_STR_DeleteBuffer( p_name );
          GFL_STR_DeleteBuffer( p_src );
        }
        break;
      case BR_RECORD_MSGWINID_M_BTL_RULE:       //コロシアム　シングル　せいげんなし
        {
          const u32 btl_rule  = RecHeader_ParamGet( p_param->p_header, RECHEAD_IDX_MODE, 0);
          p_strbuf  = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID + btl_rule );
        }
        break;
      case BR_RECORD_MSGWINID_M_BTL_NUMBER:    //ビデオナンバー
        { 
          const u64 number  = RecHeader_ParamGet( p_param->p_header, RECHEAD_IDX_DATA_NUMBER, 0);
          u64 dtmp1 = number;
          u32 dtmp2[3];

          dtmp2[ 0 ] = dtmp1 % 100000;	///< 3block
          dtmp1 /= 100000;
          dtmp2[ 1 ] = dtmp1 % 100000;	///< 2block
          dtmp1 /= 100000;
          dtmp2[ 2 ] = dtmp1;				///< 1block
          
          { 
            int check1 = ( dtmp2[ 2 ] / 10 ) % 10;

            if ( ( check1 == 0 ) && ( number != 0 ) ){
              p_strbuf = GFL_MSG_CreateString( p_msg,  msg_12_2 );		///< ｘｘｘせんめ
              OS_Printf( "特殊なデータナンバー\n");
            }
            else
            { 
              p_strbuf  = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
            }
          }
          p_src     = GFL_MSG_CreateString( p_msg, sc_msgwin_data[i].msgID );
          WORDSET_RegisterNumber( p_word, 2, dtmp2[0], 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
          WORDSET_RegisterNumber( p_word, 1, dtmp2[1], 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
          WORDSET_RegisterNumber( p_word, 0, dtmp2[2], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
          WORDSET_ExpandStr( p_word, p_strbuf, p_src );
          GFL_STR_DeleteBuffer( p_src );
        }
        break;
      }

      BR_MSGWIN_PrintBufColor( p_wk->p_msgwin_m[i], p_strbuf, p_font, BR_PRINT_COL_NORMAL );

      GFL_STR_DeleteBuffer( p_strbuf ); 
    }
  }

  //ポケアイコン
  { 
    Br_Record_AddPokeIcon( p_wk, p_param->p_unit, p_param->p_header, p_wk->heapID );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	ダブル戦用録画ヘッダ画面構築
 *
 *	@param	BR_RECORD_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Record_CreateMainDisplayDouble( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param )
{	
	BR_RES_LoadBG( p_param->p_res, BR_RES_BG_RECORD_M_BTL_DOUBLE, p_wk->heapID );

}
//----------------------------------------------------------------------------
/**
 *	@brief  録画ヘッダ画面にポケアイコンをつける
 *	        このソースはGSから移植
 *
 *	@param	BR_RECORD_WORK * p_wk ワーク
 *	@param  p_param	              引数
 */
//-----------------------------------------------------------------------------
static void Br_Record_AddPokeIcon( BR_RECORD_WORK * p_wk, GFL_CLUNIT *p_clunit, BATTLE_REC_HEADER_PTR p_header, HEAPID heapID )
{	
	int i;
	int monsno;
	int form;
	int no = 0;
	
	int gra_id;
	int monsno_tbl[ HEADER_MONSNO_MAX ];
	int form_tbl[ HEADER_MONSNO_MAX ];
	int type;
  ARCHANDLE *p_handle;
  GFL_CLWK_DATA cldata;
	
	
	static const s16 pos_tbl[][ HEADER_MONSNO_MAX ][ 2 ] = 
  {
		{
			{ 11*8, 13*8 },{ 14*8, 13*8 },{ 17*8, 13*8 },{ 20*8, 13*8 },{ 23*8, 13*8 },{ 26*8, 13*8 },
			{  3*8, 19*8 },{  6*8, 19*8 },{  9*8, 19*8 },{ 12*8, 19*8 },{ 15*8, 19*8 },{ 18*8, 19*8 },
		},
		{
			{  3*8, 17*8 },{  6*8, 17*8 },{  9*8, 17*8 },	///< 自分
			{  5*8, 20*8 },{  8*8, 20*8 },{ 11*8, 20*8 },	///< 自分
			{ 18*8, 17*8 },{ 21*8, 17*8 },{ 24*8, 17*8 },	///< 相手
			{ 20*8, 20*8 },{ 23*8, 20*8 },{ 26*8, 20*8 },	///< 相手
		},
	};
  p_handle  = GFL_ARC_OpenDataHandle( ARCID_POKEICON, GFL_HEAP_LOWID(heapID) );


  //パレット、セル
  p_wk->res_icon_plt  = GFL_CLGRP_PLTT_RegisterComp( p_handle, 
      POKEICON_GetPalArcIndex(),
      CLSYS_DRAW_MAIN, PLT_OBJ_M_POKEICON*0x20, heapID );
  p_wk->res_icon_cel = GFL_CLGRP_CELLANIM_Register( p_handle,
      POKEICON_GetCellArcIndex(), POKEICON_GetAnmArcIndex(), heapID );

	
	///< テーブルの作成
	{	
		int temp_no = 0;
		int temp_monsno = 0;
		int temp_form = 0;
		int add = 6;
		int si = 0;
		int ei = add;
		
		type = 0;

		///< 2vs2の場合
		if ( Br_Record_Check2vs2( p_header ) == TRUE ){
			add = 3;
			ei = 3;
			type = 1;
		}
		do {
			for ( i = si; i < ei; i++ ){
				monsno_tbl[ temp_no ] = 0xFF;
				
				temp_monsno = RecHeader_ParamGet( p_header, RECHEAD_IDX_MONSNO, i );
				temp_form	= RecHeader_ParamGet( p_header, RECHEAD_IDX_FORM_NO, i );

				if ( temp_monsno == 0 ){ continue; }
				
				monsno_tbl[ temp_no ] = temp_monsno;
				form_tbl[ temp_no]	  = temp_form;
				
				temp_no++;
			}
			for ( i = temp_no; i < ei; i++ ){	
				monsno_tbl[ temp_no ] = 0;
				form_tbl[ temp_no ]	  = 0;
				temp_no++;
			}		
			si += add;
			ei += add;
			
		} while ( ei <= HEADER_MONSNO_MAX );
	}
	
	for ( i = 0; i < HEADER_MONSNO_MAX; i++ ){
				
		p_wk->p_icon[ i ]     = NULL;
    p_wk->res_icon_chr[i] = GFL_CLGRP_REGISTER_FAILED;
		
		monsno = monsno_tbl[ i ];
		
		if ( monsno == 0 ){ continue; }		///< たまご無し
		
		form = form_tbl[ i ];
		

    p_wk->res_icon_chr[i] = GFL_CLGRP_CGR_Register( p_handle,
              POKEICON_GetCgxArcIndexByMonsNumber( monsno, form, 0 ),
              FALSE, CLSYS_DRAW_MAIN, heapID );
  
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

		cldata.pos_x		= pos_tbl[ type ][ i ][ 0 ] + 12 - 4;
		cldata.pos_y		= pos_tbl[ type ][ i ][ 1 ] - 12;
		cldata.anmseq 	= POKEICON_ANM_HPMAX;
				
		p_wk->p_icon[ no ] = GFL_CLACT_WK_Create( p_clunit,
          p_wk->res_icon_chr[i],p_wk->res_icon_plt,p_wk->res_icon_cel,
            &cldata, CLSYS_DEFREND_MAIN, heapID );
		
    GFL_CLACT_WK_SetPlttOffs( p_wk->p_icon[ no ], POKEICON_GetPalNum( monsno, form, 0 ),
        CLWK_PLTTOFFS_MODE_OAM_COLOR );

		
		OS_Printf( "no = %2d\n", no );
		no++;	
	}

  GFL_ARC_CloseDataHandle( p_handle );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロフィール画面構築
 *
 *	@param	BR_RECORD_WORK * p_wk ワーク
 *	@param	p_param								レコードの引数
 */
//-----------------------------------------------------------------------------
static void Br_Record_CreateMainDisplayProfile( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param )
{
  p_wk->p_profile_disp  = BR_PROFILE_CreateMainDisplay( p_param->p_profile, p_param->p_res, p_param->p_unit, p_wk->p_que, p_wk->heapID );

}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン画面破棄
 *
 *	@param	BR_RECORD_WORK * p_wk   ワーク
 *	@param	p_param                 引数
 */
//-----------------------------------------------------------------------------
static void Br_Record_DeleteMainDisplay( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM *p_param )
{ 
  //OBJ破棄
  { 
    int i;
    for( i = 0; i < HEADER_MONSNO_MAX; i++ )
    { 
      if( p_wk->p_icon[i] )
      { 
        GFL_CLACT_WK_Remove( p_wk->p_icon[i] );
        p_wk->p_icon[i] = NULL;
      }
      if( p_wk->res_icon_chr[i] != GFL_CLGRP_REGISTER_FAILED )
      { 
        GFL_CLGRP_CGR_Release(p_wk->res_icon_chr[i]);
      }
    }
    GFL_CLGRP_PLTT_Release(p_wk->res_icon_plt);
    GFL_CLGRP_CELLANIM_Release(p_wk->res_icon_cel);

  }

  //BG破棄
  { 
    int i;
    for( i = 0; i < BR_RECORD_MSGWINID_M_MAX; i++ )
    { 
      if( p_wk->p_msgwin_m[i] )
      { 
        BR_MSGWIN_Exit( p_wk->p_msgwin_m[i] );
        p_wk->p_msgwin_m[i] = NULL;
      }
    }
  }

  BR_RES_UnLoadBG( p_param->p_res, BR_RES_BG_RECORD_M_BTL_SINGLE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	下画面構築
 *
 *	@param	BR_RECORD_WORK * p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Br_Record_CreateSubDisplay( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param )
{	
  static const struct 
  { 
    u8 x;
    u8 y;
    u8 w;
    u8 h;
    u32 msgID;
  } sc_msgwin_data[BR_RECORD_MSGWINID_S_MAX]  =
  { 
    {
      9,
      3,
      14,
      2,
      msg_718
    }
  };
  GFL_FONT *p_font;
  GFL_MSGDATA *p_msg;

	BR_RES_LoadBG( p_param->p_res, BR_RES_BG_RECORD_S, p_wk->heapID );
	BR_RES_LoadOBJ( p_param->p_res, BR_RES_OBJ_SHORT_BTN_S, p_wk->heapID );


  p_font  = BR_RES_GetFont( p_param->p_res );
  p_msg   = BR_RES_GetMsgData( p_param->p_res );


  //ボタン作成
  { 
    GFL_CLWK_DATA cldata;
    BR_RES_OBJ_DATA res;

    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    cldata.pos_x    = 80;
    cldata.pos_y    = 168;
    cldata.anmseq   = 0;
    cldata.softpri  = 1;

    BR_RES_GetOBJRes( p_param->p_res, BR_RES_OBJ_SHORT_BTN_S, &res );

    p_wk->p_btn[BR_RECORD_BTNID_RETURN] = BR_BTN_Init( &cldata, msg_05, BR_BTN_DATA_SHORT_WIDTH,CLSYS_DRAW_SUB, p_param->p_unit, p_wk->p_bmpoam, p_font, p_msg, &res, p_wk->heapID );
;
  }

  //メッセージWIN作成
  {
    int i;
    for( i = 0; i < BR_RECORD_MSGWINID_S_MAX; i++ )
    { 
      p_wk->p_msgwin_s[i]  = BR_MSGWIN_Init( BG_FRAME_S_FONT, sc_msgwin_data[i].x, sc_msgwin_data[i].y, sc_msgwin_data[i].w, sc_msgwin_data[i].h, PLT_BG_S_FONT, p_wk->p_que, p_wk->heapID );
      BR_MSGWIN_PrintColor( p_wk->p_msgwin_s[i], p_msg, sc_msgwin_data[i].msgID, p_font, BR_PRINT_COL_NORMAL );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  下画面破棄
 *
 *	@param	BR_RECORD_WORK * p_wk   ワーク
 *	@param	p_param                 引数
 */
//-----------------------------------------------------------------------------
static void Br_Record_DeleteSubDisplay( BR_RECORD_WORK * p_wk, BR_RECORD_PROC_PARAM	*p_param )
{ 
  {
    int i;
    for( i = 0; i < BR_RECORD_MSGWINID_S_MAX; i++ )
    {
      if( p_wk->p_msgwin_s[i] )
      { 
        BR_MSGWIN_Exit( p_wk->p_msgwin_s[i] );
        p_wk->p_msgwin_s[i] = NULL;
      }
    }
  }

  //ボタン破棄
  { 
    int i;
    for( i = 0; i < BR_RECORD_BTNID_MAX; i++ )
    { 
      if( p_wk->p_btn[i] )
      { 
        BR_BTN_Exit( p_wk->p_btn[i] );
        p_wk->p_btn[i]  = NULL;
      }
    }
  }

  BR_RES_UnLoadOBJ( p_param->p_res, BR_RES_OBJ_SHORT_BTN_S ); 
	BR_RES_UnLoadBG( p_param->p_res, BR_RES_BG_RECORD_S );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ２ｖ２かどうかをチェックする
 *
 *	@param	BATTLE_REC_HEADER_PTR p_head  ヘッダ 
 *
 *	@return TRUEならば２ｖ２用録画ヘッダ
 */
//-----------------------------------------------------------------------------
static BOOL Br_Record_Check2vs2( BATTLE_REC_HEADER_PTR p_head )
{
	int	type;
	BOOL b2vs2 = FALSE; 
	
	type = RecHeader_ParamGet( p_head, RECHEAD_IDX_MODE, 0 );
	
	switch ( type ){
	case GT_BATTLE_MODE_TOWER_MULTI:
	case GT_BATTLE_MODE_FACTORY50_MULTI:
	case GT_BATTLE_MODE_FACTORY100_MULTI:
	case GT_BATTLE_MODE_STAGE_MULTI:
	case GT_BATTLE_MODE_CASTLE_MULTI:
	case GT_BATTLE_MODE_ROULETTE_MULTI:
	case GT_BATTLE_MODE_COLOSSEUM_MULTI:
		b2vs2 = TRUE;
		break;
	default:
		b2vs2 = FALSE;
		break;
	};
	
	return b2vs2;
}
//----------------------------------------------------------------------------
/**
 *	@brief  プロフィール切り替えボタンをおしたかどうか
 *
 *	@param	BR_RECORD_WORK * p_wk   ワーク
 *	@param  xタッチ座標
 *	@param  yタッチ座標
 *
 *	@return TRUEで押した  FALSEで押していない
 */
//-----------------------------------------------------------------------------
static BOOL Br_Record_GetTrgProfile( BR_RECORD_WORK * p_wk, u32 x, u32 y )
{ 
	GFL_RECT rect;

	rect.left		= (8)*8;
	rect.right	= (8 + 15)*8;
	rect.top		= (2)*8;
	rect.bottom	= (2 + 4)*8;

  return ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));
}
//----------------------------------------------------------------------------
/**
 *	@brief  タッチスタート
 *
 *	@param  xタッチ座標
 *	@param  yタッチ座標
 *
 *	@return TRUEで押した  FALSEで押していない
 */
//-----------------------------------------------------------------------------
static BOOL Br_Record_GetTrgStart( u32 x, u32 y )
{ 
	GFL_RECT rect;

	rect.left		= (10)*8;
	rect.right	= (10 + 12)*8;
	rect.top		= (7)*8;
	rect.bottom	= (7 + 8)*8;

  return ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));
}
