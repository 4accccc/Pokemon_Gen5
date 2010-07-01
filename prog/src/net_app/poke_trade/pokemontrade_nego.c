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
#include "net/nhttp_rap_evilcheck.h"
#include "net/dwc_rapcommon.h"
#include "net/dwc_rap.h"

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


#include "system/wipe.h"

#include "item/item.h"
#include "app/app_menu_common.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"

#include "pokemontrade_local.h"
#include "app_menu_common.naix"

#include "net_app/gts_tool.h"
#include "poke_tool/poke_regulation.h"

#define _GTSINFO03_WAIT (60*2)
#define _GTSINFO04_WAIT (60*2)
#define _GTSINFO12_WAIT (60*2)
#define _GTSINFO13_WAIT (60*2)
#define _GTSINFO11_WAIT (60*2)
#define _GTSINFO10_WAIT (60*2)
#define _GTSINFO16_WAIT (60*2)
#define _GTSINFO14_WAIT (60*2)

#define _MESSAGE_TIMER  (60*2)


static void _pokemonStatusStart(POKEMON_TRADE_WORK* pWork);
static void _menuMyPokemonMenu(POKEMON_TRADE_WORK* pWork);
static void _NEGO_Select6CancelWait3(POKEMON_TRADE_WORK* pWork);
static void _select6PokeSubMask(POKEMON_TRADE_WORK* pWork);  //下画面マスク処理
static BOOL POKEMONTRADE_SendPokemon(POKEMON_TRADE_WORK* pWork,POKEMON_PARAM* pp,int commandID);


//６体のポケモン選択
static GFL_UI_TP_HITTBL _tp_data[]={
  {  3*8,  8*8+4, 1*8, 15*8 },
  {  9*8, 14*8+4, 1*8, 15*8 },
  { 15*8, 20*8+4, 1*8, 15*8 },
  {  3*8,  8*8+4, 17*8, 31*8 },
  {  9*8, 14*8+4, 17*8, 31*8 },
  { 15*8, 20*8+4, 17*8, 31*8 },
  {GFL_UI_TP_HIT_END,0,0,0},
};

static void _rapTimingStart(GFL_NETHANDLE* pNet, const u8 no,POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_NET_HANDLE_TimeSyncStart( pNet,no,WB_NET_TRADE_SERVICEID );
  }
}


static void _selectSixButtonResetCheck(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    POKEMONTRADE_StartPokeSelectSixButton(pWork, -1);
//    pWork->pokemonselectno = -1;
  }
  else if(pWork->pokemonselectno != -1){
    POKEMONTRADE_StartPokeSelectSixButton(pWork, pWork->pokemonselectno);
  }
}

//GTSNEGOで交換してはいけないポケモン
BOOL POKE_GTS_BanPokeCheck(POKEMON_TRADE_WORK* pWork, POKEMON_PASO_PARAM* ppp)
{
  int no = PPP_Get( ppp, ID_PARA_monsno, NULL  );
  
  if(pWork->type == POKEMONTRADE_TYPE_GTSNEGO){
    if( GTS_TOOL_IsForbidRibbonPPP( ppp )){
      return TRUE;
    }
    if( PokeRegulationCheckLegend(no) ){
      return TRUE;
    }
  }
  return FALSE;
}


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
      if(pWork->GTSSelectPP[j][i]){
        GFL_STD_MemClear(pWork->GTSSelectPP[j][i], POKETOOL_GetWorkSize());
      }
    }
  }

}

static _pokeGTSSelectNum(POKEMON_TRADE_WORK* pWork)
{
  int i,pokenum=0;
  
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if((pWork->GTSSelectIndex[0][i] != -1) && ( pWork->GTSSelectBoxno[0][i] != -1)){
      pokenum++;
    }
  }
  return pokenum;
}

//
//------------------------------------------------------------------------------
/**
 * @brief   GTSネゴのみ強制切断処理
 * @param   
 * @retval  TRUEが帰ってきたら シーケンスを進めない
 */
//------------------------------------------------------------------------------

BOOL POKE_GTS_CreateCancelButton(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->type == POKEMONTRADE_TYPE_GTSNEGO){  //GTSEネゴのみ強制切断処理
    if(pWork->pAppWin==NULL){
      POKEMONTRADE_MESSAGE_CancelButtonStart(pWork, gtsnego_info_21);
    }
    if(POKEMONTRADE_MESSAGE_ButtonCheck(pWork)){
      POKEMONTRADE_CancelCall(pWork);
      _CHANGE_STATE(pWork,POKEMONTRADE_PROC_FadeoutStart);
      return TRUE;
    }
  }
  return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   GTSネゴのみ強制切断処理
 * @param   
 * @retval  void
 */
//------------------------------------------------------------------------------

void POKE_GTS_DeleteCancelButton(POKEMON_TRADE_WORK* pWork, BOOL bar,BOOL change)
{
  if(pWork->type == POKEMONTRADE_TYPE_GTSNEGO){  //GTSネゴのみ強制切断処理
    POKEMONTRADE_MESSAGE_CancelButtonDelete(pWork, bar, change);
  }
}



//登録前に呼ばれる関数の為一個少ない
BOOL POKE_GTS_IsFullMode(POKEMON_TRADE_WORK* pWork)
{
  BOX_MANAGER * pManager = GAMEDATA_GetBoxManager(pWork->pGameData);
  int selectnum,pokenum;

  selectnum = _pokeGTSSelectNum(pWork);
  pokenum = PokeParty_GetPokeCount(GAMEDATA_GetMyPokemon(pWork->pGameData));
  pokenum += BOXDAT_GetPokeExistCountTotal(pManager);
  
  if((pokenum==2) && (selectnum>=1)){
    return TRUE;
  }
  else if((pokenum>2) && (selectnum>=2)){
    return TRUE;
  }
  return FALSE;
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

//何処に設定して良いかを探す 同じポケモンは-1
int POKEMONTRADE_NEGO_CheckGTSSetPokemon(POKEMON_TRADE_WORK* pWork,int side,int index,int boxno)
{
  int i;
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if((pWork->GTSSelectIndex[side][i] == index) && (pWork->GTSSelectBoxno[side][i] == boxno)){
      return -1;
    }
  }
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if(pWork->GTSSelectIndex[side][i] == -1){
      return i;
    }
  }
  return (GTS_NEGO_POKESLT_MAX-1);
}


//ポケモン追加
static int _addPokemon(POKEMON_TRADE_WORK* pWork,int side,int index,int boxno,const POKEMON_PARAM* pp)
{
  int i;
  
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if((pWork->GTSSelectIndex[side][i] == index) && (pWork->GTSSelectBoxno[side][i] == boxno)){
      return -1;
    }
  }
  
  for(i = 0;i < GTS_NEGO_POKESLT_MAX;i++){
    if((pWork->GTSSelectIndex[side][i] == -1) || (i==GTS_NEGO_POKESLT_MAX-1)){
      OS_TPrintf("代入 %d %d\n",boxno,index);

      pWork->GTSSelectIndex[side][i] = index;
      pWork->GTSSelectBoxno[side][i] = boxno;
      if(pWork->GTSSelectPP[side][i]==NULL){
        pWork->GTSSelectPP[side][i] = GFL_HEAP_AllocClearMemory( pWork->heapID, POKETOOL_GetWorkSize() );
      }
      GFL_STD_MemCopy(pp,pWork->GTSSelectPP[side][i],POKETOOL_GetWorkSize());

      POKEMONTRADE_NEGO_SlideInit(pWork, side, (POKEMON_PARAM*)pp);

      return i;
    }
  }
  return -1;
}





//ポケモン削除
void POKE_GTS_DeletePokemonDirect(POKEMON_TRADE_WORK* pWork,int side,int index)
{
  pWork->GTSSelectIndex[side][index] = -1;
  pWork->GTSSelectBoxno[side][index] = -1;
  if(pWork->GTSSelectPP[side][index]){
    GFL_HEAP_FreeMemory(pWork->GTSSelectPP[side][index]);
  }
  pWork->GTSSelectPP[side][index] = NULL;

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
  POKEMON_PARAM* pp;
  int no;

  pp = POKEMONTRADE_CreatePokeData(pWork->pBox, pWork->selectBoxno, pWork->selectIndex, pWork);

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    no = _addPokemon(pWork,0,index,boxno, pp);
    if(no != -1){
      if( FALSE == POKEMONTRADE_SendPokemon(pWork, pWork->GTSSelectPP[0][no], _NETCMD_THREE_SELECT1+no)){
//      if( FALSE == GFL_NET_SendDataEx(pNet, GFL_NET_SENDID_ALLUSER, _NETCMD_THREE_SELECT1+no,
//                                      POKETOOL_GetWorkSize(),pWork->GTSSelectPP[0][no],
//                                      FALSE, FALSE, TRUE)){
        POKE_GTS_DeletePokemonDirect(pWork,0,no);  //取り消し
        GFL_HEAP_FreeMemory(pp);
        OS_TPrintf("送信失敗\n");
        return FALSE;
      }
      {
        int i=0;
        u8* pChar = (u8*)pWork->GTSSelectPP[0][no];
        OS_TPrintf("s\n");
        for(i=0;i<POKETOOL_GetWorkSize();i++){
          OS_TPrintf("%2x ",pChar[i]);
        }
        OS_TPrintf("\n");
      }
    }
  }
  else{
    no = _addPokemon(pWork,0,index,boxno, pp);
  }
  if(no == -1){
    GFL_HEAP_FreeMemory(pp);
    return TRUE;
  }
  POKETRADE_2D_GTSPokemonIconSet(pWork,0,no,pp,TRUE, FALSE);
#if PM_DEBUG
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    no = _addPokemon(pWork, 1, index,boxno, pp);
    POKETRADE_2D_GTSPokemonIconSet(pWork,1,no,pp,TRUE,TRUE);
  }
#endif
  GFL_HEAP_FreeMemory(pp);

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
  int msgno;
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

  if((pWork->type == POKEMONTRADE_TYPE_GTSNEGO) && (pWork->GTSlv[0]!=-1)){
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
      if(pWork->type == POKEMONTRADE_TYPE_GTSNEGO && (pWork->GTStype[side]!=-1)){
        msgno = POKETRADE_STR2_16;
      }
      else{
        msgno = POKETRADE_STR2_27;
      }
      GFL_FONTSYS_SetColor( 1, 2, 0x0 );
      GFL_MSG_GetString( pWork->pMsgData, msgno, pWork->pExStrBuf );
      WORDSET_RegisterPlayerName( pWork->pWordSet, 0,  aStBuf[side]  );
      WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), sidex[side]*8, 2, pWork->pStrBuf, pWork->pFontHandle);
      
      if((pWork->type == POKEMONTRADE_TYPE_GTSNEGO) && (pWork->GTStype[side]!=-1)){
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

void POKE_GTS_SelectStatusMessageDisp(POKEMON_TRADE_WORK* pWork, int side, BOOL bSelected,BOOL bForce)
{
  int sidex[] = {2, 19};
  int i=0,lv;
  int frame = GFL_BG_FRAME3_M;
  GFL_BMPWIN* pWin;

  if(!bForce){
    if(pWork->pokemonGTSSeq[side]==POKEMONTORADE_SEQ_NONE){
      return;
    }
    if(!pWork->pokemonGTSStateMode){
      return;
    }
    if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
      return;
    }
  }

  if(pWork->StatusWin[side]){
    GFL_BMPWIN_Delete(pWork->StatusWin[side]);
  }
  pWork->StatusWin[side] = GFL_BMPWIN_Create(frame,	sidex[side], 16, 12, 2,	_POKEMON_MAIN_FRIENDGIVEMSG_PAL, GFL_BMP_CHRAREA_GET_F);

  pWin = pWork->StatusWin[side];
  GFL_FONTSYS_SetColor( 5, 6, 0x0 );

  GFL_MSG_GetString( pWork->pMsgData,POKETRADE_STR_200 + bSelected, pWork->pStrBuf );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), 0, 0, pWork->pStrBuf, pWork->pFontHandle);
  
  GFL_BMPWIN_TransVramCharacter(pWin);
  GFL_BMPWIN_MakeScreen(pWin);
  GFL_BG_LoadScreenV_Req( frame );

  pWork->pokemonGTSSeq[side]=POKEMONTORADE_SEQ_NONE;

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
  if(pWork->StatusWin[0]){
    GFL_BMPWIN_ClearScreen(pWork->StatusWin[0]);
    GFL_BMPWIN_Delete(pWork->StatusWin[0]);
    pWork->StatusWin[0]=NULL;
  }
  if(pWork->StatusWin[1]){
    GFL_BMPWIN_ClearScreen(pWork->StatusWin[1]);
    GFL_BMPWIN_Delete(pWork->StatusWin[1]);
    pWork->StatusWin[1]=NULL;
  }
  if(pWork->GTSInfoWindow){
    GFL_BMPWIN_ClearScreen(pWork->GTSInfoWindow);
    GFL_BMPWIN_Delete(pWork->GTSInfoWindow);
    pWork->GTSInfoWindow=NULL;
  }
  if(pWork->GTSInfoWindow2){
    GFL_BMPWIN_ClearScreen(pWork->GTSInfoWindow2);
    GFL_BMPWIN_Delete(pWork->GTSInfoWindow2);
    pWork->GTSInfoWindow2=NULL;
  }
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
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
      WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                      WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
      _CHANGE_STATE(pWork, _pokemonStatusStart);
      break;
    case 1:  //もどる
      POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(pWork);
      _select6PokeSubMask(pWork);
      _selectSixButtonResetCheck(pWork);
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



static void _NEGO_BackSelect8(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),
                                 POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID)){
      POKE_GTS_CreateCancelButton(pWork);
      return;
    }
  }
  POKETRADE_MESSAGE_WindowClear(pWork);
  POKE_GTS_DeleteCancelButton(pWork,FALSE,FALSE);
  POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(pWork);
  _selectSixButtonResetCheck(pWork);
  _CHANGE_STATE(pWork, POKETRADE_NEGO_Select6keywait);
}



static void _NEGO_BackSelect7(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  _rapTimingStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_A,pWork);
  _CHANGE_STATE(pWork,_NEGO_BackSelect8);
}


static void _NEGO_BackSelect6(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrgAndSet() || GFL_UI_TP_GetTrgAndSet()){
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
    _CHANGE_STATE(pWork,_NEGO_BackSelect7);
  }
}



static void _changePokemonSendDataNetWait(POKEMON_TRADE_WORK* pWork)
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
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_E, WB_NET_TRADE_SERVICEID)){
      POKE_GTS_CreateCancelButton(pWork);  //GTSネゴのみ強制切断処理
      return;
    }
  }
  POKE_GTS_DeleteCancelButton(pWork,FALSE,FALSE);

  if((pWork->changeFactor[myID]==POKETRADE_FACTOR_TRI_LAST) &&
     (pWork->changeFactor[targetID]==POKETRADE_FACTOR_TRI_LAST)){

    _rapTimingStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_TRADEDEMO_START,pWork);
    _CHANGE_STATE(pWork, POKEMONTRADE_changeTimingDemoStart);
//    _CHANGE_STATE(pWork,POKETRE_MAIN_ChangePokemonSendDataNetwork);
    return;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_EGG){
    msgno = POKETRADE_STR_97;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_EGG){
    msgno = POKETRADE_STR_98;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_ILLEGAL){
    msgno = POKETRADE_STR2_01;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_ILLEGAL){
    msgno = POKETRADE_STR2_02;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_ILLEGAL_FRIEND){
    msgno = POKETRADE_STR2_02;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_ILLEGAL_FRIEND){
    msgno = POKETRADE_STR2_01;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_MAIL){
    msgno = POKETRADE_STR_97;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_MAIL){
    msgno = POKETRADE_STR2_32;
  }
  else if(pWork->changeFactor[targetID]==POKETRADE_FACTOR_TRI_BACK){
    msgno = gtsnego_info_18;
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_TRI_BACK){
    msgno = gtsnego_info_17;
  }
  GFL_MSG_GetString( pWork->pMsgData, msgno, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->changeFactor[0] = POKETRADE_FACTOR_NONE;
  pWork->changeFactor[1] = POKETRADE_FACTOR_NONE;
  
  _CHANGE_STATE(pWork, _NEGO_BackSelect6);

}


static void _changePokemonSendDataOk2(POKEMON_TRADE_WORK* pWork)
{
  u8 cmd = POKETRADE_FACTOR_TRI_LAST;

  if(POKEMONTRADE_IsEggAndLastBattlePokemonChange(pWork)){
    cmd = POKETRADE_FACTOR_EGG;
  }
  else if(POKEMONTRADE_IsIllegalPokemon(pWork)){
    cmd = POKETRADE_FACTOR_ILLEGAL;
  }
  else if(POKEMONTRADE_IsIllegalPokemonFriend(pWork)){
    cmd = POKETRADE_FACTOR_ILLEGAL_FRIEND;
  }
  else if(POKEMONTRADE_IsMailPokemon(pWork)){
    cmd = POKETRADE_FACTOR_MAIL;
  }
  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd)){
    _rapTimingStart( GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_E ,pWork);
    _CHANGE_STATE(pWork, _changePokemonSendDataNetWait);// あいても選ぶまで待つ
  }
}

static void _changePokemonSendDataOk(POKEMON_TRADE_WORK* pWork)
{
  u8 cmd = POKETRADE_FACTOR_TRI_LAST;
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    pWork->changeFactor[0]=cmd;
    pWork->changeFactor[1]=cmd;
    _CHANGE_STATE(pWork, _changePokemonSendDataNetWait);// あいても選ぶまで待つ
  }
  else{
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_F, WB_NET_TRADE_SERVICEID)){
      POKE_GTS_CreateCancelButton(pWork);
      return;
    }
    POKE_GTS_DeleteCancelButton(pWork,FALSE,FALSE);
    _CHANGE_STATE(pWork,_changePokemonSendDataOk2);
  }
}



static void _changePokemonSendDataNg2(POKEMON_TRADE_WORK* pWork)
{
  u8 cmd = POKETRADE_FACTOR_TRI_BACK;
  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd)){
    _rapTimingStart( GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_E ,pWork);
    _CHANGE_STATE(pWork, _changePokemonSendDataNetWait);// あいても選ぶまで待つ
  }
}


static void _changePokemonSendDataNg(POKEMON_TRADE_WORK* pWork)
{
  u8 cmd = POKETRADE_FACTOR_TRI_BACK;
  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    pWork->changeFactor[0]=cmd;
    pWork->changeFactor[1]=cmd;
    _CHANGE_STATE(pWork, _changePokemonSendDataNetWait);// あいても選ぶまで待つ
  }
  else{
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_F, WB_NET_TRADE_SERVICEID)){
      POKE_GTS_CreateCancelButton(pWork);  //GTSネゴのみ強制切断処理
      return;
    }
    POKE_GTS_DeleteCancelButton(pWork,FALSE,FALSE);
    _CHANGE_STATE(pWork, _changePokemonSendDataNg2);// あいても選ぶまで待つ
  }
}

static void _changePokemonSendDataOk0(POKEMON_TRADE_WORK* pWork)
{
  _rapTimingStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_F,pWork);
  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
  pWork->NegoWaitTime = _GTSINFO04_WAIT;
  _CHANGE_STATE(pWork,_changePokemonSendDataOk);
}


static void _changePokemonSendDataNg0(POKEMON_TRADE_WORK* pWork)
{
  _rapTimingStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_F,pWork);
  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
  pWork->NegoWaitTime = _GTSINFO04_WAIT;
  _CHANGE_STATE(pWork,_changePokemonSendDataNg);
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
      _CHANGE_STATE(pWork,_changePokemonSendDataOk0);
      break;
    case 1:  //もどる
      _CHANGE_STATE(pWork,_changePokemonSendDataNg0);
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
      POKE_GTS_CreateCancelButton(pWork);  //GTSネゴのみ強制切断処理
      return;
    }
  }
  POKE_GTS_DeleteCancelButton(pWork,FALSE,FALSE);

  if((pWork->changeFactor[myID]==POKETRADE_FACTOR_TRI_DECIDE) &&
     (pWork->changeFactor[targetID]==POKETRADE_FACTOR_TRI_DECIDE)){
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_13, pWork->pMessageStrBufEx );
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
    msgno = gtsnego_info_14;
    pWork->NegoWaitTime = _GTSINFO14_WAIT;
    if(pWork->type == POKEMONTRADE_TYPE_GTSNEGO){  //GTSEネゴのみ強制切断処理
      if(pWork->triCancel >= GTSNEGO_CANCEL_DISCONNECT_NUM){
        POKEMONTRADE_CancelCall(pWork);
        _CHANGE_STATE(pWork,POKEMONTRADE_PROC_FadeoutStart);
        return;
      }
    }
  }
  else if(pWork->changeFactor[myID]==POKETRADE_FACTOR_TRI_END){
    msgno = gtsnego_info_16;
    pWork->NegoWaitTime = _GTSINFO16_WAIT;
    if(pWork->type == POKEMONTRADE_TYPE_GTSNEGO){  //GTSEネゴのみ強制切断処理
      if(pWork->triCancel >= GTSNEGO_CANCEL_DISCONNECT_NUM){
        POKEMONTRADE_CancelCall(pWork);
        _CHANGE_STATE(pWork,POKEMONTRADE_PROC_FadeoutStart);
        return;
      }
    }
  }
  GFL_MSG_GetString( pWork->pMsgData, msgno, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->changeFactor[0] = POKETRADE_FACTOR_NONE;
  pWork->changeFactor[1] = POKETRADE_FACTOR_NONE;
  
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID);
  }
  pWork->anmCount = 0;
  _CHANGE_STATE(pWork, _NEGO_Select6CancelWait3);

}





static void _networkFriendsStandbyWait_Send(POKEMON_TRADE_WORK* pWork)
{
  u8 cmd = POKETRADE_FACTOR_TRI_DECIDE;
  
  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGEFACTOR, 1, &cmd)){
    GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_D,WB_NET_TRADE_SERVICEID );
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
  }
}

//交換選択通信送信

static void _friendSelectWait(POKEMON_TRADE_WORK* pWork)
{


  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
      POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
  pWork->NegoWaitTime = _GTSINFO13_WAIT;

  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    pWork->changeFactor[0]=POKETRADE_FACTOR_TRI_DECIDE;
    pWork->changeFactor[1]=POKETRADE_FACTOR_TRI_DECIDE;

    {
      int trgno = 3;
      POKEMON_PARAM* pp = pWork->GTSSelectPP[1-(trgno/GTS_NEGO_POKESLT_MAX)][trgno%GTS_NEGO_POKESLT_MAX];
      POKE_MAIN_Pokemonset(pWork, trgno/GTS_NEGO_POKESLT_MAX, pp);
      GFL_STD_MemCopy(pp, IRC_POKEMONTRADE_GetRecvPP(pWork,1) ,POKETOOL_GetWorkSize());
    }
    
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
    POKETRADE_2D_GTSPokemonIconChangePosUp( pWork,(sel/GTS_NEGO_POKESLT_MAX), sel%GTS_NEGO_POKESLT_MAX);

  }
}

// ポケモンのステータス表示待ち
static BOOL _pokemonStatusKeyLoop(POKEMON_TRADE_WORK* pWork)
{
  BOOL curSel=FALSE;
  
  // キー処理
  if(GFL_UI_KEY_GetTrgAndSet() == PAD_KEY_RIGHT){
    pWork->pokemonselectno = (pWork->pokemonselectno+3) % 6;
    curSel=TRUE;
  }
  if(GFL_UI_KEY_GetTrgAndSet() == PAD_KEY_LEFT){
    pWork->pokemonselectno = (pWork->pokemonselectno-3) % 6;
    if(pWork->pokemonselectno<0){
      pWork->pokemonselectno+=6;
    }
    curSel=TRUE;
  }
  if(GFL_UI_KEY_GetTrgAndSet() == PAD_KEY_UP){
    if((pWork->pokemonselectno==0) || (pWork->pokemonselectno==3)){
      pWork->pokemonselectno = pWork->pokemonselectno+3;
    }
    pWork->pokemonselectno = pWork->pokemonselectno-1;
    curSel=TRUE;
  }
  if(GFL_UI_KEY_GetTrgAndSet() == PAD_KEY_DOWN){
    if((pWork->pokemonselectno==2) || (pWork->pokemonselectno==5)){
      pWork->pokemonselectno = pWork->pokemonselectno-3;
    }
    pWork->pokemonselectno = pWork->pokemonselectno+1;
    curSel=TRUE;
  }
  return curSel;
}


static void _pokemonStatusWaitNw(POKEMON_TRADE_WORK* pWork)
{
  if(WIPE_SYS_EndCheck()){
    // 消す
    GFL_BG_SetVisible( GFL_BG_FRAME3_M , FALSE );
    POKETRADE_2D_GTSPokemonIconChangePosUp( pWork,-1,-1);

    POKETRADE_MESSAGE_ResetPokemonStatusMessage(pWork, FALSE); //20100624 mod Saito BTS6424
    {
      ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_POKETRADE, pWork->heapID );
      GF_ASSERT(pWork->subchar);

      //   pWork->subchar =
     //   GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_trade_wb_trade_bg01_NCGR,
       //                                              GFL_BG_FRAME2_M, 0, 0, pWork->heapID);
      GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
                                           NARC_trade_wb_trade_bg01_back_NSCR,
                                           GFL_BG_FRAME2_M, 0,
                                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                           pWork->heapID);
      GFL_ARC_CloseDataHandle(p_handle);
    }
    IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
    pWork->statusModeOn = FALSE;

    if(POKEMONTRADE_IsInPokemonRecvPoke(IRC_POKEMONTRADE_GetRecvPP(pWork, 1))){
      POKE_MAIN_Pokemonset(pWork, 1, IRC_POKEMONTRADE_GetRecvPP(pWork, 1));
    }
    if(POKEMONTRADE_IsInPokemonRecvPoke(IRC_POKEMONTRADE_GetRecvPP(pWork, 0))){
      POKE_MAIN_Pokemonset(pWork, 0, IRC_POKEMONTRADE_GetRecvPP(pWork, 0));
    }

    _select6PokeSubMask(pWork);
    _selectSixButtonResetCheck(pWork);
    _CHANGE_STATE(pWork, POKETRADE_NEGO_Select6keywait);
    POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(pWork);
    WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  }
}

// ポケモンのステータス表示待ち
static void _pokemonStatusWaitN(POKEMON_TRADE_WORK* pWork)
{
  int tpno;
  
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );

  if( WIPE_SYS_EndCheck() == FALSE ) return;    //20100603 add saito

  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
    _CHANGE_STATE(pWork, _pokemonStatusWaitNw);
    return;
  }
  if(TOUCHBAR_SELECT_NONE != TOUCHBAR_GetTouch(pWork->pTouchWork)){
    return;
  }
  
  tpno = GFL_UI_TP_HitTrg(_tp_data);
  if(-1!=tpno){   //タッチ処理
    GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
    pWork->pokemonselectno = tpno;
    POKEMONTRADE_StartPokeSelectSixButton(pWork, -1);
    _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno);
    PMSND_PlaySystemSE(SEQ_SE_SELECT1);
    return;
  }

  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    if(GFL_UI_KEY_GetTrgAndSet()){
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


}


// ポケモンのステータス表示
static void _pokemonStatusStart(POKEMON_TRADE_WORK* pWork)
{
  if(WIPE_SYS_EndCheck()){
    u16 trgno = pWork->pokemonselectno;
    POKEMON_PARAM* pp = pWork->GTSSelectPP[1-(trgno/GTS_NEGO_POKESLT_MAX)][trgno%GTS_NEGO_POKESLT_MAX]; //さかさまにする
    pWork->statusModeOn=TRUE;
    if(pp && PP_Get(pp,ID_PARA_poke_exist,NULL) ){
      POKETRADE_MESSAGE_CreatePokemonParamDisp(pWork,pp);
      POKETRADE_2D_GTSPokemonIconChangePosUp( pWork,(trgno/GTS_NEGO_POKESLT_MAX), trgno%GTS_NEGO_POKESLT_MAX);
    }
    _select6PokeSubMask(pWork);
    WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
    POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(pWork);
    _selectSixButtonResetCheck(pWork);
    _CHANGE_STATE(pWork, _pokemonStatusWaitN);  //選択にループ
  }
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
      _CHANGE_STATE(pWork, _friendSelectWait);
      break;
    case 1:  //つよさをみる
      WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                      WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
      TOUCHBAR_SetActive(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
      _CHANGE_STATE(pWork, _pokemonStatusStart);
      break;
    case 2:  //もどる
      _select6PokeSubMask(pWork);
      POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(pWork);
      _selectSixButtonResetCheck(pWork);
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

static void _menuMyPokemonSendG(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->pokemonSendDisableFlg == TRUE){
    return;
  }
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON,
                         POKETOOL_GetWorkSize(), IRC_POKEMONTRADE_GetRecvPP(pWork,0))){
      return;
    }
  }
  pWork->pokemonSendDisableFlg = TRUE;
  _CHANGE_STATE(pWork,_menuMyPokemonMenu);
}


static void _menuMyPokemonSend(POKEMON_TRADE_WORK* pWork)
{
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 flg = IRC_POKEMONTRADE_GetRecvBoxFlg(pWork,0);
    if(!GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON_BOX,
                         POKETOOL_GetWorkSize(), &flg) ){
      return;
    }
  }
  _CHANGE_STATE(pWork,_menuMyPokemonSendG);
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
  int side = trgno / GTS_NEGO_POKESLT_MAX;
  int poke = trgno % GTS_NEGO_POKESLT_MAX;

  if(trgno != -1){
    if(pWork->GTSSelectIndex[1-side][poke]!=-1){
      POKEMON_PARAM* pp = pWork->GTSSelectPP[ 1-side ][ poke ]; //さかさまにする
      BOOL bBoxFlg = pWork->bGTSSelectPokeBox[ 1-side ][ poke ];
      
      if(pp && PP_Get(pp,ID_PARA_poke_exist,NULL)){
        pWork->pokemonselectno = trgno;
//        OS_TPrintf("pokemonselectno %d\n",pWork->pokemonselectno);
        PMSND_PlaySystemSE(SEQ_SE_DECIDE1);
        TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
        if(trgno/GTS_NEGO_POKESLT_MAX){  //こちら側は自分のポケモンが入っている 自分のはみるだけ
          _CHANGE_STATE(pWork,_menuFriendPokemonStart);
        }
        else{
          POKE_MAIN_Pokemonset(pWork, trgno/GTS_NEGO_POKESLT_MAX, pp);
          GFL_STD_MemCopy(pp, IRC_POKEMONTRADE_GetRecvPP(pWork,0) ,POKETOOL_GetWorkSize());
          OS_TPrintf("メールセット2%d \n",bBoxFlg);
          IRC_POKEMONTRADE_SetRecvBoxFlg(pWork,0,bBoxFlg);
          _CHANGE_STATE(pWork, _menuMyPokemonSend);
        }
        return TRUE;
      }
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
    IRC_POKETRADE_GraphicInitSubDispSys(pWork);
    IRC_POKETRADE_GraphicInitSubDisp(pWork);
    
    IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
    POKETRADE_TOUCHBAR_Init(pWork);
    GFL_NET_ReloadIconTopOrBottom(FALSE, pWork->heapID);
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
  if(!CheckNegoWaitTimer(pWork)){
    return;
  }
  if(pWork->anmCount < _MESSAGE_TIMER){
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



static void _NEGO_Select6CancelWait22_0(POKEMON_TRADE_WORK* pWork)
{
  //友達の選択をまっていますメッセージ
  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
  pWork->NegoWaitTime = _GTSINFO04_WAIT;
  _CHANGE_STATE(pWork, _NEGO_Select6CancelWait22);
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
      _CHANGE_STATE(pWork, _NEGO_Select6CancelWait22_0);
      break;
    default: //いいえ
      POKETRADE_MESSAGE_WindowClear(pWork);
      POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(pWork);
      _selectSixButtonResetCheck(pWork);
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

//  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, TRUE);

  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_15, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    TOUCHBAR_SetActive(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
    _selectSixButtonResetCheck(pWork);
    _CHANGE_STATE(pWork, _NEGO_Select6CancelWait );
    return;
  }
  if(TOUCHBAR_SELECT_NONE != TOUCHBAR_GetTouch(pWork->pTouchWork)){
    return;
  }

  
  trgno = GFL_UI_TP_HitTrg(_tp_data);
  if(trgno != -1){
    GFL_UI_SetTouchOrKey(GFL_APP_END_TOUCH);
    PMSND_PlaySystemSE(SEQ_SE_SELECT1);
    POKEMONTRADE_StartPokeSelectSixButton(pWork, -1);
    if(_NEGO_Select6PokemonSelect(pWork, trgno)){
      return;
    }
  }
  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    if(GFL_UI_KEY_GetTrgAndSet()){
      pWork->pokemonselectno = pWork->pokemonselectno % 6;
      if(pWork->pokemonselectno<0){
        pWork->pokemonselectno=0;
      }
      POKEMONTRADE_StartPokeSelectSixButton(pWork,pWork->pokemonselectno);
      PMSND_PlaySystemSE(SEQ_SE_SELECT1);
      return;
    }
  }

  if(GFL_UI_KEY_GetTrgAndSet() == PAD_BUTTON_DECIDE){
    if(_NEGO_Select6PokemonSelect(pWork, pWork->pokemonselectno)){
      return ;
    }
  }

  // キー処理
  if(_pokemonStatusKeyLoop(pWork)){
    POKEMONTRADE_StartPokeSelectSixButton(pWork,pWork->pokemonselectno);
    PMSND_PlaySystemSE(SEQ_SE_SELECT1);
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
  if(GFL_UI_KEY_GetTrgAndSet() || GFL_UI_TP_GetTrgAndSet()){
    //カーソル表示更新 20100630 add Saito BTS7062
    if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
      NOZOMU_Printf("カーソル状態反映 非表示\n");
      POKEMONTRADE_StartPokeSelectSixButton(pWork, -1);
    }
    else{
      NOZOMU_Printf("カーソル状態反映 表示\n");
      POKEMONTRADE_StartPokeSelectSixButton(pWork, 0);
    }
    POKETRADE_MESSAGE_WindowClear(pWork);
    POKEMONTRADE_TOUCHBAR_SetReturnIconActiveTRUE(pWork);

    _CHANGE_STATE(pWork,POKETRADE_NEGO_Select6keywait);
  }
}

static void POKETRADE_NEGO_Select6keywaitMsg(POKEMON_TRADE_WORK* pWork)
{

  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_12, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->NegoWaitTime = _GTSINFO12_WAIT;

  _CHANGE_STATE(pWork,POKETRADE_NEGO_Select6keywaitMsg2);
}


static void _select6PokeSubMask(POKEMON_TRADE_WORK* pWork)
{
  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ , -8 );
  {
    G2S_SetWnd0InsidePlane(
      GX_WND_PLANEMASK_BG0|
      GX_WND_PLANEMASK_BG1|
      GX_WND_PLANEMASK_BG2|
      GX_WND_PLANEMASK_BG3|
      GX_WND_PLANEMASK_OBJ,
      TRUE );
    G2S_SetWnd0Position( 128,  0, 255, 192-24+1 );
    G2S_SetWndOutsidePlane(
      GX_WND_PLANEMASK_BG0|
      GX_WND_PLANEMASK_BG1|
      GX_WND_PLANEMASK_BG2|
      GX_WND_PLANEMASK_BG3|
      GX_WND_PLANEMASK_OBJ,
      FALSE );
    GXS_SetVisibleWnd( GX_WNDMASK_W0 );
  }
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );
  GFL_BG_SetVisible( GFL_BG_FRAME1_S , TRUE );
  GFL_BG_SetVisible( GFL_BG_FRAME0_S , TRUE );
}


static void POKETRADE_NEGO_Select6keywaitMsgFade(POKEMON_TRADE_WORK* pWork)
{
  // スライド演出
  if( !POKEMONTRADE_NEGOBG_Select6Scroll(pWork)){
    return;
  }

  if(GFL_UI_CheckTouchOrKey()!=GFL_APP_END_KEY){
    POKEMONTRADE_StartPokeSelectSixButton(pWork, -1);
    pWork->pokemonselectno=0;
  }
  else{
    POKEMONTRADE_StartPokeSelectSixButton(pWork, 0);
    pWork->pokemonselectno=0;
  }

  
  if(WIPE_SYS_EndCheck()){
    _select6PokeSubMask(pWork);
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
  TOUCHBAR_SetActive(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);

  POKETRADE_MESSAGE_SixStateDisp(pWork , GFL_BG_FRAME2_S);//６体表示
  
  
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
//    POKE_GTS_SelectStatusMessageDisp(pWork, 1, TRUE);
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


/*
１．認証キーを任天堂サーバから貰う
NHTTP_RAP_SvlGetTokenStart
NHTTP_RAP_SvlGetTokenMain

２．ポケモンデータを送る
NHTTP_RAP_PokemonEvilCheckCreate                  送る領域確保  ポケモン*数+367バイト くらい必要です
NHTTP_RAP_PokemonEvilCheckAdd                       ポケモン追加
NHTTP_RAP_PokemonEvilCheckConectionCreate    送信開始

NHTTP_RAP_Processでレスポンスを受ける

*/

#if 1

static void _PokeEvilChkEnd2(POKEMON_TRADE_WORK* pWork)
{
  int msgno;
  if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_EVIL, WB_NET_TRADE_SERVICEID)){
    return;
  }
  if((pWork->evilCheck[0] == 0 ) && (pWork->evilCheck[1] == 0 )){
    _CHANGE_STATE(pWork, POKE_GTS_Select6Init);
    return;
  }
  else if(pWork->evilCheck[1]==0xff){
    msgno = gtsnego_info_24;   //その他のエラー
  }
  else if(pWork->evilCheck[0]==0xff){
    msgno = gtsnego_info_24;
  }
  else if(pWork->evilCheck[1]){
    msgno = gtsnego_info_10;
  }
  else{
    msgno = gtsnego_info_11;   //不正検査
  }
  GFL_MSG_GetString( pWork->pMsgData, msgno, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  pWork->NegoWaitTime = _GTSINFO11_WAIT;
  pWork->anmCount = 0;
  _CHANGE_STATE(pWork,_NEGO_Select6CancelWait3);
}


//ポケモン不正検査
static void _PokeEvilChkEnd(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if( GFL_NET_SendData(pNet, _NETCMD_EVILCHECK,1,&pWork->evilCheck[0])){
    GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_EVIL,WB_NET_TRADE_SERVICEID );
    _CHANGE_STATE(pWork, _PokeEvilChkEnd2);
  }
}


//ポケモン不正切断コールバック
static void _PokeEvilChk_DisconnectCallback(void* pUserwork, int code, int type, int ret)
{
  POKEMON_TRADE_WORK* pWork = pUserwork;

  if(pWork->pNHTTP){
    NHTTP_RAP_ErrorClean(pWork->pNHTTP);
    NHTTP_RAP_End(pWork->pNHTTP);  //この関数を呼ぶ事
    pWork->pNHTTP  = NULL;
    DWC_RAPCOMMON_ResetSubHeapID();
  }
}



//ポケモン不正検査
static void _PokeEvilChk2(POKEMON_TRADE_WORK* pWork)
{
  int i;
  int num=0;
  {
    NHTTPError error;
    error = NHTTP_RAP_Process( pWork->pNHTTP );
    if( NHTTP_ERROR_NONE == error )
    {
      void* p_buff  = NHTTP_RAP_GetRecvBuffer(pWork->pNHTTP);
      pWork->evilCheck[0] = NHTTP_RAP_EVILCHECK_GetStatusCode( p_buff );  //検査結果格納
      _CHANGE_STATE(pWork, _PokeEvilChkEnd);
    }
    else if( NHTTP_ERROR_BUSY != error )
    {
       pWork->evilCheck[0] = 0xff;  //主に通信エラー
      _CHANGE_STATE(pWork, _PokeEvilChkEnd);
    }
    else{
      return;
    }
    if(pWork->pNHTTP)
    {
      NHTTP_RAP_PokemonEvilCheckDelete(pWork->pNHTTP);
      NHTTP_RAP_End(pWork->pNHTTP);
      pWork->pNHTTP  = NULL;
      DWC_RAPCOMMON_ResetSubHeapID();
      GFL_NET_DWC_SetErrDisconnectCallback(NULL,NULL);
    }
    
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),
                                 POKETRADE_FACTOR_TIMING_G ,WB_NET_TRADE_SERVICEID);
  }

}



//ポケモン不正検査
static void _PokeEvilChk(POKEMON_TRADE_WORK* pWork)
{
  int i,num=0;

  DWC_RAPCOMMON_SetSubHeapID( DWC_ALLOCTYPE_NHTTP, 0x10000, pWork->heapID );

  pWork->triCancel++;  //交換せずに何回もここを通ると切断
  
  pWork->pNHTTP = NHTTP_RAP_Init(pWork->heapID,
                                 MyStatus_GetProfileID(pWork->pMy), pWork->pParentWork->pSvl);

  GFL_NET_DWC_SetErrDisconnectCallback(_PokeEvilChk_DisconnectCallback, pWork );

  
  for(i=0;i<GTS_NEGO_POKESLT_MAX;i++){
    if(pWork->GTSSelectPP[1][i]){
      num++;
    }
  }
  NHTTP_RAP_PokemonEvilCheckCreate(pWork->pNHTTP, pWork->heapID,
                                   POKETOOL_GetWorkSize() * num, NHTTP_POKECHK_GTSNEGO);
  for(i=0;i<GTS_NEGO_POKESLT_MAX;i++){
    if(pWork->GTSSelectPP[1][i]){
      NHTTP_RAP_PokemonEvilCheckAdd(pWork->pNHTTP, pWork->GTSSelectPP[1][i], POKETOOL_GetWorkSize());
    }
  }
  NHTTP_RAP_PokemonEvilCheckConectionCreate(pWork->pNHTTP);

  NHTTP_RAP_StartConnect(pWork->pNHTTP);

  
  _CHANGE_STATE(pWork, _PokeEvilChk2);
}


//ポケモンプレ不正検査
static void _PokeEvilChkPre1(POKEMON_TRADE_WORK* pWork)
{
  if(WIPE_SYS_EndCheck()){
    IRC_POKETRADE_AllDeletePokeIconResource(pWork);
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG2 );
    WIPE_ResetBrightness(WIPE_PATTERN_S);
    _CHANGE_STATE(pWork, _PokeEvilChk);
  }

}

//ポケモンプレ不正検査
static void _PokeEvilChkPre(POKEMON_TRADE_WORK* pWork)
{
  WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  _CHANGE_STATE(pWork, _PokeEvilChkPre1);
}

#endif


static void _Select6MessageInit8(POKEMON_TRADE_WORK* pWork)
{
  int i;
  int msgno;
  int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  int targetID = 1 - myID;

  if(!POKEMONTRADEPROC_IsNetworkMode(pWork)){
    myID=0;
    targetID = 1 - myID;
  }

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if(!GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_B, WB_NET_TRADE_SERVICEID)){
      POKE_GTS_CreateCancelButton(pWork);  //GTSネゴのみ強制切断処理
      return;
    }
  }
  POKE_GTS_DeleteCancelButton(pWork,FALSE,FALSE);
  pWork->pokemonGTSStateMode = FALSE;
  pWork->pokemonGTSSeq[0]=POKEMONTORADE_SEQ_NONE;
  pWork->pokemonGTSSeq[1]=POKEMONTORADE_SEQ_NONE;

  if((pWork->changeFactor[myID]==POKETRADE_FACTOR_TRI_SELECT) &&
     (pWork->changeFactor[targetID]==POKETRADE_FACTOR_TRI_SELECT)){

    POKE_GTS_SelectStatusMessageDisp(pWork, 0, 1, TRUE);//強制書き換え
    POKE_GTS_SelectStatusMessageDisp(pWork, 1, 1, TRUE);
    
    if(pWork->type == POKEMONTRADE_TYPE_GTSNEGO){
      _CHANGE_STATE(pWork, _PokeEvilChkPre);
    }
    else{
      _CHANGE_STATE(pWork, POKE_GTS_Select6Init);
    }
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
  pWork->pokemonGTSSeq[0]= POKEMONTORADE_SEQ_WAIT;
  pWork->pokemonGTSSeq[1]= POKEMONTORADE_SEQ_WAIT;

//  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_A,WB_NET_TRADE_SERVICEID);
  _CHANGE_STATE(pWork, POKEMONTRAE_EndCancelState);

}



static void _Select6MessageInit81(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 flg = POKEMONTORADE_SEQ_MISERUOK;
    if( GFL_NET_SendData(pNet, _NETCMD_GTSSEQNO,1,&flg)){
      _CHANGE_STATE(pWork, _Select6MessageInit8);
    }
  }
  else{
    _CHANGE_STATE(pWork, _Select6MessageInit8);
  }
}




static void _Select6MessageInit6(POKEMON_TRADE_WORK* pWork)
{
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();

  if(!POKETRADE_MESSAGE_EndCheck(pWork)){
    return;
  }

  
  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    u8 flg = POKETRADE_FACTOR_TRI_SELECT;
    if( GFL_NET_SendData(pNet, _NETCMD_CHANGEFACTOR,1,&flg)){
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),POKETRADE_FACTOR_TIMING_B,WB_NET_TRADE_SERVICEID);
      _CHANGE_STATE(pWork, _Select6MessageInit81);
    }
  }
  else{
//    pWork->pokemonGTSSeq=POKEMONTORADE_SEQ_KAISHIOK;
    pWork->changeFactor[0]=POKETRADE_FACTOR_TRI_SELECT;
    pWork->changeFactor[1]=POKETRADE_FACTOR_TRI_SELECT;
    _CHANGE_STATE(pWork, _Select6MessageInit81);
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


static void _Select6MessageInit6_0(POKEMON_TRADE_WORK* pWork)
{
  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
  pWork->NegoWaitTime = _GTSINFO04_WAIT;
  _CHANGE_STATE(pWork,_Select6MessageInit6);
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
      _CHANGE_STATE(pWork,_Select6MessageInit6_0);
      break;
    default: //いいえ
      POKETRADE_MESSAGE_WindowClear(pWork);
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
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_04, pWork->pMessageStrBuf );
    POKETRADE_MESSAGE_WindowOpen(pWork);
    _CHANGE_STATE(pWork,_Select6MessageInit4);
  }
}



static void _Select6MessageInit3_0(POKEMON_TRADE_WORK* pWork)
{
  GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_03, pWork->pMessageStrBuf );
  POKETRADE_MESSAGE_WindowOpen(pWork);
  POKETRADE_MESSAGE_WindowTimeIconStart(pWork);
  pWork->NegoWaitTime = _GTSINFO03_WAIT;
  _CHANGE_STATE(pWork,_Select6MessageInit3);
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
      _CHANGE_STATE(pWork,_Select6MessageInit3_0);
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

  if((pWork->workPokeIndex==index) && (pWork->workBoxno==boxno)){
    OS_TPrintf("workPokeIndex %d %d\n",boxno, index);
    return TRUE;
  }
  

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
//    OS_TPrintf("selectIndex %d %d\n",boxno, index);
    return TRUE;
  }
  return FALSE;
}


static void POKE_GTS_DeletePokemonStateRelease(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_UI_TP_GetCont()==FALSE){
    _CHANGE_STATE(pWork, POKETRADE_TouchStateGTS);
  }
}



void POKE_GTS_DeletePokemonState(POKEMON_TRADE_WORK* pWork)
{
  int no = POKE_GTS_IsSelect(pWork,pWork->underSelectBoxno,pWork->underSelectIndex );


  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    if( !GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), _NETCMD_THREE_SELECT_CANCEL, sizeof(no), &no)){
      return;
    }
  }
  pWork->selectIndex = -1;
  pWork->selectBoxno = -1;
  pWork->workPokeIndex = -1;
  pWork->workBoxno = -1;
  pWork->underSelectIndex = -1;
  pWork->underSelectBoxno = -1;
  if(no!=-1){
    POKE_GTS_DeletePokemonDirect(pWork, 0, no);
    POKETRADE_2D_GTSPokemonIconReset(pWork,0, no);
  }
  _CHANGE_STATE(pWork, POKE_GTS_DeletePokemonStateRelease);
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
    
  if(GFL_UI_TP_GetPointTrg(&x,&y)){     // パネルスクロール
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

//------------------------------------------------------------------------------
/**
 * @brief   アイコン消去判定
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKE_GTS_FaceIconDeleteFunc(POKEMON_TRADE_WORK* pWork)    //顔アイコンの処理
{
  int i;

  if(!POKEMONTRADEPROC_IsTriSelect(pWork)){
    return;
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

    if(!PMSND_CheckPlayingSEIdx(SEQ_SE_SELECT3)){
      PMSND_PlaySystemSE(SEQ_SE_SELECT3);
    }
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

//------------------------------------------------------------------------------
/**
 * @brief   スライド初期化
 * @param   
 * @param   
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKEMONTRADE_NEGO_SlideInit(POKEMON_TRADE_WORK* pWork,int side,POKEMON_PARAM* pp)
{
  POKETRADE_2D_GTSPokemonIconVisible(pWork, side, FALSE);

  if(pWork->SlideWindowTimer[side]!=0){
    POKEMONTRADE_NEGOBG_SlideMessageDel(pWork, side);
    IRCPOKETRADE_PokeDeleteMcss(pWork, side);
  }

  POKEMONTRADE_NEGOBG_SlideInit(pWork, side, pp);
  POKEMONTRADE_NEGOBG_SlideMessage(pWork, side, pp);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
  IRCPOKETRADE_PokeCreateMcssGTS(pWork, side, 1-side, pp, 1-side, TRUE);
//  IRCPOKETRADE_PokeCreateMcssNormal(pWork, side, TRUE, pp, 1-side);
  
}

//------------------------------------------------------------------------------
/**
 * @brief   スライドメイン
 * @param   
 * @param   
 * @retval  none
 */
//------------------------------------------------------------------------------


void POKEMONTRADE_NEGO_SlideMain(POKEMON_TRADE_WORK* pWork)
{
  int i;

  for(i=0;i<2;i++){
    if( pWork->SlideWindowTimer[i] > 0){
      pWork->SlideWindowTimer[i]--;
      POKETRADE_2D_GTSPokemonIconVisible(pWork, i, FALSE);

      if(pWork->SlideWindowTimer[i]==0){
        POKEMONTRADE_NEGOBG_SlideMessageDel(pWork, i);
        GFL_BG_FillScreen( GFL_BG_FRAME1_M, 0x0000, i*16, 0, 16, 24, GFL_BG_SCRWRT_PALNL );
        GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
        IRCPOKETRADE_PokeDeleteMcss(pWork, i);
        POKETRADE_2D_GTSPokemonIconVisible(pWork, i, TRUE);
      }
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモンデータを送る
 * @param   POKEMON_PARAM* pp
 * @retval  送信可能でTRUE
 */
//------------------------------------------------------------------------------

static BOOL POKEMONTRADE_SendPokemon(POKEMON_TRADE_WORK* pWork,POKEMON_PARAM* pp,int commandID)
{
  BOOL ret;
  GFL_NETHANDLE* pNet = GFL_NET_HANDLE_GetCurrentHandle();
  int no;

  if(pWork->pokemonSendDisableFlg){
    return FALSE;
  }

  GFL_STD_MemCopy(pp,pWork->TempBuffer[_TEMPBUFF_SEND], POKETOOL_GetWorkSize());
  ret = GFL_NET_SendDataEx(pNet,
                           GFL_NET_SENDID_ALLUSER, commandID,
                           POKETOOL_GetWorkSize(), pWork->TempBuffer[_TEMPBUFF_SEND],
                           FALSE, FALSE, TRUE);
  if(ret == FALSE){
    return FALSE;
  }
  pWork->pokemonSendDisableFlg = TRUE;
  return TRUE;
  
}

