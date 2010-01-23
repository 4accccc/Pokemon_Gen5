//=============================================================================
/**
 * @file	  pokemontrade_ircdemo.c
 * @bfief	  ポケモン交換赤外線のデモアニメーション
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/05
 */
//=============================================================================

#include <gflib.h>


#include "arc_def.h"
#include "net/network_define.h"
#include "system/wipe.h"

#include "net_app/pokemontrade.h"
#include "system/main.h"

#include "poke_icon.naix"
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
#include "ircbattle.naix"
#include "trade.naix"
#include "net_app/connect_anm.h"
#include "net/dwc_rapfriend.h"
#include "savedata/wifilist.h"

#include "system/touch_subwindow.h"

#include "poke_tool/poke_tool_def.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"
#include "poke_tool/pokeparty.h"
#include "poke_tool/status_rcv.h"
#include "tradedemo.naix"
#include "tradeirdemo.naix"

#include "pokemontrade_local.h"
#include "app/mailtool.h"

#include "spaheadir.h"


static void _changeDemo_ModelT1(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelT2(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade0(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade1(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade2(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade3(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade20(POKEMON_TRADE_WORK* pWork);
static void _setFadeMask(_D2_PAL_FADE_WORK* pD2Fade);
static void	_FIELD_StartPaletteFade( _EFFTOOL_PAL_FADE_WORK* epfw, u8 start_evy, u8 end_evy, u8 wait, u16 rgb );
static _EFFTOOL_PAL_FADE_WORK* _createPaletteFade(GFL_G3D_RES* g3DRES,HEAPID heapID);
static void	_freePaletteFade( _EFFTOOL_PAL_FADE_WORK* pwk );
static void  _EFFTOOL_CalcPaletteFade( _EFFTOOL_PAL_FADE_WORK *epfw );
static void _pokeMoveRenew(_POKEMCSS_MOVE_WORK* pPoke,int time, const VecFx32* pPos);
static _POKEMCSS_MOVE_WORK* _pokeMoveCreate(MCSS_WORK* pokeMcss, int time, const VecFx32* pPos, HEAPID heapID);
static void _pokeMoveFunc(_POKEMCSS_MOVE_WORK* pMove);

//速度調整関数
static int ANMCNTC(int no)
{
  return no;
}


//------------------------------------------------------------------
/**
 * @brief   次のシーンに必要な値をセット
 * @param   POKEMON_TRADE_WORK ワーク
 * @param   _TRADE_SCENE_NO_E no シーン管理enum
 * @retval  none
 */
//------------------------------------------------------------------

static void _setNextAnim(POKEMON_TRADE_WORK* pWork, int timer)
{
  pWork->anmCount = timer;
}

//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCT素材INIT
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------
static void _pokemonCreateCLACTInit(POKEMON_TRADE_WORK* pWork)
{
  
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_POKETRADEDEMO, pWork->heapID );

  pWork->cellRes[CHAR_POKECREATE] =
    GFL_CLGRP_CGR_Register( p_handle , NARC_tradedemo_trade_obj01_NCGR ,
                            FALSE , CLSYS_DRAW_MAIN , pWork->heapID );
  pWork->cellRes[PLT_POKECREATE] =
    GFL_CLGRP_PLTT_RegisterEx(
      p_handle ,NARC_tradedemo_trade_obj01_NCLR , CLSYS_DRAW_MAIN ,
      PLTID_OBJ_DEMO_M * 32 , 0, 1, pWork->heapID  );
  pWork->cellRes[ANM_POKECREATE] =
    GFL_CLGRP_CELLANIM_Register(
      p_handle , NARC_tradedemo_trade01_NCER,
      NARC_tradedemo_trade01_NANR , pWork->heapID  );
  GFL_ARC_CloseDataHandle( p_handle );
  pWork->pPokemonTradeDemo->pPokeCreateCLWK=NULL;



}
//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCT素材Exit
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------

static void _pokemonCreateCLACTExit(POKEMON_TRADE_WORK* pWork)
{
  
  if(pWork->pPokemonTradeDemo->pPokeCreateCLWK){
    GFL_CLACT_WK_Remove( pWork->pPokemonTradeDemo->pPokeCreateCLWK);
  }
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_POKECREATE] );
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_POKECREATE] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_POKECREATE] );
  pWork->cellRes[CHAR_POKECREATE] = 0;
  pWork->cellRes[PLT_POKECREATE] = 0;
  pWork->cellRes[ANM_POKECREATE] = 0;
  pWork->pPokemonTradeDemo->pPokeCreateCLWK=NULL;
     
}

//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCTをだす
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------
static void _pokemonCreateCLACTAdd(POKEMON_TRADE_WORK* pWork)
{
  
  if(pWork->pPokemonTradeDemo->pPokeCreateCLWK==NULL){
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 99;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    pWork->pPokemonTradeDemo->pPokeCreateCLWK = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                  pWork->cellRes[CHAR_POKECREATE],
                                                  pWork->cellRes[PLT_POKECREATE],
                                                  pWork->cellRes[ANM_POKECREATE],
                                                  &cellInitData ,CLSYS_DRAW_MAIN , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->pPokemonTradeDemo->pPokeCreateCLWK , TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->pPokemonTradeDemo->pPokeCreateCLWK, TRUE );
  }
  else{
    GFL_CLACT_WK_ResetAnm(pWork->pPokemonTradeDemo->pPokeCreateCLWK);
  }
     
}


static void _WIPE_SYS_StartRap(int pattern, int wipe_m, int wipe_s, u16 color, int division, int piece_sync, int heap)
{
  WIPE_SYS_Start(pattern, wipe_m, wipe_s, color, division, piece_sync, heap);
}

//MCSS終了の為のコールバック
static void _McssAnmStop( u32 data, fx32 currentFrame )
{
  POKEMON_TRADE_WORK* pWork = (POKEMON_TRADE_WORK*)data;

  MCSS_SetAnmStopFlag( pWork->pokeMcss[0]);
  pWork->mcssStop[0] = TRUE;
}


static void _pEmitCBInFunc( GFL_EMIT_PTR pEmiter, unsigned int flag)
{
  if( flag == SPL_EMITTER_CALLBACK_FRONT  ){
    return;
  }
  if(pEmiter ){
    POKEMON_TRADE_WORK* pWork = (POKEMON_TRADE_WORK*)GFL_PTC_GetUserData( pEmiter );
    if( pWork->pPokemonTradeDemo->icaBallin){
      VecFx32 pos={0,0,0};
      ICA_ANIME_GetTranslate( pWork->pPokemonTradeDemo->icaBallin, &pos );
      GFL_PTC_SetEmitterPosition(pEmiter, &pos);
    }
  }
}

static void _ballinEmitFunc(GFL_EMIT_PTR pEmiter)
{
  POKEMON_TRADE_WORK* pWork = (POKEMON_TRADE_WORK*)GFL_PTC_GetTempPtr();

  GFL_PTC_SetUserData( pEmiter, pWork );
  GFL_PTC_SetCallbackFunc(pEmiter, &_pEmitCBInFunc);

}

static void _pEmitCBOutFunc( GFL_EMIT_PTR pEmiter, unsigned int flag)
{
  if( flag == SPL_EMITTER_CALLBACK_FRONT  ){
    return;
  }
  if(pEmiter  ){
    POKEMON_TRADE_WORK* pWork = (POKEMON_TRADE_WORK*)GFL_PTC_GetUserData( pEmiter );
    if(pWork->pPokemonTradeDemo->icaBallout){
      VecFx32 pos={0,0,0};
      ICA_ANIME_GetTranslate( pWork->pPokemonTradeDemo->icaBallout, &pos );
      GFL_PTC_SetEmitterPosition(pEmiter, &pos);
    }
  }
}


static void _balloutEmitFunc(GFL_EMIT_PTR pEmiter)
{
  POKEMON_TRADE_WORK* pWork = (POKEMON_TRADE_WORK*)GFL_PTC_GetTempPtr();

  GFL_PTC_SetUserData( pEmiter, pWork );
  GFL_PTC_SetCallbackFunc(pEmiter, &_pEmitCBOutFunc);
}

//------------------------------------------------------------------
/**
 * @brief   ポケモンが移動する前にアニメ終了待ち
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------
void POKMEONTRADE_IRCDEMO_ChangeDemo(POKEMON_TRADE_WORK* pWork)
{
  VecFx32 apos;
  int i;

  //待機アニメが止まるのを待つ
  pWork->mcssStop[0] = FALSE;
  MCSS_SetAnimCtrlCallBack(pWork->pokeMcss[0], (u32)pWork, _McssAnmStop, NNS_G2D_ANMCALLBACKTYPE_LAST_FRM);

  _CHANGE_STATE(pWork,_changeDemo_ModelT1);
}

//------------------------------------------------------------------
/**
 * @brief   ポケモン登場CALCTをだす
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------
static void _changeDemo_ModelT1(POKEMON_TRADE_WORK* pWork)
{
  if(pWork->mcssStop[0]!=TRUE){  //アニメ終了待ち
    return;
  }

  if(GFL_NET_IsInit()){
    GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ANIMEEND);
    _CHANGE_STATE(pWork,_changeDemo_ModelT2);
  }
  else{
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade0);
  }


}

static void _changeDemo_ModelT2(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ANIMEEND)){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade0);
  }

}

static void _changeDemo_ModelTrade0(POKEMON_TRADE_WORK* pWork)
{
  VecFx32 apos;
  int i;

  _setNextAnim(pWork, 0);

  pWork->pD2Fade = GFL_HEAP_AllocClearMemory(pWork->heapID, sizeof(_D2_PAL_FADE_WORK));
  pWork->pD2Fade->pal_fade_time = ANMCNTC(_POKEMON_CENTER_TIME/3);
  pWork->pD2Fade->pal_fade_nowcount = 0;
  pWork->pD2Fade->pal_start = 0;
  pWork->pD2Fade->pal_end = -16;

  if(pWork->modelno!=-1){
    pWork->pModelFade = _createPaletteFade(GFL_G3D_UTIL_GetResHandle(pWork->g3dUtil,0), pWork->heapID);
    _FIELD_StartPaletteFade( pWork->pModelFade, 0, 16, ANMCNTC(_POKEMON_CENTER_TIME/3)/16, 0 );
  }

  {
    VecFx32 pos={_POKEMON_PLAYER_CENTER_POSX,_POKEMON_PLAYER_CENTER_POSY, _POKEMON_PLAYER_CENTER_POSZ};
    pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[0], ANMCNTC(_POKEMON_CENTER_TIME), &pos, pWork->heapID);
  }
  {
    VecFx32 pos={_POKEMON_FRIEND_CENTER_POSX,_POKEMON_FRIEND_CENTER_POSY, _POKEMON_FRIEND_CENTER_POSZ};
    pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[1], ANMCNTC(_POKEMON_CENTER_TIME*2), &pos, pWork->heapID);
  }

  MCSS_SetAnimCtrlCallBack(pWork->pokeMcss[0], (u32)pWork, _McssAnmStop, NNS_G2D_ANMCALLBACKTYPE_LAST_FRM);

  MCSS_SetPaletteFade( pWork->pokeMcss[1], 0, 16, ANMCNTC(_POKEMON_CENTER_TIME/3)/16, 0 );
  POKETRADE_MESSAGE_WindowClose(pWork);

  GFL_BG_SetBackGroundColor(GFL_BG_FRAME1_M ,0);
  GFL_BG_SetBackGroundColor(GFL_BG_FRAME1_S ,0);

  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade1);

}

static void _changeDemo_ModelTrade1(POKEMON_TRADE_WORK* pWork)
{
  int i;

  {  // フェード中
    _setFadeMask(pWork->pD2Fade);
    if(pWork->pModelFade){
      _EFFTOOL_CalcPaletteFade(pWork->pModelFade);
    }
    _pokeMoveFunc(pWork->pMoveMcss[0]);
    _pokeMoveFunc(pWork->pMoveMcss[1]);
  }


  if(pWork->anmCount > ANMCNTC(_POKEMON_CENTER_TIME)){  //フェード完了
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( 0 );
#if 1
    POKMEONTRADE_RemoveCoreResource( pWork);
    _freePaletteFade(pWork->pModelFade);
    pWork->pModelFade = NULL;
    
    pWork->cellUnit = GFL_CLACT_UNIT_Create( 3 , 0 , pWork->heapID );
#else
    
    IRC_POKETRADEDEMO_RemoveModel( pWork);

    IRC_POKETRADE_ResetBoxNameWindow(pWork);
    IRCPOKETRADE_PokeDeleteMcss(pWork,1);
    
    IRC_POKETRADE_AllDeletePokeIconResource(pWork);
    IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,0);
    IRCPOKEMONTRADE_ResetPokemonStatusMessage(pWork,1);
    TOUCHBAR_Exit(pWork->pTouchWork);
    pWork->pTouchWork=NULL;
    IRC_POKETRADE_EndIconResource(pWork);
    GFL_CLACT_UNIT_Delete(pWork->cellUnit);
    
    pWork->cellUnit = GFL_CLACT_UNIT_Create( 3 , 0 , pWork->heapID );

    
    POKE_GTS_EndWork(pWork);
    IRC_POKETRADE_MainGraphicExit(pWork);
    IRC_POKETRADE_SubGraphicExit(pWork);

    IRC_POKETRADE_ResetSubDispGraphic(pWork);


    GFL_HEAP_FreeMemory(pWork->pD2Fade);
    pWork->pD2Fade=NULL;
    _freePaletteFade(pWork->pModelFade);
    pWork->pModelFade = NULL;
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;
#endif
    _setNextAnim(pWork, 0);

    _CHANGE_STATE(pWork,_changeDemo_ModelTrade2);
  }
}

static void _changeDemo_ModelTrade2(POKEMON_TRADE_WORK* pWork)
{
  int i;
  
  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  GFL_CLACT_SYS_Delete();
  IRC_POKETRADE_DEMOCLACT_Create(pWork);
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 3 , 0 , pWork->heapID );

  pWork->pPokemonTradeDemo = GFL_HEAP_AllocClearMemory(pWork->heapID,sizeof(POKEMONTRADE_DEMO_WORK));
  pWork->pPokemonTradeDemo->heapID = pWork->heapID;

  IRC_POKETRADE_SetSubdispGraphicDemo(pWork,1);
  G2S_BlendNone();

  G2_BlendNone();
  IRC_POKETRADEDEMO_SetModel( pWork, TRADEIR_OBJECT);

  IRCPOKETRADE_PokeCreateMcss(pWork, 1, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,1) );  //相手裏
  IRCPOKETRADE_PokeCreateMcss(pWork, 2, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,0) );  //みかた裏
  IRCPOKETRADE_PokeCreateMcss(pWork, 3, 1, IRC_POKEMONTRADE_GetRecvPP(pWork,1) );  //相手表
  MCSS_SetVanishFlag( pWork->pokeMcss[1] );
  MCSS_SetVanishFlag( pWork->pokeMcss[2] );
  MCSS_SetVanishFlag( pWork->pokeMcss[3] );
  POKEMONTRADE_DEMO_IRPTC_Init(pWork->pPokemonTradeDemo);
  POKEMONTRADE_DEMO_IRICA_Init(pWork->pPokemonTradeDemo);


  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade3);


  G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG0,GX_BLEND_PLANEMASK_BD,0,0);

}




static void _changeDemo_ModelTrade3(POKEMON_TRADE_WORK* pWork)
{


  if(pWork->anmCount == ANMCNTC(_IR_POKEMON_DELETE_TIME-3)){
    //自分を白くする
    MCSS_SetPaletteFade( pWork->pokeMcss[0], 8, 8, 0, 0xffff );
  }
  if(pWork->anmCount == ANMCNTC(_IR_POKEMON_DELETE_TIME-2)){
    //自分を白くする
    MCSS_SetPaletteFade( pWork->pokeMcss[0], 16, 16, 0, 0xffff );
  }
  if(pWork->anmCount == ANMCNTC(_IR_POKEMON_DELETE_TIME-1)){
    _pokemonCreateCLACTInit(pWork);
    _pokemonCreateCLACTAdd(pWork);
  }

  if(pWork->anmCount == ANMCNTC(_IR_POKEMON_DELETE_TIME)){
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  }

  if(pWork->anmCount == ANMCNTC(_IR_POKEMON_DELETE_TIME)){
    MCSS_SetVanishFlag( pWork->pokeMcss[0] );
  }


  if(pWork->anmCount == ANMCNTC(_IRTEX001_01_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[IRPTC_KIND_DEMO1], IR_TEX001, NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_IRTEX001_02_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[IRPTC_KIND_DEMO1_2], IR_TEX001, NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_IRTEX002_01_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[IRPTC_KIND_DEMO2], IR_TEX002, NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_IRTEX003_01_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[IRPTC_KIND_DEMO3], IR_TEX003, NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_IRTEX003_02_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[IRPTC_KIND_DEMO3_2], IR_TEX003,  NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_IRTEX004_01_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[IRPTC_KIND_DEMO4], IR_TEX004,  NULL, pWork);
  }

  if(pWork->anmCount == ANMCNTC(_IR_OAM_POKECREATE)){
    _pokemonCreateCLACTAdd(pWork);
  }
  if(pWork->anmCount == ANMCNTC(_IR_POKEUP_WHITEOUT_START)){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                       ANMCNTC(_IR_POKEUP_WHITEOUT_TIMER), 1, pWork->heapID );

  }
  if(pWork->anmCount == ANMCNTC(_IR_POKEUP_WHITEIN_START)){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                       ANMCNTC(_IR_POKEUP_WHITEIN_TIMER), 1, pWork->heapID );
  }

  if(pWork->anmCount == ANMCNTC(_IR_POKE_APPEAR_START)){
    MCSS_ResetVanishFlag( pWork->pokeMcss[2] );
    //IRCPOKETRADE_PokeCreateMcss(pWork, 0, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,0) );
    {  //初期位置設定
      VecFx32 apos;
      apos.x = _IR_POKEMON_PLAYER_APPEAR_POSX;
      apos.y = _IR_POKEMON_PLAYER_APPEAR_POSY;
      apos.z = _IR_POKEMON_PLAYER_APPEAR_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[2] ,&apos );
      apos.x = _IR_POKEMON_PLAYER_UP_POSX;
      apos.y = _IR_POKEMON_PLAYER_UP_POSY;
      apos.z = _IR_POKEMON_PLAYER_UP_POSZ;
      pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[2], ANMCNTC(_IR_POKE_APPEAR_TIME), &apos, pWork->heapID);
      //MCSS_SetAnmStopFlag(pWork->pokeMcss[2]);

      MCSS_GetScale( pWork->pokeMcss[2], &apos );
      apos.x *= _IRC_FRONT_POKMEON_SCALE_SIZE;
      apos.y *= _IRC_FRONT_POKMEON_SCALE_SIZE;
      MCSS_SetScale( pWork->pokeMcss[2], &apos );

    }
  }

  if(pWork->anmCount == ANMCNTC(_IR_POKE_APPEAR_START+1)){
    MCSS_ResetVanishFlag( pWork->pokeMcss[3] );
    
 //   IRCPOKETRADE_PokeCreateMcss(pWork, 1, 1, IRC_POKEMONTRADE_GetRecvPP(pWork,1) );
    {
      VecFx32 apos;
      apos.x = _IR_POKEMON_FRIEND_APPEAR_POSX;
      apos.y = _IR_POKEMON_FRIEND_APPEAR_POSY;
      apos.z = _IR_POKEMON_FRIEND_APPEAR_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[3] ,&apos );

      apos.x = _IR_POKEMON_FRIEND_DOWN_POSX;
      apos.y = _IR_POKEMON_FRIEND_DOWN_POSY;
      apos.z = _IR_POKEMON_FRIEND_DOWN_POSZ;
      pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[3], ANMCNTC(_IR_POKE_APPEAR_TIME), &apos , pWork->heapID);
      //MCSS_SetAnmStopFlag(pWork->pokeMcss[3]);
      MCSS_GetScale( pWork->pokeMcss[3], &apos );
      pWork->pPokemonTradeDemo->PushPos.x = apos.x;
      pWork->pPokemonTradeDemo->PushPos.y = apos.y;
      pWork->pPokemonTradeDemo->PushPos.z = apos.z;
      apos.x *= _IRC_BACK_POKMEON_SCALE_SIZE;
      apos.y *= _IRC_BACK_POKMEON_SCALE_SIZE;
      MCSS_SetScale( pWork->pokeMcss[3], &apos );
      
    }
    //移動設定
  }
  if(pWork->anmCount == ANMCNTC(_IR_POKE_APPEAR_START+2)){
      MCSS_SetAlpha(pWork->pokeMcss[2], _IRC_FRONT_POKMEON_ALPHA);
      MCSS_SetAlpha(pWork->pokeMcss[3], _IRC_BACK_POKMEON_ALPHA);
  }


  
#if 1
  if(ANMCNTC(_IR_POKEMON_CREATE_TIME) == pWork->anmCount){
      VecFx32 apos;
    MCSS_SetAlpha(pWork->pokeMcss[3], 31);
    MCSS_ResetVanishFlag(pWork->pokeMcss[3]);
    apos.x = _POKEMON_PLAYER_CENTER_POSX;
    apos.y = _POKEMON_PLAYER_CENTER_POSY;
    apos.z = _POKEMON_PLAYER_CENTER_POSZ;
    MCSS_SetPosition( pWork->pokeMcss[3] ,&apos );
    MCSS_SetAnmStopFlag(pWork->pokeMcss[3]);

    MCSS_SetScale( pWork->pokeMcss[3], &pWork->pPokemonTradeDemo->PushPos );

    
    MCSS_SetPaletteFade( pWork->pokeMcss[3], 16, 16, 0, 0x7fff );
  }
  if(ANMCNTC(_IR_POKEMON_CREATE_TIME+1) == pWork->anmCount){
    MCSS_SetPaletteFade( pWork->pokeMcss[3], 8, 8, 0, 0x7fff );
  }
  if(ANMCNTC(_IR_POKEMON_CREATE_TIME+2) == pWork->anmCount){
    MCSS_SetPaletteFade( pWork->pokeMcss[3], 0, 0, 0, 0x7fff  );
  }
#endif


  if(pWork->anmCount == ANMCNTC(_IR_POKECHANGE_WHITEOUT_START)){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                       ANMCNTC(_IR_POKECHANGE_WHITEOUT_TIMER), 1, pWork->heapID );
  }
  if(pWork->anmCount == ANMCNTC(_IR_POKECHANGE_WHITEIN_START)){
    _WIPE_SYS_StartRap(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                       ANMCNTC(_IR_POKECHANGE_WHITEIN_TIMER), 1, pWork->heapID );
  }
  OS_TPrintf("C %d\n",pWork->anmCount);

  if(pWork->anmCount == ANMCNTC(_IR_DEMO_END-1)){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;
    IRC_POKETRADEDEMO_RemoveModel( pWork);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade20);
  }
  _pokeMoveFunc(pWork->pMoveMcss[0]);
  _pokeMoveFunc(pWork->pMoveMcss[1]);
}


static void _changeDemo_ModelTrade20(POKEMON_TRADE_WORK* pWork)
{
  _pokemonCreateCLACTExit(pWork);
  POKEMONTRADE_DEMO_PTC_End(pWork->pPokemonTradeDemo, IRPTC_KIND_NUM_MAX);
  POKEMONTRADE_DEMO_IRICA_Delete(pWork->pPokemonTradeDemo);
  GFL_HEAP_FreeMemory(pWork->pPokemonTradeDemo);
  pWork->pPokemonTradeDemo = NULL;

  if(pWork->type == POKEMONTRADE_EVENT){
    pWork->pParentWork->ret = POKEMONTRADE_END;
    _CHANGE_STATE(pWork,NULL);
  }
  else{
    _CHANGE_STATE(pWork,POKMEONTRADE_SAVE_Init);
  }
}


//--------------------------------------------------
/**
 * @brief    フェード動作関数
 * @param   	pD2Fade				フェードワーク
 */
//--------------------------------------------------

static void _setFadeMask(_D2_PAL_FADE_WORK* pD2Fade)
{
  if(!pD2Fade){
    return;
  }
  if(pD2Fade->pal_fade_time < pD2Fade->pal_fade_nowcount){
    return;
  }
  pD2Fade->pal_fade_nowcount++;
  {
    int upper = pD2Fade->pal_end - pD2Fade->pal_start;
    upper *= 4096;
    upper = upper / pD2Fade->pal_fade_time;
    upper = upper * pD2Fade->pal_fade_nowcount;
    upper /= 4096;

    G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|
                           GX_BLEND_PLANEMASK_OBJ, upper);
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|
                            GX_BLEND_PLANEMASK_OBJ, upper);
  }

}

//--------------------------------------------------
/**
 * @brief パレットフェードスタート
 * @param[in]	bfw	      _EFFTOOL_PAL_FADE_WORK管理ワークへのポインタ
 * @param[in]	start_evy	セットするパラメータ（フェードさせる色に対する開始割合16段階）
 * @param[in]	end_evy		セットするパラメータ（フェードさせる色に対する終了割合16段階）
 * @param[in]	wait			セットするパラメータ（ウェイト）
 * @param[in]	rgb				セットするパラメータ（フェードさせる色）
 */
//--------------------------------------------------
static void	_FIELD_StartPaletteFade( _EFFTOOL_PAL_FADE_WORK* epfw, u8 start_evy, u8 end_evy, u8 wait, u16 rgb )
{

  epfw->pal_fade_flag      = 1;
  epfw->pal_fade_start_evy = start_evy;
  epfw->pal_fade_end_evy   = end_evy;
  epfw->pal_fade_wait      = 0;
  epfw->pal_fade_wait_tmp  = wait;
  epfw->pal_fade_rgb       = rgb;
}


//----------------------------------------
/**
 * @brief パレットフェード構造体作成
 * @param[in]	g3DRES	  GFL_G3D_RES
 * @param[in]	heapID	  HEAPID
 * @retturn   パレットフェード構造体
 */
//----------------------------------------

static _EFFTOOL_PAL_FADE_WORK* _createPaletteFade(GFL_G3D_RES* g3DRES,HEAPID heapID)
{
  //パレットフェード用ワーク生成
  NNSG3dResFileHeader*	header = GFL_G3D_GetResourceFileHeader( g3DRES );
  NNSG3dResTex*		    	pTex = NNS_G3dGetTex( header );
  u32                   size = (u32)pTex->plttInfo.sizePltt << 3;
  _EFFTOOL_PAL_FADE_WORK* pwk = GFL_HEAP_AllocClearMemory(heapID,sizeof(_EFFTOOL_PAL_FADE_WORK));

  pwk->g3DRES = g3DRES;
  pwk->pData_dst  = GFL_HEAP_AllocClearMemory( heapID, size );
  return pwk;
}

//----------------------------------------
/**
 * @brief パレットフェード構造体開放
 * @param[in]	pwk	_EFFTOOL_PAL_FADE_WORKポインタ
 */
//----------------------------------------
static void	_freePaletteFade( _EFFTOOL_PAL_FADE_WORK* pwk )
{
  if(pwk){
    GFL_HEAP_FreeMemory( pwk->pData_dst );
    GFL_HEAP_FreeMemory( pwk );
  }
}



//-------------------------------------------------
/**
 *	@brief      3Dモデルのパレットフェードアニメ実行処理   BTLVから転用
 *	@param[in]	epfw  パレットフェード管理構造体へのポインタ
 */
//-------------------------------------------------
static void  _EFFTOOL_CalcPaletteFade( _EFFTOOL_PAL_FADE_WORK *epfw )
{

  if(	(epfw == NULL) || (epfw->pal_fade_flag == 0) )
  {
    return;
  }
  if( epfw->pal_fade_wait == 0 )
  {
    NNSG3dResFileHeader*	header;
    NNSG3dResTex*		    	pTex;
    u32                   size;
    const void*           pData_src;
    u32                   from;

    epfw->pal_fade_wait = epfw->pal_fade_wait_tmp;

    {
      //テクスチャリソースポインタの取得
      header = GFL_G3D_GetResourceFileHeader( epfw->g3DRES );
      pTex = NNS_G3dGetTex( header );

      size = ( u32 )pTex->plttInfo.sizePltt << 3;
      pData_src = NNS_G3dGetPlttData( pTex );
      from = NNS_GfdGetTexKeyAddr( pTex->plttInfo.vramKey );

      SoftFade( pData_src, epfw->pData_dst, ( size / 2 ), epfw->pal_fade_start_evy, epfw->pal_fade_rgb );

      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT, from, epfw->pData_dst, size );
    }
    if( epfw->pal_fade_start_evy == epfw->pal_fade_end_evy )
    {
      epfw->pal_fade_flag = 0;
    }
    else if( epfw->pal_fade_start_evy > epfw->pal_fade_end_evy )
    {
      epfw->pal_fade_start_evy--;
    }
    else
    {
      epfw->pal_fade_start_evy++;
    }
  }
  else
  {
 	  epfw->pal_fade_wait--;
  }
}

//-------------------------------------------------
/**
 *	@brief      MCSS移動命令
 *	@param[in,out]	_POKEMCSS_MOVE_WORK   移動ポインタ
 *	@param[in]	time   移動時間
 *	@param[in]	pPos    最終移動先
 */
//-------------------------------------------------

static void _pokeMoveRenew(_POKEMCSS_MOVE_WORK* pPoke,int time, const VecFx32* pPos)
{
  {
    VecFx32 apos;
    MCSS_GetPosition(pPoke->pMcss, &apos);
    pPoke->time = time;
    pPoke->nowcount=0;
    GFL_STD_MemCopy(pPos, &pPoke->end, sizeof(VecFx32));
    GFL_STD_MemCopy(&apos, &pPoke->start, sizeof(VecFx32));
  }
}

//-------------------------------------------------
/**
 *	@brief      MCSS移動命令作成
 *	@param[in]	pokeMcss   対象MCSS
 *	@param[in]	time   移動時間
 *	@param[in]	pPos    最終移動先
 *	@param[in]	HEAPID  HEAPID
 */
//-------------------------------------------------

static _POKEMCSS_MOVE_WORK* _pokeMoveCreate(MCSS_WORK* pokeMcss, int time, const VecFx32* pPos, HEAPID heapID)
{
  _POKEMCSS_MOVE_WORK* pPoke = GFL_HEAP_AllocClearMemory(heapID, sizeof(_POKEMCSS_MOVE_WORK));

  pPoke->pMcss = pokeMcss;
  _pokeMoveRenew(pPoke,time,pPos);
  pPoke->percent=0.0f;
  return pPoke;

}

//-------------------------------------------------
/**
 *	@brief      MCSS移動命令実行
 *	@param[in]	pokeMcss   対象MCSS
 *	@param[in]	time   移動時間
 *	@param[in]	xend    Xの移動先
 *	@param[in]	zend    Zの移動先
 *	@param[in]	HEAPID  HEAPID
 */
//-------------------------------------------------

static fx32 _movemath(fx32 st,fx32 en,_POKEMCSS_MOVE_WORK* pMove,BOOL bWave)
{
  fx32 re;
  re = en - st;
  re = re / pMove->time;
  

#if 0
  if(pMove->percent != 0.0f){
    float ans = FX_FX32_TO_F32(re * pMove->nowcount);
    ans = FX_FX32_TO_F32( FX_Sqrt( FX_SinIdx(pMove->sins) )) * ans;
    re = FX_F32_TO_FX32(ans);
    re = st + re;
  }
#endif
  re = st + re * pMove->nowcount;
  if(pMove->wave && bWave){
    re = en + FX_SinIdx(pMove->sins) * _WAVE_NUM;
  }
  return re;
}


static void _pokeMoveFunc(_POKEMCSS_MOVE_WORK* pMove)
{
  if(!pMove){
    return;
  }
  if(pMove->time < pMove->nowcount){
    return;
  }
  pMove->nowcount++;
  if(pMove->time != pMove->nowcount)
  {
    VecFx32 apos,aposold;
    MCSS_GetPosition( pMove->pMcss ,&aposold );


    apos.x = _movemath(pMove->start.x, pMove->end.x ,pMove, FALSE);

    if(pMove->wave){
      float an = FX_FX32_TO_F32(aposold.x - apos.x);
      an = an * 374;
      pMove->sins -= (s32)an;
    }


    apos.y = _movemath(pMove->start.y, pMove->end.y ,pMove, TRUE);
    apos.z = _movemath(pMove->start.z, pMove->end.z ,pMove, FALSE);
//    OS_TPrintf(" posy %d\n",apos.y/FX32_ONE);
    


    
/*
    if(pMove->percent != 0.0f){
      if(pMove->percent < 1.0f){
        pMove->percent+=pMove->add;
      }
      else{
        pMove->percent-=pMove->add;
      }
      if(pMove->percent > (1.0f-pMove->add)){
        if(pMove->percent < (1.0f+pMove->add)){
          pMove->percent=0.0f;
        }
      }
    }
   */

    
    MCSS_SetPosition( pMove->pMcss ,&apos );
  }
  else{
    MCSS_SetPosition( pMove->pMcss ,&pMove->end );
  }
}

