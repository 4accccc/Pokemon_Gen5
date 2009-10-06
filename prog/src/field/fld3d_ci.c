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

#define OBJCOUNT_MAX  (2)
#define RESCOUNT_MAX  (2)

#define UNIT_NONE   (0xffffffff)


typedef struct FLD3D_CI_tag
{
  FLD_PRTCL_SYS_PTR PrtclSys;
  GFL_G3D_CAMERA *g3Dcamera;
  HEAPID HeapID;
  GFL_G3D_UTIL* Util;
  int UnitIdx;
  GFL_G3D_OBJSTATUS Status;

  GFL_PTC_PTR PrtclPtr;

  u16 PrtclWait;
  u16 MdlAnm1Wait;
  u16 MdlAnm2Wait;
  u8 PartGene;
  u8 CutInNo;
}FLD3D_CI;

//バイナリデータフォーマット
typedef struct {
  u16 SpaIdx;
  u16 SpaWait;
  u16 MdlAObjIdx;
  u16 MdlAAnm1Idx;
  u16 MdlAAnm2Idx;
  u16 MdlAAnmWait;
  u16 MdlBObjIdx;
  u16 MdlBAnm1Idx;
  u16 MdlBAnm2Idx;
  u16 MdlBAnmWait;
}RES_DEF_DAT;

typedef struct {
  GFL_G3D_UTIL_RES *Res;
  GFL_G3D_UTIL_OBJ *Obj;
  GFL_G3D_UTIL_ANM *Anm;
  GFL_G3D_UTIL_SETUP Setup;
  u16 SpaWait;
  u16 MdlAAnmWait;
  u16 MdlBAnmWait;
  u8 ResNum;
  u8 ObjNum;
  u8 AnmNum;
  u8 Dummy[3];
}RES_SETUP_DAT;

typedef struct {
  GAMESYS_WORK * gsys;
  FLD3D_CI_PTR CiPtr;
  RES_SETUP_DAT SetupDat;
}FLD3D_CI_EVENT_WORK;

static void SetupResource(FLD3D_CI_PTR ptr, RES_SETUP_DAT *outDat, const u8 inCutInNo);
static void SetupResourceCore(FLD3D_CI_PTR ptr, const GFL_G3D_UTIL_SETUP *inSetup);
static void DeleteResource(FLD3D_CI_PTR ptr, RES_SETUP_DAT *outDat);
static BOOL PlayParticle(FLD3D_CI_PTR ptr);
static BOOL PlayMdlAnm1(FLD3D_CI_PTR ptr);
static BOOL PlayMdlAnm2(FLD3D_CI_PTR ptr);
static BOOL CheckAnmEnd(FLD3D_CI_PTR ptr);

static GMEVENT_RESULT CatInEvt( GMEVENT* event, int* seq, void* work );


static void ParticleCallBack(GFL_EMIT_PTR emit);    //@todo
static void Generate(FLD3D_CI_PTR ptr);    //@todo

static void CreateRes(RES_SETUP_DAT *outDat, const u8 inResArcIdx, const HEAPID inHeapID);
static void DeleteRes(RES_SETUP_DAT *outDat);

//@todo
//--リソース関連--
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_FLD3D_CI, 1, GFL_G3D_UTIL_RESARC }, //IMD　カプセル本体
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
  //カプセル1
	{
		0, 	//モデルリソースID
		0, 							  //モデルデータID(リソース内部INDEX)
		0, 	//テクスチャリソースID
		NULL,//g3Dutil_anmTbl_cap,			//アニメテーブル(複数指定のため)
		0,//NELEMS(g3Dutil_anmTbl_cap),	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP Setup = {
  g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};


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
    VecFx32 pos = {0, 0, FX32_ONE*128};
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
 * @param   
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



#if 0  

  ptr->Unit[inIndex].ObjNum = obj_num;
  ptr->Unit[inIndex].Valid = TRUE;
  ptr->Unit[inIndex].UtilUnitIdx = unitIdx;

  status = GFL_HEAP_AllocClearMemory(ptr->HeapID, sizeof(GFL_G3D_OBJSTATUS)*obj_num);
  ptr->Unit[inIndex].ObjStatus = status;
  ptr->Unit[inIndex].AnmList = GFL_HEAP_AllocClearMemory(ptr->HeapID, sizeof(ANM_LIST)*obj_num);
  for(i=0;i<obj_num;i++){
    const GFL_G3D_UTIL_OBJ * objTbl = inSetup->objTbl;
    u16 anm_num = objTbl[i].anmCount;
    AnmCntInit(&ptr->Unit[inIndex].AnmList[i], anm_num, ptr->HeapID);

    VEC_Set( &status[i].scale, FX32_ONE, FX32_ONE, FX32_ONE );
	  MTX_Identity33( &status[i].rotate );
    VEC_Set( &status[i].trans, 0, 0, 0 );    
  }
  GF_ASSERT(GFL_HEAP_CheckHeapSafe(ptr->HeapID) == TRUE);
#endif
}

void FLD3D_CI_Draw( FLD3D_CI_PTR ptr )
{
  static fx32 testZ = 0;
  u8 i,j;
  {
    if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT){
      testZ -= (FX32_ONE*16);
    }
    VEC_Set( &ptr->Status.trans, 0, 0, testZ );
  }
  //カメラ設定
  GFL_G3D_CAMERA_Switching( ptr->g3Dcamera );
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


//カットイン呼び出し
void FLD3D_CI_CallCatIn( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, const u8 inCutInNo )
{
  FLD3D_CI_EVENT_WORK *work;
  int size;

  size = sizeof(FLD3D_CI_EVENT_WORK);
  ptr->PartGene = 0;
  ptr->CutInNo = inCutInNo;
  //イベント作成
  {
    GMEVENT * event = GMEVENT_Create( gsys, NULL, CatInEvt, size );

    work = GMEVENT_GetEventWork(event);
    MI_CpuClear8( work, size );
    work->gsys = gsys;
    work->CiPtr = ptr;

    GAMESYSTEM_SetEvent(gsys, event);
  }
}

//カットインイベント
static GMEVENT_RESULT CatInEvt( GMEVENT* event, int* seq, void* work )
{
  FLD3D_CI_PTR ptr;
  FLD3D_CI_EVENT_WORK *evt_work = work;
  ptr = evt_work->CiPtr;

  switch(*seq){
  case 0:
    //カットイン共通ホワイトアウト処理
    (*seq)++;
    break;
  case 1:
    //リソースロード
    SetupResource(ptr, &evt_work->SetupDat, ptr->CutInNo);
    (*seq)++;
    break;
  case 2:
    //画面復帰
    if (1){
      (*seq)++;
    }
    break;
  case 3:
    {
      BOOL rc1,rc2,rc3;
      //パーティクル再生
      rc1 = PlayParticle(ptr);
      //3Ｄモデル1アニメ再生
      rc2 = TRUE;
      //3Ｄモデル2アニメ再生
      rc3 = TRUE;

      if (rc1&&rc2&&rc3){
        (*seq)++;
      }
    }
    break;
  case 4:
    //終了監視
    if ( CheckAnmEnd(ptr) ){
      (*seq)++;
    }
    break;
  case 5:
    //リソース解放処理
    DeleteResource(ptr, &evt_work->SetupDat);
    (*seq)++;
    break;
  case 6:
    //終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//パーティクル再生
static BOOL PlayParticle(FLD3D_CI_PTR ptr)
{
  if (!ptr->PartGene){
    if (ptr->PrtclWait > 0){
      ptr->PrtclWait--;
    }else{
      //パーティクルジェネレート
      Generate(ptr);
      ptr->PartGene = 1;
      return TRUE;
    }
  }else{
    return TRUE;
  }
  return FALSE;
}

//モデルアニメ1再生
static BOOL PlayMdlAnm1(FLD3D_CI_PTR ptr)
{
  if (ptr->MdlAnm1Wait > 0){
    ptr->MdlAnm1Wait--;
  }else{
    //アニメ再生
    ;
  }
  return FALSE;
}

//モデルアニメ2再生
static BOOL PlayMdlAnm2(FLD3D_CI_PTR ptr)
{
  if (ptr->MdlAnm2Wait > 0){
    ptr->MdlAnm2Wait--;
  }else{
    //アニメ再生
    ;
  }
  return FALSE;
}

static void SetupResource(FLD3D_CI_PTR ptr, RES_SETUP_DAT *outDat, const u8 inCutInNo)
{
  //パーティクル生成
  ptr->PrtclPtr = FLD_PRTCL_Create(ptr->PrtclSys);
  //セットアップデータ作成
  CreateRes(outDat, inCutInNo, ptr->HeapID);
  
  //3Ｄモデルリソースをセットアップ
  SetupResourceCore(ptr, /*&Setup*/&outDat->Setup);
  //パーティクルリソースセットアップ
  {
    void *resource;
    resource = FLD_PRTCL_LoadResource(ptr->PrtclSys,
			ARCID_FLD3D_CI, /*NARC_particledata_title_part_spa*/0);

    FLD_PRTCL_SetResource(ptr->PrtclSys, resource, TRUE, NULL);
  }
  //ウェイト設定
  ptr->PrtclWait = 0;
  ptr->MdlAnm1Wait = 0;
  ptr->MdlAnm2Wait = 0;
}

static void DeleteResource(FLD3D_CI_PTR ptr, RES_SETUP_DAT *outDat)
{
  DeleteRes(outDat);
  GFL_G3D_UTIL_DelUnit( ptr->Util, ptr->UnitIdx );
  ptr->UnitIdx = UNIT_NONE;
  FLD_PRTCL_Delete(ptr->PrtclSys);
}

static BOOL CheckAnmEnd(FLD3D_CI_PTR ptr)
{
  //パーティクル終了判定
  if ( !FLD_PRTCL_CheckEmitEnd( ptr->PrtclSys ) ){
    return FALSE;
  }

  return TRUE;
}



static void Generate(FLD3D_CI_PTR ptr)
{
///  GFL_PTC_SetCameraType(sys->PrtclPtr, GFL_G3D_PRJORTH);
	GFL_PTC_CreateEmitterCallback(ptr->PrtclPtr, 0,
									ParticleCallBack, NULL);
	GFL_PTC_CreateEmitterCallback(ptr->PrtclPtr, 1,
									ParticleCallBack, NULL);
	GFL_PTC_CreateEmitterCallback(ptr->PrtclPtr, 2,
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
 * @param	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void CreateRes(RES_SETUP_DAT *outDat, const u8 inResArcIdx, const HEAPID inHeapID)
{
  RES_DEF_DAT def_dat;
  u8 res_num,obj_num,anm_num;
  //アーカイブからリソース定義をロード
  GFL_ARC_LoadData(&def_dat, ARCID_FLD3D_CI_SETUP, inResArcIdx);
  //リソース数を調べる
  res_num = 1;  //@todo
  //OBJ数を調べる
  obj_num = 1;  //@todo
  //アニメ数を調べる
  anm_num = 0;  //@todo
  //リソーステーブルアロケート
  if (res_num){
    outDat->Res = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(GFL_G3D_UTIL_RES)*res_num);
  }
  //ＯＢＪテーブルアロケート
  if(obj_num){
    outDat->Obj = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(GFL_G3D_UTIL_OBJ)*obj_num);
  }
  //アニメテーブルアロケート
  if(anm_num){
    outDat->Anm = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(GFL_G3D_UTIL_ANM)*anm_num);
  }

  //リソース数記憶
  outDat->ResNum = res_num;
  outDat->ObjNum = obj_num;
  outDat->AnmNum = anm_num;
  //ウェイト記憶
  outDat->SpaWait = def_dat.SpaWait;
  outDat->MdlAAnmWait = def_dat.MdlAAnmWait;
  outDat->MdlBAnmWait = def_dat.MdlBAnmWait;

  //↓@todo　const セットができない・・・
  outDat->Res->arcive = ARCID_FLD3D_CI;
  outDat->Res->datID = def_dat.MdlAObjIdx;
  outDat->Res->arcType = GFL_G3D_UTIL_RESARC;

  outDat->Obj->mdlresID = 0;  
  outDat->Obj->mdldatID = 0;
  outDat->Obj->texresID = 0;
  outDat->Obj->anmTbl = outDat->Anm;
  outDat->Obj->anmCount = anm_num;

  outDat->Setup.resTbl = outDat->Res;
  outDat->Setup.resCount = res_num;
  outDat->Setup.objTbl = outDat->Obj;
  outDat->Setup.objCount = obj_num;
}

//--------------------------------------------------------------
/**
 * @brief	リソース破棄関数
 *
 * @param	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void DeleteRes(RES_SETUP_DAT *outDat)
{
  if (outDat->Anm != NULL){
    GFL_HEAP_FreeMemory( outDat->Anm );
  }
  if (outDat->Obj != NULL){
    GFL_HEAP_FreeMemory( outDat->Obj );
  }
  if(outDat->Res != NULL){
    GFL_HEAP_FreeMemory( outDat->Res );
  }
}



