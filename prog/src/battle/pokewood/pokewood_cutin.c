//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		pokewood_cutin.c
 *	@brief  カットインモジュール
 *	@author	Toru=Nagihashi
 *	@data		2011.05.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>

//sys
#include "pokewood_cutin_script_search.cdat"

//arc
#include "arc/arc_def.h"

//out
#include "pokewood_cutin.h"


#ifdef PM_DEBUG

#define PW_CUTIN_PRINT

#endif


#ifdef PW_CUTIN_PRINT
#if defined(DEBUG_ONLY_FOR_nagihashi_toru)
#define PW_CUTIN_Printf(...)  OS_TFPrintf(0,__VA_ARGS__)
#endif
#endif

#ifndef PW_CUTIN_Printf
#define PW_CUTIN_Printf(...)  /*  */
#endif

//=============================================================================
/**
 *					カットイン動作モジュール
*/
//=============================================================================
#define POKEWOOD_CUTIN_CLWK_NUM (4)
#define POKEWOOD_CUTIN_CMD_PARAM_NUM (8)

#define POKEWOOD_CUTIN_EVENT_NONE (0xFFFF)

#define POKEWOOD_WND_END_POS_X    (128)
#define POKEWOOD_WND_END_POS_Y    (96)

//-------------------------------------
//	等速直線運動動作
//=====================================
typedef struct {
	GFL_POINT now_val;		//現在の値
	GFL_POINT start_val;	//開始の値
	GFL_POINT end_val;		//終了の値
	GFL_POINT add_val;		//加算値(誤差をださないようにここだけfx)
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_VELOCITY_WORK;

//-------------------------------------
///	引数
//=====================================
typedef struct
{
  u16 param[POKEWOOD_CUTIN_CMD_PARAM_NUM];
} PW_SCRIPT_CMD_PARAM;

//スクリプトコマンド型
typedef BOOL (*PW_SCRIPT_COMMAND)( POKEWOOD_CUTIN *p_wk, u16 *p_seq, const PW_SCRIPT_CMD_PARAM *cp_param );

//-------------------------------------
///	ワーク
//=====================================
struct _POKEWOOD_CUTIN
{
  const POKEWOOD_SENARIO * cp_senario;
  GFL_CLUNIT *p_clunit;
  GFL_CLSYS_REND *p_rend;
  PW_CUTIN_BINARY *p_binary;
  u16 seq;
  u16 local_seq;
  BOOL is_end;
  u16 event;
  u16 script_seq;
  PW_SCRIPT_COMMAND cmd;
  HEAPID heapID;
  GFL_CLWK *p_clwk[POKEWOOD_CUTIN_CLWK_NUM];
  BTLV_STRPARAM str_param;
  BTLV_CORE* p_core;
  PW_SCRIPT_CMD_PARAM cmd_param;
  u16 plt;
  u16 chr;
  u16 cel;
  u16 is_res_load;

  void *p_cell_buff;
  NNSG2dCellDataBank *p_cell;

  u32 cnt;

  union
  {
    PROGVAL_VELOCITY_WORK   vel;
  };
};

//-------------------------------------
///	プロトタイプ
//=====================================
static void Bw_Graphic_Load( POKEWOOD_CUTIN *p_wk, ARCDATID plt, ARCDATID chr, ARCDATID cel, ARCDATID nan );
static void Bw_Graphic_UnLoad( POKEWOOD_CUTIN *p_wk );
static ARCDATID Bw_Binary_GetArcDatId( int senario, int event );

//スクリプトコマンド
static BOOL Bw_Scrcmd_WndMove( POKEWOOD_CUTIN *p_wk, u16 *p_seq, const PW_SCRIPT_CMD_PARAM *cp_param );
static BOOL Bw_Scrcmd_WndMoveEx( POKEWOOD_CUTIN *p_wk, u16 *p_seq, const PW_SCRIPT_CMD_PARAM *cp_param );
static BOOL Bw_Scrcmd_WndScale( POKEWOOD_CUTIN *p_wk, u16 *p_seq, const PW_SCRIPT_CMD_PARAM *cp_param );
static BOOL Bw_Scrcmd_WndVisible( POKEWOOD_CUTIN *p_wk, u16 *p_seq,const PW_SCRIPT_CMD_PARAM *cp_param );
static BOOL Bw_Scrcmd_ResChange( POKEWOOD_CUTIN *p_wk, u16 *p_seq,const PW_SCRIPT_CMD_PARAM *cp_param );
static BOOL Bw_Scrcmd_Wait( POKEWOOD_CUTIN *p_wk, u16 *p_seq,const PW_SCRIPT_CMD_PARAM *cp_param );
static BOOL Bw_Scrcmd_Message( POKEWOOD_CUTIN *p_wk, u16 *p_seq,const PW_SCRIPT_CMD_PARAM *cp_param );

//移動命令
static void Bw_Move_Velocity_Init( POKEWOOD_CUTIN *p_wk, const GFL_CLWK *cp_clwk, const PW_SCRIPT_CMD_PARAM *cp_param );
static void Bw_Move_Velocity_InitEx( POKEWOOD_CUTIN *p_wk, const GFL_CLWK *cp_clwk, const PW_SCRIPT_CMD_PARAM *cp_param );
static BOOL Bw_Move_Velocity_Main( POKEWOOD_CUTIN *p_wk );
static void Bw_Move_Velocity_GetPos( POKEWOOD_CUTIN *p_wk, GFL_CLACTPOS *p_clpos );

static void Bw_Move_GetMovePos( int move_dir, const GFL_CLWK *cp_clwk, GFL_POINT *p_start, GFL_POINT *p_end );
static void Bw_Move_GetMovePosEx( int move_dir, int dst, const GFL_CLWK *cp_clwk, GFL_POINT *p_start, GFL_POINT *p_end );
static void PROGVAL_VEL_Init( PROGVAL_VELOCITY_WORK* p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, int sync );
static BOOL PROGVAL_VEL_Main( PROGVAL_VELOCITY_WORK* p_wk );
static void PROGVAL_VEL_GetPos( const PROGVAL_VELOCITY_WORK* cp_wk, GFL_CLACTPOS *p_pos );

//-------------------------------------
///	データ
//=====================================
//スクリプトコマンド
static const PW_SCRIPT_COMMAND Pw_Script_Command[] =
{
  Bw_Scrcmd_WndMove,      //PW_CUTIN_CMD_WND_MOVE 
  Bw_Scrcmd_WndMoveEx,    //PW_CUTIN_CMD_WND_MOVE_EX
  Bw_Scrcmd_WndScale,     //PW_CUTIN_CMD_WND_SCALE
  Bw_Scrcmd_WndVisible,   //PW_CUTIN_CMD_WND_VISIBLE
  Bw_Scrcmd_ResChange,    //PW_CUTIN_CMD_RES_CHANGE
  Bw_Scrcmd_Wait,         //PW_CUTIN_CMD_WAIT
  Bw_Scrcmd_Message,      //PW_CUTIN_CMD_MESSAGE
};

//移動コマンド
static const struct
{
  void (*Init)( POKEWOOD_CUTIN *p_wk, const GFL_CLWK *cp_clwk, const PW_SCRIPT_CMD_PARAM *cp_param );
  void (*InitEx)( POKEWOOD_CUTIN *p_wk, const GFL_CLWK *cp_clwk, const PW_SCRIPT_CMD_PARAM *cp_param );
  BOOL (*Main)( POKEWOOD_CUTIN *p_wk );
  void (*GetPos)( POKEWOOD_CUTIN *p_wk, GFL_CLACTPOS *p_clpos );
} Pw_Move_Command[] =
{
  //PW_CUTIN_DEF_MOVE_TYPE_VELOCITY
  {
    Bw_Move_Velocity_Init,
    Bw_Move_Velocity_InitEx,
    Bw_Move_Velocity_Main,
    Bw_Move_Velocity_GetPos,
  },
};

//-------------------------------------
///	外部公開関数
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  カットインモジュール作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
POKEWOOD_CUTIN * POKEWOOD_CUTIN_Init( HEAPID heapID )
{
  POKEWOOD_CUTIN *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(POKEWOOD_CUTIN));
  p_wk->p_clunit = GFL_CLACT_UNIT_Create( POKEWOOD_CUTIN_CLWK_NUM, 0 , heapID );

  {
    static const GFL_REND_SURFACE_INIT sc_surface =
    {
      0, 0,
      256, 192,
      CLSYS_DRAW_MAIN,
      CLSYS_REND_CULLING_TYPE_NORMAL,
    };
    p_wk->p_rend = GFL_CLACT_USERREND_Create( &sc_surface, 1, heapID );
  }

  GFL_CLACT_USERREND_SetOAMAttrCulling( p_wk->p_rend, TRUE );
  GFL_CLACT_UNIT_SetUserRend( p_wk->p_clunit, p_wk->p_rend );

  p_wk->heapID = heapID;
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  カットインモジュール破棄
 *
 *	@param	POKEWOOD_CUTIN *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void POKEWOOD_CUTIN_Exit( POKEWOOD_CUTIN *p_wk )
{
  if( p_wk->p_binary )
  {
    PW_CUTIN_BINARY_Exit( p_wk->p_binary );
    p_wk->p_binary = NULL;
  }

  GFL_CLACT_USERREND_Delete( p_wk->p_rend );
  GFL_CLACT_UNIT_Delete( p_wk->p_clunit );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  カットインモジュールメイン
 *
 *	@param	POKEWOOD_CUTIN *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void POKEWOOD_CUTIN_Main( POKEWOOD_CUTIN *p_wk )
{
  enum
  {
    SEQ_WAIT,
    SEQ_INIT,
    SEQ_START_SCRIPT,
    SEQ_MOVE_SCRIPT,
    SEQ_END_SCRIPT,
    SEQ_EXIT,
  };

  if( p_wk->is_end )
  {
    return;
  }

  switch( p_wk->seq )
  {
  case SEQ_WAIT:
    /* なにもしない */
    break;
  case SEQ_INIT:
    {
      ARCDATID datID = Bw_Binary_GetArcDatId( p_wk->cp_senario->senario_no, p_wk->event );

      GF_ASSERT_MSG( datID != POKEWOOD_CUTIN_EVENT_NONE, "シナリオ%dイベント%dのカットインは見つかりませんでした\n",p_wk->cp_senario->senario_no, p_wk->event );
      p_wk->p_binary = PW_CUTIN_BINARY_Init( datID, p_wk->heapID );

      {
        ARCDATID plt  = PW_CUTIN_BINARY_GetHeaderData( p_wk->p_binary, PW_CUTIN_BINARY_HEADER_PARAM_NCL );
        ARCDATID chr  = PW_CUTIN_BINARY_GetHeaderData( p_wk->p_binary, PW_CUTIN_BINARY_HEADER_PARAM_NCG );
        ARCDATID cel  = PW_CUTIN_BINARY_GetHeaderData( p_wk->p_binary, PW_CUTIN_BINARY_HEADER_PARAM_NCE );
        ARCDATID nan  = PW_CUTIN_BINARY_GetHeaderData( p_wk->p_binary, PW_CUTIN_BINARY_HEADER_PARAM_NAN );
        Bw_Graphic_Load( p_wk, plt, chr, cel, nan );
      }

      p_wk->seq++;
    }
    break;
  case SEQ_START_SCRIPT:
    {
      int cmd_no;
      cmd_no = PW_CUTIN_BINARY_GetScriptData( p_wk->p_binary, p_wk->script_seq, PW_CUTIN_BINARY_SCR_PARAM_CMD );
      p_wk->cmd = Pw_Script_Command[cmd_no];
      PW_CUTIN_Printf( "cutin cmd[%d] ", cmd_no );
    }
    p_wk->local_seq = 0;
    {
      int i;
      for( i = 0; i < PW_CUTIN_BINARY_SCR_PARAM_NUM; i++ )
      {
        p_wk->cmd_param.param[i] = PW_CUTIN_BINARY_GetScriptData( p_wk->p_binary, p_wk->script_seq, PW_CUTIN_BINARY_SCR_PARAM_PARAM_START+i );
        PW_CUTIN_Printf( "%d[%d] ", i, p_wk->cmd_param.param[i] );
      }
    }
    PW_CUTIN_Printf( "\n" );
    p_wk->seq++;
    break;
  case SEQ_MOVE_SCRIPT:
    if( p_wk->cmd( p_wk, &p_wk->local_seq, &p_wk->cmd_param ) )
    {
      p_wk->seq++;
    }
    break;
  case SEQ_END_SCRIPT:
    p_wk->script_seq++;
    if( p_wk->script_seq < PW_CUTIN_BINARY_GetScriotMax(p_wk->p_binary) )
    {
      p_wk->seq = SEQ_START_SCRIPT;
    }
    else
    {
      p_wk->seq++;
    }
    break;
  case SEQ_EXIT:
    Bw_Graphic_UnLoad( p_wk );
    PW_CUTIN_BINARY_Exit( p_wk->p_binary );
    p_wk->p_binary = NULL;
    p_wk->is_end = TRUE;
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  カットインモジュール
 *
 *	@param	const POKEWOOD_CUTIN *cp_wk ワーク
 *
 *	@return TRUE FALSE
 */
//-----------------------------------------------------------------------------
BOOL POKEWOOD_CUTIN_IsEnd( const POKEWOOD_CUTIN *cp_wk )
{
  return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief  カットインモジュール　開始
 *
 *	@param	POKEWOOD_CUTIN *p_wk ワーク
 *	@param  senario             シナリオデータ
 *	@param  event               イベントナンバー
 */
//-----------------------------------------------------------------------------
void POKEWOOD_CUTIN_Start( POKEWOOD_CUTIN *p_wk, const POKEWOOD_SENARIO * cp_senario, u16 event )
{
  GF_ASSERT( p_wk->seq == 0 );

  p_wk->seq++;
  p_wk->cp_senario  = cp_senario;
  p_wk->event       = event;
  p_wk->script_seq  = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief 描画モジュールをアタッチ（CUTIN_Init後に作られるため）
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	p_core  描画モジュール
 */
//-----------------------------------------------------------------------------
void POKEWOOD_CUTIN_AttachViewCore( POKEWOOD_CUTIN *p_wk, BTLV_CORE* p_core )
{
  p_wk->p_core = p_core;
}
//----------------------------------------------------------------------------
/**
 *	@brief  現在のシナリオ＋イベントがカットインがあるか
 *
 *	@param	const POKEWOOD_SENARIO * cp_senario シナリオ
 *	@param	event イベント
 *
 *	@return TRUEで存在  FALSEで存在していない
 */
//-----------------------------------------------------------------------------
BOOL POKEWOOD_CUTIN_IsExits( const POKEWOOD_SENARIO * cp_senario, u16 event )
{
  return Bw_Binary_GetArcDatId( cp_senario->senario_no, event ) != POKEWOOD_CUTIN_EVENT_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief　グラフィック読み込み
 *
 *	@param	POKEWOOD_CUTIN *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Bw_Graphic_Load( POKEWOOD_CUTIN *p_wk, ARCDATID plt, ARCDATID chr, ARCDATID cel, ARCDATID nan )
{

  PW_CUTIN_Printf( "cutin GraLoad plt%d chr%d cel%d nan%d\n", plt, chr, cel, nan );
  GF_ASSERT( p_wk->is_res_load == FALSE );

  //リソース読み込み
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_POKEWOOD_CUTIN_GRA, GFL_HEAP_LOWID( p_wk->heapID ) );

    p_wk->chr = GFL_CLGRP_CGR_Register( p_handle, chr, FALSE, CLSYS_DRAW_MAIN, p_wk->heapID );
    p_wk->plt = GFL_CLGRP_PLTT_Register( p_handle, plt, CLSYS_DRAW_MAIN, 0, p_wk->heapID );

    PaletteWorkSet_VramCopy( BTLV_EFFECT_GetPfd(), FADE_MAIN_OBJ, 0, 32*14 );

    p_wk->cel = GFL_CLGRP_CELLANIM_Register( p_handle, cel, nan, p_wk->heapID );

    p_wk->p_cell_buff = GFL_ARCHDL_UTIL_LoadCellBank( p_handle, cel, FALSE, &p_wk->p_cell, p_wk->heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  //リソース作成
  {
    int i;
    GFL_CLWK_DATA clwk_data = {0,0,0,0,1};
    for( i = 0; i < POKEWOOD_CUTIN_CLWK_NUM; i++ )
    {
      p_wk->p_clwk[i] = GFL_CLACT_WK_Create( p_wk->p_clunit, p_wk->chr, p_wk->plt, p_wk->cel,  &clwk_data, CLSYS_DRAW_MAIN, p_wk->heapID );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
    }
  }

  p_wk->is_res_load = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief　グラフィック破棄
 *
 *	@param	POKEWOOD_CUTIN *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Bw_Graphic_UnLoad( POKEWOOD_CUTIN *p_wk )
{
  if( p_wk->is_res_load == FALSE )
  {
    return;
  }

  //CLWK破棄
  {
    int i;
    for( i = 0; i < POKEWOOD_CUTIN_CLWK_NUM; i++ )
    {
      GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
    }
  }

  //リソース破棄
  {
    GFL_HEAP_FreeMemory( p_wk->p_cell_buff );

    GFL_CLGRP_CELLANIM_Release( p_wk->cel );
    GFL_CLGRP_CGR_Release( p_wk->chr );
    GFL_CLGRP_PLTT_Release( p_wk->plt );
  }

  p_wk->is_res_load = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  バイナリアーカイブID
 *
 *	@param	int senario シナリオ
 *	@param	event       イベント
 *
 *	@return アーカイブデータID
 */
//-----------------------------------------------------------------------------
static ARCDATID Bw_Binary_GetArcDatId( int senario, int event )
{
  int i;
  for( i = 0; i < NELEMS(sc_pw_cutin_dat_search); i++ )
  {
    if( sc_pw_cutin_dat_search[i].senario == senario
        && sc_pw_cutin_dat_search[i].event == event )
    {
      return i;
    }
  }

  return POKEWOOD_CUTIN_EVENT_NONE;
}
//=============================================================================
/**
 *    スクリプトコマンド
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief　コマ移動
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_seq                シーケンス
 *	@param	PW_SCRIPT_CMD_PARAM *cp_param   引数
 *
 *	@return TRUE コマンド終了 FALSE　コマンド継続中
 */
//-----------------------------------------------------------------------------
static BOOL Bw_Scrcmd_WndMove( POKEWOOD_CUTIN *p_wk, u16 *p_seq, const PW_SCRIPT_CMD_PARAM *cp_param )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };
  GFL_CLACTPOS clpos;

  GFL_CLWK *p_clwk = p_wk->p_clwk[cp_param->param[0]];

  switch( *p_seq )
  {
  case SEQ_INIT:  //位置設定
    Pw_Move_Command[ cp_param->param[1] ].Init( p_wk, p_clwk, cp_param );
    Pw_Move_Command[ cp_param->param[1] ].GetPos( p_wk, &clpos );
    GFL_CLACT_WK_SetPos( p_clwk, &clpos, 0 );
    GFL_CLACT_WK_SetAnmSeq( p_clwk, cp_param->param[0] );
    GFL_CLACT_WK_SetDrawEnable( p_clwk, TRUE );
    (*p_seq)++;
    break;

  case SEQ_MAIN:  //メイン
    if( Pw_Move_Command[ cp_param->param[1] ].Main( p_wk ) )
    {
      (*p_seq)++;
    }
    Pw_Move_Command[ cp_param->param[1] ].GetPos( p_wk, &clpos );
    GFL_CLACT_WK_SetPos( p_clwk, &clpos, 0 );
    break;

  case SEQ_EXIT:
    return TRUE;
  }

  return FALSE;

}
//----------------------------------------------------------------------------
/**
 *	@brief  コマ移動拡張版
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_seq                シーケンス
 *	@param	PW_SCRIPT_CMD_PARAM *cp_param   引数
 *
 *	@return TRUE コマンド終了 FALSE　コマンド継続中
 */
//-----------------------------------------------------------------------------
static BOOL Bw_Scrcmd_WndMoveEx( POKEWOOD_CUTIN *p_wk, u16 *p_seq, const PW_SCRIPT_CMD_PARAM *cp_param )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };
  GFL_CLACTPOS clpos;

  GFL_CLWK *p_clwk = p_wk->p_clwk[cp_param->param[0]];

  switch( *p_seq )
  {
  case SEQ_INIT:  //位置設定
    Pw_Move_Command[ cp_param->param[1] ].InitEx( p_wk, p_clwk, cp_param );
    Pw_Move_Command[ cp_param->param[1] ].GetPos( p_wk, &clpos );
    GFL_CLACT_WK_SetPos( p_clwk, &clpos, 0 );
    GFL_CLACT_WK_SetAnmSeq( p_clwk, cp_param->param[0] );
    GFL_CLACT_WK_SetDrawEnable( p_clwk, TRUE );
    (*p_seq)++;
    break;

  case SEQ_MAIN:  //メイン
    if( Pw_Move_Command[ cp_param->param[1] ].Main( p_wk ) )
    {
      (*p_seq)++;
    }
    Pw_Move_Command[ cp_param->param[1] ].GetPos( p_wk, &clpos );
    GFL_CLACT_WK_SetPos( p_clwk, &clpos, 0 );
    break;

  case SEQ_EXIT:
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  コマ拡大縮小
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_seq                シーケンス
 *	@param	PW_SCRIPT_CMD_PARAM *cp_param   引数
 *
 *	@return TRUE コマンド終了 FALSE　コマンド継続中
 */
//-----------------------------------------------------------------------------
static BOOL Bw_Scrcmd_WndScale( POKEWOOD_CUTIN *p_wk, u16 *p_seq, const PW_SCRIPT_CMD_PARAM *cp_param )
{
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief　コマ表示
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_seq                シーケンス
 *	@param	PW_SCRIPT_CMD_PARAM *cp_param   引数
 *
 *	@return TRUE コマンド終了 FALSE　コマンド継続中
 */
//-----------------------------------------------------------------------------
static BOOL Bw_Scrcmd_WndVisible( POKEWOOD_CUTIN *p_wk, u16 *p_seq,const PW_SCRIPT_CMD_PARAM *cp_param )
{
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[ cp_param->param[0] ], cp_param->param[1] );
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief　リソース切り替え
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_seq                シーケンス
 *	@param	PW_SCRIPT_CMD_PARAM *cp_param   引数
 *
 *	@return TRUE コマンド終了 FALSE　コマンド継続中
 */
//-----------------------------------------------------------------------------
static BOOL Bw_Scrcmd_ResChange( POKEWOOD_CUTIN *p_wk, u16 *p_seq,const PW_SCRIPT_CMD_PARAM *cp_param )
{
  enum
  {
    SEQ_UNLOAD,
    SEQ_LOAD,
  };

  switch( *p_seq )
  {
  case SEQ_UNLOAD:
    Bw_Graphic_UnLoad( p_wk );
    (*p_seq)++;
    break;
  case SEQ_LOAD:
    Bw_Graphic_Load( p_wk, cp_param->param[0], cp_param->param[1], cp_param->param[2], cp_param->param[3] );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief　待機
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_seq                シーケンス
 *	@param	PW_SCRIPT_CMD_PARAM *cp_param   引数
 *
 *	@return TRUE コマンド終了 FALSE　コマンド継続中
 */
//-----------------------------------------------------------------------------
static BOOL Bw_Scrcmd_Wait( POKEWOOD_CUTIN *p_wk, u16 *p_seq,const PW_SCRIPT_CMD_PARAM *cp_param )
{
  if( p_wk->cnt++ >= cp_param->param[0] )
  {
    p_wk->cnt = 0;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief　メッセージ
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_seq                シーケンス
 *	@param	PW_SCRIPT_CMD_PARAM *cp_param   引数
 *
 *	@return TRUE コマンド終了 FALSE　コマンド継続中
 */
//-----------------------------------------------------------------------------
static BOOL Bw_Scrcmd_Message( POKEWOOD_CUTIN *p_wk, u16 *p_seq,const PW_SCRIPT_CMD_PARAM *cp_param )
{
  enum
  {
    SEQ_START_MSG,
    SEQ_WAIT_MSG,
  };

  switch( *p_seq )
  {
  case SEQ_START_MSG:
    {
      const POKEWOOD_EVENT_DATA *cp_ev = &p_wk->cp_senario->event[ p_wk->event ];
      BTLV_STRPARAM_Setup( &p_wk->str_param, BTL_STRTYPE_POKEWOOD_TURNTALK, 
                         cp_ev->talk_msg[ cp_param->param[1] ][ cp_param->param[0] ] );
      BTLV_StartMsg( p_wk->p_core, &p_wk->str_param );
      (*p_seq)++;
    }
    break;
  case SEQ_WAIT_MSG:
    if( BTLV_WaitMsg( p_wk->p_core ) )
    {
      return TRUE;
    }
    break;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  直線動作メイン
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_clwk 動かすCLWK
 *	@param	PW_SCRIPT_CMD_PARAM *cp_param 引数
 */
//-----------------------------------------------------------------------------
static void Bw_Move_Velocity_Init( POKEWOOD_CUTIN *p_wk, const GFL_CLWK *cp_clwk, const PW_SCRIPT_CMD_PARAM *cp_param )
{
  GFL_POINT start;
  GFL_POINT end;

  Bw_Move_GetMovePos( cp_param->param[2], cp_clwk, &start, &end );
  PROGVAL_VEL_Init( &p_wk->vel, &start, &end, cp_param->param[3] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  直線動作メイン 拡張版
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_clwk 動かすCLWK
 *	@param	PW_SCRIPT_CMD_PARAM *cp_param 引数
 */
//-----------------------------------------------------------------------------
static void Bw_Move_Velocity_InitEx( POKEWOOD_CUTIN *p_wk, const GFL_CLWK *cp_clwk, const PW_SCRIPT_CMD_PARAM *cp_param )
{
  GFL_POINT start;
  GFL_POINT end;

  Bw_Move_GetMovePosEx( cp_param->param[2], cp_param->param[4], cp_clwk, &start, &end );
  PROGVAL_VEL_Init( &p_wk->vel, &start, &end, cp_param->param[3] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  直線動作　メイン
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Bw_Move_Velocity_Main( POKEWOOD_CUTIN *p_wk )
{
  return PROGVAL_VEL_Main( &p_wk->vel );
}
//----------------------------------------------------------------------------
/**
 *	@brief 座標取得
 *
 *	@param	POKEWOOD_CUTIN *p_wk  ワーク
 *	@param	*p_clpos  座標
 */
//-----------------------------------------------------------------------------
static void Bw_Move_Velocity_GetPos( POKEWOOD_CUTIN *p_wk, GFL_CLACTPOS *p_clpos )
{
  PROGVAL_VEL_GetPos( &p_wk->vel, p_clpos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動座標を取得
 *
 *	@param	int move_type 移動方向
 *  @param  p_clwk    CLWK
 *	@param	*p_start  開始座標
 *	@param	*p_end    終了座標
 */
//-----------------------------------------------------------------------------
static void Bw_Move_GetMovePos( int move_dir, const GFL_CLWK *cp_clwk, GFL_POINT *p_start, GFL_POINT *p_end )
{
  enum
  {
    MOVE_WIDTH = 192,
    MOVE_HEIGHT = 96,
  };

  GFL_CLACTPOS clpos;
  GFL_POINT target = {0};

  GFL_CLACT_WK_GetPos( cp_clwk, &clpos, 0 );


  switch( move_dir )
  {
    //移動方向：上から下
  case PW_CUTIN_DEF_MOVE_DIR_DOWN:
    target.y  = MOVE_HEIGHT;
    break;
    //移動方向：下から上
  case PW_CUTIN_DEF_MOVE_DIR_UP:
    target.y  = -MOVE_HEIGHT;
    break;
    //移動方向：右から左
  case PW_CUTIN_DEF_MOVE_DIR_LEFT:
    target.x  = -MOVE_WIDTH;
    break;
    //移動方向：左から右
  case PW_CUTIN_DEF_MOVE_DIR_RIGHT:
    target.x  = MOVE_WIDTH;
    break;
    //移動方向：右下から左上
  case PW_CUTIN_DEF_MOVE_DIR_LEFT_UP:
    target.x = -MOVE_WIDTH;
    target.y = -MOVE_HEIGHT;
    break;
    //移動方向：左下から右上
  case PW_CUTIN_DEF_MOVE_DIR_RIGHT_UP:
    target.x = MOVE_WIDTH;
    target.y = -MOVE_HEIGHT;
    break;
    //移動方向：右上から左下
  case PW_CUTIN_DEF_MOVE_DIR_LEFT_DOWN:
    target.x = -MOVE_WIDTH;
    target.y = MOVE_HEIGHT;
    break;
    //移動方向：左上から右下
  case PW_CUTIN_DEF_MOVE_DIR_RIGHT_DOWN:
    target.x = MOVE_WIDTH;
    target.y = MOVE_HEIGHT;
    break;
  }


  //もしCLWKが終了位置にいたら、開始が終了位置
  //いなかったら、
  if( clpos.x == POKEWOOD_WND_END_POS_X
    && clpos.y == POKEWOOD_WND_END_POS_Y )
  {
    p_start->x = POKEWOOD_WND_END_POS_X;
    p_start->y = POKEWOOD_WND_END_POS_Y;
    p_end->x   = p_start->x - target.x;
    p_end->y   = p_start->y - target.y;
  }
  else
  {
    p_end->x   = POKEWOOD_WND_END_POS_X;
    p_end->y   = POKEWOOD_WND_END_POS_Y;
    p_start->x = target.x - p_end->x;
    p_start->y = target.y - p_end->y;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動座標を取得 拡張版
 *
 *  @param  move_dir    移動方向
 *	@param	int dst    距離
 *  @param  p_clwk    CLWK
 *	@param	*p_start  開始座標
 *	@param	*p_end    終了座標
 */
//-----------------------------------------------------------------------------
static void Bw_Move_GetMovePosEx( int move_dir, int dst, const GFL_CLWK *cp_clwk, GFL_POINT *p_start, GFL_POINT *p_end )
{
  GFL_CLACTPOS clpos;
  GFL_POINT target = {0};

  //斜めに動く場合のXとYに分解した値
  int line = FX_Div( dst<<FX32_SHIFT, FX32_CONST(1.1414) ) >> FX32_SHIFT;

  GFL_CLACT_WK_GetPos( cp_clwk, &clpos, 0 );

  switch( move_dir )
  {
    //移動方向：上から下
  case PW_CUTIN_DEF_MOVE_DIR_DOWN:
    target.y  = dst;
    break;
    //移動方向：下から上
  case PW_CUTIN_DEF_MOVE_DIR_UP:
    target.y  = -dst;
    break;
    //移動方向：右から左
  case PW_CUTIN_DEF_MOVE_DIR_LEFT:
    target.x  = -dst;
    break;
    //移動方向：左から右
  case PW_CUTIN_DEF_MOVE_DIR_RIGHT:
    target.x  = dst;
    break;
    //移動方向：右下から左上
  case PW_CUTIN_DEF_MOVE_DIR_LEFT_UP:
    target.x = -line;
    target.y = -line;
    break;
    //移動方向：左下から右上
  case PW_CUTIN_DEF_MOVE_DIR_RIGHT_UP:
    target.x = line;
    target.y = -line;
    break;
    //移動方向：右上から左下
  case PW_CUTIN_DEF_MOVE_DIR_LEFT_DOWN:
    target.x = -line;
    target.y = line;
    break;
    //移動方向：左上から右下
  case PW_CUTIN_DEF_MOVE_DIR_RIGHT_DOWN:
    target.x = line;
    target.y = line;
    break;
  }

  p_start->x = clpos.x;
  p_start->y = clpos.y;

  p_end->x = p_start->x + target.x;
  p_end->y = p_start->y + target.y;
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動初期化
 *
 *	@param	PROGVAL_VELOCITY_WORK* p_wk	ワーク
 *	@param	start						開始座標
 *	@param	end							終了座標
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_VEL_Init( PROGVAL_VELOCITY_WORK* p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, int sync )
{
	p_wk->now_val	= *cp_start;
	p_wk->start_val	= *cp_start;
	p_wk->end_val	= *cp_end;
	p_wk->sync_max	= sync;
	if( sync ) {
		p_wk->add_val.x	= FX32_CONST(p_wk->end_val.x - p_wk->start_val.x) / sync;
		p_wk->add_val.y	= FX32_CONST(p_wk->end_val.y - p_wk->start_val.y) / sync;
		p_wk->sync_now	= 0;
	}else{
		//	sync == 0 の場合は即処理終了
		p_wk->sync_now	= p_wk->sync_max-2;	//(p_wk->sync_max-1)より下の値
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動メイン処理（毎フレームよぶこと）
 *
 *	@param	PROGVAL_VELOCITY_WORK* p_wk ワーク
 *
 *	@return	TRUEなら処理終了、FALSEなら処理中。
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_VEL_Main( PROGVAL_VELOCITY_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		p_wk->sync_now++;
		p_wk->now_val.x	= p_wk->start_val.x + ((p_wk->add_val.x * (p_wk->sync_now)) >> FX32_SHIFT);
		p_wk->now_val.y	= p_wk->start_val.y + ((p_wk->add_val.y * (p_wk->sync_now)) >> FX32_SHIFT);
		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_val;
		return TRUE;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  等速直線運動座標取得
 *
 *	@param	const PROGVAL_VELOCITY_WORK* cp_wk  ワーク
 *	@param	*p_pos  座標
 */
//-----------------------------------------------------------------------------
void PROGVAL_VEL_GetPos( const PROGVAL_VELOCITY_WORK* cp_wk, GFL_CLACTPOS *p_pos )
{
  p_pos->x = cp_wk->now_val.x;
  p_pos->y = cp_wk->now_val.y;
}

//=============================================================================
/**
 *					カットインスクリプトデータ読み込みモジュール
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
struct _PW_CUTIN_BINARY
{
  u16 *p_data;
  u32 size;
};

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief　カットイン用スクリプトデータ読み込み
 *
 *	@param	ARCDATID datID  データID（あとで別な定義にするかも。シナリオとイベントとか）
 *	@param	heapID  ヒープID
 *
 *	@return バイナリデータ
 */
//-----------------------------------------------------------------------------
PW_CUTIN_BINARY * PW_CUTIN_BINARY_Init( ARCDATID datID, HEAPID heapID )
{
  PW_CUTIN_BINARY *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(PW_CUTIN_BINARY) );
  p_wk->p_data = GFL_ARC_UTIL_LoadEx(ARCID_POKEWOOD_CUTIN_SCR, datID, FALSE, heapID, &p_wk->size );
  PW_CUTIN_Printf( "cutin datID%d load!\n", datID );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief　カットイン用スクリプトデータ破棄
 *
 *	@param	PW_CUTIN_BINARY *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void PW_CUTIN_BINARY_Exit( PW_CUTIN_BINARY *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ヘッダーデータを取得
 *
 *	@param	const PW_CUTIN_BINARY *cp_wk  ワーク
 *	@param	param   取得するもの
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u16 PW_CUTIN_BINARY_GetHeaderData( const PW_CUTIN_BINARY *cp_wk, PW_CUTIN_BINARY_HEADER_PARAM param )
{
  GF_ASSERT( param < PW_CUTIN_BINARY_HEADER_PARAM_MAX );
  return cp_wk->p_data[ param ];
}
//----------------------------------------------------------------------------
/**
 *	@brief スクリプトデータを取得
 *
 *	@param	const PW_CUTIN_BINARY *cp_wk  ワーク
 *	@param	num スクリプト順番（０～PW_CUTIN_BINARY_GetScriotMax）
 *	@param	param 取得するもの
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u16 PW_CUTIN_BINARY_GetScriptData( const PW_CUTIN_BINARY *cp_wk, int num, PW_CUTIN_BINARY_SCR_PARAM param )
{
  GF_ASSERT( param < PW_CUTIN_BINARY_SCR_PARAM_MAX );
  GF_ASSERT( num < PW_CUTIN_BINARY_GetScriotMax(cp_wk) );
  return cp_wk->p_data[ PW_CUTIN_BINARY_HEADER_PARAM_MAX + num*PW_CUTIN_BINARY_SCR_PARAM_MAX + param ];
}
//----------------------------------------------------------------------------
/**
 *	@brief スクリプトの個数を取得
 *
 *	@param	const PW_CUTIN_BINARY *cp_wk 　ワーク
 *
 *	@return 個数
 */
//-----------------------------------------------------------------------------
u32 PW_CUTIN_BINARY_GetScriotMax( const PW_CUTIN_BINARY *cp_wk )
{
  return (cp_wk->size - PW_CUTIN_BINARY_HEADER_PARAM_MAX*sizeof(u16))/(PW_CUTIN_BINARY_SCR_PARAM_MAX*sizeof(u16));
}
