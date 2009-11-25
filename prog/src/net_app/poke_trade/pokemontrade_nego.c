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


void POKE_GTS_EndWork(POKEMON_TRADE_WORK* pWork)
{
  int i,j;
  for(j = 0;j < GTS_PLAYER_WORK_NUM;j++){
    for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
      GFL_HEAP_FreeMemory(pWork->GTSSelectPP[j][i]);
    }
  }

}

void POKE_GTS_InitWork(POKEMON_TRADE_WORK* pWork)
{
  int i,j;
  for(j = 0;j < GTS_PLAYER_WORK_NUM;j++){
    for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
      pWork->GTSSelectIndex[j][i] = -1;
      pWork->GTSSelectPP[j][i] = GFL_HEAP_AllocClearMemory( pWork->heapID, POKETOOL_GetWorkSize() );
    }
  }

}


//ポケモン追加
static int _addPokemon(POKEMON_TRADE_WORK* pWork,int side,int index,int boxno,const POKEMON_PARAM* pp)
{
  int i;
  
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if(pWork->GTSSelectIndex[side][i] == -1){
      pWork->GTSSelectIndex[side][i] = index;
      pWork->GTSSelectBoxno[side][i] = boxno;
      GFL_STD_MemCopy(pp,pWork->GTSSelectPP[side][i],POKETOOL_GetWorkSize());
      return i;
    }
  }
  return -1;
}

//ポケモン削除
static BOOL _deletePokemon(POKEMON_TRADE_WORK* pWork,int side,int index,int boxno)
{
  int i;
  
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if((pWork->GTSSelectIndex[side][i] == index) && (pWork->GTSSelectBoxno[side][i] == boxno)){
      pWork->GTSSelectIndex[side][i] = -1;
      return TRUE;
    }
  }
  return FALSE;
}



//------------------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンリソースの作成
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------


void POKE_GTS_CreatePokeIconResource(POKEMON_TRADE_WORK* pWork,int mainsub)
{
  ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );
  pWork->cellRes[PLT_GTS_POKEICON] =
    GFL_CLGRP_PLTT_RegisterComp( arcHandlePoke ,
                                 POKEICON_GetPalArcIndex() , mainsub ,
                                 _OBJPLT_GTS_POKEICON_OFFSET_M , pWork->heapID  );
  pWork->cellRes[ANM_GTS_POKEICON] =
    GFL_CLGRP_CELLANIM_Register( arcHandlePoke ,
                                 POKEICON_GetCellSubArcIndex() , POKEICON_GetAnmArcIndex(), pWork->heapID  );
  
  GFL_ARC_CloseDataHandle(arcHandlePoke);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンリソースの開放
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------


static void POKE_GTS_ReleasePokeIconResource(POKEMON_TRADE_WORK* pWork)
{
  
  if(pWork->cellRes[PLT_GTS_POKEICON]!=0){
    GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_GTS_POKEICON] );
    pWork->cellRes[PLT_GTS_POKEICON]=0;
  }
  if(pWork->cellRes[ANM_GTS_POKEICON]!=0){
    GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_GTS_POKEICON] );
    pWork->cellRes[ANM_GTS_POKEICON]=0;
  }
}




void POKE_GTS_PokemonReset(POKEMON_TRADE_WORK* pWork,int side, int no)
{
  if(pWork->pokeIconGTS[side][no]){
    GFL_CLACT_WK_Remove( pWork->pokeIconGTS[side][no]);
    pWork->pokeIconGTS[side][no]=NULL;
    GFL_CLGRP_CGR_Release( pWork->pokeIconNcgResGTS[side][no] );
  }

}



BOOL POKE_GTS_PokemonsetAndSendData(POKEMON_TRADE_WORK* pWork,int index,int boxno)
{ //選択ポケモン表示
  BOOL bRet = FALSE;
  POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->selectBoxno, pWork->selectIndex,pWork);
  POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);
  int no;
  
//  if(!GFL_NET_IsInit() || GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON, POKETOOL_GetWorkSize(), pp)){
  {
    no = _addPokemon(pWork,0,index,boxno, pp);
    if(-1 != no){
      POKETRADE_2D_PokemonIconSet(pWork,0,no,pp,TRUE);
      bRet= TRUE;
    }
  }

  if(!GFL_NET_IsInit()){
    no = _addPokemon(pWork, 1, index,boxno, pp);
    if(-1 != no){
      POKETRADE_2D_PokemonIconSet(pWork,1,no,pp,TRUE);
      bRet= TRUE;
    }
  }




  GFL_HEAP_FreeMemory(pp);

  if(pWork->pAppTask==NULL){
    int msg[]={POKETRADE_STR2_26};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
  }

  return bRet;
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンリソースの作成
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_StatusMessageDisp(POKEMON_TRADE_WORK* pWork)
{
  int side;
  int sidex[] = {1, 17};
  int xdotpos[]={0,128};
  int i=0,lv;
  int frame = GFL_BG_FRAME3_M;
  GFL_BMPWIN* pWin;
  MYSTATUS* aStBuf[2];

  aStBuf[0] = pWork->pMy;
  aStBuf[1] = pWork->pFriend;
    
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  
  if(pWork->StatusWin[0]){
    GFL_BMPWIN_Delete(pWork->StatusWin[0]);
  }
  pWork->StatusWin[0] = GFL_BMPWIN_Create(frame,	0, 0, 32, 20,	_BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);
  pWin = pWork->StatusWin[0];

  GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_22 + pWork->GTSlv[0], pWork->pStrBuf );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), 64,  2, pWork->pStrBuf, pWork->pFontHandle);

  

  for(side = 0 ; side < GTS_PLAYER_WORK_NUM; side++){
    if(aStBuf[side]){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_16, pWork->pExStrBuf );
      WORDSET_RegisterPlayerName( pWork->pWordSet, 0,  aStBuf[side]  );
      WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), sidex[side]*8, 16+8, pWork->pStrBuf, pWork->pFontHandle);

      GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_17 + pWork->GTStype[side], pWork->pStrBuf );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), sidex[side]*8, 48+8, pWork->pStrBuf, pWork->pFontHandle);

      GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_14 + pWork->bGTSSelect[side], pWork->pStrBuf );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), sidex[side]*8, 128+8, pWork->pStrBuf, pWork->pFontHandle);

      GFL_BMPWIN_TransVramCharacter(pWin);
      GFL_BMPWIN_MakeScreen(pWin);
    }
  }
  GFL_BG_LoadScreenV_Req( frame );
}



static void _menuFriendPokemon(POKEMON_TRADE_WORK* pWork)
{


  
}


static void _menuMyPokemon(POKEMON_TRADE_WORK* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);  //下のメッセージを消す
    pWork->pAppTask=NULL;
    switch(selectno){
    case 0:  //こうかん
      _CHANGE_STATE(pWork,POKETRE_MAIN_ChangePokemonSendDataNetwork);
      break;
    case 1:  //つよさをみる
     // _CHANGE_STATE(pWork, _pokemonStatusStart);
     // break;
    case 2:  //もどる
      _CHANGE_STATE(pWork, _select6keywait);
      break;
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   6体表示まち
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _select6keywait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bTouch=FALSE;
  GFL_UI_TP_HITTBL tp_data[]={
    { 3*8, 7*8, 1*8, 15*8 },
    { 9*8, 13*8, 1*8, 15*8 },
    { 15*8, 19*8, 1*8, 15*8 },
    { 3*8, 7*8, 17*8, 31*8 },
    { 9*8, 13*8, 17*8, 31*8 },
    { 15*8, 19*8, 17*8, 31*8 },
    {GFL_UI_TP_HIT_END,0,0,0},
  };
  int trgno,target;
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );

  trgno = GFL_UI_TP_HitTrg(tp_data);
  if(trgno != -1){
    POKEMON_PARAM* pp = pWork->GTSSelectPP[trgno/GTS_NEGO_POKESLT_MAX][trgno%GTS_NEGO_POKESLT_MAX];
    if(pp){
      if(trgno/GTS_NEGO_POKESLT_MAX){
        int msg[]={ POKETRADE_STR_04, POKETRADE_STR_06};
        POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
        _CHANGE_STATE(pWork,_menuFriendPokemon);
      }
      else{
        int msg[]={POKETRADE_STR_05, POKETRADE_STR_04, POKETRADE_STR_06};
        POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
#if PM_DEBUG
        if(!GFL_NET_IsInit()){
          target = 5 - trgno;
          POKE_MAIN_Pokemonset(pWork, target/GTS_NEGO_POKESLT_MAX, pWork->GTSSelectPP[target/GTS_NEGO_POKESLT_MAX][target%GTS_NEGO_POKESLT_MAX]);
        }
#endif
        _CHANGE_STATE(pWork,_menuMyPokemon);
      }
      POKE_MAIN_Pokemonset(pWork, trgno/GTS_NEGO_POKESLT_MAX, pWork->GTSSelectPP[trgno/GTS_NEGO_POKESLT_MAX][trgno%GTS_NEGO_POKESLT_MAX]);
      _CHANGE_STATE(pWork,_menuMyPokemon);
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   6体表示
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_Select6Init(POKEMON_TRADE_WORK* pWork)
{
  GFL_BMPWIN* pWin;
  int side,poke;
  MYSTATUS* aStBuf[2];

  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_POKETRADE, pWork->heapID );
	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_trade_wb_gts_bg05_NSCR,
																				 GFL_BG_FRAME0_S, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar1), 0, 0,
																				 pWork->heapID);
  GFL_BG_SetPriority( GFL_BG_FRAME0_S , 3 );
  GFL_BG_SetVisible( GFL_BG_FRAME0_S , TRUE );
  GFL_BG_SetVisible( GFL_BG_FRAME3_S , FALSE );
	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_trade_wb_trade_bg01_back_NSCR,
																				 GFL_BG_FRAME2_M, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);
	GFL_ARC_CloseDataHandle( p_handle );
  POKE_GTS_ReleasePokeIconResource(pWork);
  POKE_GTS_CreatePokeIconResource(pWork, CLSYS_DRAW_SUB);

  GFL_BG_ClearScreenCodeVReq(GFL_BG_FRAME1_S,0);

  //GFL_BG_SetVisible( GFL_BG_FRAME1_S , FALSE );

  IRC_POKETRADE_ResetBoxNameWindow(pWork);


  IRC_POKETRADE_AllDeletePokeIconResource(pWork);
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], FALSE );

  if(pWork->StatusWin[0]){
    GFL_BMPWIN_ClearScreen(pWork->StatusWin[0]);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
    GFL_BMPWIN_Delete(pWork->StatusWin[0]);
    pWork->StatusWin[0]=NULL;
  }


  POKETRADE_MESSAGE_SixStateDisp(pWork); 
  
  pWork->bgscroll=0;
  pWork->bgscrollRenew = TRUE;
  //GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );
  GFL_BG_SetVisible( GFL_BG_FRAME1_S , TRUE );

  _CHANGE_STATE(pWork,_select6keywait);


}


