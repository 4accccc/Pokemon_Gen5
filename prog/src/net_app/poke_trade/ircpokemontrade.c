//=============================================================================
/**
 * @file	  ircpokemontrade.c
 * @brief	  ポケモン交換して通信を終了する
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/07/09
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

#include "msg/msg_poke_trade.h"
#include "msg/msg_chr.h"
#include "msg/msg_trade_head.h"
#include "ircbattle.naix"
#include "trade.naix"
#include "net_app/connect_anm.h"
#include "net/dwc_rapfriend.h"
#include "savedata/wifilist.h"

#include "system/touch_subwindow.h"
#include "net_app/gtsnego.h"

//#include "poke_tool/poke_tool_def.h"
#include "item/item.h"
#include "app/app_menu_common.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"

#include "ircpokemontrade_local.h"

#define HEAPSIZE_POKETRADE (0xd3000)   //全共通 WiFiだとほぼマックス

//#define _ENDTABLE  {192-32, 192, 256-32, 256}     //終了
//#define _SEARCHTABLE  {192-32, 192, 0, 32}        //検索ボタン


///通信コマンド
typedef enum {
  _NETCMD_SELECT_POKEMON = GFL_NET_CMD_IRCTRADE,
  _NETCMD_CHANGE_POKEMON,
  _NETCMD_EGG_AND_BATTLE,
  _NETCMD_LOOKATPOKE,
  _NETCMD_CHANGE_CANCEL,
  _NETCMD_END,
  _NETCMD_CANCEL_POKEMON,
  _NETCMD_SCROLLBAR,
} _BATTLEIRC_SENDCMD;


static void _recvSelectPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangeCancel(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

static void _touchState(POKEMON_TRADE_WORK* pWork);
static u8* _setChangePokemonBuffer(int netID, void* pWork, int size);
static void _changeWaitState(POKEMON_TRADE_WORK* pWork);
static void _endWaitState(POKEMON_TRADE_WORK* pWork);
static BOOL _PokemonsetAndSendData(POKEMON_TRADE_WORK* pWork);
static void _recvFriendScrollBar(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _changeFinish(POKEMON_TRADE_WORK* pWork);
static void _networkFriendsStandbyWait2(POKEMON_TRADE_WORK* pWork);
static void _touchStateCommon(POKEMON_TRADE_WORK* pWork);
static void _endCancelState(POKEMON_TRADE_WORK* pWork);
static void _recvLookAtPoke(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvEggAndBattle(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _cancelPokemonSendDataNetwork(POKEMON_TRADE_WORK* pWork);
static void _scrollMainFunc(POKEMON_TRADE_WORK* pWork,BOOL bSE, BOOL bNetSend);
static void _recvCancelPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _PokemonReset(POKEMON_TRADE_WORK *pWork, int side );
static void _changeMenuOpen(POKEMON_TRADE_WORK* pWork);
static BOOL IsTouchCLACTPosition(POKEMON_TRADE_WORK* pWork, BOOL bCatch);
static int _boxScrollLine2Num(int line);



///通信コマンドテーブル
static const NetRecvFuncTable _PacketTbl[] = {
  {_recvSelectPokemon,   _setChangePokemonBuffer},    ///_NETCMD_SELECT_POKEMON 2 ポケモン見せ合う
  {_recvChangePokemon,   NULL},    ///_NETCMD_CHANGE_POKEMON   3 こうかんけってい
  {_recvEggAndBattle, NULL },     ///_NETCMD_EGG_AND_BATTLE  3 たまごと最後のバトルポケモン
  {_recvLookAtPoke, NULL},       ///_NETCMD_LOOKATPOKE   2 ポケモン見せ合う
  {_recvChangeCancel,   NULL},    ///_NETCMD_CHANGE_CANCEL  3 最後にキャンセルする
  {_recvEnd, NULL},         //_NETCMD_END           1 おわり
  {_recvCancelPokemon, NULL},         //  _NETCMD_CANCEL_POKEMON
  {_recvFriendScrollBar, NULL}, //_NETCMD_SCROLLBAR

};

int IRC_TRADE_LINE2TRAY(int lineno,POKEMON_TRADE_WORK* pWork)
{
  if(lineno >= HAND_HORIZONTAL_NUM){
    return (lineno - HAND_HORIZONTAL_NUM) / BOX_HORIZONTAL_NUM;
  }
  return pWork->BOX_TRAY_MAX;
}

int IRC_TRADE_LINE2POKEINDEX(int lineno,int verticalindex)
{
  if(lineno >= HAND_HORIZONTAL_NUM){
    return ((lineno - HAND_HORIZONTAL_NUM) % BOX_HORIZONTAL_NUM) + (BOX_HORIZONTAL_NUM * verticalindex);
  }
  if(verticalindex < 3){
    return lineno + verticalindex * HAND_HORIZONTAL_NUM;
  }
  return -1;
}

//------------------------------------------------------------------
/**
 * @brief   ベクトル計算
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _vectorUpMath(POKEMON_TRADE_WORK *pWork)
{
  u32 x,y;

  {
    if(GFL_UI_TP_GetPointCont(&x,&y)){   //ベクトルを監視  // 上向き判定
      if(pWork->y > (y + 6)){   // 上に移動した場合跳ね上げる処理
        pWork->bUpVec = TRUE;
      }
      else{
        pWork->bUpVec = FALSE;
      }
      if(pWork->pCatchCLWK){
        //GFL_CLACTPOS pos;
        //GFL_CLACT_WK_GetPos( pWork->pCatchCLWK, &pos, CLSYS_DRAW_SUB);
//        aCatchOldPos
        if(GFL_STD_Abs(pWork->aCatchOldPos.x - x)+GFL_STD_Abs(pWork->aCatchOldPos.y - y) < 10){
          pWork->bUpVec = FALSE;
        }
        if(GFL_STD_Abs(pWork->aCatchOldPos.x - x) > 6  && GFL_STD_Abs(pWork->aCatchOldPos.y - y) > 6){
          pWork->bStatusDisp = FALSE;
        }

      }
      if((y >=  2*8) && ((18*8) > y)){
        pWork->x = x;
        pWork->y = y;
      }
    }
  }
}


//------------------------------------------------------------------
/**
 * @brief   つかんだポケモンをうごかす
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _CatchPokemonMoveFunc(POKEMON_TRADE_WORK *pWork)
{
    // ついてくる
  if(pWork->pCatchCLWK != NULL){  //タッチパネル操作中のアイコン移動
    u32 x,y;
    GFL_CLACTPOS pos;
    if(GFL_UI_TP_GetPointCont(&x,&y)){
      pos.x = x;
      pos.y = y;
      GFL_CLACT_WK_SetSoftPri(pWork->pCatchCLWK,_CLACT_SOFTPRI_CATCHPOKE);
      GFL_CLACT_WK_SetPos( pWork->pCatchCLWK, &pos, CLSYS_DRAW_SUB);
    }
  }
}

//------------------------------------------------------------------
/**
 * @brief   つかんだポケモンを元の位置に戻す
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _CatchPokemonPositionRewind(POKEMON_TRADE_WORK *pWork)
{
  if(pWork->pCatchCLWK){
    GFL_CLACT_WK_SetSoftPri(pWork->pCatchCLWK,_CLACT_SOFTPRI_POKELIST);
    GFL_CLACT_WK_SetPos(pWork->pCatchCLWK, &pWork->aCatchOldPos, CLSYS_DRAW_SUB);
    GFL_CLACT_WK_SetDrawEnable( pWork->pCatchCLWK, TRUE);
    pWork->pCatchCLWK=NULL;
  }
}

//------------------------------------------------------------------
/**
 * @brief   つかみじょうたいにする
 * @param   POKEMON_TRADE_WORK
 * @retval  void
 */
//------------------------------------------------------------------

static void _CatchPokemonPositionActive(POKEMON_TRADE_WORK *pWork,GFL_CLWK* pCL)
{
  pWork->pCatchCLWK = pCL;
  pWork->pSelectCLWK = pCL;
  GFL_CLACT_WK_GetPos(pWork->pCatchCLWK, &pWork->aCatchOldPos, CLSYS_DRAW_SUB);
}

//--------------------------------------------------------------
//	ポケモンMCSS作成
//--------------------------------------------------------------
void IRCPOKETRADE_PokeCreateMcss( POKEMON_TRADE_WORK *pWork ,int no, int bFront, const POKEMON_PARAM *pp )
{
  MCSS_ADD_WORK addWork;
  VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};
  int xpos[] = { PSTATUS_MCSS_POS_X1 , PSTATUS_MCSS_POS_X2 , PSTATUS_MCSS_POS_X1 , PSTATUS_MCSS_POS_X2};
  int z;

  GF_ASSERT( pWork->pokeMcss[no] == NULL );

  if(bFront){
    MCSS_TOOL_MakeMAWPP( pp , &addWork , MCSS_DIR_FRONT );
    z=PSTATUS_MCSS_POS_MYZ;
  }
  else{
    MCSS_TOOL_MakeMAWPP( pp , &addWork , MCSS_DIR_BACK );
    z=PSTATUS_MCSS_POS_YOUZ;
  }
  pWork->pokeMcss[no] = MCSS_Add( pWork->mcssSys , xpos[no] , PSTATUS_MCSS_POS_Y , z , &addWork );
  MCSS_SetScale( pWork->pokeMcss[no] , &scale );

}

//--------------------------------------------------------------
//	ポケモンMCSS削除
//--------------------------------------------------------------
void IRCPOKETRADE_PokeDeleteMcss( POKEMON_TRADE_WORK *pWork,int no  )
{
  if( pWork->pokeMcss[no] == NULL ){
    return;
  }

  MCSS_SetVanishFlag( pWork->pokeMcss[no] );
  MCSS_Del(pWork->mcssSys,pWork->pokeMcss[no]);
  pWork->pokeMcss[no] = NULL;
}



//------------------------------------------------------------------------------
/**
 * @brief   下画面ポケモン位置を計算
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

#if 1

static void _getPokeIconPos(int index, GFL_CLACTPOS* pos)
{
  static const u8	iconSize = 24;
  static const u8 iconTop = 72-24;
  static const u8 iconLeft = 72-48;

  pos->x = (index % 6) * iconSize + iconLeft;
  pos->y = (index / 6) * iconSize + iconTop;
}


POKEMON_PASO_PARAM* IRCPOKEMONTRADE_GetPokeDataAddress(BOX_MANAGER* boxData , int trayNo, int index,POKEMON_TRADE_WORK* pWork)
{
  POKEMON_PASO_PARAM* ppp;

  if(trayNo!=-1){
    if(trayNo != pWork->BOX_TRAY_MAX && (index != -1)){
      return BOXDAT_GetPokeDataAddress(boxData,trayNo,index);
    }
    else{
      POKEPARTY* party = pWork->pMyParty;
      if(index < PokeParty_GetPokeCount(party) && (index != -1)){
        return (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst( PokeParty_GetMemberPointer( party , index ) );
      }
    }
  }
  return NULL;
}

#endif

//------------------------------------------------------------------------------
/**
 * @brief   コマンド保存バッファをクリア
 * @param   POKEMON_TRADE_WORKポインタ
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _userNetCommandClear(POKEMON_TRADE_WORK* pWork)
{
  GFL_STD_MemClear(pWork->userNetCommand,sizeof(pWork->userNetCommand));
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void IRC_POKMEONTRADE_changeState(POKEMON_TRADE_WORK* pWork,StateFunc state)
{
  pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#if _TRADE_DEBUG
void IRC_POKMEONTRADE_changeStateDebug(POKEMON_TRADE_WORK* pWork,StateFunc state, int line)
{
  OS_TPrintf("trade: %d\n",line);
  IRC_POKMEONTRADE_changeState(pWork, state);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンの受信バッファを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
u8* _setChangePokemonBuffer(int netID, void* pWk, int size)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  if((netID >= 0) && (netID < 2)){
    return (u8*)pWork->recvPoke[netID];
  }
  return NULL;
}


//------------------------------------------------------------------------------
/**
 * @brief   自分と相手でindex指定でバッファを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
POKEMON_PARAM* IRC_POKEMONTRADE_GetRecvPP(POKEMON_TRADE_WORK *pWork, int index)
{
  GF_ASSERT(index < 2);

  if(!GFL_NET_IsInit() || (0==GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )  ){
    return (POKEMON_PARAM*)pWork->recvPoke[index];
  }
  return (POKEMON_PARAM*)pWork->recvPoke[1-index];
}




//_NETCMD_SELECT_POKEMON
static void _recvSelectPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは今は受け取らない
  }

  //ポケモンセットをコール  処理が重いためここではやらない
  pWork->pokemonsetCall = netID+1;

}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンのキャンセルが送られてきた
 *          _NETCMD_CANCEL_POKEMON
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _recvCancelPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())){
    return;//自分のは受け取らない
  }
  _PokemonReset(pWork,1);
  pWork->userNetCommand[netID] = 0;

}

//------------------------------------------------------------------------------
/**
 * @brief   相手のスクロールバーの移動位置が送られてきた
 *          _NETCMD_SCROLLBAR
 * @retval  none
 */
//------------------------------------------------------------------------------

//
static void _recvFriendScrollBar(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;
  short* pRecvData = (short*)pData;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return;	//自分のデータは要らない
  }
  pWork->FriendBoxScrollNum = pRecvData[0];
}

// キャンセルのコールバックの処理
static void _recvChangeCancelFunc(POKEMON_TRADE_WORK *pWork)
{
  //もどす
  if(pWork->bTouchReset){
    //    TOUCH_SW_Reset( pWork->TouchSubWindowSys );
    pWork->bTouchReset=FALSE;
  }
  IRC_POKETRADE_MessageWindowClear(pWork);
  //  _messageDelete(pWork);

  _CHANGE_STATE(pWork, _touchState);
}

//_NETCMD_LOOKATPOKE
static void _recvLookAtPoke(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  pWork->userNetCommand[netID] = _NETCMD_LOOKATPOKE;
}

//_NETCMD_EGG_AND_BATTLE
static void _recvEggAndBattle(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  pWork->userNetCommand[netID] = _NETCMD_EGG_AND_BATTLE;
}

//_NETCMD_CHANGE_CANCEL
static void _recvChangeCancel(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  pWork->userNetCommand[netID] = _NETCMD_CHANGE_CANCEL;
}

//_NETCMD_END
static void _recvEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }

  pWork->userNetCommand[netID] = _NETCMD_END;

  //  _CHANGE_STATE(pWork, NULL);
}


// _NETCMD_CHANGE_POKEMON
static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  POKEMON_TRADE_WORK *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  pWork->userNetCommand[netID] = _NETCMD_CHANGE_POKEMON;

}



//----------------------------------------------------------------------------
/**
 *	@brief	終了同期を取ったのでプロセス終了
 *	@param	POKEMON_TRADE_WORK		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendTimingCheck(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ENDNO)){
    _CHANGE_STATE(pWork,NULL);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了同期を取る
 *	@param	POKEMON_TRADE_WORK		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendTiming(POKEMON_TRADE_WORK* pWork)
{
  GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ENDNO);
  _CHANGE_STATE(pWork,_sendTimingCheck);
}

static void _noneState(POKEMON_TRADE_WORK* pWork)
{
  //なにもしない
}

///ポケモン名をWINDOWに表示
void _pokeNickNameMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,BOOL bEgg,POKEMON_TRADE_WORK* pWork)
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

//ポケモンＬＶを表示
void _pokeLvMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{
  int lv;

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_21, pWork->pExStrBuf );
  lv=	PP_Get(pp, ID_PARA_level, NULL);

  WORDSET_RegisterNumber(pWork->pWordSet, 0, lv, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );

  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWin), x, y, pWork->pStrBuf, pWork->pFontHandle);
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

void _pokeSexMsgDisp(POKEMON_PARAM* pp,GFL_BMPWIN* pWin,int x,int y,POKEMON_TRADE_WORK* pWork)
{ //とくせい
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



//--------------------------------------------------------------
//	ポケモン状態文章セット
//--------------------------------------------------------------

static void _setPokemonStatusMessage(POKEMON_TRADE_WORK *pWork, int side,POKEMON_PARAM* pp)
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
    _pokeLvMsgDisp(pp, pWork->StatusWin[side], 16, 17, pWork);
    _pokePocketItemMsgDisp(pp, pWork->StatusWin[side], 16, 16*8, pWork);
    UITemplate_BALLICON_CreateCLWK( &pWork->aBallIcon[side+UI_BALL_MYSTATUS], pp, pWork->cellUnit,
                                    xdotpos[side]+16, 16, CLSYS_DRAW_MAIN, pWork->heapID );
  }
  GFL_BMPWIN_TransVramCharacter(pWork->StatusWin[side]);
  GFL_BMPWIN_MakeScreen(pWork->StatusWin[side]);
  GFL_BG_LoadScreenV_Req( frame );

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


//--------------------------------------------------------------
//	ポケモン表示セット
//--------------------------------------------------------------
void POKE_MAIN_Pokemonset(POKEMON_TRADE_WORK *pWork, int side, POKEMON_PARAM* pp )
{
  IRCPOKETRADE_PokeDeleteMcss(pWork, side);
  IRCPOKETRADE_PokeCreateMcss(pWork, side, 1-side, pp);
  _setPokemonStatusMessage(pWork,side ,pp);
}


//--------------------------------------------------------------
//	ポケモン表示リセット
//--------------------------------------------------------------
static void _PokemonReset(POKEMON_TRADE_WORK *pWork, int side )
{
  if(pWork->type!=POKEMONTRADE_GTSNEGO){
    IRCPOKETRADE_PokeDeleteMcss(pWork, side);
    IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork, side);
  }
  else{
    int i;
    for(i=0;i<GTS_NEGO_POKESLT_MAX;i++){
      POKE_GTS_PokemonReset(pWork,side,i);
    }
  }
}





// 情報表示の変更 １まいめか２まいめがある
static void _changePokemonMyStDisp(POKEMON_TRADE_WORK* pWork,int pageno,int leftright,POKEMON_PARAM* pp)
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

static void _resetPokemonMyStDisp(POKEMON_TRADE_WORK* pWork)
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


static BOOL _IsEggAndLastBattlePokemonChange(POKEMON_TRADE_WORK* pWork)
{
  POKEMON_PARAM* pp1 = IRC_POKEMONTRADE_GetRecvPP(pWork, 0); //自分のポケモン
  POKEMON_PARAM* pp2 = IRC_POKEMONTRADE_GetRecvPP(pWork, 1); //相手のポケモン
  BOOL bEgg1 = PP_Get(pp1,ID_PARA_tamago_flag,NULL);
  BOOL bEgg2 = PP_Get(pp2,ID_PARA_tamago_flag,NULL);

  if(1==PokeParty_GetBattlePokeNum(pWork->pMyParty) && !bEgg1 && bEgg2 && (pWork->selectBoxno == pWork->BOX_TRAY_MAX)){
    return TRUE;
  }
  return FALSE;
}


//交換選択通信送信
void POKETRE_MAIN_ChangePokemonSendDataNetwork(POKEMON_TRADE_WORK* pWork)
{
  BOOL bSend = FALSE;

  if( GFL_NET_IsInit() ){
    if(_IsEggAndLastBattlePokemonChange(pWork)){
      bSend=GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_EGG_AND_BATTLE,0,NULL);
    }
    else{
      bSend=GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGE_POKEMON,0,NULL);
    }
  }
  else{
    pWork->userNetCommand[0]=_NETCMD_CHANGE_POKEMON;
    pWork->userNetCommand[1]=_NETCMD_CHANGE_POKEMON;
    bSend = TRUE;
  }
  if( bSend ){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_09, pWork->pMessageStrBuf );
    IRC_POKETRADE_MessageWindowOpen(pWork);
    _CHANGE_STATE(pWork,_changeWaitState);
  }
}

// ステータス表示の変更
static void _changePokemonStatusDisp(POKEMON_TRADE_WORK* pWork)
{
  int i,num,num2,bEgg;
  POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, pWork->pokemonselectno);
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
    MCSS_SetVanishFlag(pWork->pokeMcss[1-pWork->pokemonselectno]);

  }
  if(pWork->MyInfoWin){
    GFL_BMPWIN_Delete(pWork->MyInfoWin);
  }
  GFL_FONTSYS_SetColor( 0xf, 0xe, 0 );
  pWork->MyInfoWin =
    GFL_BMPWIN_Create(GFL_BG_FRAME3_M,
                      1, 1, 31 , 24, _BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);

  _pokeNickNameMsgDisp(pp,pWork->MyInfoWin, 16, 0,bEgg,pWork);//ニックネーム
  _pokeLvMsgDisp(pp,pWork->MyInfoWin, 8*12 , 0,pWork);
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


static void _changePokemonStatusDispAuto(POKEMON_TRADE_WORK* pWork,int sel)
{
  if(sel == 0){
    GFL_CLACTPOS pos = {_POKEMON_SELECT1_CELLX,_POKEMON_SELECT1_CELLY+4};
    GFL_CLACT_WK_SetPos( pWork->curIcon[CELL_CUR_POKE_SELECT], &pos, CLSYS_DRAW_SUB );
  }
  else{
    GFL_CLACTPOS pos = {_POKEMON_SELECT2_CELLX,_POKEMON_SELECT2_CELLY+4};
    GFL_CLACT_WK_SetPos( pWork->curIcon[CELL_CUR_POKE_SELECT], &pos, CLSYS_DRAW_SUB );
  }
  _changePokemonStatusDisp(pWork);
}



// ポケモンのステータス表示待ち
static void _pokemonStatusWait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bReturn=FALSE;
  GFL_UI_TP_HITTBL tp_data[]={
    {_POKEMON_SELECT1_CELLY-8,_POKEMON_SELECT1_CELLY+16,_POKEMON_SELECT1_CELLX-8, _POKEMON_SELECT1_CELLX+16},
    {_POKEMON_SELECT2_CELLY-8,_POKEMON_SELECT2_CELLY+16,_POKEMON_SELECT2_CELLX-8, _POKEMON_SELECT2_CELLX+16},
//    {0,8*24,0,32*8-1},
    {GFL_UI_TP_HIT_END,0,0,0},
  };
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , TRUE );

  switch(GFL_UI_TP_HitTrg(tp_data)){
  case 0:
    if(pWork->pokemonselectno!=0){
      pWork->pokemonselectno = 0;
      _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno);
    }
    break;
  case 1:
    if(pWork->pokemonselectno!=1){
      pWork->pokemonselectno = 1;
      _changePokemonStatusDispAuto(pWork,pWork->pokemonselectno);
    }
    break;
  case 2:
    bReturn=TRUE;
    break;
  default:
    break;
  }

  if(GFL_UI_KEY_GetTrg()==PAD_KEY_LEFT || GFL_UI_KEY_GetTrg()==PAD_KEY_RIGHT){
    pWork->pokemonselectno = 1 - pWork->pokemonselectno;
    PMSND_PlaySystemSE(POKETRADESE_CUR);
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
    GFL_BG_SetVisible( GFL_BG_FRAME0_S , FALSE );
    IRC_POKETRADE_GraphicInitMainDisp(pWork);
    IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
    G2_BlendNone();
    POKE_MAIN_Pokemonset(pWork, 0, IRC_POKEMONTRADE_GetRecvPP(pWork, 0));
    POKE_MAIN_Pokemonset(pWork, 1, IRC_POKEMONTRADE_GetRecvPP(pWork, 1));
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait2);
  }
}




// ポケモンのステータス表示
static void _pokemonStatusStart(POKEMON_TRADE_WORK* pWork)
{
  GFL_BG_SetVisible( GFL_BG_FRAME3_M , FALSE );

  IRC_POKETRADE_MainGraphicExit(pWork);  //上画面VRAM開放

  IRC_POKETRADEDEMO_RemoveModel( pWork);  //リールを消す
  IRC_POKETRADE_MessageWindowClear(pWork);  //下のメッセージを消す

  IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,0); //上のステータス文章+OAMを消す
  IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,1); //上のステータス文章+OAMを消す

  pWork->pokemonselectno = 0;//自分から表示
  _changePokemonStatusDisp(pWork);

  IRC_POKETRADE_SetMainStatusBG(pWork);  // 背景BGと
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
  //G2S_BlendNone();
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,TRUE );

  
  _CHANGE_STATE(pWork, _pokemonStatusWait);

}


//交換選択待ち
static void _changePokemonSendData(POKEMON_TRADE_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    IRC_POKETRADE_AppMenuClose(pWork);
    IRC_POKETRADE_MessageWindowClear(pWork);  //下のメッセージを消す
    pWork->pAppTask=NULL;
    switch(selectno){
    case 0:  //こうかん
      _CHANGE_STATE(pWork,POKETRE_MAIN_ChangePokemonSendDataNetwork);
      break;
    case 1:  //つよさをみる
      _CHANGE_STATE(pWork, _pokemonStatusStart);
      break;
    case 2:  //もどる
      _CHANGE_STATE(pWork, _cancelPokemonSendDataNetwork);
      break;
    }
  }
}

//こうかんにだす　通信相手の準備待ち
static void _networkFriendsStandbyWait2(POKEMON_TRADE_WORK* pWork)
{
  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }
  {
    int msg[]={POKETRADE_STR_05, POKETRADE_STR_04, POKETRADE_STR_06};
    IRC_POKETRADE_AppMenuOpen(pWork,msg,elementof(msg));
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  }
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);

  _CHANGE_STATE(pWork,_changePokemonSendData);

}


//こうかんにだす　通信相手の準備待ち
static void _networkFriendsStandbyWait(POKEMON_TRADE_WORK* pWork)
{
  int i;

  int targetID = 1 - GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }

  switch(pWork->userNetCommand[targetID]){
  case _NETCMD_END:
  case _NETCMD_CHANGE_CANCEL:
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_97, pWork->pMessageStrBuf );
    IRC_POKETRADE_MessageWindowOpen(pWork);
    _CHANGE_STATE(pWork, _endCancelState);
    break;
  case _NETCMD_LOOKATPOKE:
    if(pWork->userNetCommand[1-targetID]==_NETCMD_LOOKATPOKE){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_20, pWork->pMessageStrBufEx );
      for(i=0;i<2;i++){
        POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, i);
        WORDSET_RegisterPokeNickName( pWork->pWordSet, i,  pp );
      }
      WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx  );
      IRC_POKETRADE_MessageWindowOpen(pWork);
      _CHANGE_STATE(pWork,_networkFriendsStandbyWait2);
    }
    break;
  case 0:
    break;
  default:
    GF_ASSERT_MSG(0,"%d\n",pWork->userNetCommand[targetID]);//返事がおかしい
    break;
  }
}

// ポケモンデータを相手に送る
static void _pokeSendDataState(POKEMON_TRADE_WORK* pWork)
{
  if(!GFL_NET_IsInit()){
    pWork->userNetCommand[0]=_NETCMD_LOOKATPOKE;
    pWork->userNetCommand[1]=_NETCMD_LOOKATPOKE;
  }

  if(!GFL_NET_IsInit() || GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_LOOKATPOKE, 0,NULL)){
    _CHANGE_STATE(pWork, _networkFriendsStandbyWait);// あいても選ぶまで待つ
  }
}


//こうかんにだす、メニュー待ち
static void _changeMenuWait(POKEMON_TRADE_WORK* pWork)
{
  u32 x,y;
/*
  if(IsTouchCLACTPosition(pWork,FALSE)){
  }

  _vectorUpMath(pWork);
  if(GFL_UI_TP_GetCont()){
    pWork->touchON = TRUE;
  }
  else{
    if((pWork->pCatchCLWK != NULL) && pWork->bUpVec && pWork->touchON){ //ポケモンを上に登録
      PMSND_PlaySystemSE(POKETRADESE_UPPOKE);
      pWork->selectIndex = pWork->workPokeIndex;
      pWork->selectBoxno = pWork->workBoxno;
      GFL_CLACT_WK_SetDrawEnable( pWork->pSelectCLWK, FALSE);
      _CHANGE_STATE(pWork, _changeMenuOpen);
      IRC_POKETRADE_AppMenuClose(pWork);
      pWork->pAppTask=NULL;
      pWork->bUpVec =FALSE;
      pWork->touchON=FALSE;
      _PokemonReset(pWork,0);
      return;
    }
    pWork->touchON = FALSE;
  }

  _CatchPokemonMoveFunc(pWork);
*/


  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    IRC_POKETRADE_AppMenuClose(pWork);
    pWork->pAppTask=NULL;

    if(selectno==0){  //交換に出す
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_09, pWork->pMessageStrBuf );

      IRC_POKETRADE_MessageWindowOpen(pWork);

   //   G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );

      _CHANGE_STATE(pWork, _pokeSendDataState);

    }
    else{  //キャンセル
      //      _PokemonReset(pWork,0);
      _CHANGE_STATE(pWork, _touchState);
    }
  }
}


//こうかんにだす、メニュー表示
static void _changeMenuOpen(POKEMON_TRADE_WORK* pWork)
{
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,FALSE );
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], FALSE );


  if( _PokemonsetAndSendData(pWork) )
  {
    int msg[]={POKETRADE_STR_01,POKETRADE_STR_06};
    IRC_POKETRADE_AppMenuOpen(pWork,msg,elementof(msg));

    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );

    _CHANGE_STATE(pWork, _changeMenuWait);
  }
}


//ラインインデックスの位置にカーソル表示
static void IRC_POKETRADE_CursorEnable(POKEMON_TRADE_WORK* pWork,int line,int index)
{
  int i,j,k;

  pWork->MainObjCursorLine = line;   //OBJカーソルライン
  pWork->MainObjCursorIndex = index;  //OBJカーソルインデックス
  pWork->oldLine++;
  pWork->bgscrollRenew = TRUE;
  _scrollMainFunc(pWork,FALSE,FALSE);

}



static BOOL IsTouchCLACTPosition(POKEMON_TRADE_WORK* pWork, BOOL bCatch)
{
  BOOL bChange=FALSE;
  {
    u32 x,y;
    int line,index;
    if(GFL_UI_TP_GetPointTrg(&x, &y)==TRUE){
      GFL_CLWK* pCL = IRC_POKETRADE_GetCLACT(pWork,x+12,y+12, &pWork->workBoxno, &pWork->workPokeIndex, &line, &index);
      if(pCL){
        POKEMON_PASO_PARAM* ppp =
          IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->workBoxno, pWork->workPokeIndex,pWork);
        _CatchPokemonPositionRewind(pWork);          //今のつかんでる物を元の位置に戻す
        if(ppp && PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) != 0 ){
          _CatchPokemonPositionActive(pWork,pCL);
          IRC_POKETRADE_CursorEnable(pWork,line, index);  //OAMカーソル移動
          OS_TPrintf("GET %d %d\n",pWork->workBoxno,pWork->workPokeIndex);
          if(bCatch){
            pWork->underSelectBoxno  = pWork->workBoxno;
            pWork->underSelectIndex = pWork->workPokeIndex;
            pWork->workPokeIndex = 0;
            pWork->workBoxno = 0;
          }
          else{
            pWork->pCatchCLWK=pCL;
          }
          bChange = TRUE;
        }
      }
    }
  }
  return bChange;
}



//自分画面にステータス表示 相手に見せるかどうか待ち
static void _dispSubStateWait(POKEMON_TRADE_WORK* pWork)
{
  BOOL bExit=FALSE;
  BOOL bChange=FALSE;
  int selectno=-1;
  u32 x,y;

  if(GFL_UI_TP_GetTrg()){
    _CatchPokemonPositionRewind(pWork);
//    pWork->underSelectBoxno = -1;
//    pWork->underSelectIndex = -1;
  }

  
  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_CUR_L:
  case TOUCHBAR_ICON_CUR_R:
    {
      pWork->pokemonselectno = 1 - pWork->pokemonselectno;
      bChange=TRUE;
    }
    break;
  default:
    break;
  }

  _CatchPokemonMoveFunc(pWork);
  
  if(GFL_UI_TP_GetPointTrg(&x, &y)==TRUE){
    if(IsTouchCLACTPosition(pWork,TRUE)){
      pWork->x = x;
      pWork->y = y;
      PMSND_PlaySystemSE(POKETRADESE_CUR);
      bChange = TRUE;
    }
    else{
      if((y >=  2*8) && ((18*8) > y)){
        bExit = TRUE;  //とじる
      }
    }
  }

  if(bChange){
    POKEMON_PASO_PARAM* ppp =
      IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox,
                                         pWork->underSelectBoxno, pWork->underSelectIndex,pWork);
    POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->MyInfoWin), 0 );
    _resetPokemonMyStDisp(pWork);
    _changePokemonMyStDisp(pWork, pWork->pokemonselectno , (pWork->x < 128), pp);
    GFL_HEAP_FreeMemory(pp);
    return;
  }


  _CatchPokemonMoveFunc(pWork);

  if(bExit==FALSE){
    u32 x,y;

    if(GFL_UI_TP_GetCont()==FALSE && pWork->bTouch){ //タッチパネルを離した直後
      if((pWork->pCatchCLWK != NULL) && pWork->bUpVec){ //ポケモンを上に登録
        PMSND_PlaySystemSE(POKETRADESE_UPPOKE);
        selectno = 0;  //決定と同じ
        bExit=TRUE;
      }
      else{  //ポケモンを元の位置に戻す
        _CatchPokemonPositionRewind(pWork);
      }
    }
    if(GFL_UI_TP_GetPointCont(&x,&y)){   //ベクトルを監視  // 上向き判定
//      if((y >=  2*8) && ((18*8) > y)){
//        pWork->x = x;
//        pWork->y = y;
//      }
    }
  }
  _vectorUpMath(pWork);
  _CatchPokemonMoveFunc(pWork);

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    bExit=TRUE;
  }
  if(bExit){
    if(selectno==0){
      //相手に見せる
      pWork->selectIndex = pWork->underSelectIndex;
      pWork->selectBoxno = pWork->underSelectBoxno;
      GFL_CLACT_WK_SetDrawEnable( pWork->pSelectCLWK, FALSE);
      _CHANGE_STATE(pWork, _changeMenuOpen);
    }
    else{
      _CHANGE_STATE(pWork, _touchState);
      GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], TRUE );
    }
    IRC_POKETRADE_ItemIconReset(&pWork->aItemMark);
    IRC_POKETRADE_ItemIconReset(&pWork->aPokerusMark);
    _resetPokemonMyStDisp(pWork);
    IRC_POKETRADE_SubStatusEnd(pWork);
    IRC_POKETRADE_AppMenuClose(pWork);
    pWork->pAppTask=NULL;
  }

  pWork->bTouch = GFL_UI_TP_GetCont();
}


// 自分の画面にステータスを表示

static void _dispSubState(POKEMON_TRADE_WORK* pWork)
{

  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, TRUE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, TRUE);

  {
    int msg[]={POKETRADE_STR_03,POKETRADE_STR_06};
    IRC_POKETRADE_AppMenuOpen(pWork,msg,elementof(msg));
  }

  {
    POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox,
                                                                 pWork->underSelectBoxno, pWork->underSelectIndex,pWork);
    POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);
    pWork->pokemonselectno = 0;
    _changePokemonMyStDisp(pWork, pWork->pokemonselectno , (pWork->x < 128), pp);
    GFL_HEAP_FreeMemory(pp);
  }

  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], FALSE );
  _CatchPokemonPositionRewind(pWork);
  _CHANGE_STATE(pWork,_dispSubStateWait);
}

//通信同期を取る
static void _changeTimingDemoStart(POKEMON_TRADE_WORK* pWork)
{
  if(!GFL_NET_IsInit() || GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_TRADEDEMO_START)){
    switch(pWork->type){
    case POKEMONTRADE_IRC:   ///< 赤外線
      _CHANGE_STATE(pWork,POKMEONTRADE_IRCDEMO_ChangeDemo);
      break;
    case POKEMONTRADE_UNION: ///< ユニオン
    case POKEMONTRADE_WIFICLUB: ///< WIFIクラブ
    case POKEMONTRADE_GTSNEGO: ///< GTSネゴシエーション
      _CHANGE_STATE(pWork,IRC_POKMEONTRADE_STEP_ChangeDemo_PokeMove);
      break;
    }
  }
}

// 交換の返事を待つ
static void _changeWaitState(POKEMON_TRADE_WORK* pWork)
{
  int i;
  int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  int targetID = 1 - myID;

  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }

  switch(pWork->userNetCommand[myID]){
  case _NETCMD_CHANGE_CANCEL:
    if(pWork->userNetCommand[targetID]==_NETCMD_CHANGE_POKEMON || pWork->userNetCommand[targetID]==_NETCMD_EGG_AND_BATTLE ||
       pWork->userNetCommand[targetID]==_NETCMD_CHANGE_CANCEL){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_97, pWork->pMessageStrBuf );
      IRC_POKETRADE_MessageWindowOpen(pWork);
      _CHANGE_STATE(pWork, _endCancelState);
    }
    break;
  case _NETCMD_CHANGE_POKEMON:
    if(pWork->userNetCommand[targetID]==_NETCMD_CHANGE_POKEMON){
      if(GFL_NET_IsInit()){
        GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_TRADEDEMO_START);
      }
      _CHANGE_STATE(pWork, _changeTimingDemoStart);
    }
    else if(pWork->userNetCommand[targetID]==_NETCMD_EGG_AND_BATTLE ||
            pWork->userNetCommand[targetID]==_NETCMD_CHANGE_CANCEL){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_97, pWork->pMessageStrBuf );
      IRC_POKETRADE_MessageWindowOpen(pWork);
      _CHANGE_STATE(pWork, _endCancelState);
    }
    break;
  case _NETCMD_EGG_AND_BATTLE:
    if(pWork->userNetCommand[targetID]==_NETCMD_CHANGE_POKEMON || pWork->userNetCommand[targetID]==_NETCMD_EGG_AND_BATTLE||
       pWork->userNetCommand[targetID]==_NETCMD_CHANGE_CANCEL){
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_98, pWork->pMessageStrBuf );
      IRC_POKETRADE_MessageWindowOpen(pWork);
      _CHANGE_STATE(pWork, _endCancelState);
    }
    break;
  default:
    GF_ASSERT_MSG(0,"%d\n",pWork->userNetCommand[myID]);
    break;
  }
}

void IRC_POKMEONTRADE_ChangeFinish(POKEMON_TRADE_WORK* pWork)
{
  int id = 1-GFL_NET_SystemGetCurrentID();

  pWork->selectBoxno = 0;
  pWork->selectIndex = -1;
  pWork->pCatchCLWK = NULL;
  pWork->pSelectCLWK = NULL;
  IRCPOKETRADE_PokeDeleteMcss(pWork, 0);
  IRCPOKETRADE_PokeDeleteMcss(pWork, 1);

  {
    int i;
    for(i = 0;i< CELL_DISP_NUM;i++){
      if(pWork->curIcon[i]){
        GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[i], TRUE );
      }
    }
  }


  IRC_POKETRADE_MessageWindowClear(pWork);


  _CHANGE_STATE(pWork, _touchState);
}


//キャンセルまち
static void _cancelPokemonSendDataNetwork(POKEMON_TRADE_WORK* pWork)
{
  if(!GFL_NET_IsInit()){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_97, pWork->pMessageStrBuf );
    IRC_POKETRADE_MessageWindowOpen(pWork);
    _CHANGE_STATE(pWork, _endCancelState);
  }
  else if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGE_CANCEL,0,NULL)){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_09, pWork->pMessageStrBuf );
    IRC_POKETRADE_MessageWindowOpen(pWork);
    _CHANGE_STATE(pWork,_changeWaitState);
  }
}


//キャンセル画面待ち
static void _endCancelState(POKEMON_TRADE_WORK* pWork)
{
  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    IRC_POKETRADE_MessageWindowClear(pWork);
    _CHANGE_STATE(pWork,_touchState);
  }
}


//終了待ち
static void _endWaitStateNetwork2(POKEMON_TRADE_WORK* pWork)
{
  int targetID = 1 - GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  if((pWork->userNetCommand[0] == _NETCMD_END) && (pWork->userNetCommand[1] == _NETCMD_END)){
    _CHANGE_STATE(pWork, NULL);
  }
  if(pWork->userNetCommand[targetID] == _NETCMD_LOOKATPOKE){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_04, pWork->pMessageStrBuf );
    IRC_POKETRADE_MessageWindowOpen(pWork);
    _CHANGE_STATE(pWork, _endCancelState);
  }

}


//終わるのを相手に伝えている
static void _endWaitStateNetwork(POKEMON_TRADE_WORK* pWork)
{
  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }
  if(GFL_NET_IsInit()){
    if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_END,0,NULL)){
      _CHANGE_STATE(pWork,_endWaitStateNetwork2);
    }
  }
  else{
    _CHANGE_STATE(pWork, NULL);
  }
}

// おわりまち
static void _endWaitState(POKEMON_TRADE_WORK* pWork)
{
  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    IRC_POKETRADE_AppMenuClose(pWork);
    pWork->pAppTask=NULL;
    switch(selectno){
    case 0:
      GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_06, pWork->pMessageStrBuf );
      IRC_POKETRADE_MessageWindowOpen(pWork);
      _CHANGE_STATE(pWork,_endWaitStateNetwork);
      break;
    case 1:
      IRC_POKETRADE_MessageWindowClear(pWork);
      _CHANGE_STATE(pWork,_touchState);
      break;
    }
  }

}



static BOOL _PokemonsetAndSendData(POKEMON_TRADE_WORK* pWork)
{ //選択ポケモン表示
  BOOL bRet = FALSE;
  POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->selectBoxno, pWork->selectIndex,pWork);
  POKEMON_PARAM* pp = PP_CreateByPPP(ppp,pWork->heapID);

  if(!GFL_NET_IsInit() || GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON, POKETOOL_GetWorkSize(), pp)){
    POKE_MAIN_Pokemonset(pWork,0,pp);
#if PM_DEBUG
    if(!GFL_NET_IsInit()){
      const STRCODE *name;
      POKEMON_PARAM* pp2; 

      pp2 = PP_Create(MONSNO_ONOKKUSU, 100, 123456, GFL_HEAPID_APP);
      name = MyStatus_GetMyName( pWork->pMy );
      PP_Put( pp2 , ID_PARA_oyaname_raw , (u32)name );
      PP_Put( pp2 , ID_PARA_oyasex , MyStatus_GetMySex( pWork->pMy ) );
      POKE_MAIN_Pokemonset(pWork,1,pp2); 
      GFL_STD_MemCopy(pp2,pWork->recvPoke[1],POKETOOL_GetWorkSize());
      GFL_HEAP_FreeMemory(pp2);
      GFL_STD_MemCopy(pp,pWork->recvPoke[0],POKETOOL_GetWorkSize());

    }
#endif
    bRet= TRUE;
  }
  GFL_HEAP_FreeMemory(pp);
  return bRet;
}


//終了コールバック処理
static void _recvEndReqFunc(POKEMON_TRADE_WORK *pWork)
{
  //  _msgWindowCreate(pWork, POKETRADE_STR_22);

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_22, pWork->pMessageStrBuf );
  IRC_POKETRADE_MessageWindowOpen(pWork);

  {
    int msg[]={POKETRADE_STR_27, POKETRADE_STR_28};
    IRC_POKETRADE_AppMenuOpen(pWork,msg,elementof(msg));
  }

  pWork->bTouchReset=TRUE;

  _CHANGE_STATE(pWork, _endWaitState);
}



// 終了しますか？の問いかけそうしん
static void _endRequestState(POKEMON_TRADE_WORK* pWork)
{
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_RETURN, FALSE);
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE );

  _CHANGE_STATE(pWork,_recvEndReqFunc);

#if 0
  if(GFL_NET_IsInit()){
    if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_END_REQ, 0, NULL)){
      _CHANGE_STATE(pWork,_noneState);
    }
  }
  else{
    _CHANGE_STATE(pWork,_recvEndReqFunc);
  }
#endif
}


#define _MOJITOP   (44)
#define _MOJILEFT  (4)


static GFL_UI_TP_HITTBL tp_mojidata[]={
  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT , _MOJILEFT+24},  //あ
  { _MOJITOP+24  , _MOJITOP+24*2 , _MOJILEFT , _MOJILEFT+24},
  { _MOJITOP+24*2, _MOJITOP+24*3 , _MOJILEFT , _MOJILEFT+24},
  { _MOJITOP+24*3, _MOJITOP+24*4 , _MOJILEFT , _MOJILEFT+24},
  { _MOJITOP+24*4, _MOJITOP+24*5 , _MOJILEFT , _MOJILEFT+24},

  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT+24 , _MOJILEFT+24*2},  //か
  { _MOJITOP+24  , _MOJITOP+24*2 , _MOJILEFT+24 , _MOJILEFT+24*2},
  { _MOJITOP+24*2, _MOJITOP+24*3 , _MOJILEFT+24 , _MOJILEFT+24*2},
  { _MOJITOP+24*3, _MOJITOP+24*4 , _MOJILEFT+24 , _MOJILEFT+24*2},
  { _MOJITOP+24*4, _MOJITOP+24*5 , _MOJILEFT+24 , _MOJILEFT+24*2},

  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT+24*2 , _MOJILEFT+24*3},  //さ
  { _MOJITOP+24  , _MOJITOP+24*2 , _MOJILEFT+24*2 , _MOJILEFT+24*3},
  { _MOJITOP+24*2, _MOJITOP+24*3 , _MOJILEFT+24*2 , _MOJILEFT+24*3},
  { _MOJITOP+24*3, _MOJITOP+24*4 , _MOJILEFT+24*2 , _MOJILEFT+24*3},
  { _MOJITOP+24*4, _MOJITOP+24*5 , _MOJILEFT+24*2 , _MOJILEFT+24*3},

  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT+24*3 , _MOJILEFT+24*4},  //た
  { _MOJITOP+24  , _MOJITOP+24*2 , _MOJILEFT+24*3 , _MOJILEFT+24*4},
  { _MOJITOP+24*2, _MOJITOP+24*3 , _MOJILEFT+24*3 , _MOJILEFT+24*4},
  { _MOJITOP+24*3, _MOJITOP+24*4 , _MOJILEFT+24*3 , _MOJILEFT+24*4},
  { _MOJITOP+24*4, _MOJITOP+24*5 , _MOJILEFT+24*3 , _MOJILEFT+24*4},

  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT+24*4 , _MOJILEFT+24*5},  //な
  { _MOJITOP+24  , _MOJITOP+24*2 , _MOJILEFT+24*4 , _MOJILEFT+24*5},
  { _MOJITOP+24*2, _MOJITOP+24*3 , _MOJILEFT+24*4 , _MOJILEFT+24*5},
  { _MOJITOP+24*3, _MOJITOP+24*4 , _MOJILEFT+24*4 , _MOJILEFT+24*5},
  { _MOJITOP+24*4, _MOJITOP+24*5 , _MOJILEFT+24*4 , _MOJILEFT+24*5},

  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT+24*5 , _MOJILEFT+24*6},  //は
  { _MOJITOP+24  , _MOJITOP+24*2 , _MOJILEFT+24*5 , _MOJILEFT+24*6},
  { _MOJITOP+24*2, _MOJITOP+24*3 , _MOJILEFT+24*5 , _MOJILEFT+24*6},
  { _MOJITOP+24*3, _MOJITOP+24*4 , _MOJILEFT+24*5 , _MOJILEFT+24*6},
  { _MOJITOP+24*4, _MOJITOP+24*5 , _MOJILEFT+24*5 , _MOJILEFT+24*6},

  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT+24*6 , _MOJILEFT+24*7},  //ま
  { _MOJITOP+24  , _MOJITOP+24*2 , _MOJILEFT+24*6 , _MOJILEFT+24*7},
  { _MOJITOP+24*2, _MOJITOP+24*3 , _MOJILEFT+24*6 , _MOJILEFT+24*7},
  { _MOJITOP+24*3, _MOJITOP+24*4 , _MOJILEFT+24*6 , _MOJILEFT+24*7},
  { _MOJITOP+24*4, _MOJITOP+24*5 , _MOJILEFT+24*6 , _MOJILEFT+24*7},

  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT+24*7 , _MOJILEFT+24*8},  //や
  { _MOJITOP+24*2, _MOJITOP+24*3 , _MOJILEFT+24*7 , _MOJILEFT+24*8},
  { _MOJITOP+24*4, _MOJITOP+24*5 , _MOJILEFT+24*7 , _MOJILEFT+24*8},

  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT+24*8 , _MOJILEFT+24*9},  //ら
  { _MOJITOP+24  , _MOJITOP+24*2 , _MOJILEFT+24*8 , _MOJILEFT+24*9},
  { _MOJITOP+24*2, _MOJITOP+24*3 , _MOJILEFT+24*8 , _MOJILEFT+24*9},
  { _MOJITOP+24*3, _MOJITOP+24*4 , _MOJILEFT+24*8 , _MOJILEFT+24*9},
  { _MOJITOP+24*4, _MOJITOP+24*5 , _MOJILEFT+24*8 , _MOJILEFT+24*9},

  { _MOJITOP     , _MOJITOP+24   , _MOJILEFT+24*9 , _MOJILEFT+24*10},  //わ

  //  _ENDTABLE,

  {GFL_UI_TP_HIT_END,0,0,0},
};

#if 1
static void _MoveSearchPoke(POKEMON_TRADE_WORK* pWork,int moji)
{
  int i,j,monsno;
  BOOL bFind = FALSE;
  int line = POKETRADE_boxScrollNum2Line(pWork);

  for(i = 0;i < pWork->TRADEBOX_LINEMAX;i++){
    line++;
    if(line >= pWork->TRADEBOX_LINEMAX){
      line=0;
    }
    for(j = 0;j < BOX_VERTICAL_NUM ;j++){
      POKEMON_PASO_PARAM* ppp =
        IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, IRC_TRADE_LINE2TRAY(line,pWork),
                                           IRC_TRADE_LINE2POKEINDEX(line,j),pWork);
      if(ppp){
        monsno = PPP_Get(ppp,ID_PARA_monsno_egg,NULL);
        if(POKEMONTRADE_IsPokeLanguageMark(monsno, moji)){

          OS_TPrintf("みつかった BOX %d\n",line/6);
          pWork->oldLine++;
          pWork->BoxScrollNum = _boxScrollLine2Num(line) - 32;
          pWork->bgscrollRenew = TRUE;
          _scrollMainFunc(pWork,FALSE,TRUE);
          return;
        }
      }
    }
  }
}
#endif

// 検索文字パネル表示中
static void _loopSearchMojiState(POKEMON_TRADE_WORK* pWork)
{
  BOOL endflg=FALSE;
  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }
  {
    int ans = GFL_UI_TP_HitTrg(tp_mojidata);
    if(ans != GFL_UI_TP_HIT_NONE){
      PMSND_PlaySystemSE(POKETRADESE_LANG_SEARCH);
      pWork->selectMoji = ans + 1;
      endflg = TRUE;
      _MoveSearchPoke(pWork,ans);
    }
  }

  TOUCHBAR_Main(pWork->pTouchWork);
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    pWork->selectMoji = 0;
    endflg=TRUE;
    break;
  default:
    break;
  }

  if(endflg){
    OS_TPrintf("SELECTMOJI %d\n", pWork->selectMoji);
    IRC_POKETRADE_EndSubMojiBG(pWork);
    IRC_POKETRADE_MessageWindowClear(pWork);
    GXS_SetVisibleWnd( GX_WNDMASK_NONE );
    _CHANGE_STATE(pWork,_touchState);
  }

}



// 検索文字パネル開始
static void _startSearchMojiState(POKEMON_TRADE_WORK* pWork)
{
  IRC_POKETRADE_InitSubMojiBG(pWork);

  GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR_52, pWork->pMessageStrBuf );
  IRC_POKETRADE_MessageWindowOpen(pWork);

//  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE );

  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );

  {
    G2S_SetWnd0InsidePlane(
      GX_WND_PLANEMASK_BG0|
      GX_WND_PLANEMASK_OBJ,
      FALSE );
    G2S_SetWndOutsidePlane(
      GX_WND_PLANEMASK_BG0|
      GX_WND_PLANEMASK_BG1|
      GX_WND_PLANEMASK_BG2|
      GX_WND_PLANEMASK_BG3|
      GX_WND_PLANEMASK_OBJ,
      TRUE );


    G2S_SetWnd0Position( 0, 8*20, 0xff, 192 );
    GXS_SetVisibleWnd( GX_WNDMASK_W0 );
  }
  
  _CHANGE_STATE(pWork,_loopSearchMojiState);
}

//------------------------------------------------------------------------------
/**
 * @brief   Lineの値からドットピッチを得る
 * @param   line
 * @retval  none
 */
//------------------------------------------------------------------------------

static int _boxScrollLine2Num(int line)
{
  int num=0, pos, page;

  if(line==0){
    num = 0;
  }
  else if(line==1){
    num = _TEMOTITRAY_MAX/2;
  }
  else{
    pos = line - 2;
    page = pos / 6;
    pos = pos % 6;
    num = _TEMOTITRAY_MAX + _BOXTRAY_MAX*page + (_BOXTRAY_MAX/6)*pos;
  }
  return num;
}

//------------------------------------------------------------------------------
/**
 * @brief   画面の外にカーソルがあるかどうか
 * @param   line
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _isCursorInScreen(POKEMON_TRADE_WORK* pWork, int line)
{
  int i=_boxScrollLine2Num(line);


  if((pWork->BoxScrollNum < i) && ((pWork->BoxScrollNum + 224) > i)){
    return TRUE;  //範囲内
  }
  i += pWork->_DOTMAX;
  if((pWork->BoxScrollNum < i) && ((pWork->BoxScrollNum + 224) > i)){
    return TRUE;
  }
  return FALSE;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief 基本のRLキー操作
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
#define _MAXDOT ((BOX_MAX_TRAY*_BOXTRAY_MAX)+_TEMOTITRAY_MAX)
#define _NEXTPOS ((_MAXDOT+_STARTDOT_OFFSET)+(_BOXTRAY_MAX/2)+(_TEMOTITRAY_MAX/2)-_MAXDOT)

static int centerPOS[BOX_MAX_TRAY+1]={
  _NEXTPOS,
  _NEXTPOS+(_BOXTRAY_MAX),
  _NEXTPOS+(_BOXTRAY_MAX)*2,
  _NEXTPOS+(_BOXTRAY_MAX)*3,
  _NEXTPOS+(_BOXTRAY_MAX)*4,
  _NEXTPOS+(_BOXTRAY_MAX)*5,
  _NEXTPOS+(_BOXTRAY_MAX)*6,
  _NEXTPOS+(_BOXTRAY_MAX)*7,
  _NEXTPOS+(_BOXTRAY_MAX)*8,
  _NEXTPOS+(_BOXTRAY_MAX)*9,
  _NEXTPOS+(_BOXTRAY_MAX)*10,
  _NEXTPOS+(_BOXTRAY_MAX)*11,
  _NEXTPOS+(_BOXTRAY_MAX)*12,
  _NEXTPOS+(_BOXTRAY_MAX)*13,
  _NEXTPOS+(_BOXTRAY_MAX)*14,
  _NEXTPOS+(_BOXTRAY_MAX)*15,
  _NEXTPOS+(_BOXTRAY_MAX)*16,
  _NEXTPOS+(_BOXTRAY_MAX)*17,
  _NEXTPOS+(_BOXTRAY_MAX)*18,
  _NEXTPOS+(_BOXTRAY_MAX)*19,
  _NEXTPOS+(_BOXTRAY_MAX)*20,
  _NEXTPOS+(_BOXTRAY_MAX)*21,
  _NEXTPOS+(_BOXTRAY_MAX)*22,
  _NEXTPOS+(_BOXTRAY_MAX)*23,
  (_MAXDOT+_STARTDOT_OFFSET),
};

//--------------------------------------------------------------------------------------------
/**
 * @brief 基本の十字キー操作
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
static void _padUDLRFunc(POKEMON_TRADE_WORK* pWork)
{
  BOOL bChange=FALSE;
  int num,i,line;
  int key = GFL_UI_KEY_GetRepeat();

  if(key & PAD_BUTTON_R ){   //ベクトルを監視
    for(i=0;i < (pWork->BOX_TRAY_MAX+1) ; i++){
      if(centerPOS[i] > pWork->BoxScrollNum){
        pWork->BoxScrollNum = centerPOS[i];
        bChange=TRUE;
        break;
      }
    }
    if(!bChange){
      pWork->BoxScrollNum = centerPOS[0];
      bChange=TRUE;
    }
  }
  else if(key & PAD_BUTTON_L ){   //ベクトルを監視
    for(i = pWork->BOX_TRAY_MAX ; i >= 0 ; i--){
      if(centerPOS[i] < pWork->BoxScrollNum){
        pWork->BoxScrollNum = centerPOS[i];
        bChange=TRUE;
        break;
      }
    }
    if(!bChange){
      pWork->BoxScrollNum = centerPOS[pWork->BOX_TRAY_MAX];
      bChange=TRUE;
    }
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_UP ){   //ベクトルを監視
    pWork->MainObjCursorIndex--;
    if(pWork->MainObjCursorIndex<0){
      pWork->MainObjCursorIndex=0;
    }
    pWork->oldLine--;
    bChange=TRUE;
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_DOWN ){   //ベクトルを監視
    pWork->MainObjCursorIndex++;
    if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM){
      if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM){
        pWork->MainObjCursorIndex = HAND_VERTICAL_NUM-1;
      }
    }
    else{
      if(pWork->MainObjCursorIndex >= BOX_VERTICAL_NUM){
        pWork->MainObjCursorIndex = BOX_VERTICAL_NUM-1;
      }
    }
    pWork->oldLine--;
    bChange=TRUE;
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_RIGHT ){   //ベクトルを監視
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
      pWork->MainObjCursorLine=line;
    }
    else{
      pWork->MainObjCursorLine++;
      if(FALSE == _isCursorInScreen(pWork,pWork->MainObjCursorLine)){
        pWork->BoxScrollNum = _boxScrollLine2Num(pWork->MainObjCursorLine+1)-256;
      }
    }
    if(pWork->MainObjCursorLine >= pWork->TRADEBOX_LINEMAX){
      pWork->MainObjCursorLine = 0;
    }
    if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM){  //手持ちトレイ上ではカーソル位置の補正がある
      if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM){
        pWork->MainObjCursorIndex = HAND_VERTICAL_NUM-1;
      }
    }
    pWork->oldLine--;
    bChange=TRUE;
  }
  else if(GFL_UI_KEY_GetRepeat()==PAD_KEY_LEFT ){   //ベクトルを監視
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
      pWork->MainObjCursorLine=line;
    }
    else{
      pWork->MainObjCursorLine--;
      if(FALSE == _isCursorInScreen(pWork,pWork->MainObjCursorLine )){
        pWork->BoxScrollNum = _boxScrollLine2Num(pWork->MainObjCursorLine);
      }
    }
    if(pWork->MainObjCursorLine < 0){
      pWork->MainObjCursorLine = pWork->TRADEBOX_LINEMAX-1;
    }
    if(pWork->MainObjCursorLine < HAND_HORIZONTAL_NUM){  //手持ちトレイ上ではカーソル位置の補正がある
      if(pWork->MainObjCursorIndex >= HAND_VERTICAL_NUM){
        pWork->MainObjCursorIndex = HAND_VERTICAL_NUM-1;
      }
    }
    pWork->oldLine++;
    bChange=TRUE;
  }

  if(bChange){
    pWork->bgscrollRenew = TRUE;
    PMSND_PlaySystemSE(POKETRADESE_CUR);
    _scrollMainFunc(pWork,FALSE,TRUE);
  }


}

//--------------------------------------------------------------------------------------------
/**
 * @brief スクロールする実行関数
 * @param POKEMON_TRADE_WORK* ワーク
 * @param bSE      SEならすかどうか
 * @param bNetSend 通信位置を相手に送るかどうか
 */
//--------------------------------------------------------------------------------------------

static void _scrollMainFunc(POKEMON_TRADE_WORK* pWork,BOOL bSE, BOOL bNetSend)
{

  if(0 > pWork->BoxScrollNum){
    pWork->BoxScrollNum+=pWork->_DOTMAX;
  }
  if(pWork->_DOTMAX <= pWork->BoxScrollNum){
    pWork->BoxScrollNum-=pWork->_DOTMAX;
  }

  IRC_POKETRADE_TrayDisp(pWork);
  IRC_POKETRADE_SendScreenBoxNameChar(pWork);
  IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork);

  if(!PMSND_CheckPlaySE() && bSE){
    PMSND_PlaySystemSE(POKETRADESE_RIBBON);
  }

  if(GFL_NET_IsInit() && bNetSend){  // 特に失敗してもかまわない通信 相手に位置を知らせているだけ
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SCROLLBAR,4,&pWork->BoxScrollNum);
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 基本ステート初期化
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
static void _touchState(POKEMON_TRADE_WORK* pWork)
{
  int i;

  GFL_STD_MemClear(pWork->pokeIconNo,sizeof(pWork->pokeIconNo));

  _CatchPokemonPositionRewind(pWork);


  if(pWork->pAppTask!=NULL){
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }


  _PokemonReset(pWork,0);
  
  if(!GFL_NET_IsInit()){
    _PokemonReset(pWork,1);
    _userNetCommandClear(pWork);
  }
  else{
    int myID = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

    pWork->userNetCommand[myID]=0;
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CANCEL_POKEMON,0,NULL);
  }

  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetActive( pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, FALSE );
  TOUCHBAR_SetVisible( pWork->pTouchWork, TOUCHBAR_ICON_RETURN ,TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_SCROLLBAR], TRUE );

  IRC_POKETRADE_SendVramBoxNameChar(pWork); // ボックス名初期化
  G2S_BlendNone();

  pWork->oldLine++;
  pWork->bgscrollRenew = TRUE;

  _scrollMainFunc(pWork,FALSE,FALSE);

//  _userNetCommandClear(pWork);

  
  _CHANGE_STATE(pWork,_touchStateCommon);

}




//--------------------------------------------------------------------------------------------
/**
 * @brief タッチ操作で他のアクションに飛ぶ基本ステート
 * @param POKEMON_TRADE_WORK* ワーク
 */
//--------------------------------------------------------------------------------------------
static void _touchStateCommon(POKEMON_TRADE_WORK* pWork)
{
  u32 x,y,i;
  GFL_CLACTPOS pos;
  BOOL bDecided=FALSE;
  int backBoxNo = pWork->nowBoxno;

  // 監視処理
  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){
    pWork->speed=0;
  }
  if(GFL_UI_TP_GetTrg()){
    _CatchPokemonPositionRewind(pWork);
    pWork->underSelectBoxno = -1;
    pWork->underSelectIndex = -1;
  }
  
  _vectorUpMath(pWork);
  _CatchPokemonMoveFunc(pWork);

  //キー処理
  _padUDLRFunc(pWork);

  if(GFL_UI_KEY_GetTrg()== PAD_BUTTON_DECIDE){
    int line;
    if(POKETRADE_IsMainCursorDispIn(pWork, &line)==FALSE){
      pWork->MainObjCursorLine=line;
    }
    i = IRC_TRADE_LINE2POKEINDEX(pWork->MainObjCursorLine,pWork->MainObjCursorIndex);
    if(i!=-1){
      pWork->underSelectBoxno = IRC_TRADE_LINE2TRAY(pWork->MainObjCursorLine,pWork);
      pWork->underSelectIndex = IRC_TRADE_LINE2POKEINDEX(pWork->MainObjCursorLine,pWork->MainObjCursorIndex);
      _CatchPokemonPositionRewind(pWork);
      _CatchPokemonPositionActive(pWork,pWork->pokeIcon[POKETRADE_Line2RingLineIconGet(pWork->MainObjCursorLine)][pWork->MainObjCursorIndex]);
      IRC_POKETRADE_SetCursorXY(pWork);
      IRC_POKETRADE_CursorEnable(pWork,pWork->MainObjCursorLine,pWork->MainObjCursorIndex);
      bDecided=TRUE;
    }
  }
  

  if(IsTouchCLACTPosition(pWork,FALSE)){
    pWork->bStatusDisp=TRUE;
  }


  if( !GFL_UI_TP_GetCont() ){
    if(pWork->touchON && pWork->bUpVec && pWork->pCatchCLWK){
      pWork->touchON = FALSE;
      pWork->bUpVec = FALSE;
      PMSND_PlaySystemSE(POKETRADESE_UPPOKE);
      pWork->underSelectBoxno  = pWork->workBoxno;
      pWork->underSelectIndex = pWork->workPokeIndex;
      pWork->selectIndex = pWork->underSelectIndex;
      pWork->selectBoxno = pWork->underSelectBoxno;
      pWork->underSelectIndex = -1;
      pWork->underSelectBoxno = -1;
      GFL_CLACT_WK_SetDrawEnable( pWork->pSelectCLWK, FALSE);
      pWork->pCatchCLWK = NULL;

      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE);
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, TRUE);

      if(pWork->type!=POKEMONTRADE_GTSNEGO){
        _PokemonsetAndSendData(pWork);
      }
      else{
        
        POKE_GTS_PokemonsetAndSendData(pWork,pWork->selectIndex,pWork->selectBoxno);
      }
    }
    else if(pWork->touchON && pWork->pCatchCLWK && pWork->bStatusDisp){
      bDecided = TRUE;
      pWork->underSelectBoxno = pWork->workBoxno;
      pWork->underSelectIndex = pWork->workPokeIndex;
    }
    else{
      _CatchPokemonPositionRewind(pWork);
    }
    pWork->touchON = FALSE;
    pWork->bUpVec = FALSE;
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[CELL_CUR_SCROLLBAR] , FALSE );
  }

  
  if( bDecided ){
    POKEMON_PASO_PARAM* ppp = IRCPOKEMONTRADE_GetPokeDataAddress(pWork->pBox, pWork->underSelectBoxno, pWork->underSelectIndex,pWork);

    if(ppp && PPP_Get( ppp, ID_PARA_poke_exist, NULL  ) != 0 ){
      PMSND_PlaySystemSE(POKETRADESE_MARKON);
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);


      _CHANGE_STATE(pWork,_dispSubState);
      return;
    }
    else{
      _CatchPokemonPositionRewind(pWork);
      pWork->underSelectBoxno = -1;
      pWork->underSelectIndex = -1;
    }
  }


  if(GFL_UI_TP_GetPointCont(&x,&y)){     // パネルスクロール
    if(pWork->touchON){
      if((x >=  64) && ((192) > x)){
        if((y >=  152+12) && ((176+12) > y)){
          GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[CELL_CUR_SCROLLBAR] , TRUE );
          pWork->speed = (x - pWork->xspeed)*2;
          pWork->xspeed = x;
          pWork->BoxScrollNum -= pWork->speed;
          if(pWork->speed > 12){
            _scrollMainFunc(pWork,TRUE,TRUE);
          }
          else{
            _scrollMainFunc(pWork,FALSE,TRUE);
          }
        }
      }
    }
    //pWork->x = x;
    //pWork->y = y;
    pWork->touchON = TRUE;
    pWork->xspeed = x;
  }
  else{
    pWork->xspeed=0;
  }

  if(GFL_UI_TP_GetCont()==FALSE){  //慣性
    if(pWork->speed!=0){
      int k=2;
      while(k && pWork->speed){
        if(pWork->speed < 0){
          pWork->speed++;
        }
        if(pWork->speed > 0){
          pWork->speed--;
        }
        k--;
      }
      pWork->BoxScrollNum -= pWork->speed;
      _scrollMainFunc(pWork,TRUE,TRUE);
    }
  }
    
  TOUCHBAR_Main(pWork->pTouchWork);   //タッチバー
  switch( TOUCHBAR_GetTrg(pWork->pTouchWork )){
  case TOUCHBAR_ICON_RETURN:
    _CHANGE_STATE(pWork,_endRequestState);
    return;
  case TOUCHBAR_ICON_CUTSOM1:
    _CHANGE_STATE(pWork,_startSearchMojiState);
    return;
  case TOUCHBAR_ICON_CUTSOM2:
    if(pWork->selectIndex == -1){  //ポケモン選んで無い場合元に戻す
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, TRUE);
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);
    }
    else{
      _CHANGE_STATE(pWork, _changeMenuOpen);
    }
    return;
    break;
  default:
    break;
  }
  if(pWork->pAppTask){
    if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
      int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
      IRC_POKETRADE_AppMenuClose(pWork);
      pWork->pAppTask=NULL;

      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM1, FALSE);
      TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);

      _CHANGE_STATE(pWork, POKE_GTS_Select6Init);
    }
    
  }

  
}

//----------------------------------------------------------------------------
/**
 *	@brief	GTSネゴシエーションだと最初に台詞が入る
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void _gtsFirstMsgState(POKEMON_TRADE_WORK* pWork)
{
  if(!IRC_POKETRADE_MessageEndCheck(pWork)){
    return;
  }

  if(GFL_UI_KEY_GetTrg() || GFL_UI_TP_GetTrg()){

    IRC_POKETRADE_MessageWindowClear(pWork);
    G2S_BlendNone();

    _CHANGE_STATE(pWork,_touchState);

  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR初期化
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void POKETRADE_TOUCHBAR_Init(POKEMON_TRADE_WORK* pWork)
{
  //アイコンの設定
  //数分作る
  TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
  {
    {
      TOUCHBAR_ICON_RETURN,
      {	TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
    },
    {
      TOUCHBAR_ICON_CUR_L,
      {	TOUCHBAR_ICON_X_00, TOUCHBAR_ICON_Y },
    },
    {
      TOUCHBAR_ICON_CUR_R,
      {	TOUCHBAR_ICON_X_01, TOUCHBAR_ICON_Y },
    },
    {
      TOUCHBAR_ICON_CUTSOM1,
      {	TOUCHBAR_ICON_X_00, TOUCHBAR_ICON_Y },
    },
    {
      TOUCHBAR_ICON_CUTSOM2,
      {	TOUCHBAR_ICON_X_00, TOUCHBAR_ICON_Y },
    },
  };

  TOUCHBAR_SETUP	touchbar_setup;
  GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

  touchbar_setup.p_item		= touchbar_icon_tbl;				//上の窓情報
  touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
  touchbar_setup.p_unit		= pWork->cellUnit;										//OBJ読み込みのためのCLUNIT
  touchbar_setup.is_notload_bg =TRUE;  //BGはなし
  touchbar_setup.bar_frm	= GFL_BG_FRAME0_S;						//BG読み込みのためのBG面上下画面判定にも必要
  touchbar_setup.bg_plt		= 0;			//BGﾊﾟﾚｯﾄ
  touchbar_setup.obj_plt	= 0;			//OBJﾊﾟﾚｯﾄ
  touchbar_setup.mapping	= APP_COMMON_MAPPING_128K;	//マッピングモード

  touchbar_icon_tbl[3].cg_idx = pWork->cellRes[CHAR_SCROLLBAR];
  touchbar_icon_tbl[3].plt_idx = pWork->cellRes[PAL_SCROLLBAR];
  touchbar_icon_tbl[3].cell_idx = pWork->cellRes[ANM_SCROLLBAR];
  touchbar_icon_tbl[3].active_anmseq	=	6;						//アクティブのときのアニメ
  touchbar_icon_tbl[3].noactive_anmseq	=		5;						//ノンアクティブのときのアニメ
  touchbar_icon_tbl[3].push_anmseq	=		4;						//押したときのアニメ（STOPになっていること）
  touchbar_icon_tbl[3].key	=		0;		//キーで押したときに動作させたいならば、ボタン番号
  touchbar_icon_tbl[3].se	=		POKETRADESE_DECIDE;									//押したときにSEならしたいならば、SEの番号

  touchbar_icon_tbl[4].cg_idx = pWork->cellRes[CHAR_SCROLLBAR];
  touchbar_icon_tbl[4].plt_idx = pWork->cellRes[PAL_SCROLLBAR];
  touchbar_icon_tbl[4].cell_idx = pWork->cellRes[ANM_SCROLLBAR];
  touchbar_icon_tbl[4].active_anmseq	=	9;						//アクティブのときのアニメ
  touchbar_icon_tbl[4].noactive_anmseq	=		8;						//ノンアクティブのときのアニメ
  touchbar_icon_tbl[4].push_anmseq	=		7;						//押したときのアニメ（STOPになっていること）
  touchbar_icon_tbl[4].key	=		0;		//キーで押したときに動作させたいならば、ボタン番号
  touchbar_icon_tbl[4].se	=		POKETRADESE_DECIDE;									//押したときにSEならしたいならば、SEの番号

  pWork->pTouchWork = TOUCHBAR_Init(&touchbar_setup, pWork->heapID);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUR_L, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUR_R, FALSE);
  TOUCHBAR_SetVisible(pWork->pTouchWork, TOUCHBAR_ICON_CUTSOM2, FALSE);

}

//------------------------------------------------------------------------------
/**
 * @brief   文字列関連の初期化と開放
 * @retval  none
 */
//------------------------------------------------------------------------------

void POKETRADE_MessageHeapInit(POKEMON_TRADE_WORK* pWork)
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

}

void POKETRADE_MessageHeapEnd(POKEMON_TRADE_WORK* pWork)
{
  int i;
//  if(pWork->MessageWin){
  //  GFL_BMPWIN_Delete(pWork->MessageWin);
    //pWork->MessageWin=NULL;
//  }
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
 * @brief   MCSSと3D画面システムの初期化と開放  この関数はデモにいっても有効にする為LOWからとってある
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _mcssSystemHeapInit(POKEMON_TRADE_WORK* pWork)
{
  IRC_POKETRADE_3DGraphicSetUp( pWork );

  pWork->mcssSys = MCSS_Init( 4 , GetHeapLowID(pWork->heapID) );
  MCSS_SetTextureTransAdrs( pWork->mcssSys , 0x30000 );
  MCSS_SetOrthoMode( pWork->mcssSys );
}

static void _mcssSystemHeapEnd(POKEMON_TRADE_WORK* pWork)
{
  IRCPOKETRADE_PokeDeleteMcss(pWork, 0);
  IRCPOKETRADE_PokeDeleteMcss(pWork, 1);
  MCSS_Exit(pWork->mcssSys);
  GFL_G3D_CAMERA_Delete(pWork->pCamera);
  GFL_G3D_Exit();

  
}




//------------------------------------------------------------------------------
/**
 * @brief   画面システム周りの初期化と開放
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _dispSystemHeapInit(POKEMON_TRADE_WORK* pWork)
{
  GFL_BG_Init( pWork->heapID );
  GFL_BMPWIN_Init( pWork->heapID );

  IRC_POKETRADE_SetMainDispGraphic(pWork);
  IRC_POKETRADE_SetMainVram(pWork);
  IRC_POKETRADE_SetSubVram(pWork);

}

static void _dispSystemHeapEnd(POKEMON_TRADE_WORK* pWork)
{
  MCSS_Exit(pWork->mcssSys);
  GFL_CLACT_SYS_Delete();
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

//------------------------------------------------------------------------------
/**
 * @brief   画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _dispInit(POKEMON_TRADE_WORK* pWork)
{

  IRC_POKETRADE_CommonCellInit(pWork);  //共通CELL

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);

  IRC_POKETRADE_InitBoxCursor(pWork);

  IRC_POKETRADE_CreatePokeIconResource(pWork);

  IRC_POKETRADE_SendVramBoxNameChar(pWork); // ボックス名初期化

  IRC_POKETRADE_GraphicInitSubDisp(pWork);  //BGを
  IRC_POKETRADE_GraphicInitMainDisp(pWork);

  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, _BRIGHTNESS_SYNC);

  GFL_NET_WirelessIconEasy_HoldLCD(TRUE,pWork->heapID); //通信アイコン
  GFL_NET_ReloadIcon();

  IRC_POKETRADE_InitBoxIcon(pWork->pBox, pWork);  //ポケモンの表示

}


//--------------------------------------------------------------
/**
 * G3D VBlank処理
 * @param TCB GFL_TCB
 * @param work tcb work
 * @retval nothing
 */
//--------------------------------------------------------------
static void	_VBlank( GFL_TCB *tcb, void *work )
{
  POKEMON_TRADE_WORK *pWork=work;

  GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank

  if(pWork->bgscrollRenew){

    GFL_BG_SetScroll(GFL_BG_FRAME3_S,GFL_BG_SCROLL_X_SET, pWork->bgscroll);
    GFL_BG_SetScroll(GFL_BG_FRAME1_S,GFL_BG_SCROLL_X_SET, pWork->bgscroll);
    pWork->bgscrollRenew=FALSE;
  }
}


#if PM_DEBUG
//--------------------------------------------------------------
/**
 * @brief   デバッグ用に適当に手持ちポケモンをAdd
 * @param   gsys
 */
//--------------------------------------------------------------
static void DEBUG_MyPokeAdd(POKEPARTY *party,MYSTATUS *myStatus,HEAPID heapID)
{
  POKEMON_PARAM *pp;
  const STRCODE *name;
  int i,itemno;
  MAIL_DATA* pMail;
  STRBUF* pStrBuf= GFL_STR_CreateBuffer( 128, heapID );

  while( PokeParty_GetPokeCount(party)){
    PokeParty_Delete(party,0);
  }

  name = MyStatus_GetMyName( myStatus );

  pp = PP_Create(MONSNO_ONOKKUSU, 100, 123456, GFL_HEAPID_APP);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );

  MyStatus_SetMyNameFromString(myStatus, pStrBuf);

  pMail=MailData_MailEventDataMake(pp, 1, MyStatus_GetMySex( myStatus ),pStrBuf,
                                   123, heapID);
  itemno = ITEM_MailNumGet(MailData_GetDesignNo(pMail));
  PP_Put(pp,ID_PARA_mail_data,(u32)pMail);
  PP_Put(pp,ID_PARA_item,itemno);

  PokeParty_Add(party, pp);

  PP_Setup(pp, MONSNO_MUSYAANA, 100, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PokeParty_Add(party, pp);
  PokeParty_Add(party, pp);
  PokeParty_Add(party, pp);
  PP_Setup(pp, MONSNO_MANAFI, 100, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PokeParty_Add(party, pp);
  PokeParty_Add(party, pp);
  GFL_HEAP_FreeMemory(pp);
  GFL_HEAP_FreeMemory(pMail);
  GFL_HEAP_FreeMemory(pStrBuf);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   セーブ関連の初期化と開放
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _savedataHeapInit(POKEMON_TRADE_WORK* pWork,GAMESYS_WORK* pParentWork)
{
  if(pParentWork){
    GAMEDATA* pGameData = GAMESYSTEM_GetGameData(pParentWork);
    pWork->pGameData=pGameData;
    pWork->pBox = GAMEDATA_GetBoxManager(pGameData);
    pWork->pMy = GAMEDATA_GetMyStatus( pGameData );
    pWork->pMyParty = GAMEDATA_GetMyPokemon(pGameData);
    pWork->pMailBlock = SaveData_GetMailBlock(GAMEDATA_GetSaveControlWork(pGameData));
  }
#if PM_DEBUG
  else{
    pWork->pBox = BOX_DAT_InitManager(pWork->heapID,SaveControl_GetPointer());
    pWork->pMy = MyStatus_AllocWork(pWork->heapID);
    pWork->pFriend = MyStatus_AllocWork(pWork->heapID);

    MyStatus_Init(pWork->pMy);
    MyStatus_Init(pWork->pFriend);
    
    pWork->pMailBlock = GFL_HEAP_AllocClearMemory(pWork->heapID,MAIL_GetBlockWorkSize());
    MAIL_Init(pWork->pMailBlock);
    {
      POKEMON_PARAM *pp;
      int i,j;
      BOX_MANAGER* pBox = pWork->pBox;
      pp = PP_Create(MONSNO_ONOKKUSU, 100, 123456, HEAPID_IRCBATTLE);

      for(i=0;i < 3;i++){
        for(j=0;j < 2;j++){
          u16 oyaName[5] = {L'デ',L'バ',L'ッ',L'グ',0xFFFF};
          POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle(MONSNO_MARIRU+i, 0, GFL_HEAPID_APP);
          u32 ret = POKE_PERSONAL_GetParam(ppd,POKEPER_ID_sex);

          PP_SetupEx(pp, MONSNO_RIZAADON, i+j, 123456,PTL_SETUP_POW_AUTO, ret);
          PP_Put( pp , ID_PARA_oyaname_raw , (u32)oyaName );
          PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex(  pWork->pMy ) );
          PP_Put( pp , ID_PARA_item , 18 );
          PP_Put( pp , ID_PARA_pokerus , 1 );

          BOXDAT_PutPokemonBox(pBox, i, (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst(pp));

          POKE_PERSONAL_CloseHandle(ppd);
        }
      }
      GFL_HEAP_FreeMemory(pp);
    }

    pWork->pMyParty = PokeParty_AllocPartyWork(pWork->heapID);
  }
#endif
#if DEBUG_ONLY_FOR_ohno
  DEBUG_MyPokeAdd(pWork->pMyParty, pWork->pMy, pWork->heapID);
#endif
}


static void _savedataHeapEnd(POKEMON_TRADE_WORK* pWork,BOOL bParentWork)
{
  if(bParentWork){
  }
#if PM_DEBUG
  else{
    BOX_DAT_ExitManager(pWork->pBox);
    GFL_HEAP_FreeMemory(pWork->pMy);
    GFL_HEAP_FreeMemory(pWork->pFriend);
    GFL_HEAP_FreeMemory(pWork->pMyParty);
    GFL_HEAP_FreeMemory(pWork->pMailBlock );
  }
#endif

}

//------------------------------------------------------------------------------
/**
 * @brief        MAX_TRAY_NUMに関しての変数を初期化
 * @param pwk    EVENT_IRCBATTLE_WORKが必要
 * @retval       none
 */
//------------------------------------------------------------------------------

static void _maxTrayNumInit(POKEMON_TRADE_WORK *pWork)
{
  int max = BOXDAT_GetTrayMax(pWork->pBox);
  
  pWork->TRADEBOX_LINEMAX = max * 6 + 2;
  pWork->BOX_TRAY_MAX = max;
  pWork->_SRCMAX = ((max*_BOXTRAY_SCR_MAX)+_TEMOTITRAY_SCR_MAX);
  pWork->_DOTMAX =  ((max*_BOXTRAY_MAX)+_TEMOTITRAY_MAX);
//  pWork->_DOT_START_POS = (pWork->_DOTMAX - 80);
}





FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(app_mail);

//------------------------------------------------------------------------------
/**
 * @brief        PROCスタート
 * @param pwk    EVENT_IRCBATTLE_WORKが必要
 * @retval       none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT PokemonTradeProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk,int type )
{
  int i;
  GAMESYS_WORK* pParentWork = pwk;
  POKEMON_TRADE_WORK *pWork = mywk;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  GFL_OVERLAY_Load( FS_OVERLAY_ID(app_mail));
  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);

  pWork->heapID = HEAPID_IRCBATTLE;
  pWork->type = type;
  pWork->recvPoke[0] = GFL_HEAP_AllocClearMemory(pWork->heapID, POKETOOL_GetWorkSize());
  pWork->recvPoke[1] = GFL_HEAP_AllocClearMemory(pWork->heapID, POKETOOL_GetWorkSize());
  _savedataHeapInit(pWork,pParentWork);
  _maxTrayNumInit(pWork);
  _mcssSystemHeapInit(pWork);
  _dispSystemHeapInit(pWork);

  //セル系システムの作成
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 340 , 0 , pWork->heapID );
  // 通信テーブル追加
  if(GFL_NET_IsInit()){
    GFL_NET_AddCommandTable(GFL_NET_CMD_IRCTRADE,_PacketTbl,NELEMS(_PacketTbl), pWork);
  }
  // ワークの初期化
  pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );
  pWork->pCatchCLWK = NULL;
  pWork->selectIndex = -1;
  
  POKETRADE_MessageHeapInit(pWork);
  _dispInit(pWork);
  POKETRADE_TOUCHBAR_Init(pWork);
  
  IRC_POKETRADEDEMO_Init(pWork);

#if DEBUG_ONLY_FOR_ohno | DEBUG_ONLY_FOR_ibe_mana
  DEBUGWIN_InitProc( GFL_BG_FRAME3_M , pWork->pFontHandle );
  DEBUG_PAUSE_SetEnable( TRUE );
#endif

  IRC_POKETRADEDEMO_SetModel( pWork, REEL_PANEL_OBJECT);

  if(type != POKEMONTRADE_GTSNEGO){
    _CHANGE_STATE(pWork, _touchState);
  }
  else{
    POKE_GTS_InitWork(pWork);
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_13, pWork->pMessageStrBuf );
    IRC_POKETRADE_MessageWindowOpen(pWork);
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 8 );
    
    POKE_GTS_CreatePokeIconResource(pWork,CLSYS_DRAW_MAIN);      
    POKE_GTS_StatusMessageDisp(pWork);
    _CHANGE_STATE(pWork, _gtsFirstMsgState);
  }

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT PokemonTradeUnionProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_GTSNEGO_WORK* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, HEAPSIZE_POKETRADE );
  pWork = GFL_PROC_AllocWork( proc, sizeof( POKEMON_TRADE_WORK ), HEAPID_IRCBATTLE );
  GFL_STD_MemClear(pWork, sizeof(POKEMON_TRADE_WORK));

  return PokemonTradeProcInit(proc,seq,pwk,pWork,POKEMONTRADE_UNION);
}

static GFL_PROC_RESULT PokemonTradeIrcProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_GTSNEGO_WORK* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, HEAPSIZE_POKETRADE );
  pWork = GFL_PROC_AllocWork( proc, sizeof( POKEMON_TRADE_WORK ), HEAPID_IRCBATTLE );
  GFL_STD_MemClear(pWork, sizeof(POKEMON_TRADE_WORK));

  return PokemonTradeProcInit(proc,seq,pwk,pWork,POKEMONTRADE_IRC);
}

static GFL_PROC_RESULT PokemonTradeGTSNegoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_GTSNEGO_WORK* pParent=pwk;
  POKEMON_TRADE_WORK *pWork;
  int i;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, HEAPSIZE_POKETRADE );
  pWork = GFL_PROC_AllocWork( proc, sizeof( POKEMON_TRADE_WORK ), HEAPID_IRCBATTLE );
  GFL_STD_MemClear(pWork, sizeof(POKEMON_TRADE_WORK));

  for(i=0;i<2;i++){
    pWork->GTStype[i]=pParent->aUser[i].selectType;  //
    pWork->GTSlv[i]=pParent->aUser[i].selectLV;  //
  }
  pWork->pFriend=pParent->pStatus[1];

  return PokemonTradeProcInit(proc,seq,pParent->gsys ,pWork,POKEMONTRADE_GTSNEGO);
}


//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT PokemonTradeProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMON_TRADE_WORK* pWork = mywk;
  GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;
  StateFunc* state = pWork->state;

  if(DEBUGWIN_IsActive()){
    return GFL_PROC_RES_CONTINUE;
  }


  if(state != NULL){
    state(pWork);
    pWork->anmCount++;
    retCode = GFL_PROC_RES_CONTINUE;
  }
  //ポケモンセットをコール
  if(pWork->pokemonsetCall != 0){
    POKE_MAIN_Pokemonset(pWork, 1, pWork->recvPoke[pWork->pokemonsetCall-1]);
    pWork->pokemonsetCall=0;
  }

  GFL_CLACT_SYS_Main(); // CLSYSメイン
  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }

  if(pWork->pMsgTcblSys){
   u8 defL, defS, defB;
    GFL_FONTSYS_GetColor( &defL, &defS, &defB );
    GFL_FONTSYS_SetColor(FBMP_COL_BLACK, FBMP_COL_BLK_SDW, 15);
    GFL_TCBL_Main( pWork->pMsgTcblSys );
    GFL_FONTSYS_SetColor( defL, defS, defB );
    PRINTSYS_QUE_Main(pWork->SysMsgQue);
  }

  GFL_G3D_DRAW_Start();
  GFL_G3D_CAMERA_Switching( pWork->pCamera );

  GFL_G3D_DRAW_SetLookAt();

  MCSS_Main( pWork->mcssSys );
  MCSS_Draw( pWork->mcssSys );
  //IRC_POKETRADE_G3dDraw(pWork);

  IRC_POKETRADEDEMO_Main(pWork);

  GFL_G3D_DRAW_End();


  //	ConnectBGPalAnm_Main(&pWork->cbp);

  return retCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT PokemonTradeProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  int i;
  POKEMON_TRADE_WORK* pWork = mywk;


  if(pWork->type == POKEMONTRADE_GTSNEGO){
    POKE_GTS_EndWork(pWork);
  }  
  IRC_POKETRADE_ResetBoxNameWindow(pWork);
  DEBUGWIN_ExitProc();
  IRC_POKETRADE_ItemIconReset(&pWork->aItemMark);
  IRC_POKETRADE_ItemIconReset(&pWork->aPokerusMark);

  if(GFL_NET_IsInit()){
    GFL_NET_DelCommandTable(GFL_NET_CMD_IRCTRADE);
  }
  //  TOUCH_SW_FreeWork( pWork->TouchSubWindowSys );


  IRC_POKETRADE_MainGraphicExit(pWork);
  IRC_POKETRADE_SubGraphicExit(pWork);
  TOUCHBAR_Exit(pWork->pTouchWork);


  IRC_POKETRADEDEMO_RemoveModel( pWork);
  IRC_POKETRADEDEMO_End(pWork);

  IRC_POKETRADE_AllDeletePokeIconResource(pWork);

  IRC_POKETRADE_EndIconResource(pWork);  //OAMを全部消去


  GFL_HEAP_FreeMemory(pWork->recvPoke[0]);
  GFL_HEAP_FreeMemory(pWork->recvPoke[1]);

  GFL_TCB_DeleteTask( pWork->g3dVintr );

  GFL_CLACT_UNIT_Delete(pWork->cellUnit);

  _savedataHeapEnd(pWork,(pwk!=NULL));
  POKETRADE_MessageHeapEnd(pWork);
  _mcssSystemHeapEnd(pWork);
  _dispSystemHeapEnd(pWork);

  GFL_PROC_FreeWork(proc);
  GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);
  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(app_mail));

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------

//UNION WIFICLUB用
const GFL_PROC_DATA PokemonTradeProcData = {
  PokemonTradeUnionProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};

//IR用
const GFL_PROC_DATA PokemonTradeIrcProcData = {
  PokemonTradeIrcProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};

//wifiネゴシエーション用
const GFL_PROC_DATA PokemonTradeWiFiProcData = {
  PokemonTradeGTSNegoProcInit,
  PokemonTradeProcMain,
  PokemonTradeProcEnd,
};


