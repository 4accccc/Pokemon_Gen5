//======================================================================
/**
 * @file	fld3d_ci.c
 * @brief	フィールド3Dカットイン
 * @author	Saito
 *
 */
//======================================================================
#define	POKEGRA_LZ	// ポケグラ圧縮有効定義

#include "fld3d_ci.h"
#include "arc_def.h"
#include "gamesystem/game_event.h"
#include "field/field_msgbg.h"    //for FLDMSGBG_RecoveryBG
#include "system/pokegra.h"

#include "../../../resource/fld3d_ci/fldci_id_def.h"

#include "sound/pm_wb_voice.h"   //for PMV_PlayVoice
#include "sound/pm_sndsys.h"

#include "message.naix"
#include "msg/msg_sptr_name.h"
#include "system/font2tex.h"
#include "font/font.naix"

#include "enc_cutin_no.h"
#include "../../arc/enc_ci_face.naix"

#include "fldmmdl.h"  //for MMDLSYS
#include "fieldmap.h" //for FIELDMAP_～

#include "event_field_fade.h"   //for EVENT_FlySkyBrightIn

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

#define VOICE_TIME (30)
#define POKE_VOICE_WAIT (5)
#define VOICE_VOL_OFS (16)

#define CUTIN_WHITE_FADE_SPD (-1)

#define ENC_CUTIN_MDL_Z_OFS (700)
#if 0
#define FLYSKY_SE_FRAME (24)      //空を飛ぶＳＥ再生タイミング
#define CHAMP_FLYSKY_SE_FRAME (24)      //チャンピオン空を飛ぶＳＥ再生タイミング
#endif
#define CI_SE_NONE   (0xffffffff)

typedef GMEVENT_RESULT (*SETUP_CALLBACK)( GMEVENT* event, int* seq, void* work );
typedef BOOL (*MAIN_SEQ_FUNC)( GMEVENT* event, FLD3D_CI_PTR ptr );

typedef struct CI_SE_TBL_tag
{
  u32 Se;
  u32 Frame;
}CI_SE_TBL;

typedef struct NPC_FLY_WORK_tag
{
  int ObjID;
}NPC_FLY_WORK;

typedef struct POKE_WORK_tag
{
  int MonsNo;
  int FormNo;
  int Sex;
  int Rare;
  BOOL Egg;

  BOOL VoicePlayFlg;
  u16 SePlayWait;
  u16 VoiceTime;
  u32 VoicePlayerIdx;
}POKE_WORK;

typedef struct ENC_WORK_tag
{
  int MsgIdx;
  int ChrArcIdx;
  int PltArcIdx;
  BOOL PlayerTrans;
}ENC_WORK;

typedef struct FLD3D_CI_tag
{
  FLD_PRTCL_SYS_PTR PrtclSys;
  GFL_G3D_CAMERA *g3Dcamera;
  GFL_G3D_CAMERA *g3DcameraOrth;
  int CameraMode;
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
  BOOL DispSetEndFlg;
  BOOL Emit1;
  BOOL Emit2;
  BOOL Anm1Flg;
  BOOL Anm2Flg;

  //転送画像展開用
  NNSG2dCharacterData * chr;
  NNSG2dPaletteData* plt;
  GFL_BMP_DATA* ImgBitmap;
  void *chr_buf;
  void *pal_buf;

  u8 *Work[32];
  BOOL PtclEnd;
  BOOL MdlAnm1End;
  BOOL MdlAnm2End;

  SETUP_CALLBACK SetupCallBack;
  MAIN_SEQ_FUNC MainSeqFunc;
  int SubSeq;
  u16 FrameCount;
  u16 SeTblIdx;
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
  u32 Bright;
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
  u32 Bright;
}RES_SETUP_DAT;

typedef struct {
//  GAMESYS_WORK * gsys;
  FLD3D_CI_PTR CiPtr;
  RES_SETUP_DAT SetupDat;
  BOOL ObjPause;
  BOOL MainHook;
  BOOL IsWhiteOut;
}FLD3D_CI_EVENT_WORK;

static void SetupResource(GMEVENT* event, FLD3D_CI_PTR ptr, RES_SETUP_DAT *outDat, const u8 inCutInNo);
static void SetupResourceCore(FLD3D_CI_PTR ptr, const GFL_G3D_UTIL_SETUP *inSetup);
static void DeleteResource(FLD3D_CI_PTR ptr, RES_SETUP_DAT *ioDat);
static BOOL PlayParticle(FLD3D_CI_PTR ptr);
static BOOL PlayMdlAnm1(FLD3D_CI_PTR ptr);
static BOOL PlayMdlAnm2(FLD3D_CI_PTR ptr);

static GMEVENT_RESULT CutInEvt( GMEVENT* event, int* seq, void* work );
//static GMEVENT_RESULT VoiceFadeOutEvt( GMEVENT* event, int* seq, void* work );

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

static GMEVENT_RESULT PokeGraTransEvt( GMEVENT* event, int* seq, void* work );
static BOOL VoiceMain(GMEVENT* event, FLD3D_CI_PTR ptr);
static void ReTransToPokeGra(FLD3D_CI_PTR ptr);
static BOOL EncFadeMain(GMEVENT* event, FLD3D_CI_PTR ptr);

static GMEVENT_RESULT WhiteOutEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT WhiteInEvt( GMEVENT* event, int* seq, void* work );
static GMEVENT_RESULT FlySkyBlackInEvt( GMEVENT* event, int* seq, void* work );
//static GMEVENT_RESULT WhiteOutEvtNpc( GMEVENT* event, int* seq, void* work );

static BOOL IsFlySkyOut(const int inCutinNo);
static BOOL IsFlySkyIn(const int inCutinNo);

static void* GetTexStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictTexData* pData);
static void* GetTexStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name);
static void* GetPlttStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictPlttData* pData);
static void* GetPlttStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name);
void SetResName(NNSG3dResName *outName,const char *inName);
static void ReTransToGra( FLD3D_CI_PTR ptr,
                          const char *inTexName,
                          const char *inPlttName,
                          const int inChrArcID,
                          const int inPltArcID );
static GMEVENT_RESULT EncEffGraTransEvt( GMEVENT* event, int* seq, void* work );

static void SetFont2Tex(MYSTATUS *mystatus, FLD3D_CI_PTR ptr, const char* inTexName, const char* inPltName, const int inMsgIdx );

//static BOOL FlySkyMainFunc(GMEVENT* event, FLD3D_CI_PTR ptr);
static void ChangeCapDatCol(void);
static void InitCutinEvtWork(FLD3D_CI_PTR ptr, FLD3D_CI_EVENT_WORK *work);

static void PlaySE(FLD3D_CI_PTR ptr);

static void DispSetVTask( GFL_TCB *p_tcb, void *p_work );

#define DEF_CAM_NEAR	( 1 << FX32_SHIFT )
#define DEF_CAM_FAR	( 1024 << FX32_SHIFT )

FS_EXTERN_OVERLAY(enc_cutin_no);

#include "../../../resource/fld3d_ci_se/ci_se_tbl.cdat"

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

  GF_ASSERT_MSG(FLDCIID_MAX == CI_SE_TBL_MAX,"ci_def error");
  
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

  {
    VecFx32 pos = {0, 0, FX32_ONE*(128)};
    VecFx32 target = {0,0,0};

    VecFx32 defaultCameraUp = { 0, FX32_ONE, 0 };

    ptr->g3DcameraOrth = GFL_G3D_CAMERA_Create(	GFL_G3D_PRJORTH, 
									FX32_ONE*48.0f,
                  -(FX32_ONE*48.0f),
                  -(FX32_ONE*64.0f),
                  FX32_ONE*64.0f,
									DEF_CAM_NEAR, DEF_CAM_FAR, 0,
									&pos, &defaultCameraUp, &target, inHeapID );
  }

  ptr->CameraMode = GFL_G3D_PRJPERS;

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
  GFL_G3D_CAMERA_Delete(ptr->g3DcameraOrth);
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
  GFL_G3D_CAMERA *camera;

  if ( ptr->CameraMode == GFL_G3D_PRJPERS ) camera = ptr->g3Dcamera;
  else camera = ptr->g3DcameraOrth;
  
  GFL_G3D_CAMERA_Switching( camera );
  {
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
  event = FLD3D_CI_CreatePokeCutInEvt( gsys, ptr, 3, 0, 0, FALSE, FALSE );

  GAMESYSTEM_SetEvent(gsys, event);
}

//エンカウントカットイン呼び出し
void FLD3D_CI_CallEncCutIn( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr )
{
  GMEVENT * event;
  int no = ENC_CUTIN_RIVAL;
  event = FLD3D_CI_CreateEncCutInEvt(gsys, ptr, no, FALSE);

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
  ptr->SubSeq = 0;
  ptr->FrameCount = 0;
  ptr->SeTblIdx = 0;
  ptr->CutInNo = inCutInNo;
  //セットアップ後コールバックなしでセットする
  ptr->SetupCallBack = NULL;
  ptr->MainSeqFunc = NULL;
  //透視射影
  ptr->CameraMode = GFL_G3D_PRJPERS;

  //イベント作成
  {
    event = GMEVENT_Create( gsys, NULL, CutInEvt, size );
    work = GMEVENT_GetEventWork(event);
    InitCutinEvtWork(ptr, work);
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
  ptr->SetupCallBack = PokeGraTransEvt;
  ptr->MainSeqFunc = VoiceMain;
  //ポケモンカットイン用ポケモン指定変数セット
  {
    POKE_WORK *poke_work;
    poke_work = (POKE_WORK*)ptr->Work;
    poke_work->MonsNo = inMonsNo;
    poke_work->FormNo = inFormNo;
    poke_work->Sex = inSex;
    poke_work->Rare = inRare;
    poke_work->Egg  = inEgg;

    poke_work->SePlayWait = POKE_VOICE_WAIT;
    poke_work->VoicePlayFlg = TRUE;
    poke_work->VoiceTime = VOICE_TIME;
  }

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
 * エンカウントカットインイベント作成
 *
 * @param   gsys        ゲームシステムポインタ
 * @param   ptr         カットイン管理ポインタ
 * @apram   inEncCutinNo    エンカウントカットインナンバー
 * @param   inIsWhiteFade   カットイン終了時のフェードはホワイトアウトか？  TRUEで白　FALSEで黒
 *
 * @return	event       イベントポインタ
 */
//--------------------------------------------------------------------------------------------
GMEVENT *FLD3D_CI_CreateEncCutInEvt(  GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr,
                                      const int inEncCutinNo, const BOOL inWhiteFade )
{
  GMEVENT * event;
  const ENC_CUTIN_DAT *dat;

  //オーバーレイロード
  GFL_OVERLAY_Load( FS_OVERLAY_ID(enc_cutin_no) ); 
  
  dat = ENC_CUTIN_NO_GetDat(inEncCutinNo);

  event = FLD3D_CI_CreateCutInEvt(gsys, ptr, dat->CutinNo);
  
  {
    FLD3D_CI_EVENT_WORK *work;
    work = GMEVENT_GetEventWork(event);
    InitCutinEvtWork(ptr, work);
    work->IsWhiteOut = inWhiteFade;
  }

  //コールバックセット
  if ( dat->TransType != GRA_TRANS_NONE ) ptr->SetupCallBack = EncEffGraTransEvt;
  else ptr->SetupCallBack = NULL;
  ptr->MainSeqFunc = EncFadeMain;
  //カメラモードセット正射影
  ptr->CameraMode = GFL_G3D_PRJORTH;

  //カットインナンバーから、転送データ等を確定する
  {
    ENC_WORK *wk;
    wk = (ENC_WORK*)ptr->Work;
    wk->MsgIdx = dat->MsgIdx;
    wk->ChrArcIdx = dat->ChrArcIdx;
    wk->PltArcIdx = dat->PltArcIdx;
    if ( dat->TransType == GRA_TRANS_ALL ) wk->PlayerTrans = TRUE;
    else wk->PlayerTrans = FALSE;
  }

  //オーバーレイアンロード
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(enc_cutin_no) );
  
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
    if (evt_work->ObjPause)
    {
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDLSYS_PauseMoveProc(mmdlsys);
    }
    
    //プライオリティの保存
    PushPriority(ptr);
    //表示状態の保存
    PushDisp(ptr);
    //カットイン共通ホワイトアウト処理　（空飛びイン以外）
    if ( !IsFlySkyIn(ptr->CutInNo) )
    {
      GMEVENT* fade_event;
      fade_event = GMEVENT_Create(gsys, event, WhiteOutEvt, 0);
      GMEVENT_CallEvent(event, fade_event);
    }
#if 0
    if ( IsFlySkyOut(ptr->CutInNo) )
    {
//      GMEVENT* fade_event;
      //メイン処理書き換え
      ptr->MainSeqFunc = FlySkyMainFunc;
/**
      if ( (ptr->CutInNo == FLDCIID_FLY_OUT)||(ptr->CutInNo == FLDCIID_FLY_OUT2) )
      {
        fade_event = GMEVENT_Create(gsys, event, WhiteOutEvt, 0);
      }
      else
      {
        fade_event = GMEVENT_Create(gsys, event, WhiteOutEvtNpc, 0);
      }
      GMEVENT_CallEvent(event, fade_event);
*/      
    }
#endif    
    (*seq)++;
    break;
  case 1:   //そらを飛ぶカットインシーケンス
    (*seq)++;
    if ( (ptr->CutInNo == FLDCIID_FLY_OUT)||(ptr->CutInNo == FLDCIID_FLY_OUT2) )
    {
      //自機を非表示
      MMDL * mmdl;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
      mmdl = FIELD_PLAYER_GetMMdl( fld_player );
      MMDL_SetStatusBitVanish(mmdl, TRUE);
      break;
    }else if (ptr->CutInNo == FLDCIID_CHAMP_OUT){
      //指定OBJを消す
      NPC_FLY_WORK *fly_work = (NPC_FLY_WORK*)ptr->Work;
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      u16 id = fly_work->ObjID;
      MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
      if ( mmdl != NULL ) MMDL_Delete( mmdl );
//      else GF_ASSERT_MSG( 0,"OBJ DEL 対象のOBJが居ません  %d\n",id );
      break;
    }
    //no break
  case 2:
    {
      //メイン処理フック
      if ( evt_work->MainHook )
      {
        FIELDMAP_SetMainFuncHookFlg(fieldmap, TRUE);
      }
      //キャプチャリクエスト
      ReqCapture(ptr);
      (*seq)++;
    }
    break;
  case 3:
    //キャプチャ終了待ち
    if (ptr->CapEndFlg){
      //キャプチャ終わったら、プライオリティ変更と表示変更
      GFL_BG_SetPriority( 0, 0 );
      GFL_BG_SetPriority( 2, 1 );
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
  case 4:
    //リソースロード
    SetupResource(event, ptr, &evt_work->SetupDat, ptr->CutInNo);
    //セットアップ終了したので3Ｄ面もオン
    GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );
    {
      int size = GFL_HEAP_GetHeapFreeSize(ptr->HeapID);
      NOZOMU_Printf("START::FLD3DCUTIN_HEAP_REST %x\n",size);
    }
    //フィールド表示モード切替
    FIELDMAP_SetDraw3DMode(fieldmap, DRAW3DMODE_CUTIN);
    
    //輝度変更する必要がある場合セット
    if ( evt_work->SetupDat.Bright ){
      //キャプチャした画像にアクセス
      ChangeCapDatCol();
    }
    (*seq)++;
    break;
  case 5:
    {
      GMEVENT* fade_event;
      if ( IsFlySkyIn(ptr->CutInNo) )
      {
        //フェードインイベント
        fade_event = GMEVENT_Create(gsys, event, FlySkyBlackInEvt, 0);
      }
      else
      {
        //フェードインイベント
        fade_event = GMEVENT_Create(gsys, event, WhiteInEvt, 0);
      }
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq)++;
    break;
  case 6: //ＢＧＭロード完了待ち
    if ( PMSND_IsLoading() ) break;    //ロード中なら TRUE
    
    (*seq)++;   //次のシーケンスへ
    //not break;
  case 7:
    {
      BOOL rc1,rc2,rc3,main_rc;
      rc1 = FALSE;
      rc2 = FALSE;
      rc3 = FALSE;

      //フレームカウント
      ptr->FrameCount++;
      //パーティクル再生
      rc1 = PlayParticle(ptr);
      //3Ｄモデル1アニメ再生
      rc2 = PlayMdlAnm1(ptr);
      //3Ｄモデル2アニメ再生
      rc3 = PlayMdlAnm2(ptr);

      ptr->PtclEnd = rc1;
      ptr->MdlAnm1End = rc2;
      ptr->MdlAnm2End = rc3;

      //ＳＥ再生
      PlaySE(ptr);

      main_rc = FALSE;
      if (ptr->MainSeqFunc != NULL) main_rc = ptr->MainSeqFunc(event, ptr);
      else
      {
        if (ptr->PtclEnd&&ptr->MdlAnm1End&&ptr->MdlAnm2End) main_rc = TRUE;
      }

      if ( main_rc ) (*seq)++;
    }
    break;
  case 8:
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
      //自機下向き
      MMDL_SetDirDisp( mmdl, DIR_DOWN );

      //戻ってきた場所はおそらく3Ｄクリアカラーが見えない場所のはずなので
      //3Ｄ面オフの処理とクリアカラーのアルファセットの処理を行わない

      //自機描画するために空を飛ぶインのときはここでメインフック解除
      if ( evt_work->MainHook )
      {
        FIELDMAP_SetMainFuncHookFlg(fieldmap, FALSE);
      }
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
  case 9:
    //バックカラー設定
    {
      FIELD_LIGHT *light = FIELDMAP_GetFieldLight( fieldmap );
      FIELD_LIGHT_Reflect( light, TRUE );

      //タスク登録
      ptr->DispSetEndFlg = FALSE;
      GFUser_VIntr_CreateTCB(DispSetVTask, &ptr->DispSetEndFlg, 0 );
      (*seq)++;
    }
    break;
  case 10:
    //タスクが終了していたら次のシーケンスを実行
    if ( ptr->DispSetEndFlg == FALSE ) break;

    {
      int size = GFL_HEAP_GetHeapFreeSize(ptr->HeapID);
      NOZOMU_Printf("END::FLD3DCUTIN_HEAP_REST %x\n",size);
    }

/**    
    //3D面をオン
    GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );

    //描画モード戻し
    {
      const GFL_BG_SYS_HEADER bg_sys_header = 
        {
          GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
        };
      GFL_BG_SetBGMode( &bg_sys_header );
    }
*/
    {
      FLDMSGBG *fmb = FIELDMAP_GetFldMsgBG( fieldmap );
      FIELD_PLACE_NAME *place_name_sys = FIELDMAP_GetPlaceNameSys( fieldmap );

      FLDMSGBG_RecoveryBG( fmb );
      FIELD_PLACE_NAME_RecoverBG( place_name_sys );

#ifndef  PM_DEBUG
      FLDMSGBG_ReqResetBG2( fmb );  //BG2復帰リクエスト（実際の復帰はFLDMSGBGのメインで実行）
#else
      { //実行内容は↑FLDMSGBG_ReqResetBG2＋デバッグ描画OFF
        FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( fieldmap );
        FIELD_DEBUG_RecoverBgCont( debug );
      }
#endif
      {
        //ＢＧセットアップ
        G2_SetBG2ControlText(
            GX_BG_SCRSIZE_TEXT_256x256,
            FLDBG_MFRM_EFF1_COLORMODE,
            FLDBG_MFRM_EFF1_SCRBASE,
            FLDBG_MFRM_EFF1_CHARBASE
            );
      }
      //ＢＧ2面クリア
      GFL_BG_ClearFrame( GFL_BG_FRAME2_M );
    }
    
    //プライオリティの復帰
    PopPriority(ptr);
    //表示状態の復帰
    PopDisp(ptr);
    //メインフック解除
    if ( evt_work->MainHook )
    {
      FIELDMAP_SetMainFuncHookFlg(fieldmap, FALSE);
    }    
    //ＯＢＪのポーズ解除
    if (evt_work->ObjPause)
    {
      MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
      MMDLSYS_ClearPauseMoveProc(mmdlsys);
    }
    
    (*seq)++;
    break;
  case 11:
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
 * @param   event       イベントポインタ
 * @param   ptr         カットイン管理ポインタ
 * @param   outDat      セットアップデータ格納バッファ
 * @param   inCutInNo   カットインナンバー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetupResource(GMEVENT* event, FLD3D_CI_PTR ptr, RES_SETUP_DAT *outDat, const u8 inCutInNo)
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
    GMEVENT * call_event;
    GAMESYS_WORK * gsys;
    gsys = GMEVENT_GetGameSysWork(event);
    call_event = GMEVENT_Create(gsys, event, ptr->SetupCallBack, 0);
    GMEVENT_CallEvent(event, call_event);
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
//  GFL_PTC_SetCameraType(ptr->PrtclPtr, GFL_G3D_PRJORTH);
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
  outDat->Bright = def_dat.Bright;
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
 *	@brief	ポケモングラフィックテクスチャ転送イベント
 *
 *	@param	event         イベントポインタ
 *	@param  seq           シーケンサ
 *	@param  work          ワークポインタ
 *	@return GMEVENT_RESULT  イベント結果
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT PokeGraTransEvt( GMEVENT* event, int* seq, void* work )
{
  FLD3D_CI_PTR ptr;
  FLD3D_CI_EVENT_WORK *evt_work;
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  //親イベントからワークポインタを取得
  {
    GMEVENT * parent = GMEVENT_GetParentEvent(event);
    evt_work = GMEVENT_GetEventWork(parent);
  }
  
  ptr = evt_work->CiPtr;
  gsys = GMEVENT_GetGameSysWork(event);
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  switch(*seq){
  case 0:
    ReTransToPokeGra(ptr);
    (*seq)++;
    break;
  case 1:
    GFL_BMP_Delete(ptr->ImgBitmap);
    GFL_HEAP_FreeMemory( ptr->chr_buf );
    GFL_HEAP_FreeMemory( ptr->pal_buf );
    (*seq)++;
    break;
  case 2:
    //終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief	鳴き声メイン関数
 *
 *	@param	ptr         管理ポインタ
 *	@return BOOL        TRUEでシーケンス移行を許可
 *
 */
//-----------------------------------------------------------------------------
static BOOL VoiceMain(GMEVENT* event, FLD3D_CI_PTR ptr)
{
  //鳴き声再生
  BOOL voice_end;
  POKE_WORK *poke_work;
  poke_work = (POKE_WORK*)ptr->Work;
  if (poke_work->VoicePlayFlg && poke_work->SePlayWait)
  {
    poke_work->SePlayWait--;
    if ( poke_work->SePlayWait == 0 )
    {
      if ( poke_work->MonsNo == MONSNO_PERAPPU )    //ペラップ対応
      {
        PMV_REF pmvRef;
        PMV_MakeRefDataMine( &pmvRef );
        poke_work->VoicePlayerIdx = PMVOICE_Play(poke_work->MonsNo, poke_work->FormNo, PMV_PAN_C, FALSE, 0, 0, FALSE, (u32)&pmvRef);
      }
      else poke_work->VoicePlayerIdx = PMV_PlayVoice( poke_work->MonsNo, poke_work->FormNo );
    }
    poke_work->VoiceTime = VOICE_TIME;
    return FALSE;   //鳴き声鳴らしたら、一度処理をメインに戻す（鳴き声を確実に鳴らすため）
  }

  voice_end = FALSE;

  //鳴き声鳴らした後の判定
  if ( !poke_work->SePlayWait )
  {
    //鳴き声なっていないなら、終了
    if ( !PMV_CheckPlay() )
    {
      voice_end = TRUE;
    }
    else
    {
      if (poke_work->VoiceTime) poke_work->VoiceTime--;
      else{
        s8 volume;
        volume = PMVOICE_GetVolume( poke_work->VoicePlayerIdx );
        if ( volume > 0){
          PMV_SetVolume( poke_work->VoicePlayerIdx, -VOICE_VOL_OFS );
        }
        else
        {
          PMV_StopVoice();
          voice_end = TRUE;
        }
      }
    }
  }

  if (ptr->PtclEnd&&ptr->MdlAnm1End&&ptr->MdlAnm2End&&voice_end)
  {
    return TRUE;
#if 0    
    if ( PMV_CheckPlay() )  //鳴き声がなっている場合は鳴き声フェードアウトイベントをコール
    {
      GMEVENT * call_event;
      GAMESYS_WORK *gsys;
      gsys = GMEVENT_GetGameSysWork(event);
      call_event = GMEVENT_Create(gsys, event, VoiceFadeOutEvt, 0);
      GMEVENT_CallEvent(event, call_event);
      return TRUE;
    }else //鳴き声が鳴っていなければ、終了
    {
      return TRUE;
    }
#endif    
  }

  return FALSE;
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
  ptr->ImgBitmap = GFL_BMP_Create(TEXSIZ_S/8, TEXSIZ_T/8, GFL_BMP_16_COLOR, heapID);
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
    POKE_WORK *pwk;
    pwk = (POKE_WORK*)ptr->Work;
    //リソース受け取り
    cgr	= POKEGRA_GetCgrArcIndex( POKEGRA_GetArcID(), pwk->MonsNo, pwk->FormNo, pwk->Sex, pwk->Rare, POKEGRA_DIR_FRONT, pwk->Egg );
    pal = POKEGRA_GetPalArcIndex( POKEGRA_GetArcID(), pwk->MonsNo, pwk->FormNo, pwk->Sex, pwk->Rare, POKEGRA_DIR_FRONT, pwk->Egg );
    ptr->chr = NULL;
    ptr->plt = NULL;
    //リソースはOBJとして作っているので、LoadOBJじゃないと読み込めない
#ifdef	POKEGRA_LZ
    ptr->chr_buf = GFL_ARC_UTIL_LoadOBJCharacter( POKEGRA_GetArcID(), cgr, TRUE, &ptr->chr, heapID );
#else	// POKEGRA_LZ
    ptr->chr_buf = GFL_ARC_UTIL_LoadOBJCharacter( POKEGRA_GetArcID(), cgr, FALSE, &ptr->chr, heapID );
#endif	// POKEGRA_LZ
    ptr->pal_buf = GFL_ARC_UTIL_LoadPalette( POKEGRA_GetArcID(), pal, &ptr->plt, heapID );
  }
  //12x12chrポケグラを16x16chrの真ん中に貼り付ける
  {
    u8	*src, *dst;
    int x, y;
    int	chrNum = 0;

    src = ptr->chr->pRawData;
		dst = GFL_BMP_GetCharacterAdrs(ptr->ImgBitmap);
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
  GFL_BMP_DataConv_to_BMPformat(ptr->ImgBitmap, FALSE, heapID);
  //転送
  {
    BOOL rc;
    void*	src;
		u32		dst;
    src = (void*)GFL_BMP_GetCharacterAdrs(ptr->ImgBitmap);
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

#if 0
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
  FLD3D_CI_EVENT_WORK *evt_work;
  POKE_WORK *poke_work;

  s8 volume;

  //親イベントからワークポインタを取得
  {
    FLD3D_CI_PTR ptr;
    GMEVENT * parent = GMEVENT_GetParentEvent(event);
    evt_work = GMEVENT_GetEventWork(parent);
    ptr = evt_work->CiPtr;
    poke_work = (POKE_WORK*)ptr->Work;
  }

  //鳴き声なっていないなら、終了
  if ( !PMV_CheckPlay() ) return GMEVENT_RES_FINISH;

  volume = PMVOICE_GetVolume( poke_work->VoicePlayerIdx );

  if ( volume > 0){
    volume -= VOICE_VOL_OFS;
    if ( volume < 0 ) volume = 0;

    PMV_SetVolume( poke_work->VoicePlayerIdx, volume );
  }
  else
  {
    PMV_StopVoice();
    return GMEVENT_RES_FINISH;
  }
  
  return GMEVENT_RES_CONTINUE;
}
#endif
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
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, CUTIN_WHITE_FADE_SPD );
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

#if 0
//--------------------------------------------------------------------------------------------
/**
 * ホワイトアウトイベントNPCそらをとぶ用
 *
 * @param   event       イベントポインタ
 * @param   *seq        シーケンサ
 * @param   work        ワークポインタ
 *
 * @return	GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT WhiteOutEvtNpc( GMEVENT* event, int* seq, void* work )
{
  FLD3D_CI_EVENT_WORK *evt_work;
  FIELDMAP_WORK * fieldmap;
  NPC_FLY_WORK *fly_work;
  {
    GAMESYS_WORK *gsys;
    gsys = GMEVENT_GetGameSysWork(event);
    fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  }

  //親イベントからワークポインタを取得
  {
    FLD3D_CI_PTR ptr;
    GMEVENT * parent = GMEVENT_GetParentEvent(event);
    evt_work = GMEVENT_GetEventWork(parent);
    ptr = evt_work->CiPtr;
    fly_work = (NPC_FLY_WORK*)ptr->Work;
  }

  switch(*seq){
  case 0:
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, CUTIN_WHITE_FADE_SPD );
    (*seq)++;
    break;
  case 1:
    if ( GFL_FADE_CheckFade() == FALSE )
    {
      //指定OBJを消す
       MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
       u16 id = fly_work->ObjID;
       MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
       if ( mmdl != NULL ) MMDL_Delete( mmdl );
       else GF_ASSERT_MSG( 0,"OBJ DEL 対象のOBJが居ません  %d\n",id );
      (*seq)++;
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}
#endif
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
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, CUTIN_WHITE_FADE_SPD );
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
 * 空を飛ぶブラックインインイベント
 *
 * @param   event       イベントポインタ
 * @param   *seq        シーケンサ
 * @param   work        ワークポインタ
 *
 * @return	GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT FlySkyBlackInEvt( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK * fieldmap;
  gsys = GMEVENT_GetGameSysWork(event);
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  switch(*seq){
  case 0:
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0 );
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
  if ( (inCutinNo == FLDCIID_FLY_OUT) ||
       (inCutinNo == FLDCIID_FLY_OUT2)||
       (inCutinNo == FLDCIID_CHAMP_OUT))
    return TRUE;

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
  if ( (inCutinNo == FLDCIID_FLY_IN) ||
       (inCutinNo == FLDCIID_FLY_IN2) )
    return TRUE;

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	グラフィックに書き換え
 *
 *	@param	ptr       カットイン管理ポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void ReTransToGra( FLD3D_CI_PTR ptr,
                          const char *inTexName,
                          const char *inPlttName,
                          const int inChrArcID,
                          const int inPltArcID )
{
  u32 tex_adr;
  u32 pal_adr;
  HEAPID heapID = ptr->HeapID; 
  //リソースのＶＲＡＭアドレス取得
  {
    GFL_G3D_RES* res = GFL_G3D_UTIL_GetResHandle( ptr->Util, 0 );
    NNSG3dTexKey texkey = GFL_G3D_GetTextureDataVramKey( res );
    NNSG3dPlttKey palkey = GFL_G3D_GetTexturePlttVramKey( res );
    {
      u8 * adr;
      NNSG3dResTex *resTex;
      NNSG3dResFileHeader* res_head = GFL_G3D_GetResourceFileHeader( res );
      resTex = NNS_G3dGetTex( res_head );
      adr = GetTexStartVRAMAdrByName(resTex, inTexName);
      tex_adr = (u32)adr;
      adr = GetPlttStartVRAMAdrByName(resTex, inPlttName);
      pal_adr = (u32)adr;
    }

    NOZOMU_Printf("key = %d %d\n",texkey, palkey);
    NOZOMU_Printf("adr = %x %x\n",tex_adr, pal_adr);
    NOZOMU_Printf("resid = %d\n",GFL_G3D_UTIL_GetUnitResIdx( ptr->Util,0 ) );
    
  }

  {
    ptr->chr = NULL;
    ptr->plt = NULL;
    ptr->chr_buf = GFL_ARC_UTIL_LoadBGCharacter( ARCID_ENC_CI_FACE, inChrArcID, TRUE, &ptr->chr, heapID );
    ptr->pal_buf = GFL_ARC_UTIL_LoadPalette( ARCID_ENC_CI_FACE, inPltArcID, &ptr->plt, heapID );
    {
      u8	*src, *dst;
      int x, y;
      int	chrNum = 0;

      src = ptr->chr->pRawData;
		  dst = GFL_BMP_GetCharacterAdrs(ptr->ImgBitmap);
      GFL_STD_MemClear32((void*)dst, 16*16*0x20);

      for (y=0; y<16; y++){
        for(x=0; x<16; x++){
          GFL_STD_MemCopy32((void*)(&src[chrNum*0x20]), (void*)(&dst[(y*16+x)*0x20]), 0x20);
          chrNum++;
        }
      }
    }
  }

  //16x16chrフォーマットデータをＢＭＰデータに変換
  GFL_BMP_DataConv_to_BMPformat(ptr->ImgBitmap, FALSE, heapID);
  //転送
  {
    BOOL rc;
    void*	src;
		u32		dst;
    src = (void*)GFL_BMP_GetCharacterAdrs(ptr->ImgBitmap);
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

//----------------------------------------------------------------------------
/**
 *	@brief	エンカウントカットインメインシーケンス内関数
 *
 *  @param  event       イベントポインタ 
 *	@param	ptr         管理ポインタ
 *	@return BOOL        TRUEでシーケンス移行を許可
 *
 *	@note カットインエンカウントエフェクトは20100324の時点でブラックアウトフェードのみ
 *
 */
//-----------------------------------------------------------------------------
static BOOL EncFadeMain(GMEVENT* event, FLD3D_CI_PTR ptr)
{

  switch(ptr->SubSeq){
  case 0:
    if (ptr->PtclEnd&&ptr->MdlAnm1End&&ptr->MdlAnm2End) ptr->SubSeq++;
    break;
  case 1:
    {
      int mode;
      FLD3D_CI_EVENT_WORK *work;
      work = GMEVENT_GetEventWork(event);
      if ( work->IsWhiteOut ) mode = GFL_FADE_MASTER_BRIGHT_WHITEOUT;   //白フェード
      else mode = GFL_FADE_MASTER_BRIGHT_BLACKOUT;                      //黒フェード
      
      //フェード開始
      GFL_FADE_SetMasterBrightReq(mode, 0, 16, -1 );  //両画面フェードアウト
    }
    ptr->SubSeq++;
  case 2:
    //フェードアウトアウト待ち
    if ( GFL_FADE_CheckFade() != FALSE ) break;
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	エンカウントエフェクト用テクスチャ転送イベント
 *
 *	@param	event         イベントポインタ
 *	@param  seq           シーケンサ
 *	@param  work          ワークポインタ
 *	@return GMEVENT_RESULT  イベント結果
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EncEffGraTransEvt( GMEVENT* event, int* seq, void* work )
{
  FLD3D_CI_PTR ptr;
  FLD3D_CI_EVENT_WORK *evt_work;
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  ENC_WORK *enc_wk;
  MYSTATUS *mystatus;
  //親イベントからワークポインタを取得
  {
    GMEVENT * parent = GMEVENT_GetParentEvent(event);
    evt_work = GMEVENT_GetEventWork(parent);
  }

  gsys = GMEVENT_GetGameSysWork(event);
  mystatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gsys));
  ptr = evt_work->CiPtr;
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  enc_wk = (ENC_WORK*)ptr->Work;
  switch(*seq){
  case 0:
    //相手側の画像データ転送
    ptr->ImgBitmap = GFL_BMP_Create(TEXSIZ_S/8, TEXSIZ_T/8, GFL_BMP_16_COLOR, ptr->HeapID);
    ReTransToGra( ptr, "trwb_face001", "trwb_face001_pl", enc_wk->ChrArcIdx, enc_wk->PltArcIdx);
    (*seq)++;
    break;
  case 1:
    GFL_BMP_Delete(ptr->ImgBitmap);
    GFL_HEAP_FreeMemory( ptr->chr_buf );
    GFL_HEAP_FreeMemory( ptr->pal_buf );
    //相手側の文字データ転送
    SetFont2Tex(mystatus, ptr, "name_up", "name_up_pl", enc_wk->MsgIdx );
    (*seq)++;
    break;
  case 2:
    if ( !enc_wk->PlayerTrans ) (*seq) = 4;    //終了
    else
    {
      int chr,plt;
      //自分側の画像データ転送
      ptr->ImgBitmap = GFL_BMP_Create(TEXSIZ_S/8, TEXSIZ_T/8, GFL_BMP_16_COLOR, ptr->HeapID);
      if ( MyStatus_GetMySex(mystatus) == PM_MALE){
        chr = NARC_enc_ci_face_enc_hero_lz_bin;
        plt = NARC_enc_ci_face_enc_hero_nclr;
      }
      else
      {
        chr = NARC_enc_ci_face_enc_heroine_lz_bin;
        plt = NARC_enc_ci_face_enc_heroine_nclr;
      }
      ReTransToGra( ptr, "trwb_hero_ine", "trwb_hero_ine_pl", chr, plt);
      (*seq)++;
    }
    break;
  case 3:
    GFL_BMP_Delete(ptr->ImgBitmap);
    GFL_HEAP_FreeMemory( ptr->chr_buf );
    GFL_HEAP_FreeMemory( ptr->pal_buf );
    //自分側の文字データ転送
    SetFont2Tex(mystatus, ptr, "name_down", "name_down_pl", SPTR_PLAYER );
    (*seq)++;
    break;
  case 4:
    //座標セット
    VEC_Set( &ptr->Status.trans, 0, 0, -(ENC_CUTIN_MDL_Z_OFS*FX32_ONE) );
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
    //終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;

}

static void* GetTexStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictTexData* pData)
{
	// オフセット値
	u32 offset = (pData->texImageParam & NNS_G3D_TEXIMAGE_PARAM_TEX_ADDR_MASK)<<3;
	offset += NNS_GfdGetTexKeyAddr( inResTex->texInfo.vramKey );
	return (void*)(offset);

}

static void* GetPlttStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictPlttData* pData)
{
	// オフセット値
	u32 offset = pData->offset<<3;
	offset += NNS_GfdGetPlttKeyAddr( inResTex->plttInfo.vramKey );
	return (void*)(offset);

}

static void* GetTexStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name)
{
	NNSG3dResName tmpResName;
	const NNSG3dResDictTexData* pData;

	//テクスチャの実体をサーチ
	SetResName(&tmpResName, name);	//文字列をNNSG3dResNameに変換
	pData = NNS_G3dGetTexDataByName( inResTex, &tmpResName );

	if (pData == NULL){
		return NULL;
	}

	//アドレスを算出
	return GetTexStartVRAMAdrSub(inResTex, pData);
}

static void* GetPlttStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name)
{
	NNSG3dResName tmpResName;
	const NNSG3dResDictPlttData* pData;

	//テクスチャの実体をサーチ
	SetResName(&tmpResName, name);	//文字列をNNSG3dResNameに変換
	pData = NNS_G3dGetPlttDataByName( inResTex, &tmpResName );

	if (pData == NULL){
		return NULL;
	}

	//アドレスを算出
	return GetPlttStartVRAMAdrSub(inResTex, pData);
}

void SetResName(NNSG3dResName *outName,const char *inName)
{
	u8 i;
  int len = STD_StrLen(inName);

	//初期化
	for (i=0;i<4;i++){
		outName->val[i] = 0;
	}
	for (i=0;i<len;i++){
		outName->name[i] = inName[i];
	}
}

static void SetFont2Tex(MYSTATUS *mystatus, FLD3D_CI_PTR ptr, const char* inTexName, const char* inPltName, const int inMsgIdx )
{
  F2T_WORK f2t_work;
  GFL_MSGDATA* msg; // メッセージデータ
  STRBUF*   strbuf; // 描画文字列
  GFL_G3D_RES* res = GFL_G3D_UTIL_GetResHandle( ptr->Util, 0 );
  HEAPID heapID = ptr->HeapID;

  msg = GFL_MSG_Create(
        GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_sptr_name_dat, heapID );
  if( inMsgIdx == SPTR_PLAYER )
  {    
    // WORDSET展開
    STRBUF* strbuf_msg;
    WORDSET *p_word = WORDSET_Create( GFL_HEAP_LOWID(heapID) );
    strbuf = GFL_STR_CreateBuffer( PERSON_NAME_SIZE + EOM_SIZE, GFL_HEAP_LOWID(heapID) );
    strbuf_msg = GFL_MSG_CreateString( msg, inMsgIdx );
    WORDSET_RegisterPlayerName( p_word, 0, mystatus );
    WORDSET_ExpandStr( p_word, strbuf, strbuf_msg );
    GFL_STR_DeleteBuffer(strbuf_msg);
    WORDSET_Delete( p_word );
  }
  else
  {
    // メッセージをそのまま使用
    strbuf = GFL_MSG_CreateString( msg, inMsgIdx );
  }

  {
    void* plt;
    PRINTSYS_LSB  lsb = PRINTSYS_LSB_Make(1,2,0);
    ptr->pal_buf = GFL_ARC_UTIL_LoadPalette( ARCID_FONT, NARC_font_default_nclr, &ptr->plt, heapID );
    plt = ptr->plt->pRawData;
    //色変更
    {
      u16 *col;
      col = (u16*)plt;
      col[1] = 0x7fff;    //白（文字）
      col[2] = 0x0000;    //黒（影）
    }

    F2T_CopyString(res, inTexName, plt, inPltName, strbuf, 0, 0, lsb, heapID, &f2t_work );

    GFL_HEAP_FreeMemory( ptr->pal_buf );
  }

  // 後始末
  GFL_HEAP_FreeMemory( strbuf );
  GFL_MSG_Delete( msg );

}

//--------------------------------------------------------------------------------------------
/**
 * NPCそらをとぶカットインイベント作成
 *
 * @param   gsys        ゲームシステムポインタ
 * @param   ptr         カットイン管理ポインタ
 * @apram   inObjID     OBJID
 * @return	event       イベントポインタ
 */
//--------------------------------------------------------------------------------------------
GMEVENT *FLD3D_CI_CreateNpcFlyCutInEvt( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, const int inObjID )
{
  GMEVENT * event;
  int no = FLDCIID_CHAMP_OUT;
  event = FLD3D_CI_CreateCutInEvt(gsys, ptr, no);
  //NPCそらをとぶカットイン用変数セット
  {
    NPC_FLY_WORK *fly_work;
    fly_work = (NPC_FLY_WORK*)ptr->Work;
    fly_work->ObjID = inObjID;
  }

  return event;
}
#if 0
//--------------------------------------------------------------------------------------------
/**
 * そらをとぶカットイン中関数
 *
 * @param   event       イベントポインタ
 * @param   ptr         カットイン管理ポインタ
 * @return	BOOL        TRUEでカットインメイン終了
 */
//--------------------------------------------------------------------------------------------
static BOOL FlySkyMainFunc(GMEVENT* event, FLD3D_CI_PTR ptr)
{
  int judge_frm;
  if ( ptr->CutInNo == FLDCIID_CHAMP_OUT ) judge_frm = CHAMP_FLYSKY_SE_FRAME;
  else judge_frm = FLYSKY_SE_FRAME;
  //ＳＥ再生判定
  if ( ptr->FrameCount == judge_frm )
  {
    //ＳＥ鳴らす
    PMSND_PlaySE( SEQ_SE_FLD_01 );
  }

  if (ptr->PtclEnd&&ptr->MdlAnm1End&&ptr->MdlAnm2End) return TRUE;

  return FALSE;
}
#endif
//--------------------------------------------------------------------------------------------
/**
 * キャプチャ画像輝度落としのためのカラー変更
 *
 * @param   none
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ChangeCapDatCol(void)
{
  u32 adr_val;
  void *adr;
  u16 *col;
  int i;
  u8 a,r,g,b;
  GXVRamBG bg;
  //ＶＲＡＭ書き換えするので、割り当てを一時的に解除
  bg = GX_ResetBankForBG();
  //VRAM_Dのアドレス取得
  adr_val = HW_LCDC_VRAM_D;
  adr = (void*)adr_val;
  col = (u16*)adr;
  for(i=0;i<256*192;i++)
  {
//    NOZOMU_Printf("bef col=%x\n",*col);
    //RGB分解 と　1/2ブラックブレンド
    a = (*col >> 15) & 0x1;   //1ビット
    b = (*col >> 10) & 0x1f;  //5ビット
    g = (*col >> 5) & 0x1f;   //5ビット
    r = (*col) & 0x1f;        //5ビット

    b = (b >> 1);
    g = (g >> 1);
    r = (r >> 1);

    *col = (a << 15) | (b << 10) | (g<< 5) | (r);
//    NOZOMU_Printf("aft col=%x\n",*col);
    col++;
  }
  //割り当て復帰
  GX_SetBankForBG(bg);
}

//--------------------------------------------------------------------------------------------
/**
 * カットインイベントワーク初期化
 *
 * @param   ptr       カットインポインタ
 * @param   work      対象ワーク
 * @return	none
 *
 * @note　　初期化設定：1　ポインタセット
 * @note    初期化設定：2　ＯＢＪポーズする
 * @note    初期化設定：3　メインフックする
 * @note    初期化設定：4　カットイン終了時にフェードアウトする場合、フェードはブラックアウト(フェードしないときは、ここを見ない)
 */
//--------------------------------------------------------------------------------------------
static void InitCutinEvtWork(FLD3D_CI_PTR ptr, FLD3D_CI_EVENT_WORK *work)
{
  int size;
  size = sizeof(FLD3D_CI_EVENT_WORK);
  MI_CpuClear8( work, size );
  work->CiPtr = ptr;
  work->ObjPause = FALSE;
  work->MainHook = TRUE;
  work->IsWhiteOut = FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * ＳＥ再生
 *
 * @param   ptr       カットインポインタ
 * @return none
 */
//--------------------------------------------------------------------------------------------
static void PlaySE(FLD3D_CI_PTR ptr)
{
  u32 se;
  const CI_SE_TBL * tbl = SeTbl[ptr->CutInNo];
  se = tbl[ptr->SeTblIdx].Se;
  if (se != CI_SE_NONE)
  {
    //フレーム監視
    GF_ASSERT(tbl[ptr->SeTblIdx].Frame != 0);
    if ( tbl[ptr->SeTblIdx].Frame == ptr->FrameCount )
    {
      //SE再生
      PMSND_PlaySE( se );
      //インデックスインクリメント
      ptr->SeTblIdx++;
    }
  }
}

static void DispSetVTask( GFL_TCB *p_tcb, void *p_work )
{
  BOOL *end_flg = p_work;
  
  //3D面をオン
  GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );

  //描画モード戻し
  {
    const GFL_BG_SYS_HEADER bg_sys_header = 
      {
        GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
      };
    GFL_BG_SetBGMode( &bg_sys_header );
  }

  GFL_TCB_DeleteTask( p_tcb );
  if (end_flg != NULL){
    *end_flg = TRUE;
  }
}

