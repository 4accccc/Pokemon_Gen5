//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		inf_dungeon_retire.c
 *	@brief  無限ダンジョンリタイア画面
 *	@author	Masayuki Onoue
 *	@date		2011.08.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

#include "system/gfl_use.h"
#include "system/ds_system.h"
#include "system/wipe.h"
#include "system/bmp_winframe.h"

#include "gamesystem/msgspeed.h"

#include "print/printsys.h"
#include "print/wordset.h"
#include "print/gf_font.h"

#include "app/app_taskmenu.h"
#include "app/app_keycursor.h"

#include "c_gear_sw.naix"

#include "msg/msg_infinity_dungeon_retire.h"
#include "inf_dungeon_retire.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
///	会話
//=====================================
#define SYS_MSG_POS_X  (1)
#define SYS_MSG_POS_Y  (1)
#define SYS_MSG_POS_SIZX  (30)
#define SYS_MSG_POS_SIZY  (7)

#define SYS_CANNOT_MSG_POS_Y  (6)

//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_BACK_S	= GFL_BG_FRAME2_S,
	BG_FRAME_MSG_S	= GFL_BG_FRAME1_S,
	BG_FRAME_YESNO_S	= GFL_BG_FRAME0_S,
	BG_FRAME_PLATE_M	= GFL_BG_FRAME1_M,
};

//-------------------------------------
///	パレット
//=====================================
enum
{	
	//サブBG
	PLTID_BG_BACK_S				=	0,
	PLTID_BG_MSG_S,
	PLTID_BG_WIN_S,
	PLTID_BG_YESNO00_S,
	PLTID_BG_YESNO01_S,

};


//-------------------------------------
///	BGフレーム
//=====================================
static const GFL_BG_BGCNT_HEADER BGFrameBack = {
  0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, 0x6800, GX_BG_EXTPLTT_01,
  3, 0, 0, FALSE
};
static const GFL_BG_BGCNT_HEADER BGFrameMsg = {
  0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x6800, GX_BG_EXTPLTT_01,
  2, 0, 0, FALSE
};
static const GFL_BG_BGCNT_HEADER BGFrameYesNo = {
  0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x6800, GX_BG_EXTPLTT_01,
  2, 0, 0, FALSE
};

//-------------------------------------
///	YesNoボタン情報
//=====================================
enum{
  YESNO_LIST_YES,
  YESNO_LIST_NO,
  YESNO_LIST_MAX,
};

static APP_TASKMENU_ITEMWORK s_APP_TASKMENU_ITEMWORK[YESNO_LIST_MAX] = {
  {
    NULL,
    APP_TASKMENU_ITEM_MSGCOLOR,
    APP_TASKMENU_WIN_TYPE_NORMAL,
  },
  {
    NULL,
    APP_TASKMENU_ITEM_MSGCOLOR,
    APP_TASKMENU_WIN_TYPE_NORMAL,
  },
};


static APP_TASKMENU_INITWORK s_APP_TASKMENU_INITWORK = {
  0,
  
  YESNO_LIST_MAX,
  s_APP_TASKMENU_ITEMWORK,

  ATPT_LEFT_UP,
  23,
  SYS_MSG_POS_SIZY+2,
  0,0, 
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	リタイアボタンワーク
//=====================================
struct _INFRETIRESUBSCRN_WORK{
  HEAPID heapID;

  GAMESYS_WORK* p_gamesys;
  GAME_COMM_SYS_PTR p_gamecomm;
  FIELD_SUBSCREEN_WORK* p_subscreen;

  GFL_ARCUTIL_TRANSINFO back_area;
  GFL_ARCUTIL_TRANSINFO winframe_area;
  GFL_BMPWIN*   p_msgwin;


  GFL_FONT*       p_font;
  GFL_MSGDATA*    p_msgdata;
  PRINT_QUE*      p_printQue;

  APP_TASKMENU_RES*   p_yesno_res;
  APP_TASKMENU_WORK*  p_yesno;

  u16 seq;

  u16 main_end;


};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void INFRETIRESUBSCRN_InitGraphic( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID );
static void INFRETIRESUBSCRN_InitBg( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID );
static void INFRETIRESUBSCRN_InitWin( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID );
static void INFRETIRESUBSCRN_InitYesNo( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID );
static void INFRETIRESUBSCRN_PrintMsg( INFRETIRESUBSCRN_WORK* p_wk, GFL_BMPWIN* p_win, u32 strID );
static void INFRETIRESUBSCRN_CreateYesNo( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID );
static void INFRETIRESUBSCRN_UpdateYesNo( INFRETIRESUBSCRN_WORK* p_wk );
static u32 INFRETIRESUBSCRN_GetYesNo( const INFRETIRESUBSCRN_WORK* p_wk );
static const BOOL INFRETIRESUBSCRN_CheckButtonAnmEnd( const INFRETIRESUBSCRN_WORK* p_wk );
static void INFRETIRESUBSCRN_ExitGraphic( INFRETIRESUBSCRN_WORK* p_wk );
static void INFRETIRESUBSCRN_ExitWin( INFRETIRESUBSCRN_WORK* p_wk );
static void INFRETIRESUBSCRN_ExitYesNo( INFRETIRESUBSCRN_WORK* p_wk );
static void INFRETIRESUBSCRN_ExitBg( INFRETIRESUBSCRN_WORK* p_wk );
static void INFRETIRESUBSCRN_DeleteYesNo( INFRETIRESUBSCRN_WORK* p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンリタイアパラメータ初期化
 *
 *	@param	p_sub
 *	@param	p_gamesys 
 *	@param	heapID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
INFRETIRESUBSCRN_WORK* INFRETIRESUBSCRN_Init( FIELD_SUBSCREEN_WORK* p_sub,GAMESYS_WORK* p_gamesys ,HEAPID heapID)
{
  INFRETIRESUBSCRN_WORK *p_wk;
  // ワーク生成
  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(INFRETIRESUBSCRN_WORK) );

  p_wk->p_gamesys    = p_gamesys;
  p_wk->p_gamecomm   = GAMESYSTEM_GetGameCommSysPtr( p_gamesys );
  p_wk->p_subscreen  = p_sub;
 
  // 描画環境の生成
  INFRETIRESUBSCRN_InitGraphic( p_wk, heapID );
 
  // 初期化のみここで行っておく
  INFRETIRESUBSCRN_Main( p_wk );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 *
 *	@param	p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFRETIRESUBSCRN_Main( INFRETIRESUBSCRN_WORK* p_wk )
{
  switch( p_wk->seq ){
  case 0:
    // 最初のメッセージは、一気に表示しておく。
    INFRETIRESUBSCRN_PrintMsg( p_wk, p_wk->p_msgwin, msg_retire_01 );
    INFRETIRESUBSCRN_CreateYesNo( p_wk, p_wk->heapID );

    p_wk->seq++;
    break;

  case 1:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      p_wk->seq++;
    }
    break;

  case 2:
    {
      // 終了待ち
      if( INFRETIRESUBSCRN_CheckButtonAnmEnd( p_wk ))
      {
        p_wk->seq++;
      }
    }
    break;

  case 3:
    {
      // ボタン状況の判断
      if( (INFRETIRESUBSCRN_GetYesNo( p_wk ) == YESNO_LIST_YES) )
      {
        FIELD_SUBSCREEN_MainDispBrightnessOff( p_wk->p_subscreen, p_wk->heapID );
        FIELD_SUBSCREEN_SetAction( p_wk->p_subscreen, FIELD_SUBSCREEN_ACTION_INFRETIRE_CALL );
      }
      else
      {
        FIELD_SUBSCREEN_SetAction( p_wk->p_subscreen, FIELD_SUBSCREEN_ACTION_INFRETIRE_TOPMENU );
      }
      INFRETIRESUBSCRN_DeleteYesNo( p_wk );
    }
    break;
  }

  // ボタン結果待ち
  INFRETIRESUBSCRN_UpdateYesNo( p_wk );
  
  // キューメイン
  PRINTSYS_QUE_Main( p_wk->p_printQue );
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  開放処理
 *
 *	@param	p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFRETIRESUBSCRN_Exit( INFRETIRESUBSCRN_WORK* p_wk )
{
  INFRETIRESUBSCRN_ExitGraphic( p_wk );

	GFL_HEAP_FreeMemory( p_wk );

  ONOUE_Printf("開放処理実行\n");
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィック初期化
 *
 *	@param	p_wk
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_InitGraphic( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID )
{
    // ハンドル
  p_wk->p_font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , heapID );
  GFL_FONTSYS_SetDefaultColor();
  
  // メッセージ
  p_wk->p_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_infinity_dungeon_retire_dat, heapID );

  // Printキュー
  p_wk->p_printQue = PRINTSYS_QUE_Create( heapID );

  // BG環境
  INFRETIRESUBSCRN_InitBg( p_wk, heapID );

  // YesNo
  INFRETIRESUBSCRN_InitYesNo( p_wk, heapID );

  // Win
  INFRETIRESUBSCRN_InitWin( p_wk, heapID );

  // NET ICON
  GFL_NET_ReloadIconTopOrBottom( FALSE, heapID );


  // 上画面にパネルを出す。
  FIELD_SUBSCREEN_SetMainLCDNavigationScreen( p_wk->p_subscreen, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG表示環境の初期化
 *
 *	@param	p_wk     システム
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_InitBg( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID )
{
  int i = 0;
  for(i = GFL_BG_FRAME0_S;i <= GFL_BG_FRAME3_S ; i++)
  {
    GFL_BG_SetVisible( i, VISIBLE_OFF );
  }
  
  // BGControl
  GFL_BG_SetBGControl( BG_FRAME_BACK_S, &BGFrameBack, GFL_BG_MODE_TEXT );
  GFL_BG_ClearScreen( BG_FRAME_BACK_S );
  GFL_BG_LoadScreenReq( BG_FRAME_BACK_S );
  GFL_BG_SetVisible( BG_FRAME_BACK_S, VISIBLE_ON );

  GFL_BG_SetBGControl( BG_FRAME_MSG_S, &BGFrameMsg, GFL_BG_MODE_TEXT );
  GFL_BG_ClearScreen( BG_FRAME_MSG_S );
  GFL_BG_LoadScreenReq( BG_FRAME_MSG_S );
  GFL_BG_SetVisible( BG_FRAME_MSG_S, VISIBLE_ON );

  GFL_BG_SetBGControl( BG_FRAME_YESNO_S, &BGFrameYesNo, GFL_BG_MODE_TEXT );
  GFL_BG_ClearScreen( BG_FRAME_YESNO_S );
  GFL_BG_LoadScreenReq( BG_FRAME_YESNO_S );
  GFL_BG_SetVisible( BG_FRAME_YESNO_S, VISIBLE_ON );

  // バックグラウンドを設定
  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR_SW, heapID );
    MYSTATUS* p_my = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(p_wk->p_gamesys) );
    u32 sex = MyStatus_GetMySex(p_my);
    static const u32 _bgpal[]={NARC_c_gear_sw_no_gear_m_nclr,NARC_c_gear_sw_no_gear_f_nclr};
    
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, _bgpal[ sex % 2 ],
                                      PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0x20,  heapID);
    
    // サブ画面BGキャラ転送
    p_wk->back_area = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_c_gear_sw_no_gear_ncgr,
                                                                  BG_FRAME_BACK_S, 0, FALSE, heapID);

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
                                           NARC_c_gear_sw_no_gear_nscr,
                                           BG_FRAME_BACK_S, 0,
                                           GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->back_area), 0, 0,
                                           heapID);

    GFL_ARC_CloseDataHandle( p_handle );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  window表示環境の初期化
 *
 *	@param	p_wk     システム
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_InitWin( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID )
{
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr, PALTYPE_SUB_BG, PLTID_BG_MSG_S*32, 32, heapID );
  p_wk->winframe_area = BmpWinFrame_GraphicSetAreaMan( BG_FRAME_MSG_S, PLTID_BG_WIN_S, MENU_TYPE_SYSTEM, heapID );

  p_wk->p_msgwin = GFL_BMPWIN_Create( BG_FRAME_MSG_S, 
                                    SYS_MSG_POS_X, 
                                    SYS_MSG_POS_Y,
                                    SYS_MSG_POS_SIZX, 
                                    SYS_MSG_POS_SIZY, 
                                    PLTID_BG_MSG_S,
                                    GFL_BMP_CHRAREA_GET_B );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_msgwin ) , 15 );
  GFL_BMPWIN_TransVramCharacter( p_wk->p_msgwin );
  GFL_BMPWIN_MakeScreen( p_wk->p_msgwin );


}

//----------------------------------------------------------------------------
/**
 *	@brief  YesNoウィンドウの初期化
 *
 *	@param	p_wk
 *	@param	heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_InitYesNo( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID )
{
  p_wk->p_yesno_res = APP_TASKMENU_RES_Create( BG_FRAME_YESNO_S, PLTID_BG_YESNO00_S, p_wk->p_font, p_wk->p_printQue, heapID );
}


// メッセージ表示
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージの表示処理
 *
 *	@param	p_wk   システム
 *	@param	p_win   window
 *	@param	strID   文字列ID
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_PrintMsg( INFRETIRESUBSCRN_WORK* p_wk, GFL_BMPWIN* p_win, u32 strID )
{
  STRBUF* p_str;

  p_str = GFL_MSG_CreateString( p_wk->p_msgdata, strID );
  
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_win), 0, 0, p_str, p_wk->p_font );
  BmpWinFrame_WriteAreaMan( p_win, WINDOW_TRANS_ON, 
      p_wk->winframe_area, PLTID_BG_WIN_S );
  
  GFL_BMPWIN_TransVramCharacter( p_win );
  
  GFL_STR_DeleteBuffer( p_str );
}

// ボタン
//----------------------------------------------------------------------------
/**
 *	@brief  ボタン表示
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_CreateYesNo( INFRETIRESUBSCRN_WORK* p_wk, HEAPID heapID )
{
  s_APP_TASKMENU_INITWORK.heapId = heapID;

  s_APP_TASKMENU_ITEMWORK[ YESNO_LIST_YES ].str = GFL_MSG_CreateString( p_wk->p_msgdata, msg_yes );
  s_APP_TASKMENU_ITEMWORK[ YESNO_LIST_NO ].str = GFL_MSG_CreateString( p_wk->p_msgdata, msg_no );


	p_wk->p_yesno	= APP_TASKMENU_OpenMenuEx( &s_APP_TASKMENU_INITWORK, p_wk->p_yesno_res );
  APP_TASKMENU_SetDisableKey( p_wk->p_yesno, TRUE );

  GFL_STR_DeleteBuffer( s_APP_TASKMENU_ITEMWORK[ YESNO_LIST_YES ].str );
  GFL_STR_DeleteBuffer( s_APP_TASKMENU_ITEMWORK[ YESNO_LIST_NO ].str );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン結果待ち
 *
 *	@param	p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_UpdateYesNo( INFRETIRESUBSCRN_WORK* p_wk )
{
  if( p_wk->p_yesno ){
    APP_TASKMENU_UpdateMenu( p_wk->p_yesno );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン状況の取得
 *
 *	@param	INFRETIRESUBSCRN_WORK* p_wk 
 *
 *	@retval TRUEの時YES
 */
//-----------------------------------------------------------------------------
static u32 INFRETIRESUBSCRN_GetYesNo( const INFRETIRESUBSCRN_WORK* p_wk )
{
  if( p_wk->p_yesno ){
    return APP_TASKMENU_GetCursorPos( p_wk->p_yesno );
  }
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーション終了待ち
 *
 *	@param	INFRETIRESUBSCRN_WORK* p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const BOOL INFRETIRESUBSCRN_CheckButtonAnmEnd( const INFRETIRESUBSCRN_WORK* p_wk )
{
  return APP_TASKMENU_IsFinish( p_wk->p_yesno );
}

//----------------------------------------------------------------------------
/**
 *	@brief  見た目の破棄
 *
 *	@param	p_wk   システム
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_ExitGraphic( INFRETIRESUBSCRN_WORK* p_wk )
{
  // Win
  INFRETIRESUBSCRN_ExitWin( p_wk );

  // YesNo環境
  INFRETIRESUBSCRN_ExitYesNo( p_wk );

  // BG環境
  INFRETIRESUBSCRN_ExitBg( p_wk );

  // プリントキュー
  PRINTSYS_QUE_Clear( p_wk->p_printQue );
  PRINTSYS_QUE_Delete( p_wk->p_printQue );

  // メッセージ
  GFL_MSG_Delete( p_wk->p_msgdata );

  // フォント
  GFL_FONT_Delete( p_wk->p_font );
}

//----------------------------------------------------------------------------
/**
 *	@brief  window表示環境の破棄
 *
 *	@param	p_wk   システム
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_ExitWin( INFRETIRESUBSCRN_WORK* p_wk )
{
  GFL_BMPWIN_Delete( p_wk->p_msgwin );
  BmpWinFrame_GraphicFreeAreaMan( BG_FRAME_MSG_S, p_wk->winframe_area );
}

//----------------------------------------------------------------------------
/**
 *	@brief  YesNo破棄
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_DeleteYesNo( INFRETIRESUBSCRN_WORK* p_wk )
{
  if( p_wk->p_yesno ){
    APP_TASKMENU_CloseMenu( p_wk->p_yesno );
    p_wk->p_yesno = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  YesNo表示環境の破棄
 *
 *	@param	p_wk     システム
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_ExitYesNo( INFRETIRESUBSCRN_WORK* p_wk )
{
  APP_TASKMENU_RES_Delete( p_wk->p_yesno_res );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG表示環境の破棄
 *
 *	@param	p_wk   システム
 */
//-----------------------------------------------------------------------------
static void INFRETIRESUBSCRN_ExitBg( INFRETIRESUBSCRN_WORK* p_wk )
{
  GFL_BG_FreeCharacterArea( BG_FRAME_BACK_S, 
      GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->back_area),
      GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->back_area) );
  
  GFL_BG_FreeBGControl(BG_FRAME_BACK_S);
  GFL_BG_FreeBGControl(BG_FRAME_MSG_S);
  GFL_BG_FreeBGControl(BG_FRAME_YESNO_S);

  GFL_BG_SetVisible( BG_FRAME_BACK_S, VISIBLE_OFF );
  GFL_BG_SetVisible( BG_FRAME_MSG_S, VISIBLE_OFF );
  GFL_BG_SetVisible( BG_FRAME_YESNO_S, VISIBLE_OFF );
}

