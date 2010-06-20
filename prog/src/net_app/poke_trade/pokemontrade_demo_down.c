//=============================================================================
/**
 * @file	  pokemontrade_demo_down.c
 * @bfief	  ポケモン交換アニメーション  下がる部分
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
static void _changeDemo_ModelTrade2_2(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade2_3(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade3(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade20(POKEMON_TRADE_WORK* pWork);
static void _changeDemo_ModelTrade2_jump(POKEMON_TRADE_WORK* pWork);


//#include "pokemontrade_mcss.c"
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
void POKMEONTRADE_DEMO_GTSDOWN_ChangeDemo(POKEMON_TRADE_WORK* pWork)
{
  VecFx32 apos;
  int i;

  _demoBGMChangeStart(pWork);
  
  //強制停止
  pWork->mcssStop[0] = TRUE;
  MCSS_SetAnimeIndex(pWork->pokeMcss[0], 0);
  MCSS_SetVanishFlag(pWork->pokeMcss[0]);
  MCSS_SetVanishFlag(pWork->pokeMcss[1]);

  if(pWork->type==POKEMONTRADE_TYPE_GTSUP){  //デモのときは真ん中から
    VecFx32 pos={_POKEMON_PLAYER_CENTER_POSX,_POKEMON_PLAYER_CENTER_POSY, _POKEMON_PLAYER_CENTER_POSZ};
    MCSS_SetPosition( pWork->pokeMcss[0] ,&pos );
  }

  POKEMONTRADE2D_SetVisibleMainVBlank( pWork , GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
  POKEMONTRADE2D_SetVisibleSubVBlank( pWork , 0 );

//  _CHANGE_STATE(pWork,_changeDemo_ModelT1);

  ///フラグ設定
  {
    _apperFlgCheck(pWork, &pWork->bByebyeMessageEach, &pWork->bByebyeNoJump, pWork->pFriend, TRUE);
    NET_PRINT("さよならメッセージ %d\n",pWork->bByebyeMessageEach);
    _apperFlgCheck(pWork, &pWork->bEncountMessageEach, &pWork->bEncountNoJump, pWork->pMy, FALSE);
    NET_PRINT("おかえりメッセージ %d\n",pWork->bEncountMessageEach);
  }

  _CHANGE_STATE(pWork,_changeDemo_ModelTrade2);


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
  IRC_POKETRADEDEMO_SetModel( pWork, TRADEDOWN_OBJECT);  //ダウン用モデル
  
  IRCPOKETRADE_PokeCreateMcss(pWork, 1, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,1),TRUE );  //相手裏
  IRCPOKETRADE_PokeCreateMcss(pWork, 2, 0, IRC_POKEMONTRADE_GetRecvPP(pWork,0),TRUE );  //みかた裏
  IRCPOKETRADE_PokeCreateMcss(pWork, 3, 1, IRC_POKEMONTRADE_GetRecvPP(pWork,1),TRUE );  //相手表
  MCSS_SetVanishFlag( pWork->pokeMcss[0] );
  MCSS_SetVanishFlag( pWork->pokeMcss[1] );
  MCSS_SetVanishFlag( pWork->pokeMcss[2] );
  MCSS_SetVanishFlag( pWork->pokeMcss[3] );
  POKEMONTRADE_DEMO_PTC_Init(pWork->pPokemonTradeDemo);
  POKEMONTRADE_DEMO_ICA_Init(pWork->pPokemonTradeDemo,_DEMO_TYPE_DOWN);  //ICAダウン用
  POKEMONTRADE_DEMO_PTC_Load1(pWork->pPokemonTradeDemo);

  _setNextAnim(pWork, _POKECHANGE_WHITEIN_START);
  POKETRADE_SE_gtsSeStrat(pWork->anmCount);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade2_2);

//  G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG0,GX_BLEND_PLANEMASK_BD,0,0);
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

}

static void _changeDemo_ModelTrade2_2(POKEMON_TRADE_WORK* pWork)
{
  POKETRADE_SE_gtsSeStrat(pWork->anmCount);
  POKEMONTRADE_DEMO_PTC_Load2(pWork->pPokemonTradeDemo);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade2_3);
}
static void _changeDemo_ModelTrade2_3(POKEMON_TRADE_WORK* pWork)
{
  POKETRADE_SE_gtsSeStrat(pWork->anmCount);
  _CHANGE_STATE(pWork,_changeDemo_ModelTrade3);
}

#if 0
static void _pokemonRedraw(POKEMON_TRADE_WORK* pWork, int mcssno)
{
  VecFx32 apos;
  MCSS_ResetVanishFlag(pWork->pokeMcss[mcssno]);
  apos.x = _POKEMON_PLAYER_CENTER_POSX;
  apos.y = _POKEMON_PLAYER_CENTER_POSY;
  apos.z = _POKEMON_PLAYER_CENTER_POSZ;
  MCSS_SetPosition( pWork->pokeMcss[mcssno] ,&apos );
  MCSS_SetAnmStopFlag(pWork->pokeMcss[mcssno]);
  MCSS_GetScale( pWork->pokeMcss[mcssno], &apos );
  if(apos.x < 0){
    apos.x = -FX32_ONE*16;
  }
  else{
    apos.x = FX32_ONE*16;
  }
  apos.y = FX32_ONE*16;
  apos.z = FX32_ONE;
  MCSS_SetScale( pWork->pokeMcss[mcssno], &apos );
}
#endif


static void _changeDemo_ModelTrade3(POKEMON_TRADE_WORK* pWork)
{
  POKETRADE_SE_gtsSeStrat(pWork->anmCount);

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
    POKEMONTRADE2D_SetVisibleMainVBlank( pWork , GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    POKEMONTRADE2D_SetVisibleSubVBlank( pWork , GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  }

  if(pWork->anmCount == ANMCNTC(_POKEMON_DELETE_TIME)){
    MCSS_SetVanishFlag( pWork->pokeMcss[0] );
//      IRCPOKETRADE_PokeDeleteMcss(pWork,0);
  }


  if(pWork->anmCount == ANMCNTC(_BALL_PARTICLE_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO1], DERMO_TEX001, NULL, pWork);
    OS_TPrintf("_BALL_PARTICLE_START\n");
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO2_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO2], DEMO_TEX002, NULL, pWork);
    OS_TPrintf("_PARTICLE_DEMO2_START\n");
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO3_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO3], DEMO_TEX003, NULL, pWork);
    OS_TPrintf("_PARTICLE_DEMO3_START\n");
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO3_START2)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_ORG], DEMO_TEX003, NULL, pWork);
    OS_TPrintf("_PARTICLE_DEMO3_START2\n");
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO4_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO4], DEMO_TEX004,  NULL, pWork);
    OS_TPrintf("_PARTICLE_DEMO4_START\n");
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO4_START2)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO8], DEMO_TEX004,  NULL, pWork);
    OS_TPrintf("_PARTICLE_DEMO4_START2\n");
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO5_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO5], DEMO_TEX005, _ballinEmitFunc, pWork);
    OS_TPrintf("_PARTICLE_DEMO5_START\n");
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO6_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO6], DEMO_TEX006, NULL, pWork);
    OS_TPrintf("_PARTICLE_DEMO6_START\n");
  }
  if(pWork->anmCount == ANMCNTC(_PARTICLE_DEMO7_START)){
    GFL_PTC_CreateEmitterCallback(pWork->pPokemonTradeDemo->ptc[PTC_KIND_DEMO7], DEMO_TEX007,_balloutEmitFunc, pWork);
    OS_TPrintf("_PARTICLE_DEMO7_START\n");
  }

  if(pWork->anmCount == ANMCNTC(_OAM_POKECREATE_START2)){
    _pokemonCreateCLACTInit(pWork);
    _pokemonCreateCLACTAdd(pWork);
  }
  if(pWork->anmCount == ANMCNTC(_POKEUP_WHITEOUT_START)){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_WHITE,
                       ANMCNTC(_POKEUP_WHITEOUT_TIMER), 1, pWork->heapID );

  }
  if(pWork->anmCount == ANMCNTC(_POKEUP_WHITEIN_START)){
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_WHITE,
                       ANMCNTC(_POKEUP_WHITEIN_TIMER), 1, pWork->heapID );
  }

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
      POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[0]);
      pWork->pMoveMcss[0] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[2], ANMCNTC(_POKE_APPEAR_TIME), &apos, pWork->heapID);
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
      POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[1]);
      pWork->pMoveMcss[1] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[3], ANMCNTC(_POKE_APPEAR_TIME), &apos , pWork->heapID);
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
    POKEMONTRADE_pokeMoveRenew(pWork->pMoveMcss[0],ANMCNTC(_POKE_APPEAR_TIME2),&apos);
    apos.x = _POKEMON_FRIEND_DOWN_POSX;
    apos.y = _POKEMON_FRIEND_DOWN_POSY;
    apos.z = _POKEMON_FRIEND_DOWN_POSZ;
    POKEMONTRADE_pokeMoveRenew(pWork->pMoveMcss[1],ANMCNTC(_POKE_APPEAR_TIME2),&apos);
  }
  
  if(ANMCNTC(_POKE_SIDEOUT_START) == pWork->anmCount){
    _moveMcssDelete(pWork, 0);
    _moveMcssDelete(pWork, 1);

    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_SIDEOUT_POSX;
      apos.y = _POKEMON_PLAYER_SIDEOUT_POSY;
      apos.z = _POKEMON_PLAYER_SIDEOUT_POSZ;
      POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[0]);
      pWork->pMoveMcss[0] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[2], ANMCNTC(_POKE_SIDEOUT_TIME), &apos, pWork->heapID);
      apos.x = _POKEMON_FRIEND_SIDEOUT_POSX;
      apos.y = _POKEMON_FRIEND_SIDEOUT_POSY;
      apos.z = _POKEMON_FRIEND_SIDEOUT_POSZ;
      POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[1]);
      pWork->pMoveMcss[1] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[3], ANMCNTC(_POKE_SIDEOUT_TIME), &apos , pWork->heapID);
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
      POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[0]);
      pWork->pMoveMcss[0] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[0], ANMCNTC(_POKE_SIDEIN_TIME), &apos, pWork->heapID);
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
      POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[1]);
      pWork->pMoveMcss[1] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[1], ANMCNTC(_POKE_SIDEIN_TIME), &apos , pWork->heapID);
      MCSS_GetScale( pWork->pokeMcss[1], &apos );
      apos.x *= _FRONT_POKMEON_SCALE_SIZE;
      apos.y *= _FRONT_POKMEON_SCALE_SIZE;
      MCSS_SetScale( pWork->pokeMcss[1], &apos );
    }
    pWork->pMoveMcss[0]->wave=1;
    pWork->pMoveMcss[1]->wave=1;
  }

  if(ANMCNTC(_POKE_SIDEIN_START2) == pWork->anmCount){
    POKEMONTRADE_pokeMoveRenew(pWork->pMoveMcss[0],ANMCNTC(_POKE_SIDEIN_TIME2),&pWork->pMoveMcss[0]->end);
    POKEMONTRADE_pokeMoveRenew(pWork->pMoveMcss[1],ANMCNTC(_POKE_SIDEIN_TIME2),&pWork->pMoveMcss[1]->end);
  }

  
  if(ANMCNTC(_POKE_LEAVE_START) == pWork->anmCount){
    _moveMcssDelete(pWork, 0);
    _moveMcssDelete(pWork, 1);


    {  //初期位置設定
      VecFx32 apos;
      apos.x = _POKEMON_PLAYER_LEAVE_POSX;
      apos.y = _POKEMON_PLAYER_LEAVE_POSY;
      apos.z = _POKEMON_PLAYER_LEAVE_POSZ;
      POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[0]);
      pWork->pMoveMcss[0] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[0], ANMCNTC(_POKE_LEAVE_TIME), &apos, pWork->heapID);
      apos.x = _POKEMON_FRIEND_LEAVE_POSX;
      apos.y = _POKEMON_FRIEND_LEAVE_POSY;
      apos.z = _POKEMON_FRIEND_LEAVE_POSZ;
      POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[1]);
      pWork->pMoveMcss[1] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[1], ANMCNTC(_POKE_LEAVE_TIME), &apos , pWork->heapID);
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

    if(!pWork->bEncountMessageEach){     // 不一致なのでジャンプ+回転
      _pokemonRedraw(pWork,3);
      _pokemonApperLastCreate(pWork,3);
      MCSS_SetPaletteFade( pWork->pokeMcss[3], 16, 16, 0, 0x7fff );
    }
    else{
      _pokemonRedraw(pWork,3);
      MCSS_SetVanishFlag( pWork->pokeMcss[3] );
      _pokemonRedraw(pWork,1);
      _pokemonApperLastCreate(pWork,3);
      MCSS_SetPaletteFade( pWork->pokeMcss[1], 16, 16, 0, 0x7fff );
    }
  }
  if(ANMCNTC(_POKEMON_CREATE_TIME+1) == pWork->anmCount){
    MCSS_SetPaletteFade( pWork->pokeMcss[3], 8, 8, 0, 0x7fff );
  }
  if(ANMCNTC(_POKEMON_CREATE_TIME+2) == pWork->anmCount){
    MCSS_SetPaletteFade( pWork->pokeMcss[3], 0, 0, 0, 0x7fff  );
  }
  if(ANMCNTC(_POKEMON_CREATE_TIME) < pWork->anmCount){
    if(pWork->bEncountMessageEach && (pWork->pMoveMcss[3]) &&
       (pWork->pMoveMcss[3]->nowcount == _JUMPSHORT_PEEK)){
      {
        VecFx32 apos;
        MCSS_GetPosition( pWork->pokeMcss[1] ,&apos );
        MCSS_SetPosition( pWork->pokeMcss[3] ,&apos );
      }
      MCSS_ResetVanishFlag( pWork->pokeMcss[3] );
      MCSS_SetVanishFlag( pWork->pokeMcss[1] );
      pWork->pMoveMcss[3]->pMcss = pWork->pokeMcss[3];
    }
  }
  if(ANMCNTC(_POKEMON_CREATE_TIME+_JUMP_ANMST) == pWork->anmCount){
    MCSS_ResetAnmStopFlag( pWork->pokeMcss[3] );
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
    _moveMcssDelete(pWork, 0);
    _moveMcssDelete(pWork, 1);
    IRC_POKETRADEDEMO_RemoveModel( pWork);
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade20);
  }
  POKEMONTRADE_pokeMoveFunc(pWork->pMoveMcss[0],pWork);
  POKEMONTRADE_pokeMoveFunc(pWork->pMoveMcss[1],pWork);
  POKEMONTRADE_pokeMoveFunc(pWork->pMoveMcss[2],pWork);
  POKEMONTRADE_pokeMoveFunc(pWork->pMoveMcss[3],pWork);
}


