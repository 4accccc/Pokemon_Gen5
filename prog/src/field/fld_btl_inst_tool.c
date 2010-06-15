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

#if 0
#ifdef PM_DEBUG
static BOOL check_TrainerType(u8 tr_type);
#endif  //PM_DEBUG
#endif

#ifdef PM_DEBUG
//定義で独自セットアップのメモリ開放忘れチェック
#define DEBUG_FBI_CHECK_BTLPRM_HEAPSIZE
#endif

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
    st->game_data = gdata;
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
  VecFx32 balloonWinPos;
  FLDTALKMSGWIN *balloonWin;
//  FLDSYSWIN_STREAM *sysWin;
  STRBUF *strBuf;
  u16 obj_id;
  u8 padding[2];
}EVENT_WORK_TRAINER_BEFORE_MSG;

static GMEVENT_RESULT TrainerBeforeMsgEvt( GMEVENT *event, int *seq, void *wk );


//--------------------------------------------------------------
/**
 * 対戦前メッセージ 呼び出し
 * @param
 * @retval
 */
//--------------------------------------------------------------
GMEVENT * FBI_TOOL_CreateTrainerBeforeMsgEvt(
    GAMESYS_WORK *gsys, BSUBWAY_PARTNER_DATA *tr_data, int tr_idx, u16 obj_id )
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
  
  if( tr_data->bt_trd.appear_word[0] == 0xffff ){ //ROM MSG
    GFL_MSGDATA *msgdata;
    
    work->strBuf = GFL_STR_CreateBuffer( 92, HEAPID_PROC );

    msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_tower_trainer_dat, HEAPID_PROC );
    
    GFL_MSG_GetString(
        msgdata, 
        tr_data->bt_trd.appear_word[1],
        work->strBuf );
    
    GFL_MSG_Delete( msgdata );
  }
  else
  { //簡易会話    
    PMS_DATA *pms = (PMS_DATA*)tr_data[tr_idx].bt_trd.appear_word;
    work->strBuf = PMSDAT_ToString( pms, HEAPID_PROC );
    OS_Printf( "BSW TRAINER BEFORE MSG : IDX = %d, KAIWA MSG\n", tr_idx );
/**    
    GFL_MSGDATA *msgdata;
    
    msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_tower_trainer_dat, HEAPID_PROC );
    
    GFL_MSG_GetString(
        msgdata,
        0,
        work->strBuf );
    
    GFL_MSG_Delete( msgdata );
*/    
  }

  { //mmdl
    MMDLSYS *mmdlsys = GAMEDATA_GetMMdlSys( gdata );
    MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
    MMDL_GetVectorPos( mmdl, &work->balloonWinPos );
  }
  
  { //吹き出しウィンドウ
     work->balloonWin = FLDTALKMSGWIN_AddStrBuf(
        msgbg,
        FLDTALKMSGWIN_IDX_LOWER,
        &work->balloonWinPos,
        work->strBuf, TALKMSGWIN_TYPE_NORMAL, TAIL_SETPAT_NONE );
  }
  
//  work->sysWin = FLDSYSWIN_STREAM_Add( msgbg, NULL, 19 );
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
/**  
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
*/
  switch( *seq )
  {
  case 0:
    if( FLDTALKMSGWIN_Print(work->balloonWin) == TRUE ){
      (*seq)++;
    }
    break;
  case 1:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
      FLDTALKMSGWIN_StartClose( work->balloonWin );
      (*seq)++;
    }
    break;
  case 2:
    if( FLDTALKMSGWIN_WaitClose(work->balloonWin) == TRUE ){
      GFL_STR_DeleteBuffer( work->strBuf );
      return( GMEVENT_RES_FINISH );
    }
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
static void GetPokemonRomData( BSUBWAY_POKEMON_ROM_DATA *prd, int index );
static void * GetTrainerRomData( u16 tr_no, HEAPID heapID );

static BOOL IsValidEasyTalk(BSUBWAY_TRAINER *bsw_trainer);

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
//  デバッグ用文字列長チェック
//--------------------------------------------------------------
#ifdef PM_DEBUG
static int d_strcode_len( const STRCODE *sz )
{
  int len = 0;
  STRCODE eom = GFL_STR_GetEOMCode();
  
  while( *sz != eom ){
    sz++;
    len++;
  }
  
  return( len );
}
#endif

//--------------------------------------------------------------
/**
 * BATTLE_SETUP_PARAM作成
 * @param gsys GAMESYS_WORK
 * @retval BATTLE_SETUP_PARAM
 */
//--------------------------------------------------------------
BATTLE_SETUP_PARAM * FBI_TOOL_CreateBattleParam(
    GAMESYS_WORK *gsys, const POKEPARTY *my_party, int inPlayMode,
    BSUBWAY_PARTNER_DATA *partner_data,
    BSUBWAY_PARTNER_DATA *ai_multi_data,
    const int inMemNum )
{
  u16 play_mode;
  BATTLE_SETUP_PARAM *dst;
  BTL_FIELD_SITUATION sit;
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  HEAPID heapID = HEAPID_PROC;
  play_mode = inPlayMode;
  
#ifdef DEBUG_FBI_CHECK_BTLPRM_HEAPSIZE
  { //独自セットアップのメモリ開放忘れチェック
    u32 size_before,size_after;
    
    GFL_HEAP_DEBUG_PrintExistMemoryBlocks( heapID );
    
    BTL_FIELD_SITUATION_SetFromFieldStatus(
        &sit, gdata, GAMESYSTEM_GetFieldMapWork(gsys) );
    
    size_before = GFL_HEAP_GetHeapFreeSize( heapID );
    dst = BATTLE_PARAM_Create( heapID );
    BTL_SETUP_Single_Trainer( dst, gdata, &sit, TRID_NULL, heapID );
    BATTLE_PARAM_Delete( dst );
    size_after = GFL_HEAP_GetHeapFreeSize( heapID );
    
    OS_Printf(
        "BTL_SETUP_Single_Trainer() before=%xH, after=%xH\n",
        size_before, size_after );
    
    if( size_before != size_after ){
      GF_ASSERT( 0 );
    }
    
    size_before = GFL_HEAP_GetHeapFreeSize( heapID );
    dst = BATTLE_PARAM_Create( heapID );
    BTL_SETUP_Double_Trainer( dst, gdata, &sit, TRID_NULL, heapID );
    BATTLE_PARAM_Delete( dst );
    size_after = GFL_HEAP_GetHeapFreeSize( heapID );
    
    OS_Printf(
        "BTL_SETUP_Double_Trainer() before=%xH, after=%xH\n",
        size_before, size_after );
    
    if( size_before != size_after ){
      GF_ASSERT( 0 );
    }
    
    size_before = GFL_HEAP_GetHeapFreeSize( heapID );
    dst = BATTLE_PARAM_Create( heapID );
    BTL_SETUP_AIMulti_Trainer( dst, gdata, &sit,
          TRID_NULL, TRID_NULL, TRID_NULL, heapID );
    BATTLE_PARAM_Delete( dst );
    size_after = GFL_HEAP_GetHeapFreeSize( heapID );
    
    OS_Printf(
        "BTL_SETUP_AIMulti_Trainer() before=%xH, after=%xH\n",
        size_before, size_after );
    
    if( size_before != size_after ){
      GF_ASSERT( 0 );
    }

    {
      int commPos = 0;
      GFL_NETHANDLE *netHandle = GFL_NET_HANDLE_GetCurrentHandle();
      
      size_before = GFL_HEAP_GetHeapFreeSize( heapID );
      dst = BATTLE_PARAM_Create( heapID );
      BTL_SETUP_AIMulti_Comm( dst, gdata,
            netHandle, BTL_COMM_DS, commPos,
            TRID_NULL, TRID_NULL, heapID );
      BATTLE_PARAM_Delete( dst );
      size_after = GFL_HEAP_GetHeapFreeSize( heapID );
      
      OS_Printf(
          "BTL_SETUP_AIMulti_Comm() before=%xH, after=%xH\n",
          size_before, size_after );
    
      if( size_before != size_after ){
        GF_ASSERT( 0 );
      }
    }
  }
#endif
  
  {
//    BTL_FIELD_SITUATION_Init( &sit );
    BTL_FIELD_SITUATION_SetFromFieldStatus(
        &sit, gdata, GAMESYSTEM_GetFieldMapWork(gsys) );
    dst = BATTLE_PARAM_Create( heapID );

    //初期化
    switch( play_mode ){
    case BSWAY_MODE_SINGLE:
    case BSWAY_MODE_S_SINGLE:
    case BSWAY_MODE_WIFI:
      BTL_SETUP_Single_Trainer( dst, gdata, &sit, TRID_NULL, heapID );
      break;
    case BSWAY_MODE_DOUBLE:
    case BSWAY_MODE_S_DOUBLE:
      BTL_SETUP_Double_Trainer( dst, gdata, &sit, TRID_NULL, heapID );
      break;
    case BSWAY_MODE_MULTI:
    case BSWAY_MODE_S_MULTI:
      BTL_SETUP_AIMulti_Trainer( dst, gdata, &sit,
          TRID_NULL, TRID_NULL, TRID_NULL, heapID );
      break;
    case BSWAY_MODE_COMM_MULTI:
    case BSWAY_MODE_S_COMM_MULTI:
      {
        GFL_NETHANDLE *netHandle = GFL_NET_HANDLE_GetCurrentHandle();
        int commPos = 0;
      
        if( GFL_NET_SystemGetCurrentID() != GFL_NET_NO_PARENTMACHINE ){
          commPos = 2; //通信子である
        }
      
        BTL_SETUP_AIMulti_Comm( dst, gdata,
            netHandle, BTL_COMM_DS, commPos,
            TRID_NULL, TRID_NULL, heapID );
      }
      break;
    default:
      GF_ASSERT( 0 );
    }

    BTL_SETUP_AllocRecBuffer( dst, heapID );

  }
  
#if 0  
  { //トレーナーデータ確保
    dst->tr_data[BTL_CLIENT_ENEMY1] = CreateBSPTrainerData( heapID );
  }
#endif
  
  { //プレイヤーセット
    BTL_CLIENT_ID client = BTL_CLIENT_PLAYER;
    MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gdata );
    BSP_TRAINER_DATA *data;

    //MyStatus
    dst->playerStatus[client] = mystatus;
    
    //トレーナーデータ確保
    #if 0 //自身のデータ自体はplayerStatusから
          //引っ張ってくる為不要であった。
    dst->tr_data[client] = CreateBSPTrainerData( heapID );
    data = dst->tr_data[client];
    
    MyStatus_CopyNameString(
        (const MYSTATUS*)mystatus, data->name );
    data->tr_type = TRTYPE_HERO +
      MyStatus_GetMySex((const MYSTATUS*)mystatus );
    #endif
    
    //ポケモンセット
    {
      int i;
      POKEMON_PARAM *entry_pp;
      const POKEMON_PARAM *my_pp;
      POKEPARTY *party = dst->party[client];
      PokeParty_Init( party, TEMOTI_POKEMAX );
      
      entry_pp = GFL_HEAP_AllocMemoryLo(
          heapID, POKETOOL_GetWorkSize() );
      PP_Clear( entry_pp );

      for( i = 0; i < inMemNum; i++ ){
        my_pp = PokeParty_GetMemberPointer( my_party,i );

        POKETOOL_CopyPPtoPP( (POKEMON_PARAM*)my_pp, entry_pp );

        //レベル５０補正
        if( PP_Get(my_pp,ID_PARA_level,NULL) != 50 )
        {
          POKETOOL_MakeLevelRevise(entry_pp, 50);
        }

        PokeParty_Add( party, entry_pp );
      }
      GFL_HEAP_FreeMemory( entry_pp );
    }
  }
  
  { //敵トレーナーセット
    PMS_DATA *pd;
    BSP_TRAINER_DATA *tr_data;
    BTL_CLIENT_ID client = BTL_CLIENT_ENEMY1;
    BSUBWAY_PARTNER_DATA *bsw_partner;
    BSUBWAY_TRAINER *bsw_trainer;

    //トレーナーデータ確保
//バトルセットアップで生成されるようになったので不要となった
//    dst->tr_data[client] = CreateBSPTrainerData( heapID );
    tr_data = dst->tr_data[client];

    bsw_partner = &partner_data[0];
    bsw_trainer = &bsw_partner->bt_trd;

#if 0 //トレーナーID0を設定すると戦闘録画側に異常
    if ( play_mode == BSWAY_MODE_WIFI ) tr_data->tr_id = 0;
    else tr_data->tr_id = bsw_trainer->player_id;
#else //wifiデータ側で専用のトレーナーIDを設定する
    tr_data->tr_id = bsw_trainer->player_id;
#endif

    tr_data->tr_type = bsw_trainer->tr_type;
    tr_data->ai_bit = 0x00000087;  //最強

    //name
    GFL_STR_SetStringCode( tr_data->name, bsw_trainer->name );
#ifdef PM_DEBUG
    KAGAYA_Printf( "BSW TR ENEMY1 NAME LEN %d(%d)\n",
          GFL_STR_GetBufferLength(tr_data->name),
          d_strcode_len(bsw_trainer->name) );
#endif

    //win word
    PMSDAT_Clear( &tr_data->win_word );
    PMSDAT_Clear( &tr_data->lose_word );
    //サブウェイパートナーデータの勝ち負けデータが０クリアされているならば、
    //トレーナーデータのワークもクリアして渡す
    //０クリアされてなければ、簡易会話が入っているとみなし、データをセットする
    if( play_mode == BSWAY_MODE_WIFI ){
      tr_data->win_word = *(PMS_DATA*)bsw_trainer->win_word;
      tr_data->lose_word = *(PMS_DATA*)bsw_trainer->lose_word;
    }
    else if ( IsValidEasyTalk(bsw_trainer) )   //簡易会話ある
    {
      tr_data->win_word = *(PMS_DATA*)bsw_trainer->win_word;
      tr_data->lose_word = *(PMS_DATA*)bsw_trainer->lose_word;
    }

    //敵ポケモンセット
    {
      int i;
      POKEMON_PARAM*  pp;
      POKEPARTY *party = dst->party[client];
      PokeParty_Init( party, TEMOTI_POKEMAX );
      pp = GFL_HEAP_AllocMemoryLo( heapID, POKETOOL_GetWorkSize() );
      
      for( i = 0; i < inMemNum; i++ ){
        MakePokePara( &(bsw_partner->btpwd[i]), pp );
        PokeParty_Add( party, pp );
      }
      GFL_HEAP_FreeMemory( pp );
    }

    if( dst->multiMode != BTL_MULTIMODE_NONE ) //マルチ
    { //敵トレーナー２設定
      client = BTL_CLIENT_ENEMY2;
//バトルセットアップで生成されるようになったので不要となった
//    dst->tr_data[client] = CreateBSPTrainerData( heapID );
      tr_data = dst->tr_data[client];
      bsw_partner = &partner_data[1];
      bsw_trainer = &bsw_partner->bt_trd;
    
      tr_data->tr_id = bsw_trainer->player_id;
      tr_data->tr_type = bsw_trainer->tr_type;
      tr_data->ai_bit = 0x00000087;  //最強
      
      //トレーナーデータ　name
      GFL_STR_SetStringCode( tr_data->name, bsw_trainer->name );
#ifdef PM_DEBUG
      KAGAYA_Printf( "BSW TR ENEMY2 NAME LEN %d(%d)\n",
          GFL_STR_GetBufferLength(tr_data->name),
          d_strcode_len(bsw_trainer->name) );
#endif
      //トレーナーデータ　word
      //特に設定無し
    
      //ポケモンパーティ
      {
        int i;
        POKEMON_PARAM*  pp;
        POKEPARTY *party = dst->party[client];
        PokeParty_Init( party, TEMOTI_POKEMAX );
        pp = GFL_HEAP_AllocMemoryLo( heapID, POKETOOL_GetWorkSize() );
    
        for( i = 0; i < inMemNum; i++ ){
          MakePokePara( &(bsw_partner->btpwd[i]), pp );
          PokeParty_Add( party, pp );
        }
        GFL_HEAP_FreeMemory( pp );
      }
    }

    if( dst->multiMode == BTL_MULTIMODE_PA_AA ) //AIマルチ
    { //AIパートナー設定
      client = BTL_CLIENT_PARTNER;
      
      //トレーナーデータ
//バトルセットアップで生成されるようになったので不要となった
//      dst->tr_data[client] = CreateBSPTrainerData( heapID );
      tr_data = dst->tr_data[client];
      
      bsw_partner = ai_multi_data;
      bsw_trainer = &bsw_partner->bt_trd;

      tr_data->tr_id = bsw_trainer->player_id;
      tr_data->tr_type = bsw_trainer->tr_type;
      tr_data->ai_bit = 0x00000087;  //最強
      
      KAGAYA_Printf(
          "セットされたAIパートナーのトレーナータイプ=%d\n",
          tr_data->tr_type );

      //トレーナーデータ　name
      GFL_STR_SetStringCode( tr_data->name, bsw_trainer->name );
#ifdef PM_DEBUG
      KAGAYA_Printf( "BSW TR PARTNER NAME LEN %d(%d)\n",
          GFL_STR_GetBufferLength(tr_data->name),
          d_strcode_len(bsw_trainer->name) );
#endif
      //トレーナーデータ　word
      PMSDAT_Clear( &tr_data->win_word );
      PMSDAT_Clear( &tr_data->lose_word );
    
      //ポケモンパーティ
      {
        int i;
        POKEMON_PARAM*  pp;
        POKEPARTY *party = dst->party[client];
        PokeParty_Init( party, TEMOTI_POKEMAX );
        pp = GFL_HEAP_AllocMemoryLo( HEAPID_PROC, POKETOOL_GetWorkSize() );
    
        for( i = 0; i < inMemNum; i++ ){
          MakePokePara( &(bsw_partner->btpwd[i]), pp );
          PokeParty_Add( party, pp );
        }
        GFL_HEAP_FreeMemory( pp );
      }
    }
  }

  BTL_SETUP_SetSubwayMode( dst ); //一通りセットした後に呼ぶ事
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
u32 FBI_TOOL_MakePokemonParam(
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
    personal_rnd = POKETOOL_CalcPersonalRandSpec( poke_id, prd_s.mons_no, prd_s.form_no,
        PTL_SEX_SPEC_UNKNOWN, PTL_TOKUSEI_SPEC_BOTH, PTL_RARE_SPEC_FALSE );
    pwd->personal_rnd = personal_rnd;
  }else{
    pwd->personal_rnd = poke_rnd;  //0でなければ引数の値を使用
    personal_rnd = poke_rnd;
  }

  //性格セット
  pwd->seikaku=prd_s.chr;

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

  //性格セット
  PP_SetSeikaku( dest, src->seikaku );
  
  { //名前設定
    STRBUF *nick_name;
    nick_name = GFL_STR_CreateBuffer(
        MONS_NAME_SIZE+EOM_SIZE, HEAPID_PROC );
    {
      int i;
      u16 eomCode = GFL_STR_GetEOMCode();
      STRCODE code[MONS_NAME_SIZE+EOM_SIZE];
      const STRCODE *sz = src->nickname;
      
      for( i = 0; i < (MONS_NAME_SIZE+EOM_SIZE); i++ ){
        code[i] = sz[i];
      }
      
      code[i-1] = eomCode;
      GFL_STR_SetStringCode( nick_name, code );
    }
     
    PP_Put( dest, ID_PARA_nickname, (u32)nick_name );
    GFL_STR_DeleteBuffer( nick_name );
  }

#if 0  
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
#endif
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
//  index += BSW_PM_ARCDATANO_ORG;
  GFL_ARC_LoadData( (void*)prd, ARCID_BSW_PD, index );
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
  tr_no += BSW_TR_ARCDATANO_ORG;
  OS_Printf( "BSUBWAY load TrainerRomData Num = %d\n", tr_no );
  return GFL_ARC_UTIL_Load( ARCID_BSW_TR, tr_no, 0, heapID );
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
  trd = FBI_TOOL_AllocTrainerRomData(tr_data,tr_no, inHeapID);

  //ポケモンデータをセット
  ret = FBI_TOOL_SetBSWayPokemonParam(
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
 *  @param zero_org_tr_no トレーナーナンバー　ゼロオリジン
 *  @param heapID HEAPID
 *  @retval BSUBWAY_TRAINER_ROM_DATA*
 *  TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 *  TOWER_AI_MULTI_ONLY 似た処理frontier_tool.c Frontier_TrainerDataGet
 */
//--------------------------------------------------------------
BSUBWAY_TRAINER_ROM_DATA * FBI_TOOL_AllocTrainerRomData(
    BSUBWAY_PARTNER_DATA *tr_data, u16 zero_org_tr_no, HEAPID inHeapID )
{
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  GFL_MSGDATA *msgdata;
  STRBUF *name;

  msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_btdtrname_dat, inHeapID );
  
  MI_CpuClear8(tr_data, sizeof(BSUBWAY_PARTNER_DATA));
  trd = GetTrainerRomData(zero_org_tr_no, inHeapID);

  //トレーナーIDをセット
  tr_data->bt_trd.player_id = zero_org_tr_no + BSW_TR_ARCDATANO_ORG; //1 origin

  //トレーナー出現メッセージ
  tr_data->bt_trd.appear_word[0] = 0xFFFF;
  tr_data->bt_trd.appear_word[1] = zero_org_tr_no*3;
  
  //トレーナーデータをセット
  tr_data->bt_trd.tr_type=trd->tr_type;

  NOZOMU_Printf("trno=%d type=%d\n",tr_data->bt_trd.player_id, trd->tr_type);

#if 0  
#ifdef PM_DEBUG  
#if 1 
  //GSデータからの移植による処理
  //wbでは存在していないタイプを書き換え
  #if 1
  if( check_TrainerType(trd->tr_type) == FALSE ){
    GF_ASSERT_MSG(0,"unknown trainer trno=%d type=%d",tr_data->bt_trd.player_id, trd->tr_type);
    tr_data->bt_trd.tr_type = TRTYPE_TANPAN;
  }
  #endif
#endif
#endif  //PM_DEBUG
#endif  
  name = GFL_MSG_CreateString( msgdata, zero_org_tr_no );
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
BOOL FBI_TOOL_SetBSWayPokemonParam(
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
  u8   set_seikaku[4];
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
    set_seikaku[set_count] = prd_d.chr;
    set_count++;
  }

  pow_rnd=GetPowerRnd(tr_no);
  id=(GetRand()|(GetRand()<<16));

  if(loop_count >= 50){
    ret = TRUE;
  }
  for(i=0;i<set_count;i++){
    set_rnd_no[i] = FBI_TOOL_MakePokemonParam(&(pwd[i]),
      set_index_no[i],id,0,pow_rnd,i,ret,heapID);
  }
  if(poke == NULL){
    return ret;
  }
  //ポインタがNULLでなければ、抽選されたポケモンの必要なパラメータを返す
  poke->poke_tr_id = id;
  for(i = 0;i< BSUBWAY_STOCK_PAREPOKE_MAX;i++){
    poke->poke_no[i] = set_index_no[i];
    poke->poke_rnd[i] = set_rnd_no[i];
    poke->seikaku[i] = set_seikaku[i];
  }
  return ret;
}

//--------------------------------------------------------------
/**
 * 簡易会話存在するか？
 * @param bsw_trainer   バトルサブウェイトレーナーデータ
 * @retval  TRUE  ある　FFALSE ない
 */
//--------------------------------------------------------------
static BOOL IsValidEasyTalk(BSUBWAY_TRAINER *bsw_trainer)
{
  int i;
  for (i=0;i<4;i++)
  {
    if ( bsw_trainer->win_word[i] != 0 ) return TRUE;
    if ( bsw_trainer->lose_word[i] != 0 ) return TRUE;
  }

  return FALSE;
}



//===ＯＢＪコード取得===
static const u16 btower_trtype2objcode[][2] =
{
 {TRTYPE_TANPAN,  BOY2},  ///<たんパンこぞう
 {TRTYPE_MINI,  GIRL2},  ///<ミニスカート
 {TRTYPE_SCHOOLB,  BOY3},  ///<じゅくがえり
 {TRTYPE_SCHOOLG, GIRL3 },  ///<じゅくがえり
 {TRTYPE_PRINCE,  BOY4},  ///<おぼっちゃま
 {TRTYPE_PRINCESS,  GIRL4},  ///<おじょうさま
 {TRTYPE_KINDERGARTENM, BABYBOY2},    //えんじ♂
 {TRTYPE_KINDERGARTENW, BABYGIRL2},    //えんじ♀
 {TRTYPE_BACKPACKERM, BACKPACKERM},//バックパッカー♂
 {TRTYPE_BACKPACKERW, BACKPACKERW},//バックパッカー♂
 {TRTYPE_WAITER,WAITER}, //ウエーター
 {TRTYPE_WAITRESS,WAITRESS}, //ウエートレス
 {TRTYPE_DAISUKIM,  MIDDLEMAN1},  ///<だいすきクラブ
 {TRTYPE_DAISUKIW,  MIDDLEWOMAN1},  ///<だいすきクラブ
 {TRTYPE_DOCTOR, DOCTOR },   //ドクター
 {TRTYPE_NURSE,  NURSE},    //ナース
 {TRTYPE_CYCLINGM,  CYCLEM},  ///<サイクリング♂
 {TRTYPE_CYCLINGW,  CYCLEW},  ///<サイクリング♀
 {TRTYPE_GENTLE,  GENTLEMAN},  ///<ジェントルマン
 {TRTYPE_MADAM,  LADY},  ///<マダム
 {TRTYPE_BREEDERM,  BREEDERM},  ///<ポケモンブリーダー
 {TRTYPE_BREEDERW,  BREEDERW},  ///<ポケモンブリーダー
 {TRTYPE_SCIENTISTM,  ASSISTANTM},  //けんきゅういん♂
 {TRTYPE_SCIENTISTW,  ASSISTANTW},  //けんきゅういん♀
 {TRTYPE_ESPM,  ESPM},  ///<サイキッカー
 {TRTYPE_ESPW,  ESPW},  ///<サイキッカー
 {TRTYPE_KARATE,  FIGHTERM},  //からておう
 {TRTYPE_BATTLEG,  FIGHTERW},  ///<バトルガール
 {TRTYPE_RANGERM,  RANGERM},  ///<ポケモンレンジャー
 {TRTYPE_RANGERW,  RANGERW},  ///<ポケモンレンジャー
 {TRTYPE_ELITEM,  TRAINERM},  ///<エリートトレーナー
 {TRTYPE_ELITEW,  TRAINERW},  ///<エリートトレーナー
 {TRTYPE_VETERANM,  VETERANM},  ///<ベテラントレーナー
 {TRTYPE_VETERANW,  VETERANW},  ///<ベテラントレーナー
 {TRTYPE_FISHING,  FISHING},  ///<つりびと
 {TRTYPE_MOUNT,  MOUNTMAN},  ///<やまおとこ
 {TRTYPE_WORKER1,  WORKMAN},  ///<さぎょういん
 {TRTYPE_WORKER2,  WORKMAN2},  ///<さぎょういん
 {TRTYPE_JUGGLING, CLOWN },   //クラウン
 {TRTYPE_ARTIST,  OLDMAN1},  ///<げいじゅつか
 {TRTYPE_POLICE,  POLICEMAN},  ///<おまわりさん
 {TRTYPE_HEADS,  BADMAN},  ///<スキンヘッズ
 {TRTYPE_BADRIDER, BADRIDER},   //ぼうそうぞく
 {TRTYPE_CLEANING, CLEANINGM},   //せいそういん
 {TRTYPE_RAIL,RAILMAN }, //てつどういん
 {TRTYPE_PILOT, PILOT},   //パイロット
 {TRTYPE_BUSINESS1, BUSINESSMAN},    //ビジネスマン1
 {TRTYPE_BUSINESS2, BUSINESSMAN},    //ビジネスマン2
 {TRTYPE_PARASOL,  AMBRELLA},  ///<パラソルおねえさん
 {TRTYPE_BAKER,     BAKER },   //ベーカリー
 {TRTYPE_CHILDCARE, WOMAN3}, //ほいくし
 {TRTYPE_MAID, MAID},    //メイド
 {TRTYPE_OL, OL},    //ＯＬ
};

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

#if 0
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
#endif
