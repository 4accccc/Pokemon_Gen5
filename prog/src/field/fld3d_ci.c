//======================================================================
/**
 * @file	fld3d_ci.c
 * @brief	フィールド3Dカットイン
 * @author	Saito
 *
 */
//======================================================================
#include "fld3d_ci.h"
#include "arc_def.h"
#include "gamesystem/game_event.h"
#include "field/field_msgbg.h"    //for FLDMSGBG_RecoveryBG
#include "system/pokegra.h"

#include "../../../resource/fld3d_ci/fldci_id_def.h"

#include "sound/pm_wb_voice.h"   //for PMV_PlayVoice

#define EMITCOUNT_MAX  (2)
#define OBJCOUNT_MAX  (2)
#define ANM_TYPE_MAX  (4)
#define RESCOUNT_MAX  (8)

#define UNIT_NONE   (0xffffffff)

#define NONDATA  (0xffff)

#define	PRO_MAT_Z_OFS	(20*FX32_ONE)   //プロジェクションマトリックスによるデプスバッファ操作値

#define TEXSIZ_S  (128)
#define TEXSIZ_T  (128)
#define TEXVRAMSIZ		(TEXSIZ_S/8 * TEXSIZ_T/8 * 0x20)	// chrNum_x * chrNum_y * chrSiz


#define POKE_VOICE_WAIT (5)
#define VOICE_VOL_OFS (8)

#define FLYSKY_WHITE_FADE_SPD (-16)

#ifdef PM_DEBUG

#include "fieldmap.h" //for FIELDMAP_～
#include "event_mapchange.h"

#endif  //PM_DEBUG

typedef void (*SETUP_CALLBACK)(FLD3D_CI_PTR ptr);


typedef struct FLD3D_CI_tag
{
  FLD_PRTCL_SYS_PTR PrtclSys;
  GFL_G3D_CAMERA *g3Dcamera;
  HEAPID HeapID;
  GFL_G3D_UTIL* Util;
  int UnitIdx;
  GFL_G3D_OBJSTATUS Status;

  GFL_PTC_PTR PrtclPtr;

  u16 PrtclWait[EMITCOUNT_MAX];
  u16 MdlAnm1Wait;
  u16 MdlAnm2Wait;
  u16 PartGene;
  u16 CutInNo;

  GXPlaneMask Mask;
  u8 BgPriority[4];
  BOOL CapEndFlg;
  BOOL Emit1;
  BOOL Emit2;
  BOOL Anm1Flg;
  BOOL Anm2Flg;

  //ポケモン展開用
  NNSG2dCharacterData * chr;
  NNSG2dPaletteData* plt;
  GFL_BMP_DATA* pokeImgBitmap;
  void *chr_buf;
  void *pal_buf;
  int MonsNo;
  int FormNo;
  int Sex;
  int Rare;
  BOOL Egg;

  BOOL VoicePlayFlg;
  int SePlayWait;
  u32 VoicePlayerIdx;

  SETUP_CALLBACK SetupCallBack;
}FLD3D_CI;

//バイナリデータフォーマット
typedef struct {
  u32 SpaIdx;
  u16 SpaWait[EMITCOUNT_MAX];
  u16 MdlObjIdx[OBJCOUNT_MAX]; //0：Ａ　1：Ｂ
  u16 MdlAAnmWait;
  u16 MdlBAnmWait;
  u16 MdlAAnmIdx[ANM_TYPE_MAX];  //4つ目はデータなし
  u16 MdlBAnmIdx[ANM_TYPE_MAX];  //4つ目はデータなし
}RES_DEF_DAT;

typedef struct {
  GFL_G3D_UTIL_RES *Res;
  GFL_G3D_UTIL_OBJ *Obj;
  GFL_G3D_UTIL_ANM *Anm1;
  GFL_G3D_UTIL_ANM *Anm2;
  GFL_G3D_UTIL_SETUP Setup;
  u16 SpaWait[EMITCOUNT_MAX];
  u16 MdlAAnmWait;
  u16 MdlBAnmWait;
  u32 SpaIdx;
  u8 ResNum;
  u8 ObjNum;
  u8 Anm1Num;
  u8 Anm2Num;
}RES_SETUP_DAT;

typedef struct {
  GAMESYS_WORK * gsys;
  FLD3D_CI_PTR CiPtr;
  RES_SETUP_DAT SetupDat;
}FLD3D_CI_EVENT_WORK;

#ifdef PM_DEBUG
typedef struct {
  GAMESYS_WORK *gsys;
  FLD3D_CI_PTR CiPtr;
  FIELD_CAMERA * camera;
  FIELD_PLAYER * player;
  FLDNOGRID_MAPPER* mapper;
  const VecFx32 *Watch;
}FLYSKY_EFF_WORK;
#endif

static void SetupResource(FLD3D_CI_PTR ptr, RES_SETUP_DAT *outDat, const u8 inCutInNo);
static void SetupResourceCore(FLD3D_CI_PTR ptr, const GFL_G3D_UTIL_SETUP *inSetup);
static void DeleteResource(FLD3D_CI_PTR ptr, RES_SETUP_DAT *ioDat);
static BOOL PlayParticle(FLD3D_CI_PTR ptr);
static BOOL PlayMdlAnm1(FLD3D_CI_PTR ptr);
static BOOL PlayMdlAnm2(FLD3D_CI_PTR ptr);

static GMEVENT_RESULT CutInEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT VoiceFadeOutEvt( GMEVENT* event, int* seq, void* work );

static void ParticleCallBack(GFL_EMIT_PTR emit);
static void Generate(FLD3D_CI_PTR ptr, const u32 inResNo);

static void CreateRes(RES_SETUP_DAT *outDat, const u8 inResArcIdx, const HEAPID inHeapID);
static void DeleteRes(RES_SETUP_DAT *ioDat);

static void PushPriority(FLD3D_CI_PTR ptr);
static void PushDisp(FLD3D_CI_PTR ptr);
static void PopPriority(FLD3D_CI_PTR ptr);
static void PopDisp(FLD3D_CI_PTR ptr);
static void ReqCapture(FLD3D_CI_PTR ptr);

static void Graphic_Tcb_Capture( GFL_TCB *p_tcb, void *p_work );

#ifdef PM_DEBUG
static GMEVENT_RESULT DebugFlySkyEffEvt( GMEVENT* event, int* seq, void* work );
#endif  //PM_DEBUG

static void ReTransToPokeGra(FLD3D_CI_PTR ptr);

static GMEVENT_RESULT WhiteOutEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WhiteInEvt( GMEVENT* event, int* seq, void* work );

static BOOL IsFlySkyOut(const int inCutinNo);
static BOOL IsFlySkyIn(const int inCutinNo);

//--------------------------------------------------------------------------------------------
/**
 * モジュール初期化
 *
 * @param   inHeapID      ヒープＩＤ
 * @param   inPrtclSysptr パーティクルシステムポインタ
 *
 * @return	ptr         モジュールポインタ
 */
//--------------------------------------------------------------------------------------------
FLD3D_CI_PTR FLD3D_CI_Init(const HEAPID inHeapID, FLD_PRTCL_SYS_PTR inPrtclSysPtr)
{
  FLD3D_CI_PTR ptr;
  
  GF_ASSERT(inPrtclSysPtr != NULL);

  ptr = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(FLD3D_CI));
  ptr->Util = GFL_G3D_UTIL_Create( RESCOUNT_MAX, OBJCOUNT_MAX, inHeapID );
  ptr->HeapID = inHeapID;
  ptr->PrtclSys = inPrtclSysPtr;
  
  ptr->UnitIdx = UNIT_NONE;

  {
    VecFx32 pos = {0, 0, FX32_ONE*(128)};
    VecFx32 target = {0,0,0};
    //カメラ作成
    ptr->g3Dcamera = GFL_G3D_CAMERA_CreateDefault(
        &pos, &target, inHeapID );
  }
  return ptr;
}

//--------------------------------------------------------------------------------------------
/**
 * モジュール破棄
 *
 * @param   ptr     モジュールポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD3D_CI_End(FLD3D_CI_PTR ptr)
{
  u16 i;
/**  
  //解放忘れがあるかもしれないので、ユニット廃棄もしておく
  for (i=0;i<FLD_EXP_OBJ_UNIT_MAX;i++){
    if ( ptr->Unit[i].Valid){
      FLD_EXP_OBJ_DelUnit( ptr, i );
    }
  }
*/
  //カメラ解放
  GFL_G3D_CAMERA_Delete(ptr->g3Dcamera);
  GFL_G3D_UTIL_Delete( ptr->Util );
  GFL_HEAP_FreeMemory( ptr );
}

//--------------------------------------------------------------------------------------------
/**
 * リソースセットアップ
 *
 * @param   ptr       カットイン管理ポインタ
 * @param   inSetup   セットアップデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetupResourceCore(FLD3D_CI_PTR ptr, const GFL_G3D_UTIL_SETUP *inSetup)
{
  u16 i;
  u16 unitIdx;
  GFL_G3D_OBJSTATUS *status;
  u16 obj_num;

  const GFL_G3D_UTIL_SETUP *setup;
  
  setup = inSetup;

  unitIdx = GFL_G3D_UTIL_AddUnit( ptr->Util, setup );
  NOZOMU_Printf("unit_idx=%d\n",unitIdx);
  obj_num = setup->objCount;
  if (obj_num == 0){
    GF_ASSERT_MSG(0,"OBJ NOTHING\n");
  }

  ptr->UnitIdx = unitIdx;

  {
    VEC_Set( &ptr->Status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
	  MTX_Identity33( &ptr->Status.rotate );
    VEC_Set( &ptr->Status.trans, 0, 0, 0 );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * 描画
 *
 * @param   ptr     カットイン管理ポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD3D_CI_Draw( FLD3D_CI_PTR ptr )
{
  u8 i,j;
  
  GFL_G3D_CAMERA_Switching( ptr->g3Dcamera );
  {
    fx32 ofs;
	  MtxFx44 org_pm,pm;
		const MtxFx44 *m;
		m = NNS_G3dGlbGetProjectionMtx();
		org_pm = *m;
		pm = org_pm;
		pm._32 += FX_Mul( pm._22, PRO_MAT_Z_OFS );
		NNS_G3dGlbSetProjectionMtx(&pm);
		NNS_G3dGlbFlush();		  //　ジオメトリコマンドを転送
    NNS_G3dGeFlushBuffer(); // 転送まち

    //ユニット数分ループ
    if (ptr->UnitIdx != UNIT_NONE){
      u16 obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( ptr->Util, ptr->UnitIdx );
      u16 obj_count = GFL_G3D_UTIL_GetUnitObjCount( ptr->Util, ptr->UnitIdx );
      for (j=0;j<obj_count;j++){
        GFL_G3D_OBJ* pObj;
        pObj = GFL_G3D_UTIL_GetObjHandle(ptr->Util, obj_idx+j);
        GFL_G3D_DRAW_DrawObject( pObj, &ptr->Status );
      }
    }
    NNS_G3dGlbSetProjectionMtx(&org_pm);
		NNS_G3dGlbFlush();		//　ジオメトリコマンドを転送
  }
}

#ifdef PM_DEBUG
//カットイン呼び出し
void FLD3D_CI_CallCutIn( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, const u8 inCutInNo )
{
  GMEVENT * event;
  event = FLD3D_CI_CreateCutInEvt(gsys, ptr, inCutInNo);
  GAMESYSTEM_SetEvent(gsys, event);
}

//ポケモンカットイン呼び出し
void FLD3D_CI_CallPokeCutIn( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr )
{
  GMEVENT * event;
  int no = FLDCIID_POKE;
  event = FLD3D_CI_CreateCutInEvt(gsys, ptr, no);
  //セットアップ後コールバック設定
  ptr->SetupCallBack = ReTransToPokeGra;
  //ポケモンカットイン用ポケモン指定変数セット
  ptr->MonsNo = 3;
  ptr->FormNo = 0;
  ptr->Sex = 0;
  ptr->Rare = 0;
  ptr->SePlayWait = POKE_VOICE_WAIT;
  ptr->VoicePlayFlg = TRUE;

  GAMESYSTEM_SetEvent(gsys, event);
}
#endif

//--------------------------------------------------------------------------------------------
/**
 * カットインイベント作成
 *
 * @param   gsys        ゲームシステムポインタ
 * @param   ptr         カットイン管理ポインタ
 * @param   inCutInNo   カットインナンバー
 *
 * @return	event       イベントポインタ
 */
//--------------------------------------------------------------------------------------------
GMEVENT *FLD3D_CI_CreateCutInEvt(GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, const u8 inCutInNo)
{
  GMEVENT * event;
  FLD3D_CI_EVENT_WORK *work;
  int size;

  size = sizeof(FLD3D_CI_EVENT_WORK);
  ptr->PartGene = 0;
  ptr->CutInNo = inCutInNo;
  //セットアップ後コールバックなしでセットする
  ptr->SetupCallBack = NULL;
  //ＳＥ無しでセット
  ptr->SePlayWait = 0;
  ptr->VoicePlayFlg = FALSE;

  //イベント作成
  {
    event = GMEVENT_Create( gsys, NULL, CutInEvt, size );

    work = GMEVENT_GetEventWork(event);
    MI_CpuClear8( work, size );
    work->gsys = gsys;
    work->CiPtr = ptr;
  }
  return event;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンカットインイベント作成
 *
 * @param   gsys        ゲームシステムポインタ
 * @param   ptr         カットイン管理ポインタ
 * @apram   inMonsNo    モンスターナンバー
 * @param   inFormNo    フォルムナンバー
 * @param   inSex       性別
 * @param   inRare      レアか？
 * @param   inEgg       卵か
 *
 * @return	event       イベントポインタ
 */
//--------------------------------------------------------------------------------------------
GMEVENT *FLD3D_CI_CreatePokeCutInEvt( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr,
                                      const int inMonsNo, const int inFormNo,
                                      const int inSex, const int inRare, const BOOL inEgg )
{
  GMEVENT * event;
  int no = FLDCIID_POKE;
  event = FLD3D_CI_CreateCutInEvt(gsys, ptr, no);
  //セットアップ後コールバック設定
  ptr->SetupCallBack = ReTransToPokeGra;
  //ポケモンカットイン用ポケモン指定変数セット
  ptr->MonsNo = inMonsNo;
  ptr->FormNo = inFormNo;
  ptr->Sex = inSex;
  ptr->Rare = inRare;
  ptr->Egg  = inEgg;
  ptr->SePlayWait = POKE_VOICE_WAIT;
  ptr->VoicePlayFlg = TRUE;

  return event;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンカットインイベント作成(手持ちPOKEPARTYから)
 *
 * @param   gsys        ゲームシステムポインタ
 * @param   ptr         カットイン管理ポインタ
 * @param   pos         カットインのターゲットにする手持ちポケモンindex
 *
 * @return	event       イベントポインタ
 */
//--------------------------------------------------------------------------------------------
GMEVENT *FLD3D_CI_CreatePokeCutInEvtTemoti( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, u8 pos )
{
  GMEVENT * event;
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData( gsys );

  {
    POKEPARTY*     party = GAMEDATA_GetMyPokemon( gdata );
    POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, pos );

    int monsno = PP_Get( pp, ID_PARA_monsno, NULL );
    int formno = PP_Get( pp, ID_PARA_form_no, NULL );
    int sex = PP_Get( pp, ID_PARA_sex, NULL );
    int rare = PP_CheckRare( pp );
    int egg = PP_Get( pp, ID_PARA_tamago_flag, NULL );

    event = FLD3D_CI_CreatePokeCutInEvt(gsys, ptr, monsno,formno,sex,rare,egg);
  }
  return event;
}

//--------------------------------------------------------------------------------------------
/**
 * カットインイベント
 *
 * @param   event       イベントポインタ
 * @param   *seq        シーケンサ
 * @param   work        ワークポインタ
 *
 * @return	GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT CutInEvt( GMEVENT* event, int* seq, void* work )
{
  FLD3D_CI_PTR ptr;
  FLD3D_CI_EVENT_WORK *evt_work = work;
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  
  ptr = evt_work->CiPtr;
  gsys = GMEVENT_GetGameSysWork(event);
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  switch(*seq){
  case 0:
    //ＯＢＪのポーズ
    {
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDLSYS_PauseMoveProc(mmdlsys);
    }
    //プライオリティの保存
    PushPriority(ptr);
    //表示状態の保存
    PushDisp(ptr);
    //カットイン共通ホワイトアウト処理

    if ( IsFlySkyOut(ptr->CutInNo) )
    {
      GMEVENT* fade_event;
      fade_event = GMEVENT_Create(gsys, event, WhiteOutEvt, 0);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq)++;
    break;
  case 1:
    {
      //キャプチャリクエスト
      ReqCapture(ptr);
      (*seq)++;
    }
    break;
  case 2:
    //キャプチャ終了待ち
    if (ptr->CapEndFlg){
      //キャプチャ終わったら、プライオリティ変更と表示変更
      GFL_BG_SetPriority( 0, 0 );
      GFL_BG_SetPriority( 2, 3 );
      //描画モード変更
      {
        const GFL_BG_SYS_HEADER bg_sys_header = 
          {
            GX_DISPMODE_GRAPHICS,GX_BGMODE_5,GX_BGMODE_0,GX_BG0_AS_3D
          };
        GFL_BG_SetBGMode( &bg_sys_header );
      }
      //ＢＧセットアップ
      G2_SetBG2ControlDCBmp(
          GX_BG_SCRSIZE_DCBMP_256x256,
          GX_BG_AREAOVER_XLU,
          GX_BG_BMPSCRBASE_0x00000
      );
      GX_SetBankForBG(GX_VRAM_BG_128_D);
      //アルファブレンド
      G2_SetBlendAlpha(GX_BLEND_PLANEMASK_NONE,
				GX_BLEND_PLANEMASK_BG2, 0,0);
      //ここではまだキャプチャ面のみ表示(パシリ防止)
      GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
      (*seq)++;
    }
    break;
  case 3:
    //リソースロード
    SetupResource(ptr, &evt_work->SetupDat, ptr->CutInNo);
    //セットアップ終了したので3Ｄ面もオン
    GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );
    {
      int size = GFL_HEAP_GetHeapFreeSize(ptr->HeapID);
      NOZOMU_Printf("START::FLD3DCUTIN_HEAP_REST %x\n",size);
    }
    //フィールド表示モード切替
    FIELDMAP_SetDraw3DMode(fieldmap, DRAW3DMODE_CUTIN);
    (*seq)++;
    break;
  case 4:
    //1シンク待ったので、ポケモングラフィックを展開していた場合は解放する
    if (ptr->pokeImgBitmap != NULL)
    {
      GFL_BMP_Delete(ptr->pokeImgBitmap);
      ptr->pokeImgBitmap = NULL;
      GFL_HEAP_FreeMemory( ptr->chr_buf );
      GFL_HEAP_FreeMemory( ptr->pal_buf );
    }

    if ( IsFlySkyOut(ptr->CutInNo) )
    {
      GMEVENT* fade_event;
      fade_event = GMEVENT_Create(gsys, event, WhiteInEvt, 0);
      GMEVENT_CallEvent(event, fade_event);
    }

    (*seq)++;
    break;
  case 5:
    {
      BOOL rc1,rc2,rc3;
      //パーティクル再生
      rc1 = PlayParticle(ptr);
      //3Ｄモデル1アニメ再生
      rc2 = PlayMdlAnm1(ptr);
      //3Ｄモデル2アニメ再生
      rc3 = PlayMdlAnm2(ptr);

      //鳴き声再生    @todo
      {
        if (ptr->VoicePlayFlg && ptr->SePlayWait)
        {
          ptr->SePlayWait--;
          if ( ptr->SePlayWait == 0 )
          {
            ptr->VoicePlayerIdx = PMV_PlayVoice( ptr->MonsNo, ptr->FormNo );
          }
        }
      }

      if (rc1&&rc2&&rc3)
      {
        //PMV_StopVoice();    //鳴き声なっているならストップ  @todo
        if ( ptr->VoicePlayFlg )
        {
          GMEVENT * call_event;
          call_event = GMEVENT_Create(gsys, event, VoiceFadeOutEvt, 0);
          GMEVENT_CallEvent(event, call_event);
        }
        (*seq)++;
      }
    }
    break;
  case 6:
    //フィールドモード戻し
    FIELDMAP_SetDraw3DMode(fieldmap, DRAW3DMODE_NORMAL);
    //リソース解放処理
    DeleteResource(ptr, &evt_work->SetupDat);
    
    //空を飛ぶフェードインの場合の処理
    if ( IsFlySkyIn(ptr->CutInNo) )
    {
      //自機表示
      MMDL * mmdl;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
      mmdl = FIELD_PLAYER_GetMMdl( fld_player );
      MMDL_SetStatusBitVanish(mmdl, FALSE);
      //戻ってきた場所はおそらく3Ｄクリアカラーが見えない場所のはずなので
      //3Ｄ面オフの処理とクリアカラーのアルファセットの処理を行わない
    }
    else
    {
      //3D面をオフ(キャプチャ面が見えている)
      GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_OFF );
      //クリアカラーのアルファを元に戻す
      G3X_SetClearColor(GX_RGB(0,0,0),31,0x7fff,0,FALSE);
    }
    (*seq)++;
    break;
  case 7:
    //バックカラー設定
    {
      FIELD_LIGHT *light = FIELDMAP_GetFieldLight( fieldmap );
      FIELD_LIGHT_Reflect( light, TRUE );
    }
    //3D面をオン
    GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );

    {
      int size = GFL_HEAP_GetHeapFreeSize(ptr->HeapID);
      NOZOMU_Printf("END::FLD3DCUTIN_HEAP_REST %x\n",size);
    }

    //描画モード戻し
    {
      const GFL_BG_SYS_HEADER bg_sys_header = 
        {
          GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
        };
      GFL_BG_SetBGMode( &bg_sys_header );
    }

    {
      FLDMSGBG *fmb = FIELDMAP_GetFldMsgBG( fieldmap );
      FIELD_PLACE_NAME *place_name_sys = FIELDMAP_GetPlaceNameSys( fieldmap );
      FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( fieldmap );
      FLDMSGBG_RecoveryBG( fmb );
      FIELD_PLACE_NAME_RecoverBG( place_name_sys );
      FIELD_DEBUG_RecoverBgCont( debug );
      //ＢＧ2面クリア
      GFL_BG_ClearFrame( GFL_BG_FRAME2_M );
    }
    
    //プライオリティの復帰
    PopPriority(ptr);
    //表示状態の復帰
    PopDisp(ptr);

    //ＯＢＪのポーズ解除
    {
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDLSYS_ClearPauseMoveProc(mmdlsys);
    }
    
    (*seq)++;
    break;
  case 8:
    //終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * パーティクル再生
 *
 * @param   ptr         カットイン管理ポインタ
 *
 * @return	BOOL        TRUEで再生終了
 */
//--------------------------------------------------------------------------------------------
static BOOL PlayParticle(FLD3D_CI_PTR ptr)
{
  if (!ptr->PartGene){
    //エミッター1つ目
    if ( ptr->Emit1==FALSE )
    {
      if ( ptr->PrtclWait[0] > 0 ){
        ptr->PrtclWait[0]--;
      }else{
        //パーティクルジェネレート
        Generate(ptr, 0);
        ptr->Emit1 = TRUE;
      }
    }

    //エミッター2つ目
    if ( ptr->Emit2==FALSE )
    {
      if ( ptr->PrtclWait[1] > 0 ){
        ptr->PrtclWait[1]--;
      }else{
        //パーティクルジェネレート
        Generate(ptr, 1);
        ptr->Emit2 = TRUE;
      }
    }

    if ( ptr->Emit1 && ptr->Emit2 ){
      ptr->PartGene = 1;
    }
  }else{
    //パーティクル終了判定
    if ( FLD_PRTCL_CheckEmitEnd( ptr->PrtclSys ) ){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * モデルアニメ1再生
 *
 * @param   ptr         カットイン管理ポインタ
 *
 * @return	BOOL        TRUEで再生終了
 */
//--------------------------------------------------------------------------------------------
static BOOL PlayMdlAnm1(FLD3D_CI_PTR ptr)
{
  //アニメがない場合は終了とみなす
  if (!ptr->Anm1Flg) return TRUE;

  if (ptr->MdlAnm1Wait > 0){
    ptr->MdlAnm1Wait--;
  }else{
    BOOL rc;
    u8 i;
    u16 num;
    GFL_G3D_OBJ* obj;
    u16 obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( ptr->Util, ptr->UnitIdx );
    obj = GFL_G3D_UTIL_GetObjHandle( ptr->Util, obj_idx );
    num = GFL_G3D_OBJECT_GetAnimeCount( obj );
    //アニメ再生
    for (i=0;i<num;i++){
      rc = GFL_G3D_OBJECT_IncAnimeFrame( obj, i, FX32_ONE );
    }
    if (rc == FALSE){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * モデルアニメ2再生
 *
 * @param   ptr         カットイン管理ポインタ
 *
 * @return	BOOL        TRUEで再生終了
 */
//--------------------------------------------------------------------------------------------
static BOOL PlayMdlAnm2(FLD3D_CI_PTR ptr)
{
  //アニメがない場合は終了とみなす
  if (!ptr->Anm2Flg) return TRUE;

  if (ptr->MdlAnm2Wait > 0){
    ptr->MdlAnm2Wait--;
  }else{
    //アニメ再生
    BOOL rc;
    u8 i;
    u16 num;
    GFL_G3D_OBJ* obj;
    u16 obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( ptr->Util, ptr->UnitIdx );
    obj = GFL_G3D_UTIL_GetObjHandle( ptr->Util, obj_idx+1 );
    num = GFL_G3D_OBJECT_GetAnimeCount( obj );
    //アニメ再生
    for (i=0;i<num;i++){
      rc = GFL_G3D_OBJECT_IncAnimeFrame( obj, i, FX32_ONE );
    }
    if (rc == FALSE){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * リソースセットアップ
 *
 * @param   ptr         カットイン管理ポインタ
 * @param   outDat      セットアップデータ格納バッファ
 * @param   inCutInNo   カットインナンバー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetupResource(FLD3D_CI_PTR ptr, RES_SETUP_DAT *outDat, const u8 inCutInNo)
{
  void *resource;
  //パーティクル生成
  ptr->PrtclPtr = FLD_PRTCL_Create(ptr->PrtclSys);
  //セットアップデータ作成
  CreateRes(outDat, inCutInNo, ptr->HeapID);
  
  //3Ｄモデルリソースをセットアップ
  SetupResourceCore(ptr, &outDat->Setup);
  //パーティクルリソースセットアップ
  if ( outDat->SpaIdx != NONDATA )
  {
    resource = FLD_PRTCL_LoadResource(ptr->PrtclSys,
			ARCID_FLD3D_CI, outDat->SpaIdx);

    FLD_PRTCL_SetResource(ptr->PrtclSys, resource, TRUE, NULL);
  }else
  {
    resource = NULL;
  }

  //アニメを有効にする
  {
    u8 i,j;
    u16 obj_num,anm_num;
    GFL_G3D_OBJ* obj;
    u16 obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( ptr->Util, ptr->UnitIdx );
    obj_num = GFL_G3D_UTIL_GetUnitObjCount( ptr->Util, ptr->UnitIdx );
    for(i=0;i<obj_num;i++){
      obj = GFL_G3D_UTIL_GetObjHandle( ptr->Util, obj_idx+i );
      anm_num = GFL_G3D_OBJECT_GetAnimeCount( obj );
      //アニメ有効
      for (j=0;j<anm_num;j++){
        GFL_G3D_OBJECT_EnableAnime( obj, j );
      }
    }
  }
  //ウェイト設定
  ptr->PrtclWait[0] = outDat->SpaWait[0];
  ptr->PrtclWait[1] = outDat->SpaWait[1];
  ptr->MdlAnm1Wait = outDat->MdlAAnmWait;
  ptr->MdlAnm2Wait = outDat->MdlBAnmWait;
  //リソースの中のｓｐｒの数で分岐
  {
    u16 res_num;
    if (resource != NULL) res_num = GFL_PTC_GetResNum( resource );
    else res_num = 0;

    NOZOMU_Printf("resnum=%d\n",res_num);
    if ( res_num == EMITCOUNT_MAX )
    {
      ptr->Emit1 = FALSE;
      ptr->Emit2 = FALSE;
    }
    else if( res_num == 1 )
    {
      ptr->Emit1 = FALSE;
      ptr->Emit2 = TRUE;
    }
    else
    {
      ptr->Emit1 = TRUE;
      ptr->Emit2 = TRUE;
    }
  }
  //ＯＢＪのアニメ数で再生処理をするかを判断する
  {
    ptr->Anm1Flg = FALSE;
    ptr->Anm2Flg = FALSE;
    if (outDat->Anm1Num != 0) ptr->Anm1Flg = TRUE;
    if (outDat->Anm2Num != 0) ptr->Anm2Flg = TRUE;
  }

  if (ptr->SetupCallBack != NULL )
  {
    ptr->SetupCallBack(ptr);
  }
}

//--------------------------------------------------------------------------------------------
/**
 * リソース削除
 *
 * @param   ptr         カットイン管理ポインタ
 * @param   ioDat      セットアップデータ格納バッファ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void DeleteResource(FLD3D_CI_PTR ptr, RES_SETUP_DAT *ioDat)
{
  DeleteRes(ioDat);
  GFL_G3D_UTIL_DelUnit( ptr->Util, ptr->UnitIdx );
  ptr->UnitIdx = UNIT_NONE;
  FLD_PRTCL_Delete(ptr->PrtclSys);
}

//--------------------------------------------------------------------------------------------
/**
 * パーティクルジェネレート
 *
 * @param   ptr         カットイン管理ポインタ
 * @param   inResNo     リソースナンバー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void Generate(FLD3D_CI_PTR ptr, const u32 inResNo)
{
///  GFL_PTC_SetCameraType(sys->PrtclPtr, GFL_G3D_PRJORTH);
	GFL_PTC_CreateEmitterCallback(ptr->PrtclPtr, inResNo,
									ParticleCallBack, NULL);                 
}

//--------------------------------------------------------------
/**
 * @brief	コールバック
 *
 * @param	emit	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void ParticleCallBack(GFL_EMIT_PTR emit)
{
///	SOFT_SPRITE *ss = Particle_GetTempPtr();
	VecFx32 pos = { 0, 0, 0 };
	
	pos.z = 0x40;
	SPL_SetEmitterPosition(emit, &pos);
}

//--------------------------------------------------------------
/**
 * @brief	リソース作成関数
 *
 * @param     outDat          セットアップデータ格納バッファ
 * @param     inResArcIdx     アーカイブインデックス
 * @param     inHeapID        ヒープＩＤ
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void CreateRes(RES_SETUP_DAT *outDat, const u8 inResArcIdx, const HEAPID inHeapID)
{
  u8 i;
  RES_DEF_DAT def_dat;
  u8 res_num,obj_num,anm1_num,anm2_num;
  u8 obj_res_start,anm1_res_start,anm2_res_start;

  //アーカイブからリソース定義をロード
  GFL_ARC_LoadData(&def_dat, ARCID_FLD3D_CI_SETUP, inResArcIdx);

  //OBJ数を調べる
  obj_num = 0;
  for (i=0;i<OBJCOUNT_MAX;i++){
    if (def_dat.MdlObjIdx[i] != NONDATA){
      obj_num++;
    }
  }
  //アニメ数を調べる
  anm1_num = 0;
  anm2_num = 0;
  for(i=0;i<ANM_TYPE_MAX;i++){
    if (def_dat.MdlAAnmIdx[i] != NONDATA){
      anm1_num++;
    }
  }
  for(i=0;i<ANM_TYPE_MAX;i++){
    if (def_dat.MdlBAnmIdx[i] != NONDATA){
      anm2_num++;
    }
  }

  obj_res_start = 0;
  anm1_res_start = obj_res_start + obj_num;
  anm2_res_start = anm1_res_start + anm1_num;

  //リソース数を調べる
  res_num = obj_num+anm1_num+anm2_num;
#if 0
  OS_Printf("obj_anm_res = %d %d %d %d\n",obj_num, anm1_num, anm2_num, res_num);
  OS_Printf("res_start = %d %d %d \n",obj_res_start,anm1_res_start,anm2_res_start );
#endif
  //リソーステーブルアロケート
  if (res_num){
    outDat->Res = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(GFL_G3D_UTIL_RES)*res_num);
  }
  //ＯＢＪテーブルアロケート
  if(obj_num){
    outDat->Obj = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(GFL_G3D_UTIL_OBJ)*obj_num);
  }
  //アニメテーブルアロケート
  if(anm1_num){
    outDat->Anm1 = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(GFL_G3D_UTIL_ANM)*anm1_num);
  }
  //アニメテーブルアロケート
  if(anm2_num){
    outDat->Anm2 = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(GFL_G3D_UTIL_ANM)*anm2_num);
  }

  //リソース数記憶
  outDat->ResNum = res_num;
  outDat->ObjNum = obj_num;
  outDat->Anm1Num = anm1_num;
  outDat->Anm2Num = anm2_num;
  //ウェイト記憶
  outDat->SpaWait[0] = def_dat.SpaWait[0];
  outDat->SpaWait[1] = def_dat.SpaWait[1];
  outDat->MdlAAnmWait = def_dat.MdlAAnmWait;
  outDat->MdlBAnmWait = def_dat.MdlBAnmWait;

  {
    u8 i;
    u8 count = 0;
    u16 arc_id[RESCOUNT_MAX];
    arc_id[0] = def_dat.MdlObjIdx[0];
    arc_id[1] = def_dat.MdlObjIdx[1];
    arc_id[2] = def_dat.MdlAAnmIdx[0];
    arc_id[3] = def_dat.MdlAAnmIdx[1];
    arc_id[4] = def_dat.MdlAAnmIdx[2];
    arc_id[5] = def_dat.MdlBAnmIdx[0];
    arc_id[6] = def_dat.MdlBAnmIdx[1];
    arc_id[7] = def_dat.MdlBAnmIdx[2];


    for (i=0;i<RESCOUNT_MAX;i++){
      if ( arc_id[i] != NONDATA ){
        outDat->Res[count].arcive = ARCID_FLD3D_CI;
        outDat->Res[count].datID = arc_id[i];
        outDat->Res[count].arcType = GFL_G3D_UTIL_RESARC;
        count++;
      }
    }
  }

  {
    u8 i;
    for (i=0;i<anm1_num;i++){
      outDat->Anm1[i].anmresID = anm1_res_start+i;
      outDat->Anm1[i].anmdatID = 0;
    }
    for (i=0;i<anm2_num;i++){
      outDat->Anm2[i].anmresID = anm2_res_start+i;
      outDat->Anm2[i].anmdatID = 0;
    }
  }

  {
    u8 i;
    for (i=0;i<obj_num;i++){
      outDat->Obj[i].mdlresID = obj_res_start+i;  
      outDat->Obj[i].mdldatID = 0;
      outDat->Obj[i].texresID = obj_res_start+i;
    }
    if (obj_num>=1){
      outDat->Obj[0].anmTbl = outDat->Anm1;
      outDat->Obj[0].anmCount = anm1_num;
    }
    if (obj_num>=2){
      outDat->Obj[1].anmTbl = outDat->Anm2;
      outDat->Obj[1].anmCount = anm2_num;
    }
  }


  outDat->Setup.resTbl = outDat->Res;
  outDat->Setup.resCount = res_num;
  outDat->Setup.objTbl = outDat->Obj;
  outDat->Setup.objCount = obj_num;

  outDat->SpaIdx = def_dat.SpaIdx;
}

//--------------------------------------------------------------
/**
 * @brief	リソース破棄関数
 *
 * @param	  ioDat   セットアップデータ格納バッファ
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void DeleteRes(RES_SETUP_DAT *ioDat)
{
  if (ioDat->Anm1 != NULL){
    GFL_HEAP_FreeMemory( ioDat->Anm1 );
  }
  if (ioDat->Anm2 != NULL){
    GFL_HEAP_FreeMemory( ioDat->Anm2 );
  }
  if (ioDat->Obj != NULL){
    GFL_HEAP_FreeMemory( ioDat->Obj );
  }
  if(ioDat->Res != NULL){
    GFL_HEAP_FreeMemory( ioDat->Res );
  }
}

#ifdef PM_DEBUG
#define FLYSKY_CAM_MOVE_FRAME (10)
//--------------------------------------------------------------
/**
 * @brief	空を飛ぶカメラアングルになるイベントを作成する
 * @todo　この機能はこのファイルから切り離す
 *
 * @param	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void FLD3D_CI_FlySkyCameraDebug(
    GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, FIELD_CAMERA *camera, FIELD_PLAYER * player, FLDNOGRID_MAPPER* mapper )
{
  FLYSKY_EFF_WORK *work;
  int size;
  size = sizeof(FLYSKY_EFF_WORK);
  //イベント作成
  {
    GMEVENT * event = GMEVENT_Create( gsys, NULL, DebugFlySkyEffEvt, size );

    work = GMEVENT_GetEventWork(event);
    MI_CpuClear8( work, size );
    work->gsys = gsys;
    work->camera = camera;
    work->CiPtr = ptr;
    work->player = player;
    work->mapper = mapper;

    GAMESYSTEM_SetEvent(gsys, event);
  }
}

static GMEVENT_RESULT DebugFlySkyEffEvt( GMEVENT* event, int* seq, void* work )
{
  FLYSKY_EFF_WORK *wk;
  wk = work;

  switch(*seq){
  case 0:
    //カメラエリア動作をフック
    FIELD_CAMERA_SetCameraAreaActive( wk->camera, FALSE );
    //現在ウォッチターゲットを保存
    wk->Watch = FIELD_CAMERA_GetWatchTarget(wk->camera);
    FIELD_CAMERA_StopTraceRequest(wk->camera);
    (*seq)++;
    break;
  case 1:
    if ( (wk->Watch != NULL) && FIELD_CAMERA_CheckTrace(wk->camera) ){
      break;
    }
    //レールシステム更新ストップ
    FLDNOGRID_MAPPER_SetRailCameraActive( wk->mapper, FALSE );
    //カメラパージ
    FIELD_CAMERA_FreeTarget(wk->camera);
    //現在のカメラ設定を保存
    FIELD_CAMERA_SetRecvCamParam(wk->camera);
    //カメラを空飛びカメラへ線形移動リクエスト
    {
      FLD_CAM_MV_PARAM param;
      VecFx32 player_vec;
      //主人公の3Ｄ座標を取得
      FIELD_PLAYER_GetPos( wk->player, &player_vec );
      param.Core.AnglePitch = 9688;
      param.Core.AngleYaw = 0;
      param.Core.Distance = 970752;
      param.Core.Shift.x = 0;
      param.Core.Shift.y = 16384;
      param.Core.Shift.z = 0;
      param.Core.TrgtPos = player_vec;
      param.Core.Fovy = 3640;
      param.Chk.Shift = TRUE;
      param.Chk.Pitch = TRUE;
      param.Chk.Yaw = FALSE;
      param.Chk.Dist = TRUE;
      param.Chk.Fovy = TRUE;
      param.Chk.Pos = TRUE;
      NOZOMU_Printf("plyer_pos = %d,%d,%d\n",player_vec.x, player_vec.y, player_vec.z);
      FIELD_CAMERA_SetLinerParam(wk->camera, &param, FLYSKY_CAM_MOVE_FRAME);
    }
    (*seq)++;
    /*none break*/
  case 2:
    //カメラ移動待ち
    if ( !FIELD_CAMERA_CheckMvFunc(wk->camera) )
    {
      GMEVENT *child;
#if 0      
      {
        u16 pitch, yaw;
        fx32 len;
        u16 fovy;
        VecFx32 shift, target;
        OS_Printf("--DUMP_FIELD_CAMERA_PARAM--\n");
        pitch = FIELD_CAMERA_GetAnglePitch(wk->camera);
        yaw = FIELD_CAMERA_GetAngleYaw(wk->camera);
        len = FIELD_CAMERA_GetAngleLen(wk->camera);
        fovy = FIELD_CAMERA_GetFovy(wk->camera);
        FIELD_CAMERA_GetTargetOffset( wk->camera, &shift );
        FIELD_CAMERA_GetTargetPos( wk->camera, &target );
      
        OS_Printf("%d,%d,%d,%d,%d,%d\n", pitch, yaw, len, target.x, target.y, target.z );
        OS_Printf("%d,%d,%d,%d\n", fovy, shift.x, shift.y, shift.z );
      }
#endif      
      //カットイン演出開始
      child = FLD3D_CI_CreateCutInEvt(wk->gsys, wk->CiPtr, 0);
      GMEVENT_CallEvent(event, child);
      (*seq)++;
    }
    break;
  case 3:
    {
      FLD_CAM_MV_PARAM_CHK chk;
      chk.Shift = TRUE;
      chk.Pitch = TRUE;
      chk.Yaw = FALSE;
      chk.Dist = TRUE;
      chk.Fovy = TRUE;
      chk.Pos = TRUE;
      //カメラ戻し
      FIELD_CAMERA_RecvLinerParam(wk->camera, &chk, FLYSKY_CAM_MOVE_FRAME);
    }
    (*seq)++;
    /*none break*/
  case 4:
    //カメラ戻し待ち
    if ( !FIELD_CAMERA_CheckMvFunc(wk->camera) ){
      //カメラバインド
      if (wk->Watch != NULL){
        FIELD_CAMERA_BindTarget(wk->camera, wk->Watch);
      }
      FIELD_CAMERA_RestartTrace(wk->camera);
      //レールシステム更新開始
      FLDNOGRID_MAPPER_SetRailCameraActive( wk->mapper, TRUE );
      //復帰データのクリア
      FIELD_CAMERA_ClearRecvCamParam(wk->camera);
      //カメラエリア動作をフック解除
      FIELD_CAMERA_SetCameraAreaActive( wk->camera, TRUE );
      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}

#endif  //PM_DEBUG

//----------------------------------------------------------------------------
/**
 *	@brief	キャプチャリクエスト
 *
 *	@param	カットイン管理ポインタ
 *
 *	@return none
 */
//-----------------------------------------------------------------------------
static void ReqCapture(FLD3D_CI_PTR ptr)
{
  ptr->CapEndFlg = FALSE;
  GX_SetBankForLCDC(GX_VRAM_LCDC_D);
  GFUser_VIntr_CreateTCB(Graphic_Tcb_Capture, &ptr->CapEndFlg, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vブランクタスク
 *
 *	@param	GFL_TCB *p_tcb
 *	@param	*p_work 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void Graphic_Tcb_Capture( GFL_TCB *p_tcb, void *p_work )
{
  BOOL *cap_end_flg = p_work;
  
	GX_SetCapture(GX_CAPTURE_SIZE_256x192,  // Capture size
                      GX_CAPTURE_MODE_A,			   // Capture mode
                      GX_CAPTURE_SRCA_3D,						 // Blend src A
                      GX_CAPTURE_SRCB_VRAM_0x00000,     // Blend src B
                      GX_CAPTURE_DEST_VRAM_D_0x00000,   // Output VRAM
                      16,             // Blend parameter for src A
                      0);            // Blend parameter for src B

  GFL_TCB_DeleteTask( p_tcb );
  if (cap_end_flg != NULL){
    *cap_end_flg = TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	プライオリティプッシュ
 *
 *	@param	ptr       カットイン管理ポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PushPriority(FLD3D_CI_PTR ptr)
{
  int i;
  for(i=0;i<4;i++){
    ptr->BgPriority[i] = GFL_BG_GetPriority( i );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	表示状態プッシュ
 *
 *	@param	ptr       カットイン管理ポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PushDisp(FLD3D_CI_PTR ptr)
{
  ptr->Mask = GFL_DISP_GetMainVisible();
}

//----------------------------------------------------------------------------
/**
 *	@brief	プライオリティポップ
 *
 *	@param	ptr       カットイン管理ポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PopPriority(FLD3D_CI_PTR ptr)
{
  int i;
  for(i=0;i<4;i++){
    GFL_BG_SetPriority( i, ptr->BgPriority[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	表示状態ポップ
 *
 *	@param	ptr       カットイン管理ポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PopDisp(FLD3D_CI_PTR ptr)
{
  GFL_DISP_GX_SetVisibleControlDirect(ptr->Mask);
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモングラフィックに書き換え
 *
 *	@param	ptr       カットイン管理ポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void ReTransToPokeGra(FLD3D_CI_PTR ptr)
{
  u32 tex_adr;
  u32 pal_adr;
  HEAPID heapID = ptr->HeapID;
  ptr->pokeImgBitmap = GFL_BMP_Create(TEXSIZ_S/8, TEXSIZ_T/8, GFL_BMP_16_COLOR, heapID);
  //リソースのＶＲＡＭアドレス取得
  {
    GFL_G3D_RES* res = GFL_G3D_UTIL_GetResHandle( ptr->Util, 0 );
    NNSG3dTexKey texkey = GFL_G3D_GetTextureDataVramKey( res );
    NNSG3dPlttKey palkey = GFL_G3D_GetTexturePlttVramKey( res );
    tex_adr = NNS_GfdGetTexKeyAddr(texkey);
    pal_adr = NNS_GfdGetPlttKeyAddr(palkey);
    NOZOMU_Printf("key = %d %d\n",texkey, palkey);
    NOZOMU_Printf("adr = %x %x\n",tex_adr, pal_adr);
    NOZOMU_Printf("resid = %d\n",GFL_G3D_UTIL_GetUnitResIdx( ptr->Util,0 ) );
    
  }
  //ポケモングラフィック取得
  {
    u32 cgr, pal;
    //リソース受け取り
    cgr	= POKEGRA_GetCgrArcIndex( ptr->MonsNo,ptr->FormNo,ptr->Sex,ptr->Rare,POKEGRA_DIR_FRONT, ptr->Egg );
    pal = POKEGRA_GetPalArcIndex( ptr->MonsNo,ptr->FormNo,ptr->Sex,ptr->Rare,POKEGRA_DIR_FRONT, ptr->Egg );
    ptr->chr = NULL;
    ptr->plt = NULL;
    //リソースはOBJとして作っているので、LoadOBJじゃないと読み込めない
    ptr->chr_buf = GFL_ARC_UTIL_LoadOBJCharacter( POKEGRA_GetArcID(), cgr, FALSE, &ptr->chr, heapID );
    ptr->pal_buf = GFL_ARC_UTIL_LoadPalette( POKEGRA_GetArcID(), pal, &ptr->plt, heapID );
  }
  //12x12chrポケグラを16x16chrの真ん中に貼り付ける
  {
    u8	*src, *dst;
    int x, y;
    int	chrNum = 0;

    src = ptr->chr->pRawData;
		dst = GFL_BMP_GetCharacterAdrs(ptr->pokeImgBitmap);
    GFL_STD_MemClear32((void*)dst, 16*16*0x20);

    dst += ( (2*16+2) * 0x20 );
    //キャラデータは8x8、4x8、8x4、4x4の順番で1Dマッピングされている
    for (y=0; y<8; y++){
      for(x=0; x<8; x++){
        GFL_STD_MemCopy32((void*)(&src[chrNum*0x20]), (void*)(&dst[(y*16+x)*0x20]), 0x20);
        chrNum++;
      }
    }
    for (y=0; y<8; y++){
      for(x=8; x<12; x++){
        GFL_STD_MemCopy32((void*)(&src[chrNum*0x20]), (void*)(&dst[(y*16+x)*0x20]), 0x20);
        chrNum++;
      }
    }
    for (y=8; y<12; y++){
      for(x=0; x<8; x++){
        GFL_STD_MemCopy32((void*)(&src[chrNum*0x20]), (void*)(&dst[(y*16+x)*0x20]), 0x20);
        chrNum++;
      }
    }
    for (y=8; y<12; y++){
      for(x=8; x<12; x++){
        GFL_STD_MemCopy32((void*)(&src[chrNum*0x20]), (void*)(&dst[(y*16+x)*0x20]), 0x20);
        chrNum++;
      }
    }
  }
  //16x16chrフォーマットデータをＢＭＰデータに変換
  GFL_BMP_DataConv_to_BMPformat(ptr->pokeImgBitmap, FALSE, heapID);
  //転送
  {
    BOOL rc;
    void*	src;
		u32		dst;
    src = (void*)GFL_BMP_GetCharacterAdrs(ptr->pokeImgBitmap);
		dst = tex_adr;
    rc = NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_VRAM, dst, src, TEXVRAMSIZ);
    GF_ASSERT(rc);
    src = ptr->plt->pRawData;
		dst = pal_adr;
    rc = NNS_GfdRegisterNewVramTransferTask(NNS_GFD_DST_3D_TEX_PLTT, dst, src, 32);
    GF_ASSERT(rc);
  }

  {
    int size = GFL_HEAP_GetHeapFreeSize(ptr->HeapID);
    NOZOMU_Printf("::FLD3DCUTIN_HEAP_REST %x\n",size);
  }
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモン鳴き声フェードアウトイベント
 *
 * @param   event       イベントポインタ
 * @param   *seq        シーケンサ
 * @param   work        ワークポインタ
 *
 * @return	GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT VoiceFadeOutEvt( GMEVENT* event, int* seq, void* work )
{
  FLD3D_CI_PTR ptr;
  FLD3D_CI_EVENT_WORK *evt_work;

  s8 volume;

  //親イベントからワークポインタを取得
  {
    GMEVENT * parent = GMEVENT_GetParentEvent(event);
    evt_work = GMEVENT_GetEventWork(parent);
    ptr = evt_work->CiPtr;
  }

  //鳴き声なっていないなら、終了
  if ( !PMV_CheckPlay() ) return GMEVENT_RES_FINISH;

  volume = PMVOICE_GetVolume( ptr->VoicePlayerIdx );

  if ( volume > 0){
    volume -= VOICE_VOL_OFS;
    if ( volume < 0 ) volume = 0;

    PMV_SetVolume( ptr->VoicePlayerIdx, volume );
  }
  else
  {
    PMV_StopVoice();
    return GMEVENT_RES_FINISH;
  }
  
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * ホワイトアウトイベント
 *
 * @param   event       イベントポインタ
 * @param   *seq        シーケンサ
 * @param   work        ワークポインタ
 *
 * @return	GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT WhiteOutEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK * fieldmap;
  gsys = GMEVENT_GetGameSysWork(event);
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  switch(*seq){
  case 0:
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, FLYSKY_WHITE_FADE_SPD );
    (*seq)++;
    break;
  case 1:
    if ( GFL_FADE_CheckFade() == FALSE )
    {
      //自機を非表示
      MMDL * mmdl;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
      mmdl = FIELD_PLAYER_GetMMdl( fld_player );
      MMDL_SetStatusBitVanish(mmdl, TRUE);
      (*seq)++;
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * ホワイトインイベント
 *
 * @param   event       イベントポインタ
 * @param   *seq        シーケンサ
 * @param   work        ワークポインタ
 *
 * @return	GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT WhiteInEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK * fieldmap;
  gsys = GMEVENT_GetGameSysWork(event);
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  switch(*seq){
  case 0:
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, FLYSKY_WHITE_FADE_SPD );
    (*seq)++;
    break;
  case 1:
    if ( GFL_FADE_CheckFade() == FALSE )
    {
      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 空を飛ぶアウトカットインか？
 *
 * @param   einCutinNo
 *
 * @return	BOOL    TRUE:アウトカットイン
 */
//--------------------------------------------------------------------------------------------
static BOOL IsFlySkyOut(const int inCutinNo)
{
  if (inCutinNo == FLDCIID_FLY_OUT) return TRUE;

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 空を飛ぶインカットインか？
 *
 * @param   einCutinNo
 *
 * @return	BOOL    TRUE:インカットイン
 */
//--------------------------------------------------------------------------------------------
static BOOL IsFlySkyIn(const int inCutinNo)
{
  if (inCutinNo == FLDCIID_FLY_IN) return TRUE;

  return FALSE;
}




