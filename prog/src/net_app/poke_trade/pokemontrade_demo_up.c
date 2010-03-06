//=============================================================================
/**
 * @file	  pokemontrade_demo_up.c
 * @bfief	  ポケモン交換アニメーション  上に行く部分
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/09/27
 */
//=============================================================================

#include <gflib.h>


#include "arc_def.h"
#include "net/network_define.h"
#include "system\wipe.h"

#include "net_app/pokemontrade.h"
#include "system/main.h"

#include "poke_icon.naix"
#include "pokeicon/pokeicon.h"

#include "ircbattle.naix"
#include "trade.naix"

#include "system/touch_subwindow.h"

#include "poke_tool/poke_tool_def.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"
#include "poke_tool/status_rcv.h"
#include "tradedemo.naix"

#include "pokemontrade_local.h"
//#include "app/mailtool.h"

#include "spahead.h"

#include "sound/pm_voice.h"

#include "pokemontrade_demo.cdat"

static void _changeDemo_ModelTrade2_1(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelT1(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelT2(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade0(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade1(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade2(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade3(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade20(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade2_jump(POKEMON_TRADE_WORK* pWork);


#include "pokemontrade_mcss.c"
#include "pokemontrade_fade.c"
#include "pokemontrade_demo_common.c"

//------------------------------------------------------------------
/**
 * @brief   パーティクルコールバック
 * @param   GFL_EMIT_PTR
 */
//------------------------------------------------------------------

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
void POKMEONTRADE_DEMO_GTSUP_ChangeDemo(POKEMON_TRADE_WORK* pWork)
{
  VecFx32 apos;
  int i;

  _demoBGMChange();
  
  //強制停止
  pWork->mcssStop[0] = TRUE;
  MCSS_SetAnimeIndex(pWork->pokeMcss[0], 0);

  if(pWork->type==POKEMONTRADE_TYPE_GTSUP){  //デモのときは真ん中から
    VecFx32 pos={_POKEMON_PLAYER_CENTER_POSX,_POKEMON_PLAYER_CENTER_POSY, _POKEMON_PLAYER_CENTER_POSZ};
    MCSS_SetPosition( pWork->pokeMcss[0] ,&pos );
  }
  
  _CHANGE_STATE(pWork,_changeDemo_ModelT1);
}

//3dスタート
static void _changeDemo_ModelTrade2(POKEMON_TRADE_WORK* pWork)
{
  int i;

  POKMEONTRADE_RemoveCoreResource( pWork);//@new
  GFL_CLACT_SYS_Delete();
  IRC_POKETRADE_DEMOCLACT_Create(pWork);
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 3 , 0 , pWork->heapID );

  pWork->pPokemonTradeDemo = GFL_HEAP_AllocClearMemory(pWork->heapID,sizeof(POKEMONTRADE_DEMO_WORK));
  pWork->pPokemonTradeDemo->heapID = pWork->heapID;

  IRC_POKETRADE_SetSubdispGraphicDemo(pWork,0);  //通常背景
  G2S_BlendNone();

  G2_BlendNone();
  IRC_POKETRADEDEMO_SetModel( pWork, TRADEUP_OBJECT);  //上だけモデル
  
  IRCPOKETRADE_PokeCreateMcss(pWork, 1, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,1),TRUE );  //相手裏
  IRCPOKETRADE_PokeCreateMcss(pWork, 2, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,0),TRUE );  //みかた裏
  IRCPOKETRADE_PokeCreateMcss(pWork, 3, 1, IRC_POKEMONTRADE_GetRecvPP(pWork,1),TRUE );  //相手表
  MCSS_SetVanishFlag( pWork->pokeMcss[1] );
  MCSS_SetVanishFlag( pWork->pokeMcss[2] );
  MCSS_SetVanishFlag( pWork->pokeMcss[3] );
  POKEMONTRADE_DEMO_PTC_Init(pWork->pPokemonTradeDemo);
  POKEMONTRADE_DEMO_ICA_Init(pWork->pPokemonTradeDemo,_DEMO_TYPE_UP); //ICA設定


  _setNextAnim(pWork, 0);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade3);

  G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG0,GX_BLEND_PLANEMASK_BD,0,0);

}




static void _changeDemo_ModelTrade3(POKEMON_TRADE_WORK* pWork)
{


  if(pWork->anmCount == ANMCNTC(_POKEMON_DELETE_TIME-3)){
    //自分を白くする
    MCSS_SetPaletteFade( pWork->pokeMcss[0], 8, 8, 0, 0xffff );
  }
  if(pWork->anmCount == ANMCNTC(_POKEMON_DELETE_TIME-2)){
    //自分を白くする
    MCSS_SetPaletteFade( pWork->pokeMcss[0], 16, 16, 0, 0xffff );
  }

  if(pWork->anmCount == ANMCNTC(_POKEMON_DELETE_TIME-1)){
    _pokemonCreateCLACTInit(pWork);
    _pokemonCreateCLACTAdd(pWork);
  }
  
  if(pWork->anmCount == ANMCNTC(_POKEMON_DELETE_TIME)){
    GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  }

  if(pWork->anmCount == ANMCNTC(_POKEMON_DELETE_TIME)){
    MCSS_SetVanishFlag( pWork->pokeMcss[0] );
//      IRCPOKETRADE_PokeDeleteMcss(pWork,0);
  }


  if(pWork->anmCount == ANMCNTC(_BALL_PARTICLE_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO1], DERMO_TEX001, NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO2_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO2], DEMO_TEX002, NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO3_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO3], DEMO_TEX003, NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO3_START2)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_ORG], DEMO_TEX003, NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO4_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO4], DEMO_TEX004,  NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO4_START2)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO8], DEMO_TEX004,  NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO5_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO5], DEMO_TEX005, _ballinEmitFunc, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO6_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO6], DEMO_TEX006, NULL, pWork);
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO7_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO7], DEMO_TEX007,_balloutEmitFunc, pWork);
  }

  if(pWork->anmCount == ANMCNTC(_OAM_POKECREATE_START2)){
    _pokemonCreateCLACTAdd(pWork);
  }
  if(pWork->anmCount == ANMCNTC(_POKEUP_WHITEOUT_START)){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                       ANMCNTC(_POKEUP_WHITEOUT_TIMER), 1, pWork->heapID );

  }

//---演出終わり

  if(pWork->anmCount == ANMCNTC(_POKEUP_WHITEIN_START)){
    if(pWork->pMoveMcss[0]){
      GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
      pWork->pMoveMcss[0]=NULL;
    }
    if(pWork->pMoveMcss[1]){
      GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
      pWork->pMoveMcss[1]=NULL;
    }
    IRC_POKETRADEDEMO_RemoveModel( pWork);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade20);
  }

//--------------------------------------------------------------------
  

  
  if(pWork->anmCount == ANMCNTC(_POKE_APPEAR_START)){
    MCSS_ResetVanishFlag( pWork->pokeMcss[2] );
    //IRCPOKETRADE_PokeCreateMcss(pWork, 0, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,0) );
    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_APPEAR_POSX;
      apos.y = _POKEMON_PLAYER_APPEAR_POSY;
      apos.z = _POKEMON_PLAYER_APPEAR_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[2] ,&apos );
      apos.x = _POKEMON_PLAYER_UP_POSX;
      apos.y = _POKEMON_PLAYER_UP_POSY;
      apos.z = _POKEMON_PLAYER_UP_POSZ;
      pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[2], ANMCNTC(_POKE_APPEAR_TIME), &apos, pWork->heapID);
      MCSS_SetAnmStopFlag(pWork->pokeMcss[2]);

      MCSS_GetScale( pWork->pokeMcss[2], &apos );
      apos.x *= _FRONT_POKMEON_SCALE_SIZE;
      apos.y *= _FRONT_POKMEON_SCALE_SIZE;
      MCSS_SetScale( pWork->pokeMcss[2], &apos );
    }
  }

  if(pWork->anmCount == ANMCNTC(_POKE_APPEAR_START+1)){
    MCSS_ResetVanishFlag( pWork->pokeMcss[3] );
    
 //   IRCPOKETRADE_PokeCreateMcss(pWork, 1, 1, IRC_POKEMONTRADE_GetRecvPP(pWork,1) );
    {
      VecFx32 apos;
      apos.x = _POKEMON_FRIEND_APPEAR_POSX;
      apos.y = _POKEMON_FRIEND_APPEAR_POSY;
      apos.z = _POKEMON_FRIEND_APPEAR_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[3] ,&apos );

      apos.x = _POKEMON_FRIEND_DOWN_POSX;
      apos.y = _POKEMON_FRIEND_DOWN_POSY;
      apos.z = _POKEMON_FRIEND_DOWN_POSZ;
      pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[3], ANMCNTC(_POKE_APPEAR_TIME), &apos , pWork->heapID);
      MCSS_SetAnmStopFlag(pWork->pokeMcss[3]);
      MCSS_GetScale( pWork->pokeMcss[3], &apos );
      apos.x *= _BACK_POKMEON_SCALE_SIZE;
      apos.y *= _BACK_POKMEON_SCALE_SIZE;
      MCSS_SetScale( pWork->pokeMcss[3], &apos );
    }
    //移動設定
  }

  if(pWork->anmCount == ANMCNTC(_POKE_APPEAR_START2)){
    VecFx32 apos;
    apos.x = _POKEMON_PLAYER_UP_POSX;
    apos.y = _POKEMON_PLAYER_UP_POSY;
    apos.z = _POKEMON_PLAYER_UP_POSZ;
    _pokeMoveRenew(pWork->pMoveMcss[0],ANMCNTC(_POKE_APPEAR_TIME2),&apos);
    apos.x = _POKEMON_FRIEND_DOWN_POSX;
    apos.y = _POKEMON_FRIEND_DOWN_POSY;
    apos.z = _POKEMON_FRIEND_DOWN_POSZ;
    _pokeMoveRenew(pWork->pMoveMcss[1],ANMCNTC(_POKE_APPEAR_TIME2),&apos);
  }
  
  if(ANMCNTC(_POKE_SIDEOUT_START) == pWork->anmCount){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;

    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_SIDEOUT_POSX;
      apos.y = _POKEMON_PLAYER_SIDEOUT_POSY;
      apos.z = _POKEMON_PLAYER_SIDEOUT_POSZ;
      pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[2], ANMCNTC(_POKE_SIDEOUT_TIME), &apos, pWork->heapID);
      apos.x = _POKEMON_FRIEND_SIDEOUT_POSX;
      apos.y = _POKEMON_FRIEND_SIDEOUT_POSY;
      apos.z = _POKEMON_FRIEND_SIDEOUT_POSZ;
      pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[3], ANMCNTC(_POKE_SIDEOUT_TIME), &apos , pWork->heapID);
      pWork->pMoveMcss[0]->wave=1;
      pWork->pMoveMcss[1]->wave=1;
    }
  }

  if(ANMCNTC(_POKE_SIDEIN_START) == pWork->anmCount){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
    MCSS_ResetVanishFlag( pWork->pokeMcss[0] );
    MCSS_SetPaletteFade( pWork->pokeMcss[0], 0, 0, 0, 0x7fff  );
    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_SIDEST_POSX;
      apos.y = _POKEMON_PLAYER_SIDEST_POSY;
      apos.z = _POKEMON_PLAYER_SIDEST_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[0] ,&apos );
      apos.x = _POKEMON_PLAYER_SIDEIN_POSX;
      apos.y = _POKEMON_PLAYER_SIDEIN_POSY;
      apos.z = _POKEMON_PLAYER_SIDEIN_POSZ;
      pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[0], ANMCNTC(_POKE_SIDEIN_TIME), &apos, pWork->heapID);
      MCSS_GetScale( pWork->pokeMcss[0], &apos );
      apos.x *= _BACK_POKMEON_SCALE_SIZE;
      apos.y *= _BACK_POKMEON_SCALE_SIZE;
      MCSS_SetScale( pWork->pokeMcss[0], &apos );
    }

    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;
    MCSS_ResetVanishFlag( pWork->pokeMcss[1] );
    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_FRIEND_SIDEST_POSX;
      apos.y = _POKEMON_FRIEND_SIDEST_POSY;
      apos.z = _POKEMON_FRIEND_SIDEST_POSZ;
      MCSS_SetPosition( pWork->pokeMcss[1] ,&apos );
      apos.x = _POKEMON_FRIEND_SIDEIN_POSX;
      apos.y = _POKEMON_FRIEND_SIDEIN_POSY;
      apos.z = _POKEMON_FRIEND_SIDEIN_POSZ;
      pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[1], ANMCNTC(_POKE_SIDEIN_TIME), &apos , pWork->heapID);
      MCSS_GetScale( pWork->pokeMcss[1], &apos );
      apos.x *= _FRONT_POKMEON_SCALE_SIZE;
      apos.y *= _FRONT_POKMEON_SCALE_SIZE;
      MCSS_SetScale( pWork->pokeMcss[1], &apos );
    }
    pWork->pMoveMcss[0]->wave=1;
    pWork->pMoveMcss[1]->wave=1;
  }

  if(ANMCNTC(_POKE_SIDEIN_START2) == pWork->anmCount){
    _pokeMoveRenew(pWork->pMoveMcss[0],ANMCNTC(_POKE_SIDEIN_TIME2),&pWork->pMoveMcss[0]->end);
    _pokeMoveRenew(pWork->pMoveMcss[1],ANMCNTC(_POKE_SIDEIN_TIME2),&pWork->pMoveMcss[1]->end);
  }

  
  if(ANMCNTC(_POKE_LEAVE_START) == pWork->anmCount){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1]=NULL;


    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_LEAVE_POSX;
      apos.y = _POKEMON_PLAYER_LEAVE_POSY;
      apos.z = _POKEMON_PLAYER_LEAVE_POSZ;
      pWork->pMoveMcss[0] = _pokeMoveCreate(pWork->pokeMcss[0], ANMCNTC(_POKE_LEAVE_TIME), &apos, pWork->heapID);
      apos.x = _POKEMON_FRIEND_LEAVE_POSX;
      apos.y = _POKEMON_FRIEND_LEAVE_POSY;
      apos.z = _POKEMON_FRIEND_LEAVE_POSZ;
      pWork->pMoveMcss[1] = _pokeMoveCreate(pWork->pokeMcss[1], ANMCNTC(_POKE_LEAVE_TIME), &apos , pWork->heapID);
    }
  }
  if(ANMCNTC(_POKEMON_CREATE_TIME-3) == pWork->anmCount){
//    IRCPOKETRADE_PokeDeleteMcss(pWork, 1);
//    IRCPOKETRADE_PokeCreateMcss(pWork, 1, 1, IRC_POKEMONTRADE_GetRecvPP(pWork,1) );
//    MCSS_ResetVanishFlag( pWork->pokeMcss[3] );


  }

#if 1
  if(ANMCNTC(_POKEMON_CREATE_TIME) == pWork->anmCount){
      VecFx32 apos;
    MCSS_ResetVanishFlag(pWork->pokeMcss[3]);
    apos.x = _POKEMON_PLAYER_CENTER_POSX;
    apos.y = _POKEMON_PLAYER_CENTER_POSY;
    apos.z = _POKEMON_PLAYER_CENTER_POSZ;
    MCSS_SetPosition( pWork->pokeMcss[3] ,&apos );
    MCSS_SetAnmStopFlag(pWork->pokeMcss[3]);

    MCSS_SetPaletteFade( pWork->pokeMcss[3], 16, 16, 0, 0x7fff );
  }
  if(ANMCNTC(_POKEMON_CREATE_TIME+1) == pWork->anmCount){
    MCSS_SetPaletteFade( pWork->pokeMcss[3], 8, 8, 0, 0x7fff );
  }
  if(ANMCNTC(_POKEMON_CREATE_TIME+2) == pWork->anmCount){
    MCSS_SetPaletteFade( pWork->pokeMcss[3], 0, 0, 0, 0x7fff  );
  }
#endif


  if(pWork->anmCount == ANMCNTC(_POKECHANGE_WHITEOUT_START)){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                       ANMCNTC(_POKECHANGE_WHITEOUT_TIMER), 1, pWork->heapID );
  }
  if(pWork->anmCount == ANMCNTC(_POKECHANGE_WHITEIN_START)){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                       ANMCNTC(_POKECHANGE_WHITEIN_TIMER), 1, pWork->heapID );
  }
  if(pWork->anmCount == ANMCNTC(_POKESPLASH_WHITEOUT_START)){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                       ANMCNTC(_POKESPLASH_WHITEOUT_TIMER), 1, pWork->heapID );
  }
  if(pWork->anmCount == ANMCNTC(_POKESPLASH_WHITEIN_START)){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                       ANMCNTC(_POKESPLASH_WHITEIN_TIMER), 1, pWork->heapID );
  }
  //OS_TPrintf("C %d\n",pWork->anmCount);

  if(pWork->anmCount == ANMCNTC(_DEMO_END-1)){
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


