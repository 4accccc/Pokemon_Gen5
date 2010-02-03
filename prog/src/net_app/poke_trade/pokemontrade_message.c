//=============================================================================
/**
 * @file	  pokemontrade_message.c
 * @brief	  ポケモン交換して通信を終了する メッセージ関係
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/21
 */
//=============================================================================

#include <gflib.h>

#if PM_DEBUG
#include "debug/debugwin_sys.h"
#include "test/debug_pause.h"
#include "poke_tool/monsno_def.h"
#endif

#include "arc_def.h"
#include "net/network_define.h"

#include "net_app/pokemontrade.h"
#include "system/main.h"

#include "message.naix"
#include "print/printsys.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "pokeicon/pokeicon.h"


#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
#include "gamesystem\msgspeed.h"

#include "msg/msg_poke_trade.h"
#include "msg/msg_chr.h"
#include "msg/msg_trade_head.h"
#include "ircbattle.naix"
#include "trade.naix"
#include "net_app/connect_anm.h"
#include "net/dwc_rapfriend.h"
#include "savedata/wifilist.h"

#include "system/touch_subwindow.h"

#include "item/item.h"
#include "app/app_menu_common.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"

#include "pokemontrade_local.h"

static void _select6keywait(POKEMON_TRADE_WORK* pWork);
static void _pokeNickNameMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,BOOL bEgg,POKEMON_TRADE_WORK* pWork);
static void _pokeLvMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork);
static void _pokeSexMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork);



//------メッセージ関連


//----------------------------------------------------------------------------
/**
 *	@brief	属性、タイプアイコン破棄
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UITemplate_TYPEICON_DeleteCLWK( _TYPE_ICON_WORK *wk )
{
  if(wk->clwk_type_icon){
    //CLWK破棄
    GFL_CLACT_WK_Remove( wk->clwk_type_icon );
    wk->clwk_type_icon = NULL;
    //リソース破棄
    UI_EASY_CLWK_UnLoadResource( &wk->clres_type_icon );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	属性、タイプアイコン作成
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *	@param	PokeType					タイプ
 *	@param	GFL_CLUNIT *unit	CLUNIT  ユニット
 *	@param	heapID						ヒープID
 *
 *	@return	void
 */
//-----------------------------------------------------------------------------
static void UITemplate_TYPEICON_CreateCLWK( _TYPE_ICON_WORK *wk, POKEMON_PARAM* pp, int side,GFL_CLUNIT *unit, int x,int y,int draw_type,  HEAPID heapID )
{
  UI_EASY_CLWK_RES_PARAM prm;
  PokeType type = PP_Get(pp, ID_PARA_type1+side, NULL);


  UITemplate_TYPEICON_DeleteCLWK(wk);

  prm.draw_type = draw_type;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
  prm.arc_id    = APP_COMMON_GetArcId();
  prm.pltt_id   = APP_COMMON_GetPokeTypePltArcIdx();
  prm.ncg_id    = APP_COMMON_GetPokeTypeCharArcIdx(type);
  prm.cell_id   = APP_COMMON_GetPokeTypeCellArcIdx( APP_COMMON_MAPPING_128K );
  prm.anm_id    = APP_COMMON_GetPokeTypeAnimeArcIdx( APP_COMMON_MAPPING_128K );
  prm.pltt_line = PLTID_OBJ_TYPEICON_M;
  prm.pltt_src_ofs  = 0;
  prm.pltt_src_num = 3;

  UI_EASY_CLWK_LoadResource( &wk->clres_type_icon, &prm, unit, heapID );

  wk->clwk_type_icon = UI_EASY_CLWK_CreateCLWK( &wk->clres_type_icon, unit, x, y, 0, heapID );

  GFL_CLACT_WK_SetPlttOffs( wk->clwk_type_icon, APP_COMMON_GetPokeTypePltOffset(type),
                            CLWK_PLTTOFFS_MODE_PLTT_TOP );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボールアイコン破棄
 *
 *	@param	_BALL_ICON_WORK* wk ワーク
 */
//-----------------------------------------------------------------------------
static void UITemplate_BALLICON_DeleteCLWK( _BALL_ICON_WORK* wk )
{
  if(wk->clwk_ball){
    //CLWK破棄
    GFL_CLACT_WK_Remove( wk->clwk_ball );
    wk->clwk_ball=NULL;
    //リソース開放
    UI_EASY_CLWK_UnLoadResource( &wk->clres_ball );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボールアイコン作成
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk	ワーク
 *	@param	ballID	ボールID
 *	@param	*unit		CLUNIT
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
//UIテンプレート ボール
static void UITemplate_BALLICON_CreateCLWK( _BALL_ICON_WORK* wk,
                                            POKEMON_PARAM* pp, GFL_CLUNIT *unit,
                                            int x ,int y, int drawtype, HEAPID heapID )
{
  UI_EASY_CLWK_RES_PARAM prm;
  BALL_ID ballID;

  // @todo アイテム＞ボール変換ルーチン待ち
  ballID = BALLID_MASUTAABOORU;  // num=	PP_Get(pp, ID_PARA_item, NULL);

  UITemplate_BALLICON_DeleteCLWK(wk);

  prm.draw_type = drawtype;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
  prm.arc_id    = APP_COMMON_GetArcId();
  prm.pltt_id   = APP_COMMON_GetBallPltArcIdx( ballID );
  prm.ncg_id    = APP_COMMON_GetBallCharArcIdx( ballID );
  prm.cell_id   = APP_COMMON_GetBallCellArcIdx( ballID, APP_COMMON_MAPPING_128K );
  prm.anm_id    = APP_COMMON_GetBallAnimeArcIdx( ballID, APP_COMMON_MAPPING_128K );
  prm.pltt_line = PLTID_OBJ_BALLICON_M;
  prm.pltt_src_ofs  = 0;
  prm.pltt_src_num = 1;

  // リソース読み込み
  UI_EASY_CLWK_LoadResource( &wk->clres_ball, &prm, unit, heapID );
  // CLWK生成
  wk->clwk_ball = UI_EASY_CLWK_CreateCLWK( &wk->clres_ball, unit, x, y, 0, heapID );
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージウインドウ開く 下画面
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------


void POKETRADE_MESSAGE_WindowOpen(POKEMON_TRADE_WORK* pWork)
{
  GFL_BMPWIN* pwin;

  POKETRADE_MESSAGE_WindowClose(pWork);


  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  pWork->mesWin = GFL_BMPWIN_Create(GFL_BG_FRAME2_S , 1 , 1, 30 ,4 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  
  pwin = pWork->mesWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);

  GFL_FONTSYS_SetColor(FBMP_COL_BLACK, FBMP_COL_BLK_SDW, 15);
  pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pMessageStrBuf, pWork->pFontHandle,
                                        MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15 );

//  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pwin), 0, 0, pWork->pMessageStrBuf, pWork->pFontHandle);
  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);

}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージウインドウ消去
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKETRADE_MESSAGE_WindowClose(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->mesWin){
		GFL_BMPWIN_Delete(pWork->mesWin);
    pWork->mesWin=NULL;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   メッセージウインドウ画面消去＋けす
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKETRADE_MESSAGE_WindowClear(POKEMON_TRADE_WORK* pWork)
{

	if(pWork->mesWin){
		GFL_BMPWIN_ClearScreen(pWork->mesWin);
		GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
		BmpWinFrame_Clear(pWork->mesWin, WINDOW_TRANS_OFF);
		GFL_BMPWIN_Delete(pWork->mesWin);
		pWork->mesWin=NULL;
	}
}


//------------------------------------------------------------------------------
/**
 * @brief   文字列関連の初期化と開放
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKETRADE_MESSAGE_HeapInit(POKEMON_TRADE_WORK* pWork)
{
  //文字系初期化
  pWork->pWordSet    = WORDSET_Create( pWork->heapID );
  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_poke_trade_dat, pWork->heapID );
  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
  pWork->pStrBuf = GFL_STR_CreateBuffer( 128, pWork->heapID );
  pWork->pExStrBuf = GFL_STR_CreateBuffer( 128, pWork->heapID );
  pWork->pMessageStrBuf = GFL_STR_CreateBuffer( 128, pWork->heapID );
  pWork->pMessageStrBufEx = GFL_STR_CreateBuffer( 128, pWork->heapID );
  pWork->SysMsgQue = PRINTSYS_QUE_Create( pWork->heapID );
  GFL_FONTSYS_SetColor(1, 2, 15);
  pWork->pMsgTcblSys = GFL_TCBL_Init( pWork->heapID , pWork->heapID , 2 , 0 );

	pWork->pAppTaskRes	= APP_TASKMENU_RES_Create( GFL_BG_FRAME2_S, _SUBLIST_NORMAL_PAL,
			pWork->pFontHandle, pWork->SysMsgQue, pWork->heapID );
}

//------------------------------------------------------------------------------
/**
 * @brief   文字列関連の初期化と開放
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKETRADE_MESSAGE_HeapEnd(POKEMON_TRADE_WORK* pWork)
{
  int i;

  if(pWork->pMsgTcblSys==NULL){
    return;
  }
  

  if(pWork->pAppTaskRes){
    APP_TASKMENU_RES_Delete( pWork->pAppTaskRes );
    pWork->pAppTaskRes=NULL;
  }

  if(pWork->MyInfoWin){
    GFL_BMPWIN_Delete(pWork->MyInfoWin);
    pWork->MyInfoWin=NULL;
  }

  if(pWork->mesWin){
    GFL_BMPWIN_Delete(pWork->mesWin);
    pWork->mesWin=NULL;
  }

  for(i=0;i<2;i++){
    if(pWork->StatusWin[i]){
      GFL_BMPWIN_Delete(pWork->StatusWin[i]);
      pWork->StatusWin[i]=NULL;
    }
  }

  WORDSET_Delete(pWork->pWordSet);
  GFL_MSG_Delete(pWork->pMsgData);
  GFL_FONT_Delete(	pWork->pFontHandle );
  GFL_STR_DeleteBuffer(pWork->pStrBuf);
  GFL_STR_DeleteBuffer(pWork->pExStrBuf);
  GFL_STR_DeleteBuffer(pWork->pMessageStrBuf);
  GFL_STR_DeleteBuffer(pWork->pMessageStrBufEx);
  PRINTSYS_QUE_Clear(pWork->SysMsgQue);
  PRINTSYS_QUE_Delete(pWork->SysMsgQue);
  GFL_TCBL_Exit(pWork->pMsgTcblSys);
  pWork->pMsgTcblSys=NULL;
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了待ち
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL POKETRADE_MESSAGE_EndCheck(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->pStream){
    int state = PRINTSYS_PrintStreamGetState( pWork->pStream );
    switch(state){
    case PRINTSTREAM_STATE_DONE:
      PRINTSYS_PrintStreamDelete( pWork->pStream );
      pWork->pStream = NULL;
      break;
    case PRINTSTREAM_STATE_PAUSE:
      if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE){
        PRINTSYS_PrintStreamReleasePause( pWork->pStream );
      }
      break;
    default:
      break;
    }
    return FALSE;  //まだ終わってない
  }
  return TRUE;// 終わっている
}


void POKETRADE_MESSAGE_AppMenuClose(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->pAppTask){
    G2_BlendNone();
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
}

void POKETRADE_MESSAGE_AppMenuOpenCustom(POKEMON_TRADE_WORK* pWork, int *menustr,int num, int x,int y)
{
  int i;
  APP_TASKMENU_INITWORK appinit;

  appinit.heapId = pWork->heapID;
  appinit.itemNum =  num;
  appinit.itemWork =  &pWork->appitem[0];

  appinit.posType = ATPT_RIGHT_DOWN;
  appinit.charPosX = x;
  appinit.charPosY = y;
	appinit.w				 = APP_TASKMENU_PLATE_WIDTH;
	appinit.h				 = APP_TASKMENU_PLATE_HEIGHT;

  for(i=0;i<num;i++){
    pWork->appitem[i].str = GFL_STR_CreateBuffer(100, pWork->heapID);
    GFL_MSG_GetString(pWork->pMsgData, menustr[i], pWork->appitem[i].str);
    pWork->appitem[i].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  }
  pWork->pAppTask = APP_TASKMENU_OpenMenu(&appinit,pWork->pAppTaskRes);
  for(i=0;i<num;i++){
    GFL_STR_DeleteBuffer(pWork->appitem[i].str);
  }

}

void POKETRADE_MESSAGE_AppMenuOpen(POKEMON_TRADE_WORK* pWork, int *menustr,int num)
{
  if(num==1){
    POKETRADE_MESSAGE_AppMenuOpenCustom(pWork, menustr, num, 32,21);
  }
  else{
    POKETRADE_MESSAGE_AppMenuOpenCustom(pWork, menustr, num, 32,24);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン名をWINDOWに表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _pokeNickNameMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,BOOL bEgg,POKEMON_TRADE_WORK* pWork)
{
  if(!bEgg){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_30, pWork->pExStrBuf );
    WORDSET_RegisterPokeNickName( pWork->pWordSet, 0,  pp );
    WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
  }
  else{
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_07, pWork->pStrBuf );
  }
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y, pWork->pStrBuf, pWork->pFontHandle);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンＬＶをWINDOWに表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _pokeLvMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{
  int lv;

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_21, pWork->pExStrBuf );
  lv=	PP_Get(pp, ID_PARA_level, NULL);

  WORDSET_RegisterNumber(pWork->pWordSet, 0, lv, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );

  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y, pWork->pStrBuf, pWork->pFontHandle);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンSEXをWINDOWに表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _pokeSexMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{
  int sex;
  PRINTSYS_LSB fontCol;

  sex=	PP_Get(pp, ID_PARA_sex, NULL);

  if(sex > PM_FEMALE){
    return;  //表示しない
  }
  if(sex == PM_FEMALE){
    fontCol = PRINTSYS_LSB_Make( _FONT_PARAM_LETTER_RED , _FONT_PARAM_SHADOW_RED , 0 );
  }
  else{
    fontCol = PRINTSYS_LSB_Make( _FONT_PARAM_LETTER_BLUE , _FONT_PARAM_SHADOW_BLUE , 0 );
  }
  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_47 + sex, pWork->pStrBuf );
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(pWin), x, y, pWork->pStrBuf, pWork->pFontHandle,fontCol);
}



//HP－すばやさ文字列を表示
static void _pokeHPSPEEDMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{
  int i;
  for(i=0;i<6;i++){//HP－すばやさ
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_31+i, pWork->pStrBuf );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y+16*i, pWork->pStrBuf, pWork->pFontHandle);
  }
}

static void _pokePocketItemMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{ //もちもの
  int num;

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_44, pWork->pExStrBuf );
  num=	PP_Get(pp, ID_PARA_item, NULL);
  WORDSET_RegisterItemName( pWork->pWordSet, 0,  num );
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y, pWork->pStrBuf, pWork->pFontHandle);
}

//HP HPMax
static void _pokeHpHpmaxMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{
  int num,num2;

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_40, pWork->pExStrBuf );
  num=	PP_Get(pp, ID_PARA_hp, NULL);
  num2=	PP_Get(pp, ID_PARA_hpmax, NULL);
  WORDSET_RegisterNumber(pWork->pWordSet, 0, num, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber(pWork->pWordSet, 1, num2, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf);
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y, pWork->pStrBuf, pWork->pFontHandle);
}

static void _pokeATTACKSPEEDNumMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{
  int i,num;

  for(i=0;i<5;i++){//こうげき－すばやさ数字
    int paras[]={
      ID_PARA_pow,                //攻撃力
      ID_PARA_def,                //防御力
      ID_PARA_spepow,               //特攻
      ID_PARA_spedef,               //特防
      ID_PARA_agi,                //素早さ
    };
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_41, pWork->pExStrBuf );
    num=	PP_Get(pp, paras[i], NULL);
    WORDSET_RegisterNumber(pWork->pWordSet, 0, num, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
    WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y+16*i, pWork->pStrBuf, pWork->pFontHandle);
  }
}

static void _pokeTechniqueMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{
  { //わざ
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_45, pWork->pStrBuf );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y, pWork->pStrBuf, pWork->pFontHandle);
  }
}

static void _pokeTechniqueListMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{
  int i,num;

  for(i=0;i<4;i++){//わざ
    num=	PP_Get(pp, ID_PARA_waza1+i, NULL);
    if(num==0){
      continue;
    }
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_46, pWork->pExStrBuf );
    WORDSET_RegisterWazaName(pWork->pWordSet, 0,  num);
    WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y+i*16, pWork->pStrBuf, pWork->pFontHandle);
  }
}

static void _pokePersonalMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{ //せいかく
  int num;
  GFL_MSGDATA *pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,  NARC_message_chr_dat, pWork->heapID );
  num=	PP_GetSeikaku(pp);
  GFL_MSG_GetString( pMsgData, seikaku00_msg+num, pWork->pStrBuf );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y, pWork->pStrBuf, pWork->pFontHandle);
  GFL_MSG_Delete(pMsgData);
}

static void _pokeAttributeMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{ //とくせい
  int num;
  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_43, pWork->pExStrBuf );
  num=	PP_Get(pp, ID_PARA_speabino, NULL);
  WORDSET_RegisterTokuseiName( pWork->pWordSet, 0,  num );
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y, pWork->pStrBuf, pWork->pFontHandle);
}

//--------------------------------------------------------------
//	ポケモン状態文章セット
//--------------------------------------------------------------

void POKETRADE_MESSAGE_SetPokemonStatusMessage(POKEMON_TRADE_WORK *pWork, int side,POKEMON_PARAM* pp)
{
  int i=0,lv;
  int frame = GFL_BG_FRAME3_M;
  int sidex[] = {1, 17};
  int xdotpos[]={0,128};
  BOOL bEgg = PP_Get(pp,ID_PARA_tamago_flag,NULL);

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  if(pWork->StatusWin[side]){
    GFL_BMPWIN_Delete(pWork->StatusWin[side]);
  }
  pWork->StatusWin[side] = GFL_BMPWIN_Create(frame,	sidex[side], 1, 15, 20,	_BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);

  GFL_FONTSYS_SetColor( FBMP_COL_WHITE, FBMP_COL_WHITE_SDW, 0x0 );

  _pokeNickNameMsgDisp(pp, pWork->StatusWin[side], 2*8, 0, bEgg, pWork);
  if(!bEgg){
    _pokeSexMsgDisp(pp, pWork->StatusWin[side], 10*8, 0, pWork);
    GFL_FONTSYS_SetColor( FBMP_COL_BLACK, FBMP_COL_BLK_SDW, 0x0 );
    _pokeLvMsgDisp(pp, pWork->StatusWin[side], 16, 19, pWork);
    _pokePocketItemMsgDisp(pp, pWork->StatusWin[side], 16, 16*8, pWork);
    UITemplate_BALLICON_CreateCLWK( &pWork->aBallIcon[side+UI_BALL_MYSTATUS], pp, pWork->cellUnit,
                                    xdotpos[side]+16, 16, CLSYS_DRAW_MAIN, pWork->heapID );
  }
  GFL_BMPWIN_TransVramCharacter(pWork->StatusWin[side]);
  GFL_BMPWIN_MakeScreen(pWork->StatusWin[side]);
  GFL_BG_LoadScreenV_Req( frame );

}


//--------------------------------------------------------------
//	ポケモンステータス表示の消去
//--------------------------------------------------------------



void POKETRADE_MESSAGE_ResetPokemonStatusMessage(POKEMON_TRADE_WORK *pWork)
{
  IRC_POKETRADE_ItemIconReset(&pWork->aPokerusMark);
  IRC_POKETRADE_PokeStatusIconReset(pWork);
  GXS_SetVisibleWnd( GX_WNDMASK_NONE );
  UITemplate_BALLICON_DeleteCLWK(&pWork->aBallIcon[UI_BALL_SUBSTATUS]);
  UITemplate_TYPEICON_DeleteCLWK(&pWork->aTypeIcon[0]);
  UITemplate_TYPEICON_DeleteCLWK(&pWork->aTypeIcon[1]);
  IRC_POKETRADE_ResetMainStatusBG(pWork); //CLACT削除含む
  IRCPOKETRADE_PokeDeleteMcss(pWork,0);
  IRCPOKETRADE_PokeDeleteMcss(pWork,1);
  GFL_BMPWIN_ClearScreen(pWork->MyInfoWin);
  GFL_BMPWIN_Delete(pWork->MyInfoWin);
  pWork->MyInfoWin = NULL;

  IRC_POKETRADE_GraphicEndSubDispStatusDisp(pWork);


}

//--------------------------------------------------------------
//	ポケモンステータス表示の初期化
//--------------------------------------------------------------



void POKETRADE_MESSAGE_CreatePokemonParamDisp(POKEMON_TRADE_WORK* pWork,POKEMON_PARAM* pp)
{
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , FALSE );

  IRC_POKETRADE_MainGraphicExit(pWork);  //上画面VRAM開放

  IRC_POKETRADEDEMO_RemoveModel( pWork);  //リールを消す
  POKETRADE_MESSAGE_WindowClear(pWork);  //下のメッセージを消す

  IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,0); //上のステータス文章+OAMを消す
  IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,1); //上のステータス文章+OAMを消す

  pWork->pokemonselectno = 0;//自分から表示
  POKETRADE_MESSAGE_ChangePokemonStatusDisp(pWork,pp);
  IRC_POKETRADE_SetMainStatusBG(pWork);  // 背景BGと

  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    IRC_POKETRADE_SetSubStatusIcon(pWork);  //選択アイコン
    IRC_POKETRADE_GraphicInitSubDispStatusDisp(pWork);
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );
    {
      G2S_SetWnd1InsidePlane(
        GX_WND_PLANEMASK_OBJ,
        FALSE );
      G2S_SetWnd1Position( 128-8*10, 0, 128+8*10, 33 );
      G2S_SetWnd0InsidePlane(
        GX_WND_PLANEMASK_BG0|
        GX_WND_PLANEMASK_OBJ,
        FALSE );
      G2S_SetWnd0Position( 1, 8*20, 0x0, 192 );
      G2S_SetWndOutsidePlane(
        GX_WND_PLANEMASK_BG1|
        GX_WND_PLANEMASK_BG2|
        GX_WND_PLANEMASK_BG3|
        GX_WND_PLANEMASK_OBJ,
        TRUE );
      GXS_SetVisibleWnd( GX_WNDMASK_W0|GX_WNDMASK_W1 );
    }
  }
  //G2S_BlendNone();
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,TRUE );
}

// 情報表示の変更 １まいめか２まいめがある
void POKETRADE_MESSAGE_ChangePokemonMyStDisp(POKEMON_TRADE_WORK* pWork,int pageno,int leftright,POKEMON_PARAM* pp)
{
  int i,num,num2,bEgg;
  int xwinpos[]={0,16};
  int xdotpos[]={0,128};
  {
 //   GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
   //                               0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);

    IRC_POKETRADE_StatusWindowMessagePaletteTrans(pWork,_STATUS_MSG_PAL,PALTYPE_SUB_BG);

    if(pWork->MyInfoWin){
      GFL_BMPWIN_Delete(pWork->MyInfoWin);
    }
    pWork->MyInfoWin =
      GFL_BMPWIN_Create(GFL_BG_FRAME2_S,
                        xwinpos[leftright], 0, 16 , 18, _STATUS_MSG_PAL, GFL_BMP_CHRAREA_GET_F);
  }
  GFL_FONTSYS_SetColor( 0xe, 0xf, 0x0 );
  bEgg = PP_Get(pp,ID_PARA_tamago_flag,NULL);

  if(pageno == 0){  //こうげきとか

    _pokeNickNameMsgDisp(pp,pWork->MyInfoWin, 32, 0, bEgg,pWork);
    if(bEgg==FALSE){
      _pokeLvMsgDisp(pp,pWork->MyInfoWin, 32 ,16,pWork);
      _pokeHPSPEEDMsgDisp(pp,pWork->MyInfoWin, 8 ,32,pWork);
      _pokePocketItemMsgDisp(pp,pWork->MyInfoWin, 4*8 ,16*8,pWork);
      _pokeHpHpmaxMsgDisp(pp, pWork->MyInfoWin, 8*8,4*8, pWork);
      _pokeATTACKSPEEDNumMsgDisp(pp, pWork->MyInfoWin, 10*8,6*8, pWork);
      _pokeSexMsgDisp(pp, pWork->MyInfoWin, 12*8, 0, pWork);

      IRC_POKETRADE_ItemIconDisp(pWork, leftright, pp);
      IRC_POKETRADE_PokerusIconDisp(pWork, leftright,FALSE, pp);
      UITemplate_BALLICON_CreateCLWK( &pWork->aBallIcon[UI_BALL_SUBSTATUS], pp, pWork->cellUnit,
                                      xdotpos[leftright]+16, 16, CLSYS_DRAW_SUB, pWork->heapID );
    }
  }
  else{
    UITemplate_BALLICON_DeleteCLWK(&pWork->aBallIcon[UI_BALL_SUBSTATUS]);
    IRC_POKETRADE_ItemIconReset(&pWork->aItemMark);
    IRC_POKETRADE_ItemIconReset(&pWork->aPokerusMark);
    if(bEgg==FALSE){
      _pokeTechniqueMsgDisp(pp, pWork->MyInfoWin, 2*8,0, pWork);

      _pokeTechniqueListMsgDisp(pp, pWork->MyInfoWin, 2*8,2*8, pWork);
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_37, pWork->pStrBuf );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->MyInfoWin), 2*8, 10*8, pWork->pStrBuf, pWork->pFontHandle);
      _pokePersonalMsgDisp(pp, pWork->MyInfoWin, 2*8, 12*8, pWork);
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_38, pWork->pStrBuf );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->MyInfoWin), 2*8, 14*8, pWork->pStrBuf, pWork->pFontHandle);
      _pokeAttributeMsgDisp(pp, pWork->MyInfoWin, 2*8, 16*8, pWork);
    }
  }

  IRC_POKETRADE_SubStatusInit(pWork,pWork->x, pageno);  //画面BG表示

  GFL_BMPWIN_MakeScreen(pWork->MyInfoWin);
  GFL_BMPWIN_TransVramCharacter(pWork->MyInfoWin);
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
}

// ステータス表示の変更
void POKETRADE_MESSAGE_ChangePokemonStatusDisp(POKEMON_TRADE_WORK* pWork,POKEMON_PARAM* pp)
{
  int i,num,num2,bEgg;
  //POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, pWork->pokemonselectno);
  //  pWork->recvPoke[pWork->pokemonselectno];

  bEgg = PP_Get(pp,ID_PARA_tamago_flag,NULL);

  {//自分の位置調整
    VecFx32 apos;
    apos.x = _MCSS_POS_X(51);
    apos.y = _MCSS_POS_Y(8);
    apos.z = _MCSS_POS_Z(0);
    MCSS_SetPosition( pWork->pokeMcss[pWork->pokemonselectno] ,&apos );
    MCSS_ResetVanishFlag(pWork->pokeMcss[pWork->pokemonselectno]);

    //相手のはOFF
    if(pWork->pokeMcss[1-pWork->pokemonselectno]){
      MCSS_SetVanishFlag(pWork->pokeMcss[1-pWork->pokemonselectno]);
    }

  }
  if(pWork->MyInfoWin){
    GFL_BMPWIN_Delete(pWork->MyInfoWin);
  }
  GFL_FONTSYS_SetColor( 0xf, 0xe, 0 );
  pWork->MyInfoWin =
    GFL_BMPWIN_Create(GFL_BG_FRAME3_M,
                      1, 1, 31 , 24, _BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);

  _pokeNickNameMsgDisp(pp,pWork->MyInfoWin, 16, 0,bEgg,pWork);//ニックネーム
  _pokeLvMsgDisp(pp,pWork->MyInfoWin, 8*12 , 2,pWork);
  _pokeSexMsgDisp(pp, pWork->MyInfoWin, 10*8, 0, pWork);

  _pokeHPSPEEDMsgDisp(pp,pWork->MyInfoWin, 0 ,3*8,pWork);

  for(i=0;i<3;i++){//せいかく-もちもの
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_37+i, pWork->pStrBuf );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->MyInfoWin), 0, 16*8+16*i, pWork->pStrBuf, pWork->pFontHandle);
  }
  _pokeHpHpmaxMsgDisp(pp, pWork->MyInfoWin, 8*7, 3*8, pWork);  //HP HPMax

  _pokeATTACKSPEEDNumMsgDisp(pp, pWork->MyInfoWin, 8*9, 5*8, pWork);


  _pokePersonalMsgDisp(pp, pWork->MyInfoWin, 9*8, 16*8, pWork);  //性格
  _pokeAttributeMsgDisp(pp, pWork->MyInfoWin, 9*8, 18*8, pWork);  //特性
  _pokePocketItemMsgDisp(pp,pWork->MyInfoWin, 9*8, 20*8,pWork);  //もちもの
  _pokeTechniqueMsgDisp(pp, pWork->MyInfoWin, 20*8, 13*8, pWork);  //わざ
  _pokeTechniqueListMsgDisp(pp, pWork->MyInfoWin, 20*8,15*8, pWork); //わざリスト

  UITemplate_BALLICON_CreateCLWK( &pWork->aBallIcon[UI_BALL_SUBSTATUS], pp, pWork->cellUnit,
                                  16, 16, CLSYS_DRAW_MAIN, pWork->heapID );

  UITemplate_TYPEICON_CreateCLWK(&pWork->aTypeIcon[0], pp, 0, pWork->cellUnit,
                                 25*8, 12*8, CLSYS_DRAW_MAIN, pWork->heapID );

  if( PP_Get(pp, ID_PARA_type1, NULL) != PP_Get(pp, ID_PARA_type2, NULL)){
    UITemplate_TYPEICON_CreateCLWK(&pWork->aTypeIcon[1], pp, 1, pWork->cellUnit,
                                   29*8, 12*8, CLSYS_DRAW_MAIN, pWork->heapID );
  }
  IRC_POKETRADE_PokeStatusIconDisp(pWork,pp);  //●▲■
  IRC_POKETRADE_PokerusIconDisp(pWork, 0,TRUE, pp);


  GFL_BMPWIN_MakeScreen(pWork->MyInfoWin);
  GFL_BMPWIN_TransVramCharacter(pWork->MyInfoWin);
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
}


//--------------------------------------------------------------
//	ポケモン状態文章消去
//--------------------------------------------------------------

void IRCPOKEMONTRADE_ResetPokemonStatusMessage(POKEMON_TRADE_WORK *pWork, int side)
{

  if(pWork->StatusWin[side]){
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->StatusWin[side]), 0);
    GFL_BMPWIN_ClearScreen(pWork->StatusWin[side]);
    GFL_BMPWIN_Delete(pWork->StatusWin[side]);
    GFL_BG_LoadScreenV_Req(GFL_BG_FRAME3_M);
    pWork->StatusWin[side] = NULL;
    UITemplate_BALLICON_DeleteCLWK( &pWork->aBallIcon[side+UI_BALL_MYSTATUS]);
  }
}

void POKETRADE_MESSAGE_ResetPokemonMyStDisp(POKEMON_TRADE_WORK* pWork)
{
  UITemplate_BALLICON_DeleteCLWK(&pWork->aBallIcon[UI_BALL_SUBSTATUS]);
  IRC_POKETRADE_ItemIconReset(&pWork->aItemMark);
  IRC_POKETRADE_ItemIconReset(&pWork->aPokerusMark);
  if(pWork->MyInfoWin){
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->MyInfoWin), 0 );
    GFL_BMPWIN_ClearScreen(pWork->MyInfoWin);
    GFL_BMPWIN_TransVramCharacter(pWork->MyInfoWin);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
    GFL_BMPWIN_Delete(pWork->MyInfoWin);
    pWork->MyInfoWin = NULL;
  }
}




//--------------------------------------------------------------
//	６体ポケモンステータス表示
//--------------------------------------------------------------

void POKETRADE_MESSAGE_SixStateDisp(POKEMON_TRADE_WORK* pWork)
{
  GFL_BMPWIN* pWin;
  int side,poke;
  MYSTATUS* aStBuf[GTS_PLAYER_WORK_NUM];

  aStBuf[0] = pWork->pMy;
  aStBuf[1] = pWork->pFriend;



#if 1
  GFL_FONTSYS_SetColor( FBMP_COL_WHITE, FBMP_COL_WHITE_SDW, 0x0 );
  for(side=0;side<GTS_PLAYER_WORK_NUM;side++){
    pWork->TriStatusWin[side] =
      GFL_BMPWIN_Create(GFL_BG_FRAME1_S,
                        side*16+ 1 , 0, 14, 2, _BUTTON_MSG_PAL,GFL_BMP_CHRAREA_GET_F);

    pWin = pWork->TriStatusWin[side];
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_27, pWork->pExStrBuf );
    WORDSET_RegisterPlayerName( pWork->pWordSet, 0,  aStBuf[side]  );
    WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), 0, 0, pWork->pStrBuf, pWork->pFontHandle);
    GFL_BMPWIN_TransVramCharacter(pWin);
    GFL_BMPWIN_MakeScreen(pWin);
  }
#endif


  GFL_FONTSYS_SetColor( FBMP_COL_WHITE, FBMP_COL_WHITE_SDW, 0x0 );
  for(side=0;side<GTS_PLAYER_WORK_NUM;side++){
    pWork->TriStatusWin[side+2] =
      GFL_BMPWIN_Create(GFL_BG_FRAME1_S,
                        side*16 + 5 , 3, 9, 16, _BUTTON_MSG_PAL,GFL_BMP_CHRAREA_GET_F);
    pWin = pWork->TriStatusWin[side+2];
    GFL_FONTSYS_SetColor( FBMP_COL_BLACK, FBMP_COL_BLK_SDW, 0x0 );
    for(poke = 0;poke < GTS_NEGO_POKESLT_MAX;poke++){
      POKEMON_PARAM* pp = pWork->GTSSelectPP[side][poke];
      if(pWork->GTSSelectIndex[side][poke]!=-1){
        BOOL bEgg = PP_Get(pp,ID_PARA_tamago_flag,NULL);

        POKETRADE_2D_GTSPokemonIconSet(pWork, side, poke, pp,FALSE);
        
        _pokeNickNameMsgDisp(pp, pWin, 0, 6*8*poke , bEgg, pWork);
        if(!bEgg){
          _pokeSexMsgDisp(pp, pWin, 7*8, 6*8*poke + 2*8, pWork);
          _pokeLvMsgDisp(pp, pWin, 0, 6*8*poke + 2*8, pWork);
        }
      }
    }
    GFL_BMPWIN_TransVramCharacter(pWin);
    GFL_BMPWIN_MakeScreen(pWin);
  }
}


//--------------------------------------------------------------
//	６体ポケモンステータス消去
//--------------------------------------------------------------

void POKETRADE_MESSAGE_SixStateDelete(POKEMON_TRADE_WORK* pWork)
{
  int i;

  for(i=0;i<GTS_PLAYER_WORK_NUM*2 ;i++){
    if( pWork->TriStatusWin[i] ){
      GFL_BMPWIN_Delete(pWork->TriStatusWin[i]);
      pWork->TriStatusWin[i]=NULL;
    }
  }
}

