//======================================================================
/**
 * @file	plist_demo.c
 * @brief	ポケモンリスト：フォルムチェンジ演出
 * @author	ariizumi
 * @data	10/02/22
 *
 * モジュール名：PLIST_DEMO
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/rtc_tool.h"
#include "system/wipe.h"
#include "item/item.h"
#include "sound/pm_wb_voice.h"
#include "savedata/shortcut.h"

#include "pokelist_gra.naix"
#include "msg/msg_pokelist.h"

#include "plist_sys.h"
#include "plist_plate.h"
#include "plist_message.h"
#include "plist_demo.h"
#include "plist_snd_def.h"

#include "pokelist_particle_lst.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define PLIST_DEMO_EFFECT_WORK_SIZE (0x5000)

///パーティクルが出て、この時間だけ経ったらポケモンアイコン変更
#define PLIST_DEMO_CHANGE_WAIT_GIRATHINA		(65)
///パーティクルが出て、この時間だけ経ったらポケモンアイコン変更
#define PLIST_DEMO_CHANGE_WAIT_SHEIMI			(38)
///パーティクルが出て、この時間だけ経ったらホワイトアウト開始
#define PLIST_DEMO_CHANGE_WAIT_KYUREMU_UNION_WHITEOUT		(110)
#define PLIST_DEMO_CHANGE_WAIT_KYUREMU_PERGE_WHITEOUT		(130)
#define PLIST_DEMO_KYUREMU_WIPEOUT_DIV    (12)
#define PLIST_DEMO_KYUREMU_WIPEOUT_SYNC   (1)
#define PLIST_DEMO_KYUREMU_WIPEIN_DIV    (12)
#define PLIST_DEMO_KYUREMU_WIPEIN_SYNC   (1)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void PLIST_DEMO_InitEffect( PLIST_WORK *work );
static void PLIST_DEMO_TermEffect( PLIST_WORK *work );
static BOOL PLIST_DEMO_FormChangeSeq( PLIST_WORK* work );
static BOOL PLIST_DEMO_FormChangeKyuremuSeq( PLIST_WORK* work );

//--------------------------------------------------------------
//	デモ初期化
//--------------------------------------------------------------
void PLIST_DEMO_DemoInit( PLIST_WORK *work )
{
  GF_ASSERT_MSG(work->demoType!=PDT_NONE,"Plz set demotype!\n");
  
  work->demoCnt = 0;
  
  //チラツキを消すため一回非表示
  GFL_BG_SetVisible( PLIST_BG_3D , FALSE ); 
  //BGを3D用に切り替える
  PLIST_TermBG0_2DMenu( work );
  PLIST_InitBG0_3DParticle( work );

  PLIST_DEMO_InitEffect( work );
	//G2_SetBlendAlpha(GX_BLEND_PLANEMASK_NONE, 0x3f, 31, 0);
  
  work->mainSeq = PSMS_FORM_CHANGE_MAIN;
  work->demoSeq = 0;
}

//--------------------------------------------------------------
//	デモ開放
//--------------------------------------------------------------
void PLIST_DEMO_DemoTerm( PLIST_WORK *work )
{
  PLIST_CallbackFunc cbFunc = PLIST_MSGCB_ExitCommon;

  PLIST_DEMO_TermEffect( work );
  //3DをBG用に切り替える
  PLIST_TermBG0_3DParticle( work );
  PLIST_InitBG0_2DMenu( work );

  //多分2Dの初期化で消えてるのでリロード
  PLIST_MSG_ReloadWinFrame( work , work->msgWork );

  switch( work->demoType )
  {
  case PDT_GIRATHINA_TO_ORIGIN:
  case PDT_GIRATHINA_TO_ANOTHER:
    if( work->plData->mode == PL_MODE_FIELD )
    {
      //リストから持たせる→バッグ帰り
      cbFunc = PLIST_MSGCB_ReturnSelectCommon;
    }
    //break throw
  case PDT_SHEIMI_TO_SKY:
  case PDT_KYUREMU_TO_COLOR:
  case PDT_KYUREMU_TO_NORMAL:
    PLIST_MSG_CreateWordSet( work , work->msgWork );
    PLIST_MSG_AddWordSet_PokeName( work , work->msgWork , 0 , work->selectPokePara );
    PLIST_MessageWaitInit( work , mes_pokelist_12_01 , TRUE , cbFunc );
    PLIST_MSG_DeleteWordSet( work , work->msgWork );
    break;
  default:
    work->mainSeq = PSMS_FADEOUT;
    break;
  }
}

//--------------------------------------------------------------
//	デモ更新
//--------------------------------------------------------------
void PLIST_DEMO_DemoMain( PLIST_WORK *work )
{
  BOOL ret = FALSE;

  switch( work->demoType ){
  case PDT_GIRATHINA_TO_ORIGIN:
  case PDT_GIRATHINA_TO_ANOTHER:
  case PDT_SHEIMI_TO_SKY:
    ret = PLIST_DEMO_FormChangeSeq( work );
    break;
  case PDT_KYUREMU_TO_COLOR:
  case PDT_KYUREMU_TO_NORMAL:
    ret = PLIST_DEMO_FormChangeKyuremuSeq( work );
    break;
  }
  if(ret){
    work->mainSeq = PSMS_FORM_CHANGE_TERM;
  }
  GFL_PTC_CalcAll();
  
  //3D描画  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    GFL_PTC_DrawAll();
  }
  GFL_G3D_DRAW_End();

  //GFL_PTC_Main();
}
  
typedef struct PLIST_DEMO_PARTICLE{
  u8  file;
  u8  emmit_num;
}PLIST_DEMO_PARTICLE;

static const PLIST_DEMO_PARTICLE DATA_ParticleResTbl[SPA_POKELIST_PARTICLE_LST_MAX] = {
  { NARC_pokelist_gra_demo_girathina_spa, DEMO_GIRATHINA_SPAMAX },  //ギラティナ
  { NARC_pokelist_gra_demo_sheimi_spa,    DEMO_SHEIMI_SPAMAX },     //シェイミ
  { NARC_pokelist_gra_demo_kyuremu_union_spa,    DEMO_KYUREMU_UNION_SPAMAX },     //キュレム合体
  { NARC_pokelist_gra_demo_resiramu_spa,         DEMO_RESIRAMU_SPAMAX },          //キュレム合体レシラム
  { NARC_pokelist_gra_demo_zekuromu_spa,         DEMO_ZEKUROMU_SPAMAX },          //キュレム合体ゼクロム
  { NARC_pokelist_gra_demo_kyuremu_perge_r_spa,  DEMO_KYUREMU_PERGE_R_SPAMAX },   //キュレム分離レシラム
  { NARC_pokelist_gra_demo_kyuremu_perge_z_spa,  DEMO_KYUREMU_PERGE_Z_SPAMAX },   //キュレム分離ゼクロム
};

//--------------------------------------------------------------
//	エフェクト読み込み
//--------------------------------------------------------------
static void PLIST_DEMO_InitEffectCore( PLIST_WORK *work, int wk_idx, int res_id, int plate_id )
{
  u8 i;
  void* effRes;
  const PLIST_DEMO_PARTICLE* res = &DATA_ParticleResTbl[res_id];

  effRes = GFL_PTC_LoadArcResource( ARCID_POKELIST, res->file, work->heapId );
  
  //まずマネージャの作成
  work->effTempWork[wk_idx] = GFL_NET_Align32Alloc( work->heapId , PLIST_DEMO_EFFECT_WORK_SIZE );
  work->ptcWork[wk_idx] = GFL_PTC_Create( work->effTempWork[wk_idx],PLIST_DEMO_EFFECT_WORK_SIZE,FALSE,work->heapId);
  
	//リソースとマネージャの関連付け
	GFL_PTC_SetResource( work->ptcWork[wk_idx] , effRes , FALSE , GFUser_VIntr_GetTCBSYS() );
	
  //エミッターの作成
  {
    GFL_CLACTPOS pos2d;
    VecFx32 pos;
    PLIST_PLATE_GetPlatePosition( work , work->plateWork[plate_id] , &pos2d );
    pos2d.x = pos2d.x-( (PLIST_PLATE_WIDTH /2)*8 ) + PLIST_PLATE_POKE_POS_X;
    pos2d.y = pos2d.y-( (PLIST_PLATE_HEIGHT/2)*8 ) + PLIST_PLATE_POKE_POS_Y + 12;
    pos.x = FX32_CONST(pos2d.x)/PLIST_DEMO_SCALE;
    pos.y = FX32_CONST(pos2d.y)/PLIST_DEMO_SCALE;
    pos.z = FX32_CONST(64);
    for( i = 0; i < res->emmit_num; i++ )
    {
      GFL_PTC_CreateEmitter( work->ptcWork[wk_idx] , i , &pos );
      IWASAWA_Printf("#EmmiterCreate %d\n",i);
    }
  }
}

//--------------------------------------------------------------
//	エフェクトファイルの初期化
//--------------------------------------------------------------
static void PLIST_DEMO_InitEffect( PLIST_WORK *work )
{
  int i;

  for(i = 0;i < PLIST_DEMO_PARTICLE_MAX; i++){
    work->effTempWork[i] = NULL;
    work->ptcWork[i] = NULL;
  }
  //リソース読み込み
  switch( work->demoType ){
  case PDT_GIRATHINA_TO_ORIGIN:
  case PDT_GIRATHINA_TO_ANOTHER:
    PLIST_DEMO_InitEffectCore( work, 0, SPA_DEMO_GIRATHINA, work->pokeCursor );
    work->demoChangeTimming = PLIST_DEMO_CHANGE_WAIT_GIRATHINA;
    work->demoSeNo = PLIST_SND_DEMO_GRATHINA;
    break;
  case PDT_SHEIMI_TO_SKY:
    PLIST_DEMO_InitEffectCore( work, 0, SPA_DEMO_SHEIMI, work->pokeCursor );
    work->demoChangeTimming = PLIST_DEMO_CHANGE_WAIT_SHEIMI;
    work->demoSeNo = PLIST_SND_DEMO_SHEIMI;
    break;
  case PDT_KYUREMU_TO_COLOR:
    {
      int mode = (work->demoParam-KYUREMU_UNION_RESIRAMU);
      PLIST_DEMO_InitEffectCore( work, 0, SPA_DEMO_KYUREMU_UNION, work->changeTarget );
      PLIST_DEMO_InitEffectCore( work, 1, SPA_DEMO_RESIRAMU+mode, work->pokeCursor );
      work->demoChangeTimming = PLIST_DEMO_CHANGE_WAIT_KYUREMU_UNION_WHITEOUT;
      if( mode ){
        work->demoSeNo = PLIST_SND_DEMO_KYUREMU_UNION_Z;
      }else{
        work->demoSeNo = PLIST_SND_DEMO_KYUREMU_UNION_R;
      }
    }
    break;
  case PDT_KYUREMU_TO_NORMAL:
    {
      const POKEMON_PARAM* pp = RANDOMMAP_SAVE_GetKyuremuUnionPokeParam( (RANDOMMAP_SAVE*)work->plData->rnd_map );
      int mode = (PP_Get( pp, ID_PARA_monsno, NULL ) == MONSNO_ZEKUROMU);
      PLIST_DEMO_InitEffectCore( work, 0, SPA_DEMO_KYUREMU_PERGE_R+mode, work->pokeCursor );
      work->demoChangeTimming = PLIST_DEMO_CHANGE_WAIT_KYUREMU_PERGE_WHITEOUT;
      if( mode ){
        work->demoSeNo = PLIST_SND_DEMO_KYUREMU_PERGE_Z;
      }else{
        work->demoSeNo = PLIST_SND_DEMO_KYUREMU_PERGE_R;
      }
    }
    break;
  }
}

//--------------------------------------------------------------
//	エフェクトファイルの開放
//--------------------------------------------------------------
static void PLIST_DEMO_TermEffect( PLIST_WORK *work )
{
  int i;

  for( i = 0;i < PLIST_DEMO_PARTICLE_MAX; i++ ){
    if( work->ptcWork[i] != NULL ){
      GFL_PTC_Delete( work->ptcWork[i] );
      GFL_NET_Align32Free( work->effTempWork[i] );
      work->effTempWork[i] = NULL;
      work->ptcWork[i] = NULL;
    }
  }
}

//--------------------------------------------------------------
//	ギラティナ/シェイミ　フォルムチェンジデモ
//--------------------------------------------------------------
static BOOL PLIST_DEMO_FormChangeSeq( PLIST_WORK* work )
{
  switch( work->demoSeq ){
  case 0:
    //チラツキを消すため一回非表示にした面の再表示
    GFL_BG_SetVisible( PLIST_BG_3D , TRUE ); 

    //SEの再生
    PMSND_PlaySystemSE( work->demoSeNo );
    work->demoSeq++;
    //break throw
  case 1:
    if( ++work->demoCnt > work->demoChangeTimming )
    {
      PLIST_PLATE_ResetParam( work , work->plateWork[work->pokeCursor] , work->selectPokePara , 0 );
      work->demoSeq++;
    }
    break;
  case 2:
    if( GFL_PTC_GetEmitterNum( work->ptcWork[0] ) <= 0 &&
        PMSND_CheckPlaySE() == FALSE )
    {
      const u32 monsNo = PP_Get( work->selectPokePara , ID_PARA_monsno , NULL );
      const u32 formNo = PP_Get( work->selectPokePara , ID_PARA_form_no , NULL );
      work->demoVoicePlayer = PMVOICE_Play( monsNo , formNo , 64 , FALSE , 0 , 0 , FALSE , 0 );
    
      work->demoSeq++;
    }
    break; 
  case 3:
    if( PMVOICE_CheckPlay( work->demoVoicePlayer ) == FALSE )
    {
      return TRUE;
    }
    break;
  default:
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//	キュレム　フォルムチェンジデモ
//--------------------------------------------------------------
static BOOL PLIST_DEMO_FormChangeKyuremuSeq( PLIST_WORK* work )
{
  switch( work->demoSeq ){
  case 0:
    //チラツキを消すため一回非表示にした面の再表示
    GFL_BG_SetVisible( PLIST_BG_3D , TRUE ); 
    //SEの再生
    PMSND_PlaySystemSE( work->demoSeNo );
    work->demoSeq++;
    //break throw
  case 1:
    if( ++work->demoCnt > work->demoChangeTimming )
    {
      WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
          WIPE_FADE_WHITE , PLIST_DEMO_KYUREMU_WIPEOUT_DIV , PLIST_DEMO_KYUREMU_WIPEOUT_SYNC , work->heapId );
      work->demoSeq++;
    }
    break;
  case 2:
    {
      int i;
      if( WIPE_SYS_EndCheck() == FALSE || PMSND_CheckPlaySE() == TRUE ){
        break;
      }
      for(i = 0;i < PLIST_DEMO_PARTICLE_MAX; i++ ){
        if( work->ptcWork[i] != NULL && GFL_PTC_GetEmitterNum( work->ptcWork[i] ) > 0 ){
          break;
        }
      }
    }
    //リスト書き換え
    if( work->demoParam == KYUREMU_UNION_PERGE ){
      PLIST_DEMO_ChangeKyuremuNormal( work, work->selectPokePara );
    }else{
      PLIST_DEMO_ChangeKyuremuColor( work, work->demoParam, work->changeTarget, work->pokeCursor );
    }

    if( work->pokeCursor < work->changeTarget ){
      work->pokeCursor = --work->changeTarget;  //カーソルずらしておく
      work->selectPokePara = PokeParty_GetMemberPointer(work->plData->pp, work->pokeCursor );
    }
    {
      int i;
      int partyMax = PokeParty_GetPokeCount(work->plData->pp);

      work->btlJoinNum = 0;

      //プレートを再生成
      for( i=0;i<PLIST_LIST_MAX;i++ )
      {
        PLIST_PLATE_ClearPlate( work , work->plateWork[i], 0 );
        PLIST_PLATE_DeletePlate( work, work->plateWork[i] );
        
        if( i<partyMax )
        {
          POKEMON_PARAM *pp = PokeParty_GetMemberPointer(work->plData->pp, i);
          work->plateWork[i] = PLIST_PLATE_CreatePlate( work , i , pp );
        }
        else
        {
          work->plateWork[i] = PLIST_PLATE_CreatePlate_Blank( work , i );
        }
    
        if( PLIST_PLATE_GetBattleOrder( work->plateWork[i] ) <= PPBO_JOIN_6 )
        {
          work->btlJoinNum++;
        }
      }
    }
    work->demoSeq++;
    break;
  case 3:
    {
      int i;
      int count = 0;
      //プレートのアップデート待ち
      for( i=0;i<PLIST_LIST_MAX;i++ )
      {
        count = PLIST_PLATE_IsUpdatePlate( work->plateWork[i] );
      }
      if( count > 0 ){
        break;
      }
    }
    {
      POKEMON_PARAM* pp = PokeParty_GetMemberPointer(work->plData->pp, work->changeTarget );
      const u32 monsNo = PP_Get( pp , ID_PARA_monsno , NULL );
      const u32 formNo = PP_Get( pp , ID_PARA_form_no , NULL );
      work->demoVoicePlayer = PMVOICE_Play( monsNo , formNo , 64 , FALSE , 0 , 0 , FALSE , 0 );
    
      work->demoSeq++;
    }
    break; 
  case 4:
    if( PMVOICE_CheckPlay( work->demoVoicePlayer ) == FALSE )
    {
      WIPE_SYS_Start( WIPE_PATTERN_M , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
        WIPE_FADE_WHITE , PLIST_DEMO_KYUREMU_WIPEIN_DIV , PLIST_DEMO_KYUREMU_WIPEIN_SYNC , work->heapId );
      work->demoSeq++;
    }
    break;
  case 5:
  default:
    if( WIPE_SYS_EndCheck() == TRUE ){
      return TRUE;
    }
  }
  return FALSE;
}


//--------------------------------------------------------------
//	ギラティナ：アナザー→オリジンチェック
//--------------------------------------------------------------
const BOOL PLIST_DEMO_CheckGirathnaToOrigin( PLIST_WORK *work , POKEMON_PARAM *pp )
{
  if( PP_Get( work->selectPokePara , ID_PARA_monsno , NULL ) != MONSNO_GIRATHINA )
  {
    //ギラティナじゃない！
    return FALSE;
  }
  if( PP_Get( work->selectPokePara , ID_PARA_item , NULL ) != ITEM_HAKKINDAMA )
  {
    //はっきんだま持ってない！
    return FALSE;
  }
  if( PP_Get( work->selectPokePara , ID_PARA_form_no , NULL ) != FORMNO_GIRATHINA_ANOTHER )
  {
    //アナザーフォルムじゃない！
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
//	ギラティナ：アナザー→オリジン変更(再描画はしない
//--------------------------------------------------------------
void PLIST_DEMO_ChangeGirathinaToOrigin( PLIST_WORK *work , POKEMON_PARAM *pp )
{
  PP_ChangeFormNo( pp , FORMNO_GIRATHINA_ORIGIN );
  ZUKANSAVE_SetPokeGet( work->plData->zkn , pp );
}

//--------------------------------------------------------------
//	ギラティナ：オリジン→アナザーチェック
//--------------------------------------------------------------
const BOOL PLIST_DEMO_CheckGirathnaToAnother( PLIST_WORK *work , POKEMON_PARAM *pp )
{
  if( PP_Get( work->selectPokePara , ID_PARA_item , NULL ) == ITEM_HAKKINDAMA )
  {
    //はっきんだま持ってる！
    return FALSE;
  }
  if( PP_Get( work->selectPokePara , ID_PARA_monsno , NULL ) != MONSNO_GIRATHINA )
  {
    //ギラティナじゃない！
    return FALSE;
  }
  if( PP_Get( work->selectPokePara , ID_PARA_form_no , NULL ) != FORMNO_GIRATHINA_ORIGIN )
  {
    //オリジンフォルムじゃない！
    return FALSE;
  }
  return TRUE;
}
//--------------------------------------------------------------
//	ギラティナ：オリジン→アナザー変更(再描画はしない
//--------------------------------------------------------------
void PLIST_DEMO_ChangeGirathinaToAnother( PLIST_WORK *work , POKEMON_PARAM *pp )
{
  PP_ChangeFormNo( pp , FORMNO_GIRATHINA_ANOTHER );
  ZUKANSAVE_SetPokeGet( work->plData->zkn , pp );
}

//--------------------------------------------------------------
//	シェイミ：ランド→スカイチェック
//--------------------------------------------------------------
const BOOL PLIST_DEMO_CheckSheimiToSky( PLIST_WORK *work , POKEMON_PARAM *pp )
{
  if( PP_Get( work->selectPokePara , ID_PARA_monsno , NULL ) != MONSNO_SHEIMI )
  {
    //シェイミじゃない！
    return FALSE;
  }
  if( PP_Get( work->selectPokePara , ID_PARA_form_no , NULL ) != FORMNO_SHEIMI_LAND )
  {
    //ランドフォルムじゃない！
    return FALSE;
  }
  if( PP_Get( work->selectPokePara , ID_PARA_event_get_flag , NULL ) != 1 )
  {
    //イベント配布じゃない！
    return FALSE;
  }
  if( PP_GetSick( work->selectPokePara ) & PTL_CONDITION_KOORI )
  {
    //凍ってる！
    return FALSE;
  }
  {
    const int startTime = PM_RTC_GetTimeZoneChangeHour( work->plData->season , TIMEZONE_MORNING );
    const int endTime = PM_RTC_GetTimeZoneChangeHour( work->plData->season , TIMEZONE_NIGHT );
    
    RTCTime time;
    GFL_RTC_GetTime( &time );
    if( time.hour<startTime || time.hour>=endTime )
    {
      //時間外！
      return FALSE;
    }
  }
  
  return TRUE;
}
//--------------------------------------------------------------
//	シェイミ：ランド→スカイ変更(再描画はしない
//--------------------------------------------------------------
void PLIST_DEMO_ChangeSheimiToSky( PLIST_WORK *work , POKEMON_PARAM *pp )
{
  PP_ChangeFormNo( pp , FORMNO_SHEIMI_SKY );
  ZUKANSAVE_SetPokeGet( work->plData->zkn , pp );
}

//========================================================================
//キュレム合体/分離処理
//========================================================================
static void PLIST_DEMO_KyuremuWazaChange( POKEMON_PARAM* pp, u8 s_idx, u8 d_idx );

#define KYUREMU_WAZA_CHANGE_MAX (2)

static const u16 DATA_KyuremuWazaChange[KYUREMU_WAZA_CHANGE_MAX][3] = {
  { WAZANO_KOWAIKAO, WAZANO_KUROSUHUREIMU, WAZANO_KUROSUSANDAA },
  { WAZANO_KOGOERUSEKAI, WAZANO_KOORUDOHUREA, WAZANO_HURIIZUBORUTO },
};

//--------------------------------------------------------------
//	キュレム：ノーマル→カラーチェック
//--------------------------------------------------------------
const KYUREMU_CHECK_CODE PLIST_DEMO_CheckKyuremuToColor( PLIST_WORK *work , POKEMON_PARAM *pp )
{
  if( PP_Get( work->selectPokePara , ID_PARA_monsno , NULL ) != MONSNO_KYUREMU )
  {
    //キュレムじゃない！
    return KYUREMU_CHECK_NG;
  }
  if( PP_Get( work->selectPokePara , ID_PARA_form_no , NULL ) != FORMNO_KYUREMU_A )
  {
    //ノーマルフォルムじゃない！
    return KYUREMU_CHECK_NG_FORM;
  }
  if( PP_Get( work->selectPokePara, ID_PARA_hp, NULL ) == 0 )
  {
    //瀕死
    return KYUREMU_CHECK_NG_DETH;
  }
  return KYUREMU_CHECK_OK_UNION;
}

//--------------------------------------------------------------
//	キュレム：カラー→ノーマルチェック
//--------------------------------------------------------------
const BOOL PLIST_DEMO_CheckKyuremuToNormal( PLIST_WORK *work , POKEMON_PARAM *pp )
{
  if( PP_Get( work->selectPokePara , ID_PARA_monsno , NULL ) != MONSNO_KYUREMU )
  {
    //キュレムじゃない！
    return KYUREMU_CHECK_NG;
  }
  if( PP_Get( work->selectPokePara , ID_PARA_form_no , NULL ) == FORMNO_KYUREMU_A )
  {
    //すでにノーマルフォルム！
    return KYUREMU_CHECK_NG_FORM;
  }
  if( PokeParty_GetPokeCount(work->plData->pp) == PLIST_LIST_MAX ){
    //リストいっぱい
    return KYUREMU_CHECK_NG_LIST_MAX;
  }
  return KYUREMU_CHECK_OK_PERGE;
}

//--------------------------------------------------------------------------
//  キュレム合体
//--------------------------------------------------------------------------
void PLIST_DEMO_ChangeKyuremuColor( PLIST_WORK *work, KYUREMU_UNION_CHECK mode, int kyuremu_idx, int target_idx )
{
  POKEMON_PARAM *pp_k = PokeParty_GetMemberPointer(work->plData->pp, kyuremu_idx );
  POKEMON_PARAM *pp_t = PokeParty_GetMemberPointer(work->plData->pp, target_idx );

  //合体相手のパラメータを丸ごとセーブ保存
  RANDOMMAP_SAVE_SetKyuremuUnionPokeParam( (RANDOMMAP_SAVE*)work->plData->rnd_map, (const POKEMON_PARAM*)pp_t );

  //キュレム専用技変化
  PLIST_DEMO_KyuremuWazaChange( pp_k, 0, mode+1 );

  //キュレムフォルムチェンジ
  PP_ChangeFormNo( pp_k, FORMNO_KYUREMU_WHITE+mode );
  
  ZUKANSAVE_SetPokeGet( work->plData->zkn , pp_k );
  
  //合体相手を削除
  PokeParty_Delete( work->plData->pp, target_idx );

  //アイテム変化
  MYITEM_KyuremuItemSwap( (MYITEM_PTR)work->plData->myitem, KYUREMU_ITEM_SWAP_UNION );
  SHORTCUT_SwapRegisterForKyuremuItem( (SHORTCUT*)work->plData->shortcut, SHORTCUT_ID_IDENSINOKUSABI_U );
}

//--------------------------------------------------------------------------
//  キュレム分離
//--------------------------------------------------------------------------
void PLIST_DEMO_ChangeKyuremuNormal( PLIST_WORK *work, POKEMON_PARAM* pp_k )
{
  int form;

  //現在のフォルムNo取得
  form = PP_Get( pp_k, ID_PARA_form_no, NULL );

  //キュレム専用技変化
  PLIST_DEMO_KyuremuWazaChange( pp_k, form, 0 );

  //キュレムフォルムチェンジ
  PP_ChangeFormNo( pp_k, FORMNO_KYUREMU_A );
  
  //合体相手復活
  PokeParty_Add( work->plData->pp, RANDOMMAP_SAVE_GetKyuremuUnionPokeParam( (RANDOMMAP_SAVE*)work->plData->rnd_map ) );
  
  //アイテム変化
  MYITEM_KyuremuItemSwap( (MYITEM_PTR)work->plData->myitem, KYUREMU_ITEM_SWAP_PERGE );
  SHORTCUT_SwapRegisterForKyuremuItem( (SHORTCUT*)work->plData->shortcut, SHORTCUT_ID_IDENSINOKUSABI_P );
}

//--------------------------------------------------------------------------
//  キュレム合体/分離時技チェンジ
//--------------------------------------------------------------------------
static void PLIST_DEMO_KyuremuWazaChange( POKEMON_PARAM* pp, u8 s_idx, u8 d_idx )
{
  int i,j;

  for(i = 0;i < 4;i++){
    u16 wazano = PP_Get( pp, ID_PARA_waza1+i, NULL ); 
    u16 waza_pt = PP_Get( pp, ID_PARA_pp1+i, NULL ); 

    if( wazano == WAZANO_NULL ){
      continue; 
    }
    //技入れ替えチェック
    for(j = 0;j < KYUREMU_WAZA_CHANGE_MAX;j++){
      u16 no;
      u16 pt;

      if( DATA_KyuremuWazaChange[j][s_idx] != wazano ){
        continue;
      }
      //ヒットした
      no = DATA_KyuremuWazaChange[j][d_idx];
      pt = WAZADATA_GetParam( no, WAZAPARAM_BASE_PP );

      //新しい技のmaxが現在のポイントよりすくなければmaxにあわせて補正
      PP_Put( pp, ID_PARA_waza1+i, no );
      if( pt < waza_pt ){
        PP_Put( pp, ID_PARA_pp1+i, pt ); 
      }
      break;
    }
  }
}


