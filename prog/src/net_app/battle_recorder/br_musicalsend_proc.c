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
FS_EXTERN_OVERLAY( musical );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define BR_MUSICALSEND_SUBSEQ_END (0xFFFFFFFF)

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

  //common
  BMPOAM_SYS_PTR	  	p_bmpoam;	//BMPOAMシステム
  PRINT_QUE           *p_que;
  u32                 sub_seq;
  u32                 next_sub_seq;
	HEAPID heapID;
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
typedef BOOL (*SUBSEQ_FUNCTION)( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );
//ミュージカル
static BOOL Br_MusicalSend_Seq_Photo_Init( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );
static BOOL Br_MusicalSend_Seq_Photo_Main( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );
static BOOL Br_MusicalSend_Seq_Photo_Exit( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );
//送信中
static BOOL Br_MusicalSend_Seq_Send_Init( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );
static BOOL Br_MusicalSend_Seq_Send_Main( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );
static BOOL Br_MusicalSend_Seq_Send_Exit( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param );

//-------------------------------------
///	その他
//=====================================

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
 *					データ
 */
//=============================================================================
//-------------------------------------
///	流れ
//=====================================
enum
{ 
  SUBSEQ_INIT,
  SUBSEQ_MAIN,
  SUBSEQ_EXIT,
  SUBSEQ_MAX,
};
enum
{ 
  SUBSEQ_PHOTO,
  SUBSEQ_SEND,
};
static const SUBSEQ_FUNCTION sc_subseq_tbl[][SUBSEQ_MAX] =
{ 
  { 
    Br_MusicalSend_Seq_Photo_Init,
    Br_MusicalSend_Seq_Photo_Main,
    Br_MusicalSend_Seq_Photo_Exit,
  },
  { 
    Br_MusicalSend_Seq_Send_Init,
    Br_MusicalSend_Seq_Send_Main,
    Br_MusicalSend_Seq_Send_Exit,
  },

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

  GFL_OVERLAY_Load( FS_OVERLAY_ID(musical) );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_MUSICALSEND_WORK), BR_PROC_SYS_GetHeapID( p_param->p_procsys ) );
	GFL_STD_MemClear( p_wk, sizeof(BR_MUSICALSEND_WORK) );	
	p_wk->heapID	= BR_PROC_SYS_GetHeapID( p_param->p_procsys );

  //モジュール作成
  p_wk->p_que   = PRINTSYS_QUE_Create( p_wk->heapID );
  { 
    GFL_CLUNIT *p_unit;
    p_unit  = BR_GRAPHIC_GetClunit( p_param->p_graphic );
    p_wk->p_bmpoam	= BmpOam_Init( p_wk->heapID, p_unit );
  }

  //最初の初期化
  sc_subseq_tbl[ p_wk->sub_seq ][SUBSEQ_INIT]( p_wk, p_param );

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

  sc_subseq_tbl[ p_wk->sub_seq ][SUBSEQ_EXIT]( p_wk, p_param );
  
	//モジュール破棄
  BmpOam_Exit( p_wk->p_bmpoam );
  PRINTSYS_QUE_Delete( p_wk->p_que );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(musical) );

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

	BR_MUSICALSEND_WORK	*p_wk	= p_wk_adrs;
	BR_MUSICALSEND_PROC_PARAM	*p_param	= p_param_adrs;

 switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
    (*p_seq)++;
    break;
  case SEQ_FADEIN_WAIT:
    *p_seq  = SEQ_MAIN;
    break;
  case SEQ_MAIN:
    {
      if( sc_subseq_tbl[ p_wk->sub_seq ][SUBSEQ_MAIN]( p_wk, p_param ) )
      { 
        if( p_wk->next_sub_seq == BR_MUSICALSEND_SUBSEQ_END )
        { 
          *p_seq  = SEQ_FADEOUT_START;
        }
        else
        { 
          *p_seq  = SEQ_CHANGEOUT_START;
        }
      }
    }
    break;

  case SEQ_CHANGEOUT_START:
    (*p_seq)++;
    break;
  case SEQ_CHANGEOUT_WAIT:
    (*p_seq)++;
    break;
  case SEQ_CHANGEIN_START:
    { 
      sc_subseq_tbl[ p_wk->sub_seq ][SUBSEQ_EXIT]( p_wk, p_param );
      p_wk->sub_seq = p_wk->next_sub_seq;
      sc_subseq_tbl[ p_wk->sub_seq ][SUBSEQ_INIT]( p_wk, p_param );
    }
    (*p_seq)++;
    break;
  case SEQ_CHANGEIN_WAIT:
    *p_seq  = SEQ_MAIN;
    break;

  case SEQ_FADEOUT_START:
    (*p_seq)++;
    break;
  case SEQ_FADEOUT_WAIT:
    *p_seq  = SEQ_EXIT;
    break;
  case SEQ_EXIT:
    NAGI_Printf( "MUSICAL_SEND: Exit!\n" );
    BR_PROC_SYS_Pop( p_param->p_procsys );
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
 *	@brief  写真  初期化
 *
 *	@param	BR_MUSICALSEND_WORK	*p_wk ワーク
 *	@param	*p_param                  引数
 *
 *	@return TRUEならば次へ  FALSEならばループ （MAINのみ  他は必ず次へ）
 */
//-----------------------------------------------------------------------------
static BOOL Br_MusicalSend_Seq_Photo_Init( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
{ 
  GFL_FONT    *p_font;
  GFL_MSGDATA *p_msg;

  p_font  = BR_RES_GetFont( p_param->p_res );
  p_msg   = BR_RES_GetMsgData( p_param->p_res );

  // ----上画面設定

  //上画面読み直しのため破棄
  BR_RES_UnLoadBG( p_param->p_res, BR_RES_BG_START_M );
  BR_RES_UnLoadCommon( p_param->p_res, CLSYS_DRAW_MAIN );
  //上画面読み直し
  BR_GRAPHIC_SetMusicalMode( p_param->p_graphic, p_wk->heapID );

  //ミュージカルショット作成
  { 
    MUSICAL_SHOT_DATA data;
    DEBUG_Br_MusicalLook_GetPhotData( &data );
    p_wk->p_photo = MUS_SHOT_PHOTO_InitSystem( &data, p_wk->heapID );
  }

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
      p_wk->p_btn[i] = BR_BTN_Init( &cldata, msgID, CLSYS_DRAW_SUB, p_unit, p_wk->p_bmpoam, p_font, p_msg, &res, p_wk->heapID );
    }
  }


  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  写真  破棄
 *
 *	@param	BR_MUSICALSEND_WORK	*p_wk ワーク
 *	@param	*p_param                  引数
 *
 *	@return TRUEならば次へ  FALSEならばループ （MAINのみ  他は必ず次へ）
 */
//-----------------------------------------------------------------------------
static BOOL Br_MusicalSend_Seq_Photo_Exit( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
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
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  写真  メイン処理
 *
 *	@param	BR_MUSICALSEND_WORK	*p_wk ワーク
 *	@param	*p_param                  引数
 *
 *	@return TRUEならば次へ  FALSEならばループ （MAINのみ  他は必ず次へ）
 */
//-----------------------------------------------------------------------------
static BOOL Br_MusicalSend_Seq_Photo_Main( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
{ 
  u32 x, y;
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  {
    if( BR_BTN_GetTrg( p_wk->p_btn[ BR_MUSICALSEND_BTN_RETURN ], x, y ) )
    { 
      p_wk->next_sub_seq  = BR_MUSICALSEND_SUBSEQ_END;
      return TRUE;
    }
    if( BR_BTN_GetTrg( p_wk->p_btn[ BR_MUSICALSEND_BTN_SEND ], x, y ) )
    { 
      p_wk->next_sub_seq  = BR_MUSICALSEND_SUBSEQ_END;
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  送信  初期化
 *
 *	@param	BR_MUSICALSEND_WORK	*p_wk ワーク
 *	@param	*p_param                  引数
 *
 *	@return TRUEならば次へ  FALSEならばループ （MAINのみ  他は必ず次へ）
 */
//-----------------------------------------------------------------------------
static BOOL Br_MusicalSend_Seq_Send_Init( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
{ 

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  送信  破棄
 *
 *	@param	BR_MUSICALSEND_WORK	*p_wk ワーク
 *	@param	*p_param                  引数
 *
 *	@return TRUEならば次へ  FALSEならばループ （MAINのみ  他は必ず次へ）
 */
//-----------------------------------------------------------------------------

static BOOL Br_MusicalSend_Seq_Send_Exit( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
{ 

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  送信  メイン処理
 *
 *	@param	BR_MUSICALSEND_WORK	*p_wk ワーク
 *	@param	*p_param                  引数
 *
 *	@return TRUEならば次へ  FALSEならばループ （MAINのみ  他は必ず次へ）
 */
//-----------------------------------------------------------------------------
static BOOL Br_MusicalSend_Seq_Send_Main( BR_MUSICALSEND_WORK	*p_wk, BR_MUSICALSEND_PROC_PARAM *p_param )
{ 
  return TRUE;
}

//=============================================================================
/**
 *        その他
 */
//=============================================================================

