//=============================================================================
/**
 * @file	  pokemontrade_nego.c
 * @brief	  ポケモン交換して通信を終了する ネゴシエーションのフロー
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
#include "system/wipe.h"

#include "item/item.h"
#include "app/app_menu_common.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"

#include "pokemontrade_local.h"
#include "app_menu_common.naix"

#define KARI_EN (1)  //スライド演出がはいるまでかり

#define _GTSINFO03_WAIT (60*2)
#define _GTSINFO04_WAIT (60*2)
#define _GTSINFO12_WAIT (60*2)
#define _GTSINFO13_WAIT (60*2)


//static void _pokeNickNameMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,BOOL bEgg,POKEMON_TRADE_WORK* pWork);
//static void _pokeLvMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork);
//static void _pokeSexMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork);
static void _Select6MessageInitEnd(POKEMON_TRADE_WORK* pWork);
static void _pokemonStatusStart(POKEMON_TRADE_WORK* pWork);
static void _menuMyPokemonMenu(POKEMON_TRADE_WORK* pWork);
static void _NEGO_Select6CancelWait3(POKEMON_TRADE_WORK* pWork);


//６体のポケモン選択
static GFL_UI_TP_HITTBL _tp_data[]={
  { 3*8, 7*8, 1*8, 15*8 },
  { 9*8, 13*8, 1*8, 15*8 },
  { 15*8, 19*8, 1*8, 15*8 },
  { 3*8, 7*8, 17*8, 31*8 },
  { 9*8, 13*8, 17*8, 31*8 },
  { 15*8, 19*8, 17*8, 31*8 },
  {GFL_UI_TP_HIT_END,0,0,0},
};

//GTS用遅延タイマー
static BOOL CheckNegoWaitTimer(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->NegoWaitTime!=0){
    if(POKEMONTRADEPROC_IsTimeWaitSelect(pWork)){  //つまりGTSネゴだったら
      pWork->NegoWaitTime--;
    }
    else{
      pWork->NegoWaitTime=0;
    }
  }
  if(pWork->NegoWaitTime==0){
    return TRUE;
  }
  return FALSE;
}

void POKE_GTS_EndWork(POKEMON_TRADE_WORK* pWork)
{
  int i,j;
  for(j = 0;j < GTS_PLAYER_WORK_NUM;j++){
    for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
      if(pWork->GTSSelectPP[j][i]){
        GFL_HEAP_FreeMemory(pWork->GTSSelectPP[j][i]);
        pWork->GTSSelectPP[j][i]=NULL;
      }
    }
  }

}

void POKE_GTS_InitWork(POKEMON_TRADE_WORK* pWork)
{
  int i,j;
  for(j = 0;j < GTS_PLAYER_WORK_NUM;j++){
    for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
      pWork->GTSSelectIndex[j][i] = -1;
      pWork->GTSSelectBoxno[j][i] = -1;
    }
  }

}

//自分のPOKEがいるかどうか確認する
BOOL POKE_GTS_IsMyIn(POKEMON_TRADE_WORK* pWork)
{
  int i;
  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
      if((pWork->GTSSelectIndex[0][i] != -1) && (pWork->GTSSelectBoxno[0][i] != -1)){
        return TRUE;
      }
    }
  }
  else{
    if((pWork->selectIndex != -1) && (pWork->selectBoxno != -1)){
      return TRUE;
    }
  }
  return FALSE;
}



// GTSPOKEを選択してるかどうか判断する
int POKE_GTS_IsSelect(POKEMON_TRADE_WORK* pWork,int boxno,int index)
{
  int i,j;
  
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if((pWork->GTSSelectIndex[0][i] == index) &&   (pWork->GTSSelectBoxno[0][i] == boxno)){
      return i;
    }
  }
  return -1;
}



//ダイレクトにポケモン追加 相手用 
void POKE_GTS_DirectAddPokemon(POKEMON_TRADE_WORK* pWork,int index,const POKEMON_PARAM* pp)
{
  

  pWork->GTSSelectIndex[1][index] = index;
  pWork->GTSSelectBoxno[1][index] = index;
  if(pWork->GTSSelectPP[1][index]==NULL){
    pWork->GTSSelectPP[1][index] = GFL_HEAP_AllocClearMemory( pWork->heapID, POKETOOL_GetWorkSize() );
  }
  GFL_STD_MemCopy(pp,pWork->GTSSelectPP[1][index],POKETOOL_GetWorkSize());
}

//ポケモン追加
static int _addPokemon(POKEMON_TRADE_WORK* pWork,int side,int index,int boxno,const POKEMON_PARAM* pp)
{
  int i;
  
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if((pWork->GTSSelectIndex[side][i] == -1) || (i==GTS_NEGO_POKESLT_MAX-1)){
      OS_TPrintf("代入 %d %d\n",boxno,index);

      pWork->GTSSelectIndex[side][i] = index;
      pWork->GTSSelectBoxno[side][i] = boxno;
      if(pWork->GTSSelectPP[side][i]==NULL){
        pWork->GTSSelectPP[side][i] = GFL_HEAP_AllocClearMemory( pWork->heapID, POKETOOL_GetWorkSize() );
      }
      GFL_STD_MemCopy(pp,pWork->GTSSelectPP[side][i],POKETOOL_GetWorkSize());
      return i;
    }
  }
  return -1;  //帰ることは無い
}





//ポケモン削除
void POKE_GTS_DeletePokemonDirect(POKEMON_TRADE_WORK* pWork,int side,int index)
{
  pWork->GTSSelectIndex[side][index] = -1;
  pWork->GTSSelectBoxno[side][index] = -1;
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
  int pal;
  
  if(mainsub==CLSYS_DRAW_SUB){
    pal = _OBJPLT_POKEICON_OFFSET;
  }
  else{
    pal = _OBJPLT_GTS_POKEICON_OFFSET_M;
  }

  pWork->cellRes[PLT_GTS_POKEICON] =
    GFL_CLGRP_PLTT_RegisterComp( arcHandlePoke ,
                                 POKEICON_GetPalArcIndex() , mainsub ,
                                 pal , pWork->heapID  );
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


void POKE_GTS_ReleasePokeIconResource(POKEMON_TRADE_WORK* pWork)
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


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン登録＋相手に情報転送
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL POKE_GTS_PokemonsetAndSendData(POKEMON_TRADE_WORK* pWork,int index,int boxno)
{ //選択ポケモン表示
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
  POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->selectBoxno, pWork->selectIndex,pWork);
  POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);
  int no;

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    no = _addPokemon(pWork,0,index,boxno, pp);
    if( FALSE == GFL_NET_SendDataEx(pNet, GFL_NET_SENDID_ALLUSER, _NETCMD_THREE_SELECT1+no,
                                    POKETOOL_GetWorkSize(),pWork->GTSSelectPP[0][no],
                                    FALSE, FALSE, TRUE)){
      POKE_GTS_DeletePokemonDirect(pWork,0,no);  //取り消し
      GFL_HEAP_FreeMemory(pp);
      return FALSE;
    }
  }
  else{
    no = _addPokemon(pWork,0,index,boxno, pp);
  }
  POKETRADE_2D_GTSPokemonIconSet(pWork,0,no,pp,TRUE);
#if PM_DEBUG
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    no = _addPokemon(pWork, 1, index,boxno, pp);
    POKETRADE_2D_GTSPokemonIconSet(pWork,1,no,pp,TRUE);
  }
#endif
  GFL_HEAP_FreeMemory(pp);

//  if(pWork->pAppTask==NULL){
  //  int msg[]={POKETRADE_STR2_26};
    //POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
//  }

  return TRUE;
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモンネゴシエーション ２人の説明を表示
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
    
  GFL_ARC_UTIL_TransVramPalette( APP_COMMON_GetArcId() , 
                                 NARC_app_menu_common_task_menu_NCLR ,
                                 PALTYPE_MAIN_BG , _MAINBG_APP_MSG_PAL*32 , 32*1 , pWork->heapID );	
//  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
  //                              0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  
  if(pWork->GTSInfoWindow){
    GFL_BMPWIN_Delete(pWork->GTSInfoWindow);
    GFL_BMPWIN_Delete(pWork->GTSInfoWindow2);
  }
  pWork->GTSInfoWindow = GFL_BMPWIN_Create(frame,	0, 0, 32, 2,	_MAINBG_APP_MSG_PAL, GFL_BMP_CHRAREA_GET_F);

  pWin = pWork->GTSInfoWindow;
  GFL_FONTSYS_SetColor( 0xe, 0xf, 0x0 );

  if(pWork->type == POKEMONTRADE_TYPE_GTSNEGO){
    GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_31 , pWork->pStrBuf );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), 8,  2, pWork->pStrBuf, pWork->pFontHandle);
    GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_22 + pWork->GTSlv[0], pWork->pStrBuf );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), 128+8,  2, pWork->pStrBuf, pWork->pFontHandle);
  }

  GFL_BMPWIN_TransVramCharacter(pWin);
  GFL_BMPWIN_MakeScreen(pWin);
  
  pWork->GTSInfoWindow2 = GFL_BMPWIN_Create(frame,	0, 3, 32, 8,	_POKEMON_MAIN_FRIENDGIVEMSG_PAL, GFL_BMP_CHRAREA_GET_F);
  pWin = pWork->GTSInfoWindow2;

  for(side = 0 ; side < GTS_PLAYER_WORK_NUM; side++){
    if(aStBuf[side]){
      GFL_FONTSYS_SetColor( 1, 2, 0x0 );
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_16, pWork->pExStrBuf );
      WORDSET_RegisterPlayerName( pWork->pWordSet, 0,  aStBuf[side]  );
      WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), sidex[side]*8, 2, pWork->pStrBuf, pWork->pFontHandle);

      if(pWork->type == POKEMONTRADE_TYPE_GTSNEGO){
        GFL_FONTSYS_SetColor( 3, 4, 0x0 );
        GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_17 + pWork->GTStype[side], pWork->pStrBuf );
        PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), sidex[side]*8+4, 32+2, pWork->pStrBuf, pWork->pFontHandle);
      }

    }
  }
  GFL_BMPWIN_TransVramCharacter(pWin);
  GFL_BMPWIN_MakeScreen(pWin);
  GFL_BG_LoadScreenV_Req( frame );
}



//------------------------------------------------------------------------------
/**
 * @brief   ポケモンネゴシエーション ２人の説明を表示
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_SelectStatusMessageDisp(POKEMON_TRADE_WORK* pWork, int side, BOOL bSelected)
{
  int sidex[] = {5, 21};
  int i=0,lv;
  int frame = GFL_BG_FRAME3_M;
  GFL_BMPWIN* pWin;

    
//  GFL_ARC_UTIL_TransVramPalette( APP_COMMON_GetArcId() , 
  //                               NARC_app_menu_common_task_menu_NCLR ,
    //                             PALTYPE_MAIN_BG , _MAINBG_APP_MSG_PAL*32 , 32*1 , pWork->heapID );	
//  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
  //                              0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  
  if(pWork->StatusWin[side]){
    GFL_BMPWIN_Delete(pWork->StatusWin[side]);
  }
  pWork->StatusWin[side] = GFL_BMPWIN_Create(frame,	sidex[side], 16, 14, 2,	_POKEMON_MAIN_FRIENDGIVEMSG_PAL, GFL_BMP_CHRAREA_GET_F);

  pWin = pWork->StatusWin[side];
  GFL_FONTSYS_SetColor( 5, 6, 0x0 );

  GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR2_14 + bSelected, pWork->pStrBuf );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), 0, 0, pWork->pStrBuf, pWork->pFontHandle);
  
  GFL_BMPWIN_TransVramCharacter(pWin);
  GFL_BMPWIN_MakeScreen(pWin);
  GFL_BG_LoadScreenV_Req( frame );
}



//------------------------------------------------------------------------------
/**
 * @brief   ポケモンネゴシエーション ２人の説明を消去
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------
void POKE_GTS_SelectStatusMessageDelete(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->GTSInfoWindow){
    GFL_BMPWIN_ClearScreen(pWork->StatusWin[0]);
    GFL_BMPWIN_ClearScreen(pWork->StatusWin[1]);
    GFL_BMPWIN_ClearScreen(pWork->GTSInfoWindow);
    GFL_BMPWIN_ClearScreen(pWork->GTSInfoWindow2);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
    GFL_BMPWIN_Delete(pWork->StatusWin[0]);
    GFL_BMPWIN_Delete(pWork->StatusWin[1]);
    GFL_BMPWIN_Delete(pWork->GTSInfoWindow);
    GFL_BMPWIN_Delete(pWork->GTSInfoWindow2);
    pWork->StatusWin[0]=NULL;
    pWork->StatusWin[1]=NULL;
    pWork->GTSInfoWindow=NULL;
    pWork->GTSInfoWindow2=NULL;
  }
}


static void _menuFriendPokemon(POKEMON_TRADE_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);  //下のメッセージを消す
    pWork->pAppTask=NULL;
    switch(selectno){
    case 0:  //つよさをみる
      _CHANGE_STATE(pWork, _pokemonStatusStart);
      break;
    case 1:  //もどる
      _CHANGE_STATE(pWork, POKETRADE_NEGO_Select6keywait);
      break;
    }
  }
}

static void _menuFriendPokemonStart(POKEMON_TRADE_WORK* pWork)
{

  int msg[]={ POKETRADE_STR_04, POKETRADE_STR_06};
  POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
    TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
  _CHANGE_STATE(pWork, _menuFriendPokemon);
}



//交換選択待ち
static void _changePokemonSendData(POKEMON_TRADE_WORK* pWork)
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
    case 1:  //もどる

      if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
        pWork->userNetCommand[0]=0;
        pWork->userNetCommand[1]=0;
      }
      else{
        int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
        pWork->userNetCommand[myID]=0;
        GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CANCEL_POKEMON,0,NULL);
      }
      _CHANGE_STATE(pWork, POKETRADE_NEGO_Select6keywait);
      break;
    }
  }
}



static void _networkFriendsStandbyWait2(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_05, POKETRADE_STR_06};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  }
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);

  _CHANGE_STATE(pWork,_changePokemonSendData);


}


static void _networkFriendsStandbyWait(POKEMON_TRADE_WORK* pWork)
{
  int i;
  int msgno;
  int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  int targetID = 1 - myID;

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(!CheckNegoWaitTimer(pWork)){
    return;
  }
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_D, WB_NET_TRADE_SERVICEID)){
      return;
    }
  }

  if((pWork->changeFactor[myID]==POKETRADE_FACTOR_TRI_DECIDE) &&
     (pWork->changeFactor[targetID]==POKETRADE_FACTOR_TRI_DECIDE)){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_20, pWork->pMessageStrBufEx );
    for(i=0;i<2;i++){
      POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, i);
      WORDSET_RegisterPokeNickName( pWork->pWordSet, i,  pp );
    }
    WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx  );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork,_networkFriendsStandbyWait2);
    return;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_TRI_END){
    msgno = POKETRADE_STR_97;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_TRI_END){
    msgno = POKETRADE_STR2_04;
  }
  GFL_MSG_GetString( pWork->pMsgData, msgno, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->changeFactor[0] = POKETRADE_FACTOR_NONE;
  pWork->changeFactor[1] = POKETRADE_FACTOR_NONE;
  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID);
  _CHANGE_STATE(pWork, _NEGO_Select6CancelWait3);  //@todo


  


#if 0
  if(pWork->userNetCommand[targetID]==_NETCMD_LOOKATPOKE){
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_13, pWork->pMessageStrBufEx );
    for(i=0;i<2;i++){
      POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, i);
      WORDSET_RegisterPokeNickName( pWork->pWordSet, i,  pp );
    }
    WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx  );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork,_networkFriendsStandbyWait2);
  }

  if(pWork->pokemonGTSSeq == POKEMONTORADE_SEQ_SELECTNG){  //キャンセル時
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_14, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    pWork->pokemonGTSSeqSend = POKEMONTORADE_SEQ_SELECTNG;
    _CHANGE_STATE(pWork,_NEGO_Select6CancelWait3);
  }
#endif
}





static void _networkFriendsStandbyWait_Send(POKEMON_TRADE_WORK* pWork)
{
  ///@todo
  u8 cmd = POKETRADE_FACTOR_TRI_DECIDE;
  
  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd)){
    GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_D,WB_NET_TRADE_SERVICEID );
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
  }
}

//交換選択通信送信

static void _friendSelectWait(POKEMON_TRADE_WORK* pWork)
{

#if PM_DEBUG
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    POKE_MAIN_Pokemonset(pWork, 1, pWork->GTSSelectPP[1][0]);
    {
      const STRCODE *name;
      POKEMON_PARAM* pp2; 

      pp2 = PP_Create(MONSNO_ONOKKUSU, 100, 123456, GFL_HEAPID_APP);
      name = MyStatus_GetMyName( pWork->pMy );
      PP_Put( pp2 , ID_PARA_oyaname_raw , (u32)name );
      PP_Put( pp2 , ID_PARA_oyasex , MyStatus_GetMySex( pWork->pMy ) );
      POKE_MAIN_Pokemonset(pWork,1,pp2); 
      GFL_STD_MemCopy(pp2,pWork->recvPoke[1],POKETOOL_GetWorkSize());
      GFL_HEAP_FreeMemory(pp2);
    }
  }
#endif
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    pWork->userNetCommand[0]=_NETCMD_LOOKATPOKE;
    pWork->userNetCommand[1]=_NETCMD_LOOKATPOKE;
  }

  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->NegoWaitTime = _GTSINFO13_WAIT;

  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
  }
  else{
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait_Send);// あいても選ぶまで待つ
  }
}

// ポケモンのステータス表示待ち
static void _changePokemonStatusDispAuto(POKEMON_TRADE_WORK* pWork,int sel)
{
  POKEMON_PARAM* pp = pWork->GTSSelectPP[1-(sel/GTS_NEGO_POKESLT_MAX)][sel%GTS_NEGO_POKESLT_MAX]; //さかさまにする

  if(POKEMONTRADE_IsInPokemonRecvPoke(pp)){
//    pWork->pokemonselectno
    int mcssno = 1-(sel/GTS_NEGO_POKESLT_MAX);
    POKETRADE_MESSAGE_ChangePokemonStatusDisp(pWork, pp, mcssno, TRUE);
  }
}

// ポケモンのステータス表示待ち
static BOOL _pokemonStatusKeyLoop(POKEMON_TRADE_WORK* pWork)
{
  BOOL curSel=FALSE;
  
  // キー処理
  if(GFL_UI_KEY_GetTrg() == PAD_KEY_RIGHT){
    pWork->pokemonselectno = (pWork->pokemonselectno+3) % 6;
    curSel=TRUE;
  }
  if(GFL_UI_KEY_GetTrg() == PAD_KEY_LEFT){
    pWork->pokemonselectno = (pWork->pokemonselectno-3) % 6;
    if(pWork->pokemonselectno<0){
      pWork->pokemonselectno+=6;
    }
    curSel=TRUE;
  }
  if(GFL_UI_KEY_GetTrg() == PAD_KEY_UP){
    if((pWork->pokemonselectno==0) || (pWork->pokemonselectno==3)){
      pWork->pokemonselectno = pWork->pokemonselectno+3;
    }
    pWork->pokemonselectno = pWork->pokemonselectno-1;
    curSel=TRUE;
  }
  if(GFL_UI_KEY_GetTrg() == PAD_KEY_DOWN){
    if((pWork->pokemonselectno==2) || (pWork->pokemonselectno==5)){
      pWork->pokemonselectno = pWork->pokemonselectno-3;
    }
    pWork->pokemonselectno = pWork->pokemonselectno+1;
    curSel=TRUE;
  }
  return curSel;
}


// ポケモンのステータス表示待ち
static void _pokemonStatusWaitN(POKEMON_TRADE_WORK* pWork)
{
  BOOL bReturn=FALSE;
  int tpno;
  
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );

  tpno = GFL_UI_TP_HitTrg(_tp_data);
  if(-1!=tpno){   //タッチ処理
    GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
    pWork->pokemonselectno = tpno;
    _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno);
  }

  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    if(GFL_UI_KEY_GetTrg()){
      GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
      POKEMONTRADE_StartPokeSelectSixButton(pWork,pWork->pokemonselectno);
      PMSND_PlaySystemSE(SEQ_SE_SELECT1);
      return;
    }
  }

  if(_pokemonStatusKeyLoop(pWork)){
    PMSND_PlaySystemSE(POKETRADESE_CUR);
    POKEMONTRADE_StartPokeSelectSixButton(pWork,pWork->pokemonselectno);
    _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno);
  }

  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    bReturn=TRUE;
    break;
  default:
    break;
  }

  if(bReturn){
    // 消す
    GFL_BG_SetVisible( GFL_BG_FRAME3_M , FALSE );

    POKETRADE_MESSAGE_ResetPokemonStatusMessage(pWork);
    {
      ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_POKETRADE, pWork->heapID );
      pWork->subchar =
        GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_trade_wb_trade_bg01_NCGR,
                                                     GFL_BG_FRAME2_M, 0, 0, pWork->heapID);
      GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
                                           NARC_trade_wb_trade_bg01_back_NSCR,
                                           GFL_BG_FRAME2_M, 0,
                                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                           pWork->heapID);
      GFL_ARC_CloseDataHandle(p_handle);
    }
    IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
   // G2_BlendNone();

/*
    if(POKEMONTRADE_IsInPokemonRecvPoke(IRC_POKEMONTRADE_GetRecvPP(pWork, 0))){
      POKE_MAIN_Pokemonset(pWork, 0, IRC_POKEMONTRADE_GetRecvPP(pWork, 0));

      //pWork,pWork->pokemonselectno
    }
*/

   
    if(POKEMONTRADE_IsInPokemonRecvPoke(IRC_POKEMONTRADE_GetRecvPP(pWork, 1))){
      POKE_MAIN_Pokemonset(pWork, 1, IRC_POKEMONTRADE_GetRecvPP(pWork, 1));
    }
    if(pWork->pokemonselectno/GTS_NEGO_POKESLT_MAX){
      _CHANGE_STATE(pWork, _menuFriendPokemonStart); //交換含まない
    }
    else{
      _CHANGE_STATE(pWork, _menuMyPokemonMenu); //交換含む
    }
    
  }
}


// ポケモンのステータス表示
static void _pokemonStatusStart(POKEMON_TRADE_WORK* pWork)
{
  u16 trgno = pWork->pokemonselectno;
  
  POKEMON_PARAM* pp = pWork->GTSSelectPP[1-(trgno/GTS_NEGO_POKESLT_MAX)][trgno%GTS_NEGO_POKESLT_MAX]; //さかさまにする

  if(pp && PP_Get(pp,ID_PARA_poke_exist,NULL) ){
    POKETRADE_MESSAGE_CreatePokemonParamDisp(pWork,pp);
  }
  _CHANGE_STATE(pWork, _pokemonStatusWaitN);
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
      _CHANGE_STATE(pWork, _friendSelectWait);//POKETRE_MAIN_ChangePokemonSendDataNetwork);
      break;
    case 1:  //つよさをみる
      _CHANGE_STATE(pWork, _pokemonStatusStart);
      break;
    case 2:  //もどる
      _CHANGE_STATE(pWork, POKETRADE_NEGO_Select6keywait);
      break;
    }
  }
}


//交換するのかメニューを出す
static void _menuMyPokemonMenu(POKEMON_TRADE_WORK* pWork)
{
  {    
    int msg[]={POKETRADE_STR_05, POKETRADE_STR_04, POKETRADE_STR_06};
    POKETRADE_MESSAGE_AppMenuOpen(pWork,msg,elementof(msg));

    TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
    
    _CHANGE_STATE(pWork,_menuMyPokemon);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   選択ポケモンを送る
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _menuMyPokemonSend(POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON,
                         POKETOOL_GetWorkSize(), IRC_POKEMONTRADE_GetRecvPP(pWork,0))){
      return;
    }
  }
  _CHANGE_STATE(pWork,_menuMyPokemonMenu);
}


//------------------------------------------------------------------------------
/**
 * @brief   6体表示まちの時のポケモン選択
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _NEGO_Select6PokemonSelect(POKEMON_TRADE_WORK* pWork, int trgno)
{
  if(trgno != -1){
    POKEMON_PARAM* pp = pWork->GTSSelectPP[1-(trgno/GTS_NEGO_POKESLT_MAX)][trgno%GTS_NEGO_POKESLT_MAX]; //さかさまにする
    if(pp && PP_Get(pp,ID_PARA_poke_exist,NULL)){
      pWork->pokemonselectno = trgno;

      PMSND_PlaySystemSE(SEQ_SE_DECIDE1);
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);

      
      if(trgno/GTS_NEGO_POKESLT_MAX){  //こちら側は自分のポケモンが入っている 自分のはみるだけ
        _CHANGE_STATE(pWork,_menuFriendPokemonStart);
      }
      else{
        POKE_MAIN_Pokemonset(pWork, trgno/GTS_NEGO_POKESLT_MAX, pp);
        GFL_STD_MemCopy(pp, IRC_POKEMONTRADE_GetRecvPP(pWork,0) ,POKETOOL_GetWorkSize());
        _CHANGE_STATE(pWork, _menuMyPokemonSend);
      }
      return TRUE;
    }
  }
  return FALSE;
}



//フェードをかけて終了
static void _NEGO_Select6CancelWait5(POKEMON_TRADE_WORK* pWork)
{
  if(WIPE_SYS_EndCheck()){
    POKMEONTRADE_RemoveCoreResource(pWork);

//    GFL_CLACT_UNIT_Delete(pWork->cellUnit);
    IRC_POKETRADE_SetBgMode(SETUP_TRADE_BG_MODE_NORMAL);
    IRC_POKETRADE_SetSubVram(pWork);
    pWork->cellUnit = GFL_CLACT_UNIT_Create( 340 , 0 , pWork->heapID );
    POKETRADE_MESSAGE_HeapInit(pWork);
    IRC_POKETRADE_InitBoxCursor(pWork);  // タスクバー
    IRC_POKETRADE_CreatePokeIconResource(pWork);  // ポケモンアイコンCLACT+リソース常駐化
    IRC_POKETRADE_GraphicInitMainDisp(pWork);
    IRC_POKETRADE_GraphicInitSubDisp(pWork);
    IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
    POKETRADE_TOUCHBAR_Init(pWork);
    GFL_NET_ReloadIcon();
    GXS_SetVisibleWnd( GX_WNDMASK_NONE );

    _CHANGE_STATE(pWork,IRC_POKMEONTRADE_ChangeFinish);
  }
}


//終了を待つ
static void _NEGO_Select6CancelWait3(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  _CHANGE_STATE(pWork, _NEGO_Select6CancelWait5);
}



static void _NEGO_Select6CancelWait22(POKEMON_TRADE_WORK* pWork)
{
  u8 cmd = POKETRADE_FACTOR_TRI_END;

  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd)){
    GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_D,WB_NET_TRADE_SERVICEID );
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
  }
}

//見せ合いをやめて戻る

static void _NEGO_Select6CancelWait2(POKEMON_TRADE_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);
    pWork->pAppTask=NULL;

    switch(selectno){
    case 0:  //はい
      //友達の選択をまっていますメッセージ
      GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      pWork->NegoWaitTime = _GTSINFO04_WAIT;
      _CHANGE_STATE(pWork, _NEGO_Select6CancelWait22);
      break;
    default: //いいえ
      POKETRADE_MESSAGE_WindowClear(pWork);
      _CHANGE_STATE(pWork, POKETRADE_NEGO_Select6keywait);
      break;
    }
  }
}


//見せ合いをやめて戻るかどうか はいいいえ
static void _NEGO_Select6CancelWait(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_27, POKETRADE_STR_28};
    POKETRADE_MESSAGE_AppMenuOpenCustom(pWork, msg, elementof(msg), 32, 12);
    _CHANGE_STATE(pWork, _NEGO_Select6CancelWait2 );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   6体表示まち
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKETRADE_NEGO_Select6keywait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bTouch=FALSE;
  int trgno,target;
  BOOL curSel = FALSE;

  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, TRUE);
  
  trgno = GFL_UI_TP_HitTrg(_tp_data);
  if(trgno != -1){
    GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
    POKEMONTRADE_StartPokeSelectSixButton(pWork, -1);
    if(_NEGO_Select6PokemonSelect(pWork, trgno)){
      return;
    }
  }
  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    if(GFL_UI_KEY_GetTrg()){
      GFL_UI_SetTouchOrKey(GFL_APP_END_KEY);
      pWork->pokemonselectno = pWork->pokemonselectno % 6;
      if(pWork->pokemonselectno<0){
        pWork->pokemonselectno=0;
      }
      POKEMONTRADE_StartPokeSelectSixButton(pWork,pWork->pokemonselectno);
      PMSND_PlaySystemSE(SEQ_SE_SELECT1);
      return;
    }
  }

  if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE){
    if(_NEGO_Select6PokemonSelect(pWork, pWork->pokemonselectno)){
      return ;
    }
  }

  // キー処理
  if(_pokemonStatusKeyLoop(pWork)){
    POKEMONTRADE_StartPokeSelectSixButton(pWork,pWork->pokemonselectno);
    PMSND_PlaySystemSE(SEQ_SE_SELECT1);
  }
  if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL){
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_15, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork, _NEGO_Select6CancelWait );
    return;
  }
  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_15, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork, _NEGO_Select6CancelWait );
    break;
  default:
    break;
  }


}


static void POKETRADE_NEGO_Select6keywaitMsg2(POKEMON_TRADE_WORK* pWork)
{
  if(!CheckNegoWaitTimer(pWork)){
    return;
  }
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    POKETRADE_MESSAGE_WindowClear(pWork);

#if KARI_EN
//    POKETRADE_MESSAGE_SixStateDisp(pWork , GFL_BG_FRAME2_S);//６体表示
#endif

    _CHANGE_STATE(pWork,POKETRADE_NEGO_Select6keywait);
  }
}



static void POKETRADE_NEGO_Select6keywaitMsg(POKEMON_TRADE_WORK* pWork)
{

  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_12, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->NegoWaitTime = _GTSINFO12_WAIT;
//  _CHANGE_STATE(pWork,_Select6MessageInit6);

  _CHANGE_STATE(pWork,POKETRADE_NEGO_Select6keywaitMsg2);
}


static void POKETRADE_NEGO_Select6keywaitMsgFade(POKEMON_TRADE_WORK* pWork)
{
  if(WIPE_SYS_EndCheck()){
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ , -8 );
    if(1){
      G2S_SetWnd0InsidePlane(
        GX_WND_PLANEMASK_BG0|
        GX_WND_PLANEMASK_BG1|
        GX_WND_PLANEMASK_BG2|
        GX_WND_PLANEMASK_BG3|
        GX_WND_PLANEMASK_OBJ,
        FALSE );
      G2S_SetWnd0Position( 0, 0, 128, 192-24+1 );
      G2S_SetWndOutsidePlane(
        GX_WND_PLANEMASK_BG0|
        GX_WND_PLANEMASK_BG1|
        GX_WND_PLANEMASK_BG2|
        GX_WND_PLANEMASK_BG3|
        GX_WND_PLANEMASK_OBJ,
        TRUE );
      GXS_SetVisibleWnd( GX_WNDMASK_W0 );
    }
    GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S , TRUE );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S , TRUE );
    _CHANGE_STATE(pWork,POKETRADE_NEGO_Select6keywaitMsg);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   6体表示
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _Select6Init(POKEMON_TRADE_WORK* pWork)
{
  GFL_BMPWIN* pWin;
  int side,poke;
  MYSTATUS* aStBuf[2];

//  pWork->bgscrollRenew=FALSE;  //スクロールはここからはしないようにFALSEに念のためしておく
  IRC_POKETRADE_SubGraphicExit(pWork);

  POKE_GTS_DeleteFaceIcon(pWork);
  POKE_GTS_DeleteEruptedIcon(pWork);
  POKETRADE_2D_GTSPokemonIconResetAll( pWork );
  POKE_GTS_ReleasePokeIconResource(pWork);
 // if(pWork->pTouchWork){
   // TOUCHBAR_Exit(pWork->pTouchWork);
  //  pWork->pTouchWork=NULL;
//  }
  

  IRC_POKETRADE_ResetBoxNameWindow(pWork);
  POKE_GTS_SelectStatusMessageDelete(pWork);

  IRC_POKETRADE_AllDeletePokeIconResource(pWork);
  
  POKE_GTS_CreatePokeIconResource(pWork, CLSYS_DRAW_SUB);

  GFL_BG_ClearScreenCodeVReq(GFL_BG_FRAME2_S,0);


  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], FALSE );
//  POKETRADE_TOUCHBAR_Init(pWork);

  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, TRUE);
  TOUCHBAR_SetActive(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, TRUE);


  POKETRADE_MESSAGE_SixStateDisp(pWork , GFL_BG_FRAME2_S);//６体表示
  
  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    POKEMONTRADE_StartPokeSelectSixButton(pWork, -1);
    pWork->pokemonselectno=-1;
  }
  else{
    POKEMONTRADE_StartPokeSelectSixButton(pWork, 0);
    pWork->pokemonselectno=0;
  }
  
  pWork->bgscroll=0;
  pWork->bgscrollRenew = TRUE;
  //GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );

  POKEMONTRADE_NEGOBG_Select6Create(pWork);

  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  
  _CHANGE_STATE(pWork,POKETRADE_NEGO_Select6keywaitMsgFade);


}


//------------------------------------------------------------------------------
//    6体表示確認まち
//------------------------------------------------------------------------------
static void _send6Wait(POKEMON_TRADE_WORK* pWork)
{
  if(WIPE_SYS_EndCheck()){
    POKE_GTS_SelectStatusMessageDisp(pWork, 1, TRUE);
    _CHANGE_STATE(pWork,_Select6Init);
  }
}

//------------------------------------------------------------------------------
//    6体表示確認まち
//------------------------------------------------------------------------------
static void _send5Wait(POKEMON_TRADE_WORK* pWork)
{
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork) || pWork->pokemonThreeSet){   //相手から届いたかどうかいまだとずっとまつ
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
    _CHANGE_STATE(pWork,_send6Wait);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   6体表示OK転送
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_Select6Init(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 flg = 1;
    if( GFL_NET_SendData(pNet,_NETCMD_THREE_SELECT_END,1,&flg)){
      _CHANGE_STATE(pWork, _send5Wait);
    }
  }
  else{
    _CHANGE_STATE(pWork, _send5Wait);
  }
}
///---------------------------------********************************************





static void _Select6MessageInit8(POKEMON_TRADE_WORK* pWork)
{
  int i;
  int msgno;
  int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  int targetID = 1 - myID;


  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_B, WB_NET_TRADE_SERVICEID)){
      return;
    }
  }
  if((pWork->changeFactor[myID]==POKETRADE_FACTOR_TRI_SELECT) &&
     (pWork->changeFactor[targetID]==POKETRADE_FACTOR_TRI_SELECT)){
    //@todo  不正検査    GTSネゴは今度なので今は書かない
    _CHANGE_STATE(pWork, POKE_GTS_Select6Init);
    return;
  }
  else if((pWork->changeFactor[myID]==POKETRADE_FACTOR_END) &&
          (pWork->changeFactor[targetID]==POKETRADE_FACTOR_END)){
    pWork->pParentWork->ret = POKEMONTRADE_MOVE_END;
    _CHANGE_STATE(pWork, POKEMONTRADE_PROC_FadeoutStart);
    return;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_END){
    msgno = POKETRADE_STR_97;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_END){
    msgno = POKETRADE_STR2_04;
  }
  GFL_MSG_GetString( pWork->pMsgData, msgno, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);

  pWork->changeFactor[0] = POKETRADE_FACTOR_NONE;
  pWork->changeFactor[1] = POKETRADE_FACTOR_NONE;

  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID);
  _CHANGE_STATE(pWork, _Select6MessageInitEnd);
}


static void _Select6MessageInit6(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 flg = POKETRADE_FACTOR_TRI_SELECT;
//    if( GFL_NET_SendData(pNet, _NETCMD_GTSSEQNO,1,&flg)){     ////@todo
    if( GFL_NET_SendData(pNet, _NETCMD_CHANGEFACTOR,1,&flg)){     ////@todo
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_B,WB_NET_TRADE_SERVICEID);
      _CHANGE_STATE(pWork, _Select6MessageInit8);
    }
  }
  else{
//    pWork->pokemonGTSSeq=POKEMONTORADE_SEQ_KAISHIOK;
    pWork->changeFactor[0]=POKETRADE_FACTOR_TRI_SELECT;
    pWork->changeFactor[1]=POKETRADE_FACTOR_TRI_SELECT;
    _CHANGE_STATE(pWork, _Select6MessageInit8);
  }
}


static void _Select6MessageInitEnd(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),
                                 POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID)){


      _CHANGE_STATE(pWork, POKETRADE_TouchStateGTS);
    }
  }
  else{
    _CHANGE_STATE(pWork, POKETRADE_TouchStateGTS);
  }
}


static void _Select6MessageInitEndNoSend(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    _CHANGE_STATE(pWork, POKETRADE_TouchStateGTS);
  }
  else if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_RETURN,WB_NET_TRADE_SERVICEID)){
    _CHANGE_STATE(pWork, POKETRADE_TouchStateGTS);
  }
}

static void _Select6MessageInitEndNoSend2(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  _CHANGE_STATE(pWork, POKETRADE_TouchStateGTS);
}



//------------------------------------------------------------------------------
/**
 * @brief   6体表示に移動してよいかどうか確認する
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _Select6MessageInit5(POKEMON_TRADE_WORK* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);
    pWork->pAppTask=NULL;

    switch(selectno){
    case 0:  //はい
      //もう一回選択待ち
      GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      pWork->NegoWaitTime = _GTSINFO04_WAIT;
      _CHANGE_STATE(pWork,_Select6MessageInit6);
      break;
    default: //いいえ
      POKETRADE_MESSAGE_WindowClear(pWork);
//      pWork->pokemonGTSSeqSend = POKEMONTORADE_SEQ_KAISHING;
      _CHANGE_STATE(pWork,_Select6MessageInitEndNoSend2);
      break;
    }
  }

}

//------------------------------------------------------------------------------
/**
 * @brief   6体表示に移動してよいかどうか確認する
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _Select6MessageInit4(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_27, POKETRADE_STR_28};
    POKETRADE_MESSAGE_AppMenuOpenCustom(pWork, msg, elementof(msg), 32, 12);
  }
  _CHANGE_STATE(pWork,_Select6MessageInit5);
}

//------------------------------------------------------------------------------
/**
 * @brief   6体表示に移動してよいかどうか確認する
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _Select6MessageInit3(POKEMON_TRADE_WORK* pWork)
{
  int targetID = 1 - GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  if(CheckNegoWaitTimer(pWork)){

    if( pWork->userNetCommand[targetID] == _NETCMD_END){
      //あいてはポケモン交換したがっているようです でもどす
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_04, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_RETURN,WB_NET_TRADE_SERVICEID);
      _CHANGE_STATE(pWork, _Select6MessageInitEndNoSend);
      return;
    }
    if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
      if(pWork->pokemonGTSSeq==POKEMONTORADE_SEQ_MISERUOK){
        GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_04, pWork->pMessageStrBuf );
        POKETRADE_MESSAGE_WindowOpen(pWork);
        _CHANGE_STATE(pWork,_Select6MessageInit4);
      }
    }
    else{
      GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_04, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      _CHANGE_STATE(pWork,_Select6MessageInit4);
    }
  }
}

static void _Select6MessageInit11(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 flg = POKEMONTORADE_SEQ_MISERUOK;
    if( GFL_NET_SendData(pNet, _NETCMD_GTSSEQNO,1,&flg)){
      _CHANGE_STATE(pWork, _Select6MessageInit3);
    }
  }
  else{
    _CHANGE_STATE(pWork, _Select6MessageInit3);
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   6体表示に移動してよいかどうか確認する
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _Select6MessageInit2(POKEMON_TRADE_WORK* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    POKETRADE_MESSAGE_AppMenuClose(pWork);
    POKETRADE_MESSAGE_WindowClear(pWork);
    pWork->pAppTask=NULL;

    switch(selectno){
    case 0:  //はい
      GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
      POKETRADE_MESSAGE_WindowOpen(pWork);
      pWork->NegoWaitTime = _GTSINFO03_WAIT;
      _CHANGE_STATE(pWork,_Select6MessageInit11);
      break;
    default: //いいえ
      POKETRADE_MESSAGE_WindowClear(pWork);
      _CHANGE_STATE(pWork,POKETRADE_TouchStateGTS);
      break;
    }
  }

}

//------------------------------------------------------------------------------
/**
 * @brief   6体表示に移動してよいかどうか確認する
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _Select6MessageInit1(POKEMON_TRADE_WORK* pWork)
{
  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_27, POKETRADE_STR_28};
    POKETRADE_MESSAGE_AppMenuOpenCustom(pWork, msg, elementof(msg), 32, 12);
  }
  _CHANGE_STATE(pWork,_Select6MessageInit2);
}


//------------------------------------------------------------------------------
/**
 * @brief   6体表示に移動してよいかどうか確認する
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------
void POKE_GTS_Select6MessageInit(POKEMON_TRADE_WORK* pWork)
{
  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_02, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);

  TOUCHBAR_SetActive(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);

  _CHANGE_STATE(pWork,_Select6MessageInit1);

  POKEMON_TRADE_MaskCommon(pWork);



  
}



//------------------------------------------------------------------------------
/**
 * @brief   ポケモンステータスウインドウ用のパレットを合成する
 * @param   line       横順番
 * @param   height     縦順番
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL POKETRADE_NEGO_IsSelect(POKEMON_TRADE_WORK* pWork,int line , int height)
{
  int l,m;
  int boxno = IRC_TRADE_LINE2TRAY(line,pWork);
  int index = IRC_TRADE_LINE2POKEINDEX(line, height);


  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    for(m=0;m<GTS_NEGO_POKESLT_MAX;m++){
      if(pWork->GTSSelectIndex[0][m] != -1){
        if((pWork->GTSSelectIndex[0][m]==index) && (pWork->GTSSelectBoxno[0][m]==boxno)){
          return TRUE;
        }
      }
    }
  }
  else{
    if((pWork->selectIndex==index) && (pWork->selectBoxno==boxno)){
      if((pWork->workPokeIndex==-1) && (pWork->workBoxno==-1)){
        OS_TPrintf("selectIndex %d %d\n",boxno, index);
        return TRUE;
      }
    }
    if((pWork->workPokeIndex==index) && (pWork->workBoxno==boxno)){
      OS_TPrintf("workPokeIndex %d %d\n",boxno, index);
      return TRUE;
    }
  }
  return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   ステータスを見ているかどうか調べる
 * @param   line       横順番
 * @param   height     縦順番
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL POKETRADE_NEGO_IsStatusLookAt(POKEMON_TRADE_WORK* pWork,int line , int height)
{
  int l,m;
  int boxno = IRC_TRADE_LINE2TRAY(line,pWork);
  int index = IRC_TRADE_LINE2POKEINDEX(line, height);

  if((pWork->underSelectIndex==index) && (pWork->underSelectBoxno==boxno)){
    OS_TPrintf("selectIndex %d %d\n",boxno, index);
    return TRUE;
  }
  return FALSE;
}






void POKE_GTS_DeletePokemonState(POKEMON_TRADE_WORK* pWork)
{
  int no = POKE_GTS_IsSelect(pWork,pWork->underSelectBoxno,pWork->underSelectIndex );


  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if( !GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), _NETCMD_THREE_SELECT_CANCEL+no, sizeof(no), &no)){
      return;
    }
  }
  if(no!=-1){
    POKE_GTS_DeletePokemonDirect(pWork, 0, no);
    POKETRADE_2D_GTSPokemonIconReset(pWork,0, no);
  }
  _CHANGE_STATE(pWork, POKETRADE_TouchStateGTS);
}


#define GTSFACEICON_POSX (8)
#define GTSFACEICON_POSY (144)
#define GTSFACEICON_HEIGHT (24)
#define GTSFACEICON_WIDTH (24)


//------------------------------------------------------------------------------
/**
 * @brief   アイコン表示
 * @param   
 * @param   
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_InitFaceIcon(POKEMON_TRADE_WORK* pWork)
{
  int i;

  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    for(i = 0; i < GTS_FACE_BUTTON_NUM;i++){
      POKEMONTRADE_StartFaceButtonGTS(pWork, i);
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   アイコン隠す
 * @param   
 * @param   
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_VisibleFaceIcon(POKEMON_TRADE_WORK* pWork,BOOL bVisible)
{
  int i;

  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    for(i = 0; i < GTS_FACE_BUTTON_NUM;i++){
      POKEMONTRADE_VisibleFaceButtonGTS(pWork, i, bVisible);
    }
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   アイコン消去
 * @param   
 * @param   
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_DeleteFaceIcon(POKEMON_TRADE_WORK* pWork)
{
  int i;

  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    for(i = 0; i < GTS_FACE_BUTTON_NUM;i++){
      POKEMONTRADE_RemoveFaceButtonGTS(pWork, i);
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   アイコンタッチ判定
 * @param   
 * @param   
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_FaceIconFunc(POKEMON_TRADE_WORK* pWork)    //顔アイコンの処理
{
  u32 x,y;
  u8 index;
  int i;

  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    return;
  }
    
  if(GFL_UI_TP_GetPointCont(&x,&y)){     // パネルスクロール
    if(pWork->touchON){
      if((x >=  GTSFACEICON_POSX) && ((GTS_FACE_BUTTON_NUM*GTSFACEICON_WIDTH+GTSFACEICON_POSX) > x)){
        if((y >=  GTSFACEICON_POSY) && ((GTSFACEICON_POSY+GTSFACEICON_HEIGHT) > y)){
          index = (x-GTSFACEICON_POSX)/GTSFACEICON_WIDTH;
          if(index >= GTS_FACE_BUTTON_NUM){
            index = GTS_FACE_BUTTON_NUM-1;
          }
          POKEMONTRADE_TouchFaceButtonGTS( pWork, index);
          POKE_GTS_InitEruptedIcon(pWork,index,0);
          if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
            GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_FACEICON, 1, &index);
          }
        }
      }
    }
  }
  for(i=0;i< 2;i++){
    if(pWork->timerErupted[i]!=0){
      pWork->timerErupted[i]--;
      if(pWork->timerErupted[i]==0){
        POKEMONTRADE_RemoveEruptedGTS(pWork, i);
      }
    }
    
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   アイコン表示
 * @param   
 * @param   
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_InitEruptedIcon(POKEMON_TRADE_WORK* pWork,int faceNo, int index)
{
  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    POKEMONTRADE_StartEruptedGTS(pWork, faceNo, index);
    pWork->timerErupted[index] = _TIMER_ERUPTED_NUM;
    PMSND_PlaySystemSE(SEQ_SE_SELECT3);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   アイコン消去 リソース消去
 * @param   
 * @param   
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_DeleteEruptedIcon(POKEMON_TRADE_WORK* pWork)
{
  int i;
  
  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    for(i = 0; i < 2;i++){
      POKEMONTRADE_RemoveEruptedGTS(pWork, i);
    }
    IRC_POKETRADE_ReleaseMainObj(pWork);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   アイコンリソース初期化
 * @param   
 * @param   
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_InitEruptedIconResource(POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADEPROC_IsTriSelect(pWork)){
    IRC_POKETRADE_InitMainObj(pWork);
  }
}

