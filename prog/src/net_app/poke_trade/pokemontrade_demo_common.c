#include "msg/msg_poke_trade.h"
#include "sound/pm_wb_voice.h"







//------------------------------------------------------------------
/**
 * @brief   登場時の演出の検査
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------

static void _apperFlgCheck(POKEMON_TRADE_WORK* pWork, u8* msg,u8* nojump,MYSTATUS* pMystatus, BOOL bBye)
{
  int select;
  if(bBye){ //帰るほう
    if(POKEMONTRADEPROC_IsTriSelect(pWork)){
      select = 1;
    }
    else{
      select = 0;
    }
  }
  else{//おくるほう
    if(POKEMONTRADEPROC_IsTriSelect(pWork)){
      select = 0;
    }
    else{
      select = 1;
    }
  }

  {
    POKEMON_PARAM* pp= IRC_POKEMONTRADE_GetRecvPP(pWork, select);
    u32 id = PP_Get(pp,ID_PARA_id_no,NULL);
    int monsno = PP_Get(pp,ID_PARA_monsno_egg,NULL);
    int frm = PP_Get( pp, ID_PARA_form_no, NULL );
    int tamago = PP_Get(pp,ID_PARA_tamago_flag,NULL);
    BOOL nojimpflg;

    if(!tamago){
      POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle(monsno, frm, pWork->heapID);
      nojimpflg = POKE_PERSONAL_GetParam(ppd, POKEPER_ID_no_jump);
      POKE_PERSONAL_CloseHandle(ppd);
    }
    if( tamago ){
      *nojump = TRUE;
    }
    else if(nojimpflg){
      *nojump = TRUE;
    }
    else{
      *nojump = FALSE;
    }
    NET_PRINT("Status %d Pokemon %d Select %d\n", MyStatus_GetID(pMystatus), id, select);
    if(MyStatus_GetID(pMystatus) != id){
      *msg = FALSE;
    }
    else{
      *msg = TRUE;
    }
  }
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
  if(pWork->cellRes[CHAR_POKECREATE] ){
    GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_POKECREATE] );
    GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_POKECREATE] );
    GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_POKECREATE] );
  }
  pWork->cellRes[CHAR_POKECREATE] = 0;
  pWork->cellRes[PLT_POKECREATE] = 0;
  pWork->cellRes[ANM_POKECREATE] = 0;
  pWork->pPokemonTradeDemo->pPokeCreateCLWK=NULL;

}

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



static void _demoBGMChangeStart(POKEMON_TRADE_WORK* pWork)
{
  pWork->demoBGM = 1;
  NET_PRINT("demoBGM1\n");
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
  //  _demoBGMChange(pWork->anmCount);

  POKEMONTRADE3D_3DReelPaletteFade(pWork);

  if(pWork->mcssStop[0]!=TRUE){  //アニメ終了待ち
    return;
  }

  ///フラグ設定
  {
    _apperFlgCheck(pWork, &pWork->bByebyeMessageEach, &pWork->bByebyeNoJump, pWork->pFriend, TRUE);
    NET_PRINT("さよならメッセージ %d\n",pWork->bByebyeMessageEach);
    _apperFlgCheck(pWork, &pWork->bEncountMessageEach, &pWork->bEncountNoJump, pWork->pMy, FALSE);
    NET_PRINT("おかえりメッセージ %d\n",pWork->bEncountMessageEach);
  }

  IRC_POKETRADEDEMO_RemoveModel(pWork);

  if(POKEMONTRADEPROC_IsNetworkMode(pWork)){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ANIMEEND,WB_NET_TRADE_SERVICEID);
    _CHANGE_STATE(pWork,_changeDemo_ModelT2);
  }
  else{
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade0);
  }
}

//ネットワーク同期処理
static void _changeDemo_ModelT2(POKEMON_TRADE_WORK* pWork)
{
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ANIMEEND,WB_NET_TRADE_SERVICEID)){
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade0);
  }

}

//------------------------------------------------------------------
/**
 * @brief   ポケモン同期待ち終了後 移動開始
 * @param   POKEMON_TRADE_WORK ワーク
 * @retval  none
 */
//------------------------------------------------------------------

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
    //    pWork->pModelFade = _createPaletteFade(GFL_G3D_UTIL_GetResHandle(pWork->g3dUtil,0), pWork->heapID);
    //    _FIELD_StartPaletteFade( pWork->pModelFade, 0, 16, ANMCNTC(_POKEMON_CENTER_TIME/3)/16, 0 );
  }

  // ポケモン中央に移動開始
  {
    VecFx32 pos={_POKEMON_PLAYER_CENTER_POSX,_POKEMON_PLAYER_CENTER_POSY, _POKEMON_PLAYER_CENTER_POSZ};
    POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[0], ANMCNTC(_POKEMON_CENTER_TIME), &pos, pWork->heapID);
  }
  {
    VecFx32 pos={_POKEMON_FRIEND_CENTER_POSX,_POKEMON_FRIEND_CENTER_POSY, _POKEMON_FRIEND_CENTER_POSZ};
    POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[1]);
    pWork->pMoveMcss[1] = POKEMONTRADE_pokeMoveCreate(pWork->pokeMcss[1], ANMCNTC(_POKEMON_CENTER_TIME*2), &pos, pWork->heapID);
  }

  MCSS_SetAnimCtrlCallBack(pWork->pokeMcss[0], (u32)pWork, POKEMONTRADE_McssAnmStop, NNS_G2D_ANMCALLBACKTYPE_LAST_FRM);

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
    POKEMONTRADE_pokeMoveFunc(pWork->pMoveMcss[0],pWork);
    POKEMONTRADE_pokeMoveFunc(pWork->pMoveMcss[1],pWork);
  }

  if(pWork->anmCount > ANMCNTC(_POKEMON_CENTER_TIME)){  //フェード完了
    POKEMONTRADE2D_SetVisibleMainVBlank( pWork , GX_PLANEMASK_BG0|GX_PLANEMASK_OBJ );
    POKEMONTRADE2D_SetVisibleSubVBlank( pWork , 0 );
    POKETRADE_2D_GTSPokemonIconResetAll(pWork);
    IRC_POKETRADE_AllDeletePokeIconResource(pWork);
    POKETRADE_MESSAGE_SixStateDelete(pWork);

    _setNextAnim(pWork, 0);

    {
      POKEMON_PARAM* pp= IRC_POKEMONTRADE_GetRecvPP(pWork, POKEMONTRADEPROC_IsTriSelect(pWork));
      int id = PP_Get(pp,ID_PARA_id_no,NULL);
      if(! PP_Get(pp,ID_PARA_tamago_flag,NULL) ){
        int monsno = PP_Get(pp,ID_PARA_monsno_egg,NULL);
        int formno = PP_Get(pp,ID_PARA_form_no,NULL);
        PMV_REF pmvRef;
        PMV_MakeRefDataMine( &pmvRef );
        OS_TPrintf("泣き声 %d\n",monsno);
        PMVOICE_Play( monsno, formno, 64, FALSE, 0, 0, FALSE, (u32)&pmvRef );
      }
      if(pWork->pMoveMcss[0]){
        GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
        pWork->pMoveMcss[0]=NULL;
      }
#if 0
      if(MyStatus_GetID(pWork->pFriend) != id){
        pWork->bByebyeMessageEach = FALSE;
      }
      else{
        pWork->bByebyeMessageEach = TRUE;
      }
#endif

      if(!pWork->bByebyeMessageEach){     // 不一致なのでジャンプ+回転
        VecFx32 apos;
        POKEMON_PARAM* pp;

        MCSS_GetPosition(pWork->pokeMcss[0], &apos);
        POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[0]);
        if(pWork->bByebyeNoJump){
          pWork->pMoveMcss[0] = POKEMONTRADE_pokeTblMoveCreate(pWork->pokeMcss[0], elementof(_noJumpTbl), &apos, _noJumpTbl,  pWork->heapID);
        }
        else{
          pWork->pMoveMcss[0] = POKEMONTRADE_pokeTblMoveCreate(pWork->pokeMcss[0], elementof(_shortJumpTbl), &apos, _shortJumpTbl,  pWork->heapID);
        }

        pp = IRC_POKEMONTRADE_GetRecvPP(pWork,POKEMONTRADEPROC_IsTriSelect(pWork));
        IRCPOKETRADE_PokeCreateMcss(pWork, 2, 0, pp,TRUE );  //みかた裏
        MCSS_SetAnmStopFlag(pWork->pokeMcss[2]);
        MCSS_SetVanishFlag( pWork->pokeMcss[2] );

      }
      else{// 一致しているのでジャンプ
        VecFx32 apos;
        MCSS_GetPosition(pWork->pokeMcss[0], &apos);
        POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[0]);
        if(pWork->bByebyeNoJump){
          pWork->pMoveMcss[0] = POKEMONTRADE_pokeTblMoveCreate(pWork->pokeMcss[0], elementof(_noJumpTbl), &apos, _noJumpTbl,  pWork->heapID);
        }
        else{
          pWork->pMoveMcss[0] = POKEMONTRADE_pokeTblMoveCreate(pWork->pokeMcss[0], elementof(_triJumpTbl), &apos, _triJumpTbl,  pWork->heapID);
        }
      }
    }

    _CHANGE_STATE(pWork,_changeDemo_ModelTrade2_jump);
  }
}


static void _byebyeMessage(POKEMON_TRADE_WORK* pWork)
{
  GFL_NET_ReloadIconTopOrBottom(FALSE, pWork->heapID);
  
  if(pWork->type == POKEMONTRADE_TYPE_GTSUP){
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_19, pWork->pMessageStrBufEx );
  }
  else if(pWork->type == POKEMONTRADE_TYPE_GTSMID){
    GFL_MSG_GetString( pWork->pMsgData, gtsnego_info_22, pWork->pMessageStrBufEx );
  }
  else if(pWork->bByebyeMessageEach){
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_29, pWork->pMessageStrBufEx );
  }
  else{
    GFL_MSG_GetString( pWork->pMsgData, POKETRADE_STR2_28, pWork->pMessageStrBufEx );
  }
  {
    POKEMON_PARAM* pp = IRC_POKEMONTRADE_GetRecvPP(pWork, POKEMONTRADEPROC_IsTriSelect(pWork));
    WORDSET_RegisterPokeNickName( pWork->pWordSet, 0,  pp );
  }
  WORDSET_RegisterPlayerName( pWork->pWordSet, 1, pWork->pFriend  );
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBuf, pWork->pMessageStrBufEx);
  GFL_BG_FillScreen( GX_PLANEMASK_BG2, 0x0000, 0, 0, 32, 24, GFL_BG_SCRWRT_PALIN );
  POKETRADE_MESSAGE_WindowOpenCustom(pWork,TRUE,FALSE);

  POKEMONTRADE2D_SetVisibleSubVBlank( pWork , GX_PLANEMASK_BG2|GX_PLANEMASK_OBJ );
  G2S_BlendNone();
}

static void _changeDemo_ModelTrade2_jump(POKEMON_TRADE_WORK* pWork)
{
  //_jumpSeStrat(pWork->anmCount,pWork);
  if(pWork->anmCount == 10){
    _byebyeMessage(pWork);  //別れのメッセージ
  }

  if(!pWork->bByebyeMessageEach && (pWork->anmCount == _JUMP_PEEK)){  //回転
    {
      VecFx32 apos;
      MCSS_GetPosition( pWork->pokeMcss[0] ,&apos );
      MCSS_SetPosition( pWork->pokeMcss[2] ,&apos );
    }
    MCSS_ResetVanishFlag( pWork->pokeMcss[2] );
    MCSS_SetVanishFlag( pWork->pokeMcss[0] );
    pWork->pMoveMcss[0]->pMcss = pWork->pokeMcss[2];
  }

  if((pWork->anmCount > _POKEMON_VOICE_TIME) && (POKETRADE_MESSAGE_EndCheck(pWork))){
    POKETRADE_MESSAGE_WindowClose(pWork);

    if(!pWork->bByebyeMessageEach){
/*
      {
        VecFx32 apos;
        MCSS_GetPosition( pWork->pokeMcss[2] ,&apos );
        MCSS_SetPosition( pWork->pokeMcss[0] ,&apos );
      }
      MCSS_ResetVanishFlag( pWork->pokeMcss[0] );
      MCSS_SetVanishFlag( pWork->pokeMcss[2] );
      IRCPOKETRADE_PokeDeleteMcss(pWork,2);
      */
      
      IRCPOKETRADE_PokeDeleteMcss(pWork,0);
      pWork->pokeMcss[0] = pWork->pokeMcss[2];
      pWork->pokeMcss[2] = NULL;
    }

    if(pWork->pMoveMcss[0]){
      GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
      pWork->pMoveMcss[0]=NULL;
    }
    _CHANGE_STATE(pWork,_changeDemo_ModelTrade2);
  }
  POKEMONTRADE_pokeMoveFunc(pWork->pMoveMcss[0],pWork);
}


static void _changeDemo_ModelTrade20(POKEMON_TRADE_WORK* pWork)
{
  _pokemonCreateCLACTExit(pWork);
  POKEMONTRADE_DEMO_PTC_End(pWork->pPokemonTradeDemo, IRPTC_KIND_NUM_MAX);
  POKEMONTRADE_DEMO_IRICA_Delete(pWork->pPokemonTradeDemo);
  GFL_HEAP_FreeMemory(pWork->pPokemonTradeDemo);
  pWork->pPokemonTradeDemo = NULL;
  _CHANGE_STATE(pWork,POKMEONTRADE_SAVE_Init);
}


static void _moveMcssDelete(POKEMON_TRADE_WORK* pWork,int no)
{
  if(pWork->pMoveMcss[0]){
    GFL_HEAP_FreeMemory(pWork->pMoveMcss[0]);
    pWork->pMoveMcss[0]=NULL;
  }
}


static void _MCSS_ApperSet(MCSS_WORK *mcss, POKEMON_TRADE_WORK* pWork)
{
  VecFx32 apos;
  MCSS_SetAlpha(mcss, 31);
  MCSS_ResetVanishFlag(mcss);
  apos.x = _POKEMON_PLAYER_CENTER_POSX;
  apos.y = _POKEMON_PLAYER_CENTER_POSY;
  apos.z = _POKEMON_PLAYER_CENTER_POSZ;
  MCSS_SetPosition( mcss ,&apos );
  MCSS_SetAnmStopFlag(mcss);

  MCSS_SetScale( mcss, &pWork->pPokemonTradeDemo->PushPos );
  MCSS_SetPaletteFade( mcss, 16, 16, 0, 0x7fff );
}


//最後にポケモンを登場させる時に泣き声＋うごき

static void _pokemonApperLastCreate(POKEMON_TRADE_WORK* pWork, int num)
{
  VecFx32 apos;
  {  //声を出す
    POKEMON_PARAM* pp= IRC_POKEMONTRADE_GetRecvPP(pWork, 1-POKEMONTRADEPROC_IsTriSelect(pWork));
    int id = PP_Get(pp,ID_PARA_id_no,NULL);
    if(! PP_Get(pp,ID_PARA_tamago_flag,NULL) ){
      int monsno = PP_Get(pp,ID_PARA_monsno_egg,NULL);
      int formno = PP_Get(pp,ID_PARA_form_no,NULL);
      OS_TPrintf("泣き声 %d\n",monsno);
      PMVOICE_Play( monsno, formno, 64, FALSE, 0, 0, FALSE, 0 );
    }
  }
  
  if(!pWork->bEncountMessageEach){     // 不一致なのでジャンプ+回転
    MCSS_GetPosition(pWork->pokeMcss[num], &apos);
    POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[3]);
    if(pWork->bEncountNoJump){
      pWork->pMoveMcss[3] = POKEMONTRADE_pokeTblMoveCreate(pWork->pokeMcss[num], elementof(_noJumpTbl), &apos, _noJumpTbl,  pWork->heapID);
    }
    else{
      pWork->pMoveMcss[3] = POKEMONTRADE_pokeTblMoveCreate(pWork->pokeMcss[num], elementof(_triJumpTbl), &apos, _triJumpTbl,  pWork->heapID);
    }
  }
  else{// 一致しているのでジャンプ
    int buff[]={2,3,0,1};
    MCSS_WORK* pMcss= pWork->pokeMcss[buff[num]];
    MCSS_GetPosition(pWork->pokeMcss[num], &apos);
    POKEMONTRADE_McssMoveDel(pWork->pMoveMcss[3]);
    if(pWork->bEncountNoJump){
      pWork->pMoveMcss[3] = POKEMONTRADE_pokeTblMoveCreate(pMcss, elementof(_noJumpTbl), &apos, _noJumpTbl,  pWork->heapID);
    }
    else{
      pWork->pMoveMcss[3] = POKEMONTRADE_pokeTblMoveCreate(pMcss, elementof(_shortJumpTbl), &apos, _shortJumpTbl2,  pWork->heapID);
    }
  }
}


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


