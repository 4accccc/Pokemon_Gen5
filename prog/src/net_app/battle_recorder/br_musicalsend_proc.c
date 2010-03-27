//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_musicalsend_proc.c
 *	@brief	ミュージカル送信プロセス
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

//ミュージカル
#include "musical/mus_shot_photo.h"
#include "poke_tool/monsno_def.h" //debug用

//アーカイブ
#include "msg/msg_battle_rec.h"

//自分のモジュール
#include "br_pokesearch.h"
#include "br_inner.h"
#include "br_util.h"
#include "br_btn.h"

//外部参照
#include "br_musicalsend_proc.h"


//ミュージカルモジュールを使うので
FS_EXTERN_OVERLAY( musical_shot );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
///	ボタン
//=====================================
typedef enum
{
  BR_MUSICALSEND_BTN_RETURN,
  BR_MUSICALSEND_BTN_SEND,
  BR_MUSICALSEND_BTN_MAX,
  BR_MUSICALSEND_BTN_NONE = BR_MUSICALSEND_BTN_MAX,
} BR_MUSICALSEND_BTNID;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ミュージカル送信メインワーク
//=====================================
typedef struct 
{

  MUS_SHOT_PHOTO_WORK *p_photo;
  BR_BTN_WORK         *p_btn[ BR_MUSICALSEND_BTN_MAX ];
  BR_SEQ_WORK         *p_seq;
  BR_TEXT_WORK        *p_text;
  BMPOAM_SYS_PTR	  	p_bmpoam;	//BMPOAMシステム
  PRINT_QUE           *p_que;
	HEAPID              heapID;
  MUSICAL_SHOT_DATA   *p_musical_shot;

  BR_MUSICALSEND_PROC_PARAM *p_param;
} BR_MUSICALSEND_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// ミュージカル送信プロセス
//=====================================
static GFL_PROC_RESULT BR_MUSICALSEND_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_MUSICALSEND_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_MUSICALSEND_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	流れ
//=====================================
static void Br_MusicalSend_Seq_FadeIn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_MusicalSend_Seq_FadeOut( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_MusicalSend_Seq_Main( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_MusicalSend_Seq_Upload( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	描画
//=====================================
static void Br_MusicalSend_CreatePhoto( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );
static void Br_MusicalSend_DeletePhoto( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );

static void Br_MusicalSend_CreateSubDisplay( BR_MUSICALSEND_WORK *p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );
static void Br_MusicalSend_DeleteSubDisplay( BR_MUSICALSEND_WORK *p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//-------------------------------------
///	ミュージカル送信プロセス
//=====================================
const GFL_PROC_DATA BR_MUSICALSEND_ProcData =
{	
	BR_MUSICALSEND_PROC_Init,
	BR_MUSICALSEND_PROC_Main,
	BR_MUSICALSEND_PROC_Exit,
};

//=============================================================================
/**
 *					ミュージカル送信プロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ミュージカル送信プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_MUSICALSEND_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_MUSICALSEND_WORK				*p_wk;
	BR_MUSICALSEND_PROC_PARAM	*p_param	= p_param_adrs;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(musical_shot) );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_MUSICALSEND_WORK), BR_PROC_SYS_GetHeapID( p_param->p_procsys ) );
	GFL_STD_MemClear( p_wk, sizeof(BR_MUSICALSEND_WORK) );	
	p_wk->heapID	= BR_PROC_SYS_GetHeapID( p_param->p_procsys );
  p_wk->p_param = p_param;

  //モジュール作成
  p_wk->p_que   = PRINTSYS_QUE_Create( p_wk->heapID );
  { 
    GFL_CLUNIT *p_unit;
    p_unit  = BR_GRAPHIC_GetClunit( p_param->p_graphic );
    p_wk->p_bmpoam	= BmpOam_Init( p_wk->heapID, p_unit );
  }

  //自分のミュージカルショット読み込み
  { 
    SAVE_CONTROL_WORK *p_sv      = GAMEDATA_GetSaveControlWork( p_param->p_gamedata );
    MUSICAL_SAVE  *p_musical_sv = MUSICAL_SAVE_GetMusicalSave( p_sv );
    p_wk->p_musical_shot  = MUSICAL_SAVE_GetMusicalShotData( p_musical_sv );
  }

  //描画
  Br_MusicalSend_CreatePhoto( p_wk, p_param );
  Br_MusicalSend_CreateSubDisplay( p_wk, p_param );

  //シーケンス管理
  p_wk->p_seq = BR_SEQ_Init( p_wk, Br_MusicalSend_Seq_FadeIn, p_wk->heapID );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ミュージカル送信プロセス	破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_MUSICALSEND_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_MUSICALSEND_WORK				*p_wk	= p_wk_adrs;
	BR_MUSICALSEND_PROC_PARAM	*p_param	= p_param_adrs;

  //シーケンス管理破棄
  BR_SEQ_Exit( p_wk->p_seq );
  

  //描画
  Br_MusicalSend_DeleteSubDisplay( p_wk, p_param );
  Br_MusicalSend_DeletePhoto( p_wk, p_param );
  
	//モジュール破棄
  if( p_wk->p_text )
  { 
    BR_TEXT_Exit( p_wk->p_text, p_param->p_res );
  }
  BmpOam_Exit( p_wk->p_bmpoam );
  PRINTSYS_QUE_Delete( p_wk->p_que );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(musical_shot) );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ミュージカル送信プロセス	メイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_MUSICALSEND_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_MUSICALSEND_WORK	*p_wk	= p_wk_adrs;
	BR_MUSICALSEND_PROC_PARAM	*p_param	= p_param_adrs;

  BR_SEQ_Main( p_wk->p_seq );
  if( BR_SEQ_IsEnd( p_wk->p_seq ) )
  { 
    return GFL_PROC_RES_FINISH;
  }

  //各プリント処理
  PRINTSYS_QUE_Main( p_wk->p_que );

  if( p_wk->p_photo )
  { 
    MUS_SHOT_PHOTO_UpdateSystem( p_wk->p_photo );
  }

	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *    各サブプロセス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_MusicalSend_Seq_FadeIn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_FADEIN_END,
  };

  BR_MUSICALSEND_WORK	*p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_MASTERBRIGHT_AND_ALPHA, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    *p_seq  = SEQ_FADEIN_WAIT;
    break;
  case SEQ_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_FADEIN_END;
    }
    break;
  case SEQ_FADEIN_END:
    BR_SEQ_SetNext( p_seqwk, Br_MusicalSend_Seq_Main );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_MusicalSend_Seq_FadeOut( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_FADEOUT_END,
  };

  BR_MUSICALSEND_WORK	*p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_FADEOUT_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_MASTERBRIGHT_AND_ALPHA, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;
  case SEQ_FADEOUT_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_FADEOUT_END;
    }
    break;
  case SEQ_FADEOUT_END:
    BR_SEQ_End( p_wk->p_seq );
    BR_PROC_SYS_Pop( p_wk->p_param->p_procsys );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_MusicalSend_Seq_Main( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BR_MUSICALSEND_WORK	*p_wk = p_wk_adrs;

  u32 x, y;
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  {
    if( BR_BTN_GetTrg( p_wk->p_btn[ BR_MUSICALSEND_BTN_RETURN ], x, y ) )
    { 
      BR_SEQ_SetNext( p_seqwk, Br_MusicalSend_Seq_FadeOut );
    }
    if( BR_BTN_GetTrg( p_wk->p_btn[ BR_MUSICALSEND_BTN_SEND ], x, y ) )
    { 
      //BR_SEQ_SetNext( p_seqwk, Br_MusicalSend_Seq_FadeOut );
    }
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  アップロード
 *
 *	@param	BR_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_MusicalSend_Seq_Upload( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_CHANGE_FADEOUT_START,
    SEQ_CHANGE_FADEOUT_MAIN,
    SEQ_CHANGE_MAIN,
    SEQ_CHANGE_FADEIN_START,
    SEQ_CHANGE_FADEIN_MAIN,

    SEQ_UPLOAD_START,
    SEQ_UPLOAD_WAIT,
    SEQ_UPLOAD_END,

    SEQ_FADEOUT_START,
    SEQ_FADEOUT_MAIN,

    SEQ_END,
  };

  BR_MUSICALSEND_WORK	*p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_CHANGE_FADEOUT_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_MASTERBRIGHT_AND_ALPHA, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_CHANGE_FADEOUT_MAIN;
    break;
  case SEQ_CHANGE_FADEOUT_MAIN:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    {
      *p_seq  = SEQ_CHANGE_MAIN;
    }
    break;
  case SEQ_CHANGE_MAIN:
    Br_MusicalSend_DeletePhoto( p_wk, p_wk->p_param );
    p_wk->p_text  = BR_TEXT_Init( p_wk->p_param->p_res, p_wk->p_que, p_wk->heapID );
    BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_017 );
    *p_seq  = SEQ_CHANGE_FADEIN_START;
    break;
  case SEQ_CHANGE_FADEIN_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_MASTERBRIGHT_AND_ALPHA, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    *p_seq  = SEQ_CHANGE_FADEIN_MAIN;
    break;
  case SEQ_CHANGE_FADEIN_MAIN:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    {
      *p_seq  = SEQ_UPLOAD_START;
    }
    break;

    //-------------------------------------
    ///	アップロード開始
    //=====================================
  case SEQ_UPLOAD_START:
    {
      BR_NET_REQUEST_PARAM  req_param;
      GFL_STD_MemClear( &req_param, sizeof(BR_NET_REQUEST_PARAM) );
      req_param.cp_upload_musical_shot_data = p_wk->p_musical_shot;
      BR_NET_StartRequest( p_wk->p_param->p_net, BR_NET_REQUEST_MUSICAL_SHOT_UPLOAD, &req_param );
    }
    *p_seq  = SEQ_UPLOAD_WAIT;
    break;

  case SEQ_UPLOAD_WAIT:
    if( BR_NET_WaitRequest( p_wk->p_param->p_net ) )
    { 
      *p_seq  = SEQ_UPLOAD_END;
    }
    break;

  case SEQ_UPLOAD_END:
    *p_seq  = SEQ_FADEOUT_START;
    break;


  case SEQ_FADEOUT_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    *p_seq  = SEQ_FADEOUT_MAIN;
    break;

  case SEQ_FADEOUT_MAIN:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_END;
    }
    break;


  case SEQ_END:
    BR_SEQ_End( p_seqwk );
    BR_PROC_SYS_Pop( p_wk->p_param->p_procsys );
    break;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  写真  初期化
 *
 *	@param	BR_MUSICALSEND_WORK	*p_wk ワーク
 *	@param	*p_param                  引数
 */
//-----------------------------------------------------------------------------
static void Br_MusicalSend_CreatePhoto( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
{ 
  if( p_wk->p_photo == NULL )
  { 
    // ----上画面設定

    //上画面読み直しのため破棄
    BR_RES_UnLoadBG( p_param->p_res, BR_RES_BG_START_M );
    BR_RES_UnLoadCommon( p_param->p_res, CLSYS_DRAW_MAIN );
    //上画面読み直し
    BR_GRAPHIC_SetMusicalMode( p_param->p_graphic, p_wk->heapID );

    //ミュージカルショット作成
    p_wk->p_photo = MUS_SHOT_PHOTO_InitSystem( p_wk->p_musical_shot, p_wk->heapID );
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  写真  破棄
 *
 *	@param	BR_MUSICALSEND_WORK	*p_wk ワーク
 *	@param	*p_param                  引数
 */
//-----------------------------------------------------------------------------
static void Br_MusicalSend_DeletePhoto( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
{ 
  if( p_wk->p_photo )
  { 
    //上画面--------------
    //モジュール破棄
    MUS_SHOT_PHOTO_ExitSystem( p_wk->p_photo );
    p_wk->p_photo = NULL;

    //グラフィック読み替え
    BR_GRAPHIC_ReSetMusicalMode( p_param->p_graphic, p_wk->heapID );
    //読み込みなおし
    BR_RES_LoadCommon( p_param->p_res, CLSYS_DRAW_MAIN, p_wk->heapID );
    BR_RES_LoadBG( p_param->p_res, BR_RES_BG_START_M, p_wk->heapID );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  下画面作成
 *
 *	@param	BR_MUSICALSEND_WORK *p_wk ワーク
 *	@param	*p_param                  引数
 */
//-----------------------------------------------------------------------------
static void Br_MusicalSend_CreateSubDisplay( BR_MUSICALSEND_WORK *p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
{ 
 // ----下画面設定
  BR_RES_LoadBG( p_param->p_res, BR_RES_BG_PHOTO_SEND_S, p_wk->heapID );
  BR_RES_LoadOBJ( p_param->p_res, BR_RES_OBJ_SHORT_BTN_S, p_wk->heapID );

  { 
    int i;
    GFL_CLUNIT  *p_unit;
    GFL_CLWK_DATA cldata;
    BR_RES_OBJ_DATA res;
    BOOL ret;
    u32 msgID;

    GFL_FONT    *p_font;
    GFL_MSGDATA *p_msg;

    p_font  = BR_RES_GetFont( p_param->p_res );
    p_msg   = BR_RES_GetMsgData( p_param->p_res );

    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    cldata.pos_y    = 168;
    cldata.softpri  = 1;

    p_unit  = BR_GRAPHIC_GetClunit( p_param->p_graphic );
    ret = BR_RES_GetOBJRes( p_param->p_res, BR_RES_OBJ_SHORT_BTN_S, &res );
    GF_ASSERT( ret );

    for( i = 0; i < BR_MUSICALSEND_BTN_MAX; i++ )
    { 
      if( i == 0 )
      { 
        msgID = msg_05;
        cldata.pos_x    = 32;
        cldata.anmseq   = 0;
      }
      else
      { 
        msgID = msg_302;
        cldata.pos_x    = 128;
        cldata.anmseq   = 0;
      }
      p_wk->p_btn[i] = BR_BTN_Init( &cldata, msgID, BR_BTN_DATA_SHORT_WIDTH, CLSYS_DRAW_SUB, p_unit, p_wk->p_bmpoam, p_font, p_msg, &res, p_wk->heapID );
    }
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  下画面破棄
 *
 *	@param	BR_MUSICALSEND_WORK *p_wk ワーク
 *	@param	*p_param                  引数
 */
//-----------------------------------------------------------------------------
static void Br_MusicalSend_DeleteSubDisplay( BR_MUSICALSEND_WORK *p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
{ 
  //下画面-------------
  BR_RES_UnLoadOBJ( p_param->p_res, BR_RES_OBJ_SHORT_BTN_S );
  BR_RES_UnLoadBG( p_param->p_res, BR_RES_BG_PHOTO_SEND_S );

  { 
    int i;
    for( i = 0; i < BR_MUSICALSEND_BTN_MAX; i++ )
    { 
      BR_BTN_Exit( p_wk->p_btn[i] );
    }
  }
}
