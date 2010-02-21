//======================================================================
/**
 * @file  fld_btl_inst_tool.c
 * @brief  施設バトル呼び出し等の共通部分
 *
 * @note  施設：institution
 * モジュール名：FBI_TOOL_
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "event_fieldmap_control.h"
#include "poke_tool/poke_regulation.h"
#include "app/pokelist.h"
#include "app/p_status.h"
#include "battle/battle.h"

#include "event_battle.h"

#include "fieldmap.h"

#include "field/field_msgbg.h"
#include "message.naix"

#include "fld_btl_inst_tool.h"


#include "item/itemsym.h"
#include "waza_tool/wazano_def.h"

#ifdef PM_DEBUG
static BOOL check_TrainerType(u8 tr_type);
#endif  //PM_DEBUG

//--------------------------------------------------------------
/// ポケモンリスト選択イベント
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldmap;
  
  PSTATUS_DATA StatusData;
  PLIST_DATA ListData;
  REGULATION Regulation;
  
  PL_RETURN_TYPE *ReturnMode;
  PL_SELECT_POS *ResultSelect;
  u8 *ResultNoAry;

  POKEPARTY *SrcParty;
  POKEPARTY *DstParty;
}EVENT_WORK_POKE_LIST;

static GMEVENT_RESULT PokeSelEvt( GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/**
 * ポケモンリスト呼び出しイベント作成
 * @param gsys          ゲームシステムポインタ
 * @param inType        リストタイプ
 * @param inReg         レギュレーション
 * @param pp            対象ポケパーティポインタ
 * @param outSelNum     選択リスト番号配列へのポインタ
 * @param outResult     リスト結果選択された位置
 * @param outRetMode    リスト戻りタイプ
 * @param outParty      ＮＵＬＬでない場合、結果を格納するポケパーティ
 *
 * @retval GMEVENT      イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *FBI_TOOL_CreatePokeListEvt(
    GAMESYS_WORK *gsys,
    const PL_LIST_TYPE inType, const PL_MODE_TYPE inListMode, const int inReg,
    POKEPARTY *inTargetParty,
    u8 *outSelNoAry, PL_SELECT_POS *outResult, PL_RETURN_TYPE *outRetMode, POKEPARTY *outParty )
{
  GMEVENT *event;
  FIELDMAP_WORK *fieldmap;
  EVENT_WORK_POKE_LIST *work;

  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create( gsys, NULL,
      PokeSelEvt, sizeof(EVENT_WORK_POKE_LIST) );

  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldmap = fieldmap;
  work->ResultNoAry = outSelNoAry;
  work->ResultSelect = outResult;
  work->ReturnMode = outRetMode;
  work->SrcParty = inTargetParty;
  work->DstParty = outParty;
  {
    PLIST_DATA *list = &work->ListData;
    PokeRegulation_LoadData(inReg, &work->Regulation);  //レギュレーションロード
    list->pp = inTargetParty;
    list->reg = &work->Regulation;
    list->type = inType;
    list->mode = inListMode;
  }
  {
    PSTATUS_DATA *st = &work->StatusData;
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gdata );
    MI_CpuClear8( st, sizeof(PSTATUS_DATA) );
    st->ppd = inTargetParty;
    st->ppt = PST_PP_TYPE_POKEPARTY;
    st->max = PokeParty_GetPokeCount( inTargetParty );
    st->mode = PST_MODE_NORMAL;
    st->page = PPT_INFO;
    st->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
  }

  return event;

}

//--------------------------------------------------------------
/**
 * ポケモンリスト選択イベント
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT PokeSelEvt( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_POKE_LIST *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  
  switch( *seq )
  {
  case 0:
		GMEVENT_CallEvent( event,
        EVENT_FieldFadeOut_Black(gsys,work->fieldmap,FIELD_FADE_WAIT) );
    (*seq)++;
    break;
  case 1:
		GMEVENT_CallEvent( event, EVENT_FieldClose(gsys,work->fieldmap) );
    (*seq)++;
    break;
  case 2:
    GMEVENT_CallEvent( event, EVENT_PokeSelect(
          gsys,work->fieldmap, &work->ListData, &work->StatusData) );
    (*seq)++;
    break;
  case 3:
		GMEVENT_CallEvent( event, EVENT_FieldOpen(gsys) );
    (*seq)++;
    break;
  case 4:
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black(
          gsys, work->fieldmap, FIELD_FADE_WAIT) );

    MI_CpuCopy8( work->ListData.in_num, work->ResultNoAry, 6 );
    *work->ResultSelect = work->ListData.ret_sel;
    *work->ReturnMode = work->ListData.ret_mode;

    //ポケパーティにデータセット
    if ( (*work->ResultSelect != PL_SEL_POS_EXIT) &&
         (*work->ResultSelect != PL_SEL_POS_EXIT2) && 
         (work->DstParty != NULL) ){
      int i;
      int num = PokeParty_GetPokeCountMax( work->DstParty );
      NOZOMU_Printf("party num %d\n", num);
      for( i=0;i < num; i++ )
      {
        POKEMON_PARAM *param;
        int pos;
        pos = work->ResultNoAry[i]-1;
        if( pos >= 6 ){
          GF_ASSERT( 0 );
          work->ResultNoAry[i] = 1;
        }
        param = PokeParty_GetMemberPointer( work->SrcParty, pos );
        PokeParty_Add( work->DstParty, param );
      }
    }
    (*seq)++;
    break;
  case 5:
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}



//===対戦前メッセージ出力関連===
#include "bsubway_scrwork.h"

typedef struct
{
  GAMESYS_WORK *gsys;
  FLDSYSWIN_STREAM *sysWin;
  STRBUF *strBuf;
}EVENT_WORK_TRAINER_BEFORE_MSG;

static GMEVENT_RESULT TrainerBeforeMsgEvt( GMEVENT *event, int *seq, void *wk );


//--------------------------------------------------------------
/**
 * 対戦前メッセージ 呼び出し  @todo
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * FBI_TOOL_CreateTrainerBeforeMsgEvt(
    GAMESYS_WORK *gsys, BSUBWAY_PARTNER_DATA *tr_data )
{
  GMEVENT *event;
  GAMEDATA *gdata;
  FLDMSGBG *msgbg;
  FIELDMAP_WORK *fieldmap;
  EVENT_WORK_TRAINER_BEFORE_MSG *work;
  
  gdata = GAMESYSTEM_GetGameData( gsys );
  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  msgbg = FIELDMAP_GetFldMsgBG( fieldmap );
  
  event = GMEVENT_Create( gsys, NULL,
    TrainerBeforeMsgEvt, sizeof(EVENT_WORK_TRAINER_BEFORE_MSG) );
  
  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->strBuf = GFL_STR_CreateBuffer( 92, HEAPID_PROC );
  
  if( tr_data->bt_trd.appear_word[0] == 0xffff ){ //ROM MSG
    GFL_MSGDATA *msgdata;
    
    msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_tower_trainer_dat, HEAPID_PROC );
    
    GFL_MSG_GetString(
        msgdata, 
        tr_data->bt_trd.appear_word[1],
        work->strBuf );
    
    GFL_MSG_Delete( msgdata );
  }else{ //簡易会話 kari
    GFL_MSGDATA *msgdata;
    
    msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_tower_trainer_dat, HEAPID_PROC );
    
    GFL_MSG_GetString(
        msgdata,
        0,
        work->strBuf );
    
    GFL_MSG_Delete( msgdata );
  }
  
  work->sysWin = FLDSYSWIN_STREAM_Add( msgbg, NULL, 19 );
  return( event );
}

//--------------------------------------------------------------
/**
 * 対戦前メッセージ
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT TrainerBeforeMsgEvt(
    GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_TRAINER_BEFORE_MSG *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  
  switch( *seq )
  {
  case 0:
    FLDSYSWIN_STREAM_PrintStrBufStart( work->sysWin, 0, 0, work->strBuf );
    (*seq)++;
    break;
  case 1:
    if( FLDSYSWIN_STREAM_Print(work->sysWin) == TRUE ){
      (*seq)++;
    }
    break;
  case 2:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
      FLDSYSWIN_STREAM_Delete( work->sysWin );
      GFL_STR_DeleteBuffer( work->strBuf );
      return( GMEVENT_RES_FINISH );
    }
    break;
  }
  return( GMEVENT_RES_CONTINUE );
}


//===バトルパラムセット関連===

#include "gamesystem/btl_setup.h"
#include "sound/pm_sndsys.h"
#include "pm_define.h"
#include "tr_tool/trtype_def.h"
#include "battle/battle.h"

static BSP_TRAINER_DATA *CreateBSPTrainerData( HEAPID heapID );
static void MakePokePara( const BSUBWAY_POKEMON *src, POKEMON_PARAM *dest );
static u16 GetRand( void );
static u8 GetPowerRnd(u16 tr_no);
static BSUBWAY_TRAINER_ROM_DATA * AllocTrainerRomData( BSUBWAY_PARTNER_DATA *tr_data, u16 tr_no, HEAPID inHeapID );
static BOOL SetBSWayPokemonParam(
    BSUBWAY_TRAINER_ROM_DATA *trd,
    u16 tr_no, BSUBWAY_POKEMON *pwd, u8 cnt,
    u16 *set_poke_no, u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM *poke, HEAPID heapID );
static u32 MakePokemonParam(
    BSUBWAY_POKEMON *pwd,
    u16 poke_no, u32 poke_id, u32 poke_rnd, u8 pow_rnd,
    u8 mem_idx, BOOL itemfix, HEAPID heapID );
static void GetPokemonRomData( BSUBWAY_POKEMON_ROM_DATA *prd, int index );
static void * GetTrainerRomData( u16 tr_no, HEAPID heapID );

//--------------------------------------------------------------
/// アイテムテーブル
//  持ちポケモン決定はランダムでしているが無限ループ防止のため、
//  ある程度まわしたら、ポケモンの不一致のみをチェックして、
//  アイテムを固定で持たせるためのアイテムテーブル
//--------------------------------------------------------------
static const u16 BattleTowerPokemonItem[]={
  ITEM_HIKARINOKONA,
  ITEM_RAMUNOMI,
  ITEM_TABENOKOSI,
  ITEM_SENSEINOTUME,
};

//--------------------------------------------------------------
/**
 * BATTLE_SETUP_PARAM作成
 * @param gsys GAMESYS_WORK
 * @retval BATTLE_SETUP_PARAM
 */
//--------------------------------------------------------------
BATTLE_SETUP_PARAM * FBI_TOOL_CreateBattleParam(
    GAMESYS_WORK *gsys, const POKEPARTY *my_party, int inPlayMode, BSUBWAY_PARTNER_DATA *partner_data, const int inMemNum )
{
  u16 play_mode;
  BATTLE_SETUP_PARAM *dst;
  BTL_FIELD_SITUATION sit;
  GAMEDATA *gameData = GAMESYSTEM_GetGameData( gsys );
  
  play_mode = inPlayMode;
  dst = BATTLE_PARAM_Create( HEAPID_PROC );
  
  {
    BTL_FIELD_SITUATION_Init( &sit );
    
//  dst->netID = 0;
    dst->netHandle = NULL;
    dst->commMode = BTL_COMM_NONE;
    dst->commPos = 0;
    
    dst->multiMode = 0;
    dst->recBuffer = NULL;
    dst->fRecordPlay = FALSE;
    
    switch( play_mode ){
    case BSWAY_PLAYMODE_MULTI:
    case BSWAY_PLAYMODE_S_MULTI:
      dst->multiMode = 1;
      break;
    case BSWAY_PLAYMODE_COMM_MULTI:
    case BSWAY_PLAYMODE_S_COMM_MULTI:
      dst->multiMode = 1;
      dst->netHandle = GFL_NET_HANDLE_GetCurrentHandle();
      dst->commMode = BTL_COMM_DS;
      
      if( GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE ){
        dst->commPos = 0;
      }else{
        dst->commPos = 1;
      }
      break;
    }
    
    dst->party[BTL_CLIENT_PLAYER] = NULL;
    dst->party[BTL_CLIENT_ENEMY1] = NULL;
    dst->party[BTL_CLIENT_PARTNER] = NULL;
    dst->party[BTL_CLIENT_ENEMY2] = NULL;

    dst->competitor = BTL_COMPETITOR_TRAINER;

    dst->playerStatus[BTL_CLIENT_PLAYER] = GAMEDATA_GetMyStatus( gameData );
    dst->playerStatus[BTL_CLIENT_ENEMY1] = NULL;
    dst->playerStatus[BTL_CLIENT_PARTNER] = NULL;
    dst->playerStatus[BTL_CLIENT_ENEMY2] = NULL;

    dst->itemData     = GAMEDATA_GetMyItem( gameData );
    dst->bagCursor    = GAMEDATA_GetBagCursor( gameData );
    dst->zukanData    = GAMEDATA_GetZukanSave( gameData );
//  dst->commSupport  = GAMEDATA_GetCommPlayerSupportPtr( gameData );
    dst->commSupport  = NULL;

    {
      SAVE_CONTROL_WORK *saveCtrl = GAMEDATA_GetSaveControlWork( gameData );
      dst->configData = SaveData_GetConfig( saveCtrl );
    }

    MI_CpuCopy8( &sit, &dst->fieldSituation, sizeof(BTL_FIELD_SITUATION) );

    dst->musicDefault = SEQ_BGM_VS_NORAPOKE;
    dst->musicPinch = SEQ_BGM_BATTLEPINCH;
    dst->result = BTL_RESULT_WIN;
  }
  
  BTL_SETUP_SetSubwayMode( dst );

  switch( play_mode )
  {
  case BSWAY_MODE_SINGLE:
  case BSWAY_MODE_WIFI:
  case BSWAY_MODE_S_SINGLE:
    dst->rule = BTL_RULE_SINGLE;
    break;
  case BSWAY_MODE_DOUBLE:
  case BSWAY_MODE_MULTI:
  case BSWAY_MODE_COMM_MULTI:
  case BSWAY_MODE_S_DOUBLE:
  case BSWAY_MODE_S_MULTI:
  case BSWAY_MODE_S_COMM_MULTI:
    dst->rule = BTL_RULE_DOUBLE;
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  { //トレーナーデータ確保
    dst->tr_data[BTL_CLIENT_PLAYER] = CreateBSPTrainerData( HEAPID_PROC );
    dst->tr_data[BTL_CLIENT_ENEMY1] = CreateBSPTrainerData( HEAPID_PROC );
  }

  { //敵トレーナーセット
    PMS_DATA *pd;
    BTL_CLIENT_ID client = BTL_CLIENT_ENEMY1;
    POKEPARTY **party = &dst->party[client];
    BSP_TRAINER_DATA *tr_data = dst->tr_data[client];

    BSUBWAY_PARTNER_DATA *bsw_partner = partner_data;
    BSUBWAY_TRAINER *bsw_trainer = &bsw_partner->bt_trd;

    tr_data->tr_id = bsw_trainer->player_id;
    tr_data->tr_type = bsw_trainer->tr_type;
    tr_data->ai_bit = 0xFFFFFFFF;  //最強

    //name
    GFL_STR_SetStringCode( tr_data->name, bsw_trainer->name );

    //win word
    pd = (PMS_DATA*)bsw_trainer->win_word;
    tr_data->win_word = *pd;

    pd = (PMS_DATA*)bsw_trainer->lose_word;
    tr_data->lose_word = *pd;

    //敵ポケモンセット
    {
      int i;
      POKEMON_PARAM*  pp;

      pp = GFL_HEAP_AllocMemoryLo( HEAPID_PROC, POKETOOL_GetWorkSize() );
      *party = PokeParty_AllocPartyWork( HEAPID_PROC );
      PokeParty_Init( *party, TEMOTI_POKEMAX );

      for( i = 0; i < inMemNum; i++ ){
        MakePokePara( &(bsw_partner->btpwd[i]), pp );
        PokeParty_Add( *party, pp );
      }
      GFL_HEAP_FreeMemory( pp );
    }
  }

  { //プレイヤーセット
    BTL_CLIENT_ID client = BTL_CLIENT_PLAYER;
    POKEPARTY **party = &dst->party[client];
    BSP_TRAINER_DATA *data = dst->tr_data[client];
    PLAYER_WORK * player = GAMEDATA_GetMyPlayerWork( gameData );

    MyStatus_CopyNameString(
        (const MYSTATUS*)&player->mystatus, data->name );

    data->tr_type = TRTYPE_HERO +
      MyStatus_GetMySex((const MYSTATUS*)&player->mystatus );

    //ポケモンセット
    {
      int i;
      POKEMON_PARAM *entry_pp;
      const POKEMON_PARAM *my_pp;

      entry_pp = GFL_HEAP_AllocMemoryLo(
          HEAPID_PROC, POKETOOL_GetWorkSize() );
      PP_Clear( entry_pp );

      *party = PokeParty_AllocPartyWork( HEAPID_PROC );
      PokeParty_Init( *party, TEMOTI_POKEMAX );

      for( i = 0; i < inMemNum; i++ ){
        my_pp = PokeParty_GetMemberPointer( my_party,i/*wk->member[i]*/ );

        POKETOOL_CopyPPtoPP( (POKEMON_PARAM*)my_pp, entry_pp );

        if( PP_Get(my_pp,ID_PARA_level,NULL) != 50 ){
          u32 exp = POKETOOL_GetMinExp(
              PP_Get(my_pp,ID_PARA_monsno,NULL),
              PP_Get(my_pp,ID_PARA_form_no,NULL),
              50 );

          PP_Put( entry_pp, ID_PARA_exp, exp );
          PP_Renew( entry_pp );
        }

#ifdef DEBUG_ONLY_FOR_kagaya
        PP_Put( entry_pp, ID_PARA_hp, 1 );
#endif
        PokeParty_Add( *party, entry_pp );
      }

      #ifdef DEBUG_ONLY_FOR_kagaya
      {
        int count = PokeParty_GetPokeCount( *party );
        KAGAYA_Printf( "ポケモンセット　カウント=%d, max=%d\n", count, i );
      }
      #endif

      GFL_HEAP_FreeMemory( entry_pp );
    }
  }

  return dst;
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　トレーナーデータ生成
 * @param heapID HEAPID
 * @retval BSP_TRAINER_DATA*
 */
//--------------------------------------------------------------
static BSP_TRAINER_DATA *CreateBSPTrainerData( HEAPID heapID )
{
  BSP_TRAINER_DATA* tr_data = GFL_HEAP_AllocClearMemory( heapID, sizeof( BSP_TRAINER_DATA ) );
  tr_data->name =   GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, heapID );
  return tr_data;
}

//--------------------------------------------------------------
/**
 *  @brief  バトルサブウェイのポケモンパラメータ生成
 *  @param pwd  [in/out]ポケモンパラメータの展開場所
 *  @param poke_no  [in]タワーromデータポケモンナンバー
 *  @param poke_id  [in]ポケモンにセットするid
 *  @param poke_rnd [in]セットする個性乱数(0が引き渡されたら関数内で生成)
 *  @param pow_rnd  [in]ポケモンにセットするpow_rnd値
 *  @param mem_idx  [in]メンバーindex。一体目or二体目
 *  @param itemfix  [in]TRUEなら固定アイテム。FALSEならromデータのアイテム
 *  @param heapID [in]テンポラリメモリを確保するヒープID
 *  @retval u32 personal_rnd:生成されたポケモンの個性乱数値
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY 似た処理frontier_tool.c Frontier_PokemonParamMake
 */
//--------------------------------------------------------------
static u32 MakePokemonParam(
    BSUBWAY_POKEMON *pwd,
    u16 poke_no, u32 poke_id, u32 poke_rnd, u8 pow_rnd,
    u8 mem_idx, BOOL itemfix, HEAPID heapID )
{
  int i;
  int  exp;
  u32  personal_rnd;
  u8  friend;
  BSUBWAY_POKEMON_ROM_DATA  prd_s;

  MI_CpuClear8(pwd,sizeof(BSUBWAY_POKEMON));

  //ロムデータロード
  GetPokemonRomData(&prd_s,poke_no);

  //モンスターナンバー
  pwd->mons_no=prd_s.mons_no;

  //フォルムナンバー
  pwd->form_no=prd_s.form_no;

  //装備道具
  if(itemfix){
    //50回以上まわしていたフラグがTUREなら、
    //装備アイテムは固定のものを持たせる
    pwd->item_no=BattleTowerPokemonItem[mem_idx];
  }else{
    //romデータに登録されたアイテムを持たせる
    pwd->item_no=prd_s.item_no;
  }

  //なつき度は255がデフォルト
  friend=255;
  for(i=0;i<WAZA_TEMOTI_MAX;i++){
    pwd->waza[i]=prd_s.waza[i];
    //やつあたりを持っているときは、なつき度を0にする
    if(prd_s.waza[i]==WAZANO_YATUATARI){
      friend=0;
    }
  }

  //IDNo
  pwd->id_no=poke_id;

  if(poke_rnd == 0){
    //個性乱数          //※ＷＢでは性格と個性乱数は切り離して考えて良いので、再抽選処理は不要
    personal_rnd=(GetRand()|GetRand()<<16);
#if 0
    do{
//    personal_rnd=(gf_rand()|gf_rand()<<16);
      personal_rnd=(GetRand()|GetRand()<<16);
#if 0
    }while((prd_s.chr!=get_PokeSeikaku(personal_rnd))&&(PokeRareGetPara(poke_id,personal_rnd)==TRUE));
#else
    //プラチナはタワーも修正する(08.03.17)(似た処理がfrontier_tool.cにもあるので注意！)
    //データの性格と一致していない"もしくは"レアの時は、ループを回す
    }while((prd_s.chr!=get_PokeSeikaku(personal_rnd))||(
          POKETOOL_CheckRare(poke_id,personal_rnd)==TRUE));
#endif
#endif  
    //OS_Printf( "決定したpersonal_rnd = %d\n", personal_rnd );
    //OS_Printf( "get_PokeSeikaku = %d\n",get_PokeSeikaku(personal_rnd) );
    //OS_Printf( "レアじゃないか = %d\n", PokeRareGetPara(poke_id,personal_rnd) );
    pwd->personal_rnd = personal_rnd;
  }else{
    pwd->personal_rnd = poke_rnd;  //0でなければ引数の値を使用
    personal_rnd = poke_rnd;
  }


  //パワー乱数
  pwd->hp_rnd=pow_rnd;
  pwd->pow_rnd=pow_rnd;
  pwd->def_rnd=pow_rnd;
  pwd->agi_rnd=pow_rnd;
  pwd->spepow_rnd=pow_rnd;
  pwd->spedef_rnd=pow_rnd;

  //努力値
#if 0 //wb null
  exp=0;
  for(i=0;i<6;i++){
    if(prd_s.exp_bit&No2Bit(i)){
      exp++;
    }
  }
  if((PARA_EXP_TOTAL_MAX/exp)>255){
    exp=255;
  }else{
    exp=PARA_EXP_TOTAL_MAX/exp;
  }
  for(i = 0;i < 6;i++){
    if(prd_s.exp_bit&No2Bit(i)){
      pwd->exp[i]=exp;
    }
  }
#endif

  //技ポイント
  pwd->pp_count=0;

  //国コード
  pwd->country_code = 0;

  //特性
  i= POKETOOL_GetPersonalParam( pwd->mons_no,0,POKEPER_ID_speabi2);
  if(i){
    if(pwd->personal_rnd&1){
      pwd->tokusei=i;
    }else{
      pwd->tokusei=POKETOOL_GetPersonalParam(pwd->mons_no,0,POKEPER_ID_speabi1);
    }
  }else{
    pwd->tokusei=POKETOOL_GetPersonalParam( pwd->mons_no,0,POKEPER_ID_speabi1);
  }

  //なつき度
  pwd->natuki=friend;

  //ニックネーム
  {
   GFL_MSGDATA *msgdata = GFL_MSG_Create(
       GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monsname_dat,
       heapID );
    GFL_MSG_GetStringRaw( msgdata,
        pwd->mons_no, pwd->nickname, MONS_NAME_SIZE+EOM_SIZE );
    GFL_MSG_Delete( msgdata );
  }
  return personal_rnd;
}

//--------------------------------------------------------------
/**
 *  @brief バトルサブウェイ用ポケモンデータからPOKEMON_PARAMを生成
 *  @param src BSUBWAY_POKEMON
 *  @param dest POKEMON_PARAM
 *  @retval nothing
 */
//--------------------------------------------------------------
static void MakePokePara( const BSUBWAY_POKEMON *src, POKEMON_PARAM *dest )
{
  int i;
  u8 buf8,waza_pp;
  u16 buf16;
  u32 buf32;

  PP_Clear(dest);

  //monsno,level,pow_rnd,idno
  PP_SetupEx( dest, src->mons_no, 50, PTL_SETUP_ID_AUTO,
      (src->power_rnd & 0x3FFFFFFF), src->personal_rnd);

  //form_no
  buf8 = src->form_no;
  PP_Put( dest, ID_PARA_form_no, buf8 );

  //装備アイテム設定
  PP_Put( dest,ID_PARA_item, src->item_no );

  //技設定
  for(i = 0;i < 4;i++){
    buf16 = src->waza[i];
    PP_Put(dest,ID_PARA_waza1+i,buf16);
    buf8 = (src->pp_count >> (i*2))&0x03;
    PP_Put(dest,ID_PARA_pp_count1+i, buf8);

    //pp再設定
    waza_pp = (u8)PP_Get(dest,ID_PARA_pp_max1+i,NULL);
    PP_Put(dest,ID_PARA_pp1+i,waza_pp);
  }

  //ID設定
  buf32 = src->id_no;
  PP_Put(dest,ID_PARA_id_no,buf32);

  //経験値設定
  buf8 = src->hp_exp;
  PP_Put(dest,ID_PARA_hp_exp,buf8);
  buf8 = src->pow_exp;
  PP_Put(dest,ID_PARA_pow_exp,buf8);
  buf8 = src->def_exp;
  PP_Put(dest,ID_PARA_def_exp,buf8);
  buf8 = src->agi_exp;
  PP_Put(dest,ID_PARA_agi_exp,buf8);
  buf8 = src->spepow_exp;
  PP_Put(dest,ID_PARA_spepow_exp,buf8);
  buf8 = src->spedef_exp;
  PP_Put(dest,ID_PARA_spedef_exp,buf8);

  //特性設定
  PP_Put(dest,ID_PARA_speabino, src->tokusei);

  //なつきど設定
  PP_Put(dest,ID_PARA_friend,src->natuki);

  //NGネームフラグをチェック
  if( 1 ){ //デフォルトネームを展開する
#if 0
    GFL_MSGDATA *msgdata;
    STRBUF *def_name;

    msgdata= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
        ARCID_MESSAGE, NARC_message_monsname_dat, HEAPID_PROC );
    def_name = GFL_STR_CreateBuffer( HEAPID_PROC, MONS_NAME_SIZE+EOM_SIZE );

    GFL_MSG_GetString( msgdata, src->mons_no, def_name );
    PP_Put( dest, ID_PARA_nickname, (u32)def_name );

    GFL_STR_DeleteBuffer( def_name );
    GFL_MSG_Delete( msgdata );
#endif
  }else{ //ニックネーム
    STRBUF *nick_name;
    nick_name = GFL_STR_CreateBuffer(
        HEAPID_PROC, MONS_NAME_SIZE+EOM_SIZE );
    GFL_STR_SetStringCode( nick_name, src->nickname );
    PP_Put( dest, ID_PARA_nickname, (u32)nick_name );
    GFL_STR_DeleteBuffer( nick_name );
  }

  PP_Put(dest,ID_PARA_country_code,src->country_code); //カントリーコード

  //パラメータ再計算
  {
    u32 lv = PP_CalcLevel( dest );
    PP_Put( dest, ID_PARA_level,lv );
  }
}

//--------------------------------------------------------------
/**
 *  バトルサブウェイポケモンロムデータの読み出し
 * @param[in]  prd    読み出したポケモンデータの格納先
 * @param[in]  index  読み出すポケモンデータのインデックス
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY 似た処理 frontier_tool.c Frontier_PokemonRomDataGet
 */
//--------------------------------------------------------------
static void GetPokemonRomData(
    BSUBWAY_POKEMON_ROM_DATA *prd,int index)
{
  //ここは通信はありえないのでプラチナ限定！(AIマルチ)
  GFL_ARC_LoadData( (void*)prd, ARCID_PL_BTD_PM, index );
}

//--------------------------------------------------------------
/**
 *  バトルサブウェイトレーナーロムデータの読み出し
 * @param tr_no  [in]トレーナーナンバー
 * @param heapID  [in]メモリ確保するためのヒープID
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY 似た処理 frontier_tool.c Frontier_TrainerRomDataGet
 */
//--------------------------------------------------------------
static void * GetTrainerRomData( u16 tr_no, HEAPID heapID )
{
#ifdef DEBUG_ONLY_FOR_kagaya
  OS_Printf( "BSUBWAY load TrainerRomData Num = %d\n", tr_no );
#endif
  //AIマルチ限定なのでプラチナ！
  return GFL_ARC_UTIL_Load( ARCID_PL_BTD_TR, tr_no, 0, heapID );
}

//--------------------------------------------------------------
/**
 * 乱数取得
 * @retval u16 乱数
 * @note 乱数固定の名残
 */
//--------------------------------------------------------------
static u16 GetRand( void )
{
  return( GFUser_GetPublicRand(0xffffffff)/65535 );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイトレーナーの持ちポケモンのパワー乱数を決定する
 * @param  tr_no  トレーナーナンバー
 * @return  パワー乱数
 * b_tower_fld.c→b_tower_evに移動
 */
//--------------------------------------------------------------
static u8 GetPowerRnd(u16 tr_no)
{
  u8  pow_rnd;

  if(tr_no<100){
    pow_rnd=(0x1f/8)*1;
  }
  else if(tr_no<120){
    pow_rnd=(0x1f/8)*2;
  }
  else if(tr_no<140){
    pow_rnd=(0x1f/8)*3;
  }
  else if(tr_no<160){
    pow_rnd=(0x1f/8)*4;
  }
  else if(tr_no<180){
    pow_rnd=(0x1f/8)*5;
  }
  else if(tr_no<200){
    pow_rnd=(0x1f/8)*6;
  }
  else if(tr_no<220){
    pow_rnd=(0x1f/8)*7;
  }
  else{
    pow_rnd=0x1f;
  }
  return pow_rnd;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイトレーナーデータ生成
 * （ロムデータをBSUBWAY_PARTNER_DATA構造体に展開）
 * @param tr_data  [in/out]生成するBSUBWAY_PARTNAER_DATA構造体
 * @param tr_no [in]生成元になるトレーナーID
 * @param cnt [in]トレーナーに持たせるポケモンの数
 * @param set_poke_no [in]ペアトレーナーのポケモン（NULL チェックなし）
 * @param set_item_no [in]ペアのポケモンの装備アイテム（NULL チェックなし）
 * @param poke [in/out] 抽選ポケモン二体のパラメータ格納先(NULLだとなし）
 * @param heapID  [in]ヒープID
 * @retval TRUE 抽選ループが50回超えた FALSE 抽選ループが50回以内で終わった
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY 似た処理tower_tool.c FSRomBattleTowerTrainerDataMake
 * TOWER_AI_MULTI_ONLY サロンで、5人衆の手持ちポケモンを生成している
 */
//--------------------------------------------------------------
BOOL FBI_TOOL_MakeRomTrainerData(
    BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no, int cnt,
    u16 *set_poke_no, u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM* poke, HEAPID inHeapID)
{
  BOOL      ret = 0;
  BSUBWAY_TRAINER_ROM_DATA  *trd;

  //トレーナーデータセット
  trd = AllocTrainerRomData(tr_data,tr_no, inHeapID);

  //ポケモンデータをセット
  ret = SetBSWayPokemonParam(
      trd, tr_no, &tr_data->btpwd[0],cnt,
      set_poke_no, set_item_no, poke, inHeapID);
  GFL_HEAP_FreeMemory( trd );
  return ret;
}

//--------------------------------------------------------------
/**
 *  バトルサブウェイ　romトレーナーデータ展開
 *  BSUBWAY_TRAINER_ROM_DATA型をメモリ確保して返すので、
 *  呼び出し側が明示的に解放すること
 *  @param tr_data BSUBWAY_PARTER_DATA
 *  @param tr_no トレーナーナンバー
 *  @param heapID HEAPID
 *  @retval BSUBWAY_TRAINER_ROM_DATA*
 *  TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 *  TOWER_AI_MULTI_ONLY 似た処理frontier_tool.c Frontier_TrainerDataGet
 */
//--------------------------------------------------------------
static BSUBWAY_TRAINER_ROM_DATA * AllocTrainerRomData(
    BSUBWAY_PARTNER_DATA *tr_data, u16 tr_no, HEAPID inHeapID )
{
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  GFL_MSGDATA *msgdata;
  STRBUF *name;

  msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_btdtrname_dat, inHeapID );
  
  MI_CpuClear8(tr_data, sizeof(BSUBWAY_PARTNER_DATA));
  trd = GetTrainerRomData(tr_no, inHeapID);

  //トレーナーIDをセット
  tr_data->bt_trd.player_id=tr_no;

  //トレーナー出現メッセージ
  tr_data->bt_trd.appear_word[0] = 0xFFFF;
  tr_data->bt_trd.appear_word[1] = tr_no*3;
  
  //トレーナーデータをセット
  tr_data->bt_trd.tr_type=trd->tr_type;

#ifdef PM_DEBUG  
#if 1 
  //GSデータからの移植による処理
  //wbでは存在していないタイプを書き換え
  #if 1
  if( check_TrainerType(trd->tr_type) == FALSE ){
    tr_data->bt_trd.tr_type = TRTYPE_TANPAN;
  }
  #endif
#endif        //@todo 果たしてＷＢで必要な処理なのか？  
#endif  //PM_DEBUG  
  name = GFL_MSG_CreateString( msgdata, tr_no );
  GFL_STR_GetStringCode( name,
      &tr_data->bt_trd.name[0], BUFLEN_PERSON_NAME );
  GFL_STR_DeleteBuffer(name);
  GFL_MSG_Delete( msgdata );
  return trd;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイのポケモンを決める
 * @param trd [in]トレーナーデータ
 * @param tr_no [in]トレーナーナンバー
 * @param pwd [in/out]BSUBWAY_POKEMON構造体
 * @param cnt [in]トレーナーに持たせるポケモンの数
 * @param set_poke_no [in]ペアトレーナーポケモン (NULLだとチェックなし
 * @param set_item_no [in]ペアトレーナーポケモンアイテム (NULL チェックなし
 * @param poke [in/out]抽選ポケモン二体のパラメータ格納先(NULL チェックなし
 * @param heapID [in]ヒープID
 * @retval  FALSE  抽選ループが50回以内で終わった
 * @retval  TRUE  抽選ループが50回以内で終わらなかった
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 */
//--------------------------------------------------------------
static BOOL SetBSWayPokemonParam(
    BSUBWAY_TRAINER_ROM_DATA *trd,
    u16 tr_no, BSUBWAY_POKEMON *pwd, u8 cnt,
    u16 *set_poke_no, u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM *poke, HEAPID heapID )
{
  int  i,j;
  u8  pow_rnd;
  u8  poke_index;
  u32  id;
  int  set_index;
  int  set_index_no[4];
  u32  set_rnd_no[4];
  int  set_count;
  int  loop_count;
  BOOL  ret = 0;
  BSUBWAY_POKEMON_ROM_DATA  prd_s;
  BSUBWAY_POKEMON_ROM_DATA  prd_d;

  //手持ちポケモンのMAXは4体まで
  GF_ASSERT(cnt<=4);

  set_count=0;
  loop_count=0;
  while(set_count!=cnt){
//    poke_index = gf_rand()%trd->use_poke_cnt;
    poke_index = GetRand()%trd->use_poke_cnt;
    set_index=trd->use_poke_table[poke_index];
    GetPokemonRomData(&prd_d,set_index);

    //モンスターナンバーのチェック（同一のポケモンは持たない）
    for(i=0;i<set_count;i++){
      GetPokemonRomData(&prd_s,set_index_no[i]);
      if(prd_s.mons_no==prd_d.mons_no){
        break;
      }
    }
    if(i!=set_count){
      continue;
    }

    //ペアを組んでいるトレーナーの持ちポケモンとのチェック
    if(set_poke_no!=NULL){
      //モンスターナンバーのチェック（同一のポケモンは持たない）
      for(i=0;i<cnt;i++){
        if(set_poke_no[i]==prd_d.mons_no){
          break;
        }
      }
      if(i!=cnt){
        continue;
      }
    }

    //50回まわして、決まらないようなら、同一アイテムチェックはしない
    if(loop_count<50){
      //装備アイテムのチェック（同一のアイテムは持たない）
      for(i=0;i<set_count;i++){
        GetPokemonRomData(&prd_s,set_index_no[i]);
        if((prd_s.item_no)&&(prd_s.item_no==prd_d.item_no)){
          break;
        }
      }
      if(i!=set_count){
        loop_count++;
        continue;
      }
      //ペアを組んでいるトレーナーの持ちポケモンの装備アイテムとのチェック
      if(set_item_no!=NULL){
        //装備アイテムのチェック（同一のアイテムは持たない）
        for(i=0;i<cnt;i++){
          if((set_item_no[i]==prd_d.item_no) && (set_item_no[i]!=0)){
            break;
          }
        }
        if(i!=cnt){
          loop_count++;
          continue;
        }
      }
    }

    set_index_no[set_count]=set_index;
    set_count++;
  }

  pow_rnd=GetPowerRnd(tr_no);
//id=(gf_rand()|(gf_rand()<<16));
  id=(GetRand()|(GetRand()<<16));

  if(loop_count >= 50){
    ret = TRUE;
  }
  for(i=0;i<set_count;i++){
    set_rnd_no[i] = MakePokemonParam(&(pwd[i]),
      set_index_no[i],id,0,pow_rnd,i,ret,heapID);
  }
  if(poke == NULL){
    return ret;
  }
  //ポインタがNULLでなければ、抽選されたポケモンの必要なパラメータを返す
  poke->poke_id = id;
  for(i = 0;i< BSUBWAY_STOCK_PAREPOKE_MAX;i++){
    poke->poke_no[i] = set_index_no[i];
    poke->poke_rnd[i] = set_rnd_no[i];
  }
  return ret;
}



//===ＯＢＪコード取得===
static const u16 btower_trtype2objcode[][2] =
{
 {TRTYPE_TANPAN,  BOY2},  ///<たんパンこぞう
 {TRTYPE_MINI,  GIRL1},  ///<ミニスカート
 {TRTYPE_SCHOOLB,  BOY1},  ///<じゅくがえり
// {TRTYPE_PL_SCHOOLG,  GIRL3},  ///<じゅくがえり
// {TRTYPE_PL_PRINCE,  GORGGEOUSM},  ///<おぼっちゃま
// {TRTYPE_PL_PRINCESS,  GORGGEOUSW},  ///<おじょうさま
// {TRTYPE_CAMPB,  CAMPBOY},  ///<キャンプボーイ
// {TRTYPE_PICNICG,  PICNICGIRL},  ///<ピクニックガール
// {TRTYPE_PL_UKIWAB,  BABYBOY1},  ///<うきわボーイ
// {TRTYPE_PL_UKIWAG,  BABYGIRL1},  ///<うきわガール
 {TRTYPE_DAISUKIM,  MIDDLEMAN1},  ///<だいすきクラブ
 {TRTYPE_DAISUKIW,  MIDDLEWOMAN1},  ///<だいすきクラブ
// {TRTYPE_PL_WAITER,  WAITER},  ///<ウエーター
// {TRTYPE_PL_WAITRESS,  WAITRESS},  ///<ウエートレス
// {TRTYPE_PL_BREEDERM,  MAN1},  ///<ポケモンブリーダー
// {TRTYPE_PL_BREEDERW,  WOMAN1},  ///<ポケモンブリーダー
// {TRTYPE_PL_CAMERAMAN,  CAMERAMAN},  ///<カメラマン
// {TRTYPE_PL_REPORTER,  REPORTER},  ///<レポーター
// {TRTYPE_PL_FARMER,  FARMER},  ///<ぼくじょうおじさん
// {TRTYPE_PL_COWGIRL,  COWGIRL},  ///<カウガール
// {TRTYPE_PL_CYCLINGM,  CYCLEM},  ///<サイクリング♂
// {TRTYPE_PL_CYCLINGW,  CYCLEW},  ///<サイクリング♀
 {TRTYPE_KARATE,  FIGHTERM},  ///<からておう
// {TRTYPE_PL_BATTLEG,  GIRL2},  ///<バトルガール
// {TRTYPE_PL_VETERAN,  OLDMAN1},  ///<ベテラントレーナー
// {TRTYPE_PL_MADAM,  LADY},  ///<マダム
 {TRTYPE_ESPM,  MAN1},  ///<サイキッカー
// {TRTYPE_PL_ESPW,  MYSTERY},  ///<サイキッカー
// {TRTYPE_PL_RANGERM,  MAN3},  ///<ポケモンレンジャー
// {TRTYPE_PL_RANGERW,  WOMAN3},  ///<ポケモンレンジャー
// {TRTYPE_ELITEM,  MAN3},  ///<エリートトレーナー
// {TRTYPE_ELITEW,  WOMAN3},  ///<エリートトレーナー
// {TRTYPE_PL_COLDELITEM,  MAN5},  ///<エリートトレーナー♂（厚着）
// {TRTYPE_PL_COLDELITEW,  WOMAN5},  ///<エリートトレーナー♀（厚着）
// {TRTYPE_PL_DRAGON,  MAN3},  ///<ドラゴンつかい
// {TRTYPE_MUSHI,  BOY3},  ///<むしとりしょうねん
// {TRTYPE_PL_SHINOBI,  BABYBOY1},  ///<にんじゃごっこ
// {TRTYPE_PL_JOGGER,  SPORTSMAN},  ///<ジョギング♂
 {TRTYPE_FISHING,  FISHING},  ///<つりびと
// {TRTYPE_SAILOR,  SEAMAN},  ///<ふなのり
 {TRTYPE_MOUNT,  MAN1},  ///<やまおとこ
// {TRTYPE_PL_ISEKI,  EXPLORE},  ///<いせきマニア
// {TRTYPE_GUITARIST,  MAN2},  ///<ギタリスト
// {TRTYPE_PL_COLLECTOR,  BIGMAN},  ///<ポケモンコレクター
// {TRTYPE_PL_HEADS,  BADMAN},  ///<スキンヘッズ
// {TRTYPE_SCIENTIST,  ASSISTANTM},  ///<けんきゅういん♂
 {TRTYPE_GENTLE,  GENTLEMAN},  ///<ジェントルマン
// {TRTYPE_PL_WORKER,  WORKMAN},  ///<さぎょういん
// {TRTYPE_PL_PIERROT,  CLOWN},  ///<ピエロ
// {TRTYPE_POLICE,  POLICEMAN},  ///<おまわりさん
// {TRTYPE_PL_GAMBLER,  GORGGEOUSM},  ///<ギャンブラー
// {TRTYPE_BIRD,  WOMAN3},  ///<とりつかい
// {TRTYPE_PL_PARASOL,  AMBRELLA},  ///<パラソルおねえさん
// {TRTYPE_SISTER,  WOMAN2},  ///<おとなのおねえさん
// {TRTYPE_PL_AROMA,  WOMAN1},  ///<アロマなおねえさん
// {TRTYPE_PL_IDOL,  IDOL},  ///<アイドル
// {TRTYPE_PL_ARTIST,  ARTIST},  ///<げいじゅつか
// {TRTYPE_PL_POKEGIRL,  PIKACHU},  ///<ポケモンごっこ♀
};

#define TRTYPE2OBJCODE_MAX  (NELEMS(btower_trtype2objcode))

#define TRTYPE2OBJCODE_MAX  (NELEMS(btower_trtype2objcode))

//--------------------------------------------------------------
/**
 * @brief  トレーナータイプから人物OBJコードを返す
 * @param tr_type トレーナータイプ
 * @retval u16 OBJコード
 */
//--------------------------------------------------------------
u16 FBI_TOOL_GetTrainerOBJCode( u32 inTrType )
{
  int i;

  for( i = 0; i < TRTYPE2OBJCODE_MAX; i++ ){
    if( btower_trtype2objcode[i][0] == inTrType ){
      return btower_trtype2objcode[i][1];
    }
  }
  return BOY1;
}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * @brief  トレーナータイプが対応しているか
 * @param tr_type トレーナータイプ
 * @retval BOOL TRUE=対応
 */
//--------------------------------------------------------------
static BOOL check_TrainerType(u8 tr_type)
{
  int i;

  for( i = 0;i < TRTYPE2OBJCODE_MAX; i++ ){
    if( btower_trtype2objcode[i][0] == tr_type ){
      return( TRUE );
    }
  }
  return( FALSE );
}

#endif  //PM_DEBUG

