//======================================================================
/**
 * @file	fld_exp_obj.c
 * @brief	フィールド3Ｄ拡張オブジェクト
 * @author	Saito
 * @date	08.08.27
 *
 */
//======================================================================

#include "fld_exp_obj.h"

#define FLD_EXP_OBJ_UNIT_MAX  (4)

typedef struct EXP_OBJ_ANM_CNT_tag
{
  u8 Stop;
  u8 Loop;
  u8 Valid;
  u8 dummy;
  fx32 AddFrm;
  BOOL Continue;
  GFL_G3D_ANM* g3Danm;
}EXP_OBJ_ANM_CNT;

typedef struct ANM_LIST_tag
{
  int AnmNum;
  EXP_OBJ_ANM_CNT *AnmCnt;
}ANM_LIST;

typedef struct EXP_OBJ_STATUS_tag
{
  BOOL Culling;
  BOOL Vanish;
  GFL_G3D_OBJSTATUS ObjStatus;
}EXP_OBJ_STATUS;

typedef struct EXP_UNIT_tag
{
  BOOL Valid;
//  GFL_G3D_OBJSTATUS *ObjStatus;
  EXP_OBJ_STATUS *ExpObjStatus;
  ANM_LIST  *AnmList;
  u16 UtilUnitIdx;
  u16 ObjNum;
}EXP_UNIT;

typedef struct FLD_EXP_OBJ_CNT_tag
{
  int HeapID;
  GFL_G3D_UTIL* ObjUtil;
  EXP_UNIT  Unit[FLD_EXP_OBJ_UNIT_MAX];

}FLD_EXP_OBJ_CNT;

static void AnmCntInit(ANM_LIST *lst, const int inAnmNum, const int inHeapID);


//--------------------------------------------------------------------------------------------
/**
 * モジュール作成
 *
 * @param   inResCountMax			設定リソース最大数定
 * @param   inObjCountMax			設定オブジェクト最大数
 * @param   inHeapID				ヒープＩＤ
 *
 * @return	FLD_EXP_OBJ_CNT_PTR   モジュールポインタ
 */
//--------------------------------------------------------------------------------------------
FLD_EXP_OBJ_CNT_PTR FLD_EXP_OBJ_Create( const int inResCountMax,
                                        const int inObjCountMax,
                                        const int inHeapID )
{
  FLD_EXP_OBJ_CNT_PTR ptr;
  ptr = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(FLD_EXP_OBJ_CNT));
  ptr->ObjUtil = GFL_G3D_UTIL_Create( inResCountMax, inObjCountMax, inHeapID );
  ptr->HeapID = inHeapID;

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
void FLD_EXP_OBJ_Delete(FLD_EXP_OBJ_CNT_PTR ptr)
{
  u16 i;
  //解放忘れがあるかもしれないので、ユニット廃棄もしておく
  for (i=0;i<FLD_EXP_OBJ_UNIT_MAX;i++){
    if ( ptr->Unit[i].Valid){
      FLD_EXP_OBJ_DelUnit( ptr, i );
    }
  }
  GFL_G3D_UTIL_Delete( ptr->ObjUtil );
  GFL_HEAP_FreeMemory( ptr );
}

//--------------------------------------------------------------------------------------------
/**
 * ユニット追加
 *
 * @param	ptr     モジュールポインタ
 * @param	inSetup		設定データ
 * @param inIndex   登録するインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_AddUnit(  FLD_EXP_OBJ_CNT_PTR ptr,
                          const GFL_G3D_UTIL_SETUP *inSetup,
                          const u16 inIndex)
{
  u16 i;
  u16 unitIdx;
  EXP_OBJ_STATUS *exp_status;
  u16 obj_num;

  if ( ptr->Unit[inIndex].Valid == TRUE ){
    GF_ASSERT_MSG(0,"CAN NOT ADD UNIT\n");
  }

  unitIdx = GFL_G3D_UTIL_AddUnitResShare( ptr->ObjUtil, inSetup );

  obj_num = inSetup->objCount;
  if (obj_num == 0){
    GF_ASSERT_MSG(0,"OBJ NOTHING\n");
  }
  ptr->Unit[inIndex].ObjNum = obj_num;
  ptr->Unit[inIndex].Valid = TRUE;
  ptr->Unit[inIndex].UtilUnitIdx = unitIdx;

  exp_status = GFL_HEAP_AllocClearMemory(ptr->HeapID, sizeof(EXP_OBJ_STATUS)*obj_num);
  ptr->Unit[inIndex].ExpObjStatus = exp_status;

  ptr->Unit[inIndex].AnmList = GFL_HEAP_AllocClearMemory(ptr->HeapID, sizeof(ANM_LIST)*obj_num);

  for(i=0;i<obj_num;i++){
    const GFL_G3D_UTIL_OBJ * objTbl = inSetup->objTbl;
    u16 anm_num = objTbl[i].anmCount;
    AnmCntInit(&ptr->Unit[inIndex].AnmList[i], anm_num, ptr->HeapID);
    
    VEC_Set( &exp_status[i].ObjStatus.scale, FX32_ONE, FX32_ONE, FX32_ONE );
	  MTX_Identity33( &exp_status[i].ObjStatus.rotate );
    VEC_Set( &exp_status[i].ObjStatus.trans, 0, 0, 0 );

    //デフォルトはカリングしない
    exp_status[i].Culling = FALSE;
    //デフォルトは表示状態
    exp_status[i].Vanish = FALSE;
    {
      int j;
      GFL_G3D_OBJ *g3Dobj = FLD_EXP_OBJ_GetUnitObj(ptr, inIndex, i);
      ANM_LIST *lst;
      lst = &ptr->Unit[inIndex].AnmList[i];
      for (j=0;j<lst->AnmNum;j++){
        lst->AnmCnt[j].g3Danm = GFL_G3D_OBJECT_GetG3Danm( g3Dobj, j );
      }
    }
  }
  GF_ASSERT(GFL_HEAP_CheckHeapSafe(ptr->HeapID) == TRUE);
}

//--------------------------------------------------------------------------------------------
/**
 * ユニット破棄
 *
 * @param	ptr         モジュールポインタ
 * @param	inUnitIdx   ユニットインデックス 
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_DelUnit( FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx )
{
  u16 i;
  if (ptr->Unit[inUnitIdx].Valid == FALSE){
    GF_ASSERT_MSG(0,"%d UNIT INVALID\n",inUnitIdx);
    return;
  }
  if (inUnitIdx >= FLD_EXP_OBJ_UNIT_MAX){
    GF_ASSERT_MSG(0,"%d UNIT IDX OVER\n",inUnitIdx);
    return;
  }
  GF_ASSERT(GFL_HEAP_CheckHeapSafe(ptr->HeapID) == TRUE);

  for(i=0;i<ptr->Unit[inUnitIdx].ObjNum;i++){
    GFL_HEAP_FreeMemory( ptr->Unit[inUnitIdx].AnmList[i].AnmCnt );
  }
  GFL_HEAP_FreeMemory( ptr->Unit[inUnitIdx].AnmList );

  GFL_HEAP_FreeMemory( ptr->Unit[inUnitIdx].ExpObjStatus );
  GFL_G3D_UTIL_DelUnit( ptr->ObjUtil, ptr->Unit[inUnitIdx].UtilUnitIdx );

  ptr->Unit[inUnitIdx].Valid = FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 *  ユーティリティポインタ取得 
 *
 * @param	ptr               モジュールポインタ
 *
 * @return	GFL_G3D_UTIL*   ユーティリティポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_UTIL *FLD_EXP_OBJ_GetUtil(FLD_EXP_OBJ_CNT_PTR ptr)
{
  GF_ASSERT(ptr != NULL);
  return ptr->ObjUtil;
}

//--------------------------------------------------------------------------------------------
/**
 *  ユーティリティが管理するユニットインデックスを取得
 *
 * @param	ptr               モジュールポインタ
 * @param inUnitIdx         モジュールが管理するユニットのインデックス
 *
 * @return	u16   ユーティリティの持つユニットのインデックス
 */
//--------------------------------------------------------------------------------------------
u16 FLD_EXP_OBJ_GetUtilUnitIdx(FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx)
{
  GF_ASSERT(ptr != NULL);
  return ptr->Unit[inUnitIdx].UtilUnitIdx;
}

//--------------------------------------------------------------------------------------------
/**
 *  ユニットステータスポインタ取得
 *
 * @param   ptr               モジュールポインタ
 * @param   inUnitIdx         ユニットインデックス
 * @param   inObjIdx          ユニット内ＯＢＪインデックス
 *
 * @return	GFL_G3D_OBJSTATUS*    ステータスポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJSTATUS *FLD_EXP_OBJ_GetUnitObjStatus(FLD_EXP_OBJ_CNT_PTR ptr,
                                                const u16 inUnitIdx,
                                                const u16 inObjIdx)
{
  GF_ASSERT(ptr != NULL);
  if (ptr->Unit[inUnitIdx].Valid == FALSE){
    GF_ASSERT_MSG(0,"%d UNIT INVALID\n",inUnitIdx);
    return NULL;
  }
  if (inUnitIdx >= FLD_EXP_OBJ_UNIT_MAX){
    GF_ASSERT_MSG(0,"%d UNIT IDX OVER\n",inUnitIdx);
    return NULL;
  }
//  GF_ASSERT(GFL_HEAP_CheckHeapSafe(ptr->HeapID) == TRUE);
  return &ptr->Unit[inUnitIdx].ExpObjStatus[inObjIdx].ObjStatus;
}

//--------------------------------------------------------------------------------------------
/**
 *  カリングするかどうかを決定する
 *
 * @param   ptr               モジュールポインタ
 * @param   inUnitIdx         ユニットインデックス
 * @param   inObjIdx          ユニット内ＯＢＪインデックス
 * @param   inCullFlg         TRUEでカリングする
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_SetCulling(FLD_EXP_OBJ_CNT_PTR ptr,
                            const u16 inUnitIdx,
                            const u16 inObjIdx,
                            const BOOL inCulling)
{
  ptr->Unit[inUnitIdx].ExpObjStatus[inObjIdx].Culling = inCulling;
}

//--------------------------------------------------------------------------------------------
/**
 *  表示状態をセット
 *
 * @param   ptr               モジュールポインタ
 * @param   inUnitIdx         ユニットインデックス
 * @param   inObjIdx          ユニット内ＯＢＪインデックス
 * @param   inVanish          TRUEで非表示
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_SetVanish(FLD_EXP_OBJ_CNT_PTR ptr,
                            const u16 inUnitIdx,
                            const u16 inObjIdx,
                            const BOOL inVanish)
{
  ptr->Unit[inUnitIdx].ExpObjStatus[inObjIdx].Vanish = inVanish;
}

//--------------------------------------------------------------------------------------------
/**
 *  ユニットOBJポインタ取得
 *
 * @param   ptr               モジュールポインタ
 * @param   inUnitIdx         ユニットインデックス
 * @param   inObjIdx          ユニット内ＯＢＪインデックス
 *
 * @return	GFL_G3D_OBJSTATUS*    ステータスポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJ *FLD_EXP_OBJ_GetUnitObj(FLD_EXP_OBJ_CNT_PTR ptr,
                                    const u16 inUnitIdx,
                                    const u16 inObjIdx)
{
  u16 obj_idx;
  GFL_G3D_OBJ *pObj;

  GF_ASSERT(ptr != NULL);
  if (ptr->Unit[inUnitIdx].Valid == FALSE){
    GF_ASSERT_MSG(0,"%d UNIT INVALID\n",inUnitIdx);
    return NULL;
  }
  if (inUnitIdx >= FLD_EXP_OBJ_UNIT_MAX){
    GF_ASSERT_MSG(0,"%d UNIT IDX OVER\n",inUnitIdx);
    return NULL;
  }

  obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( ptr->ObjUtil, ptr->Unit[inUnitIdx].UtilUnitIdx );
  pObj = GFL_G3D_UTIL_GetObjHandle(ptr->ObjUtil, obj_idx+inObjIdx);
  
  return pObj;
}


//--------------------------------------------------------------------------------------------
/**
 *  ユニット描画
 *
 * @param   ptr               モジュールポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_Draw( FLD_EXP_OBJ_CNT_PTR ptr )
{
  u8 i,j;
  //ユニット数分ループ
  for (i=0;i<FLD_EXP_OBJ_UNIT_MAX;i++){
    if (ptr->Unit[i].Valid == TRUE){
      u16 obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( ptr->ObjUtil, ptr->Unit[i].UtilUnitIdx );
      u16 obj_count = GFL_G3D_UTIL_GetUnitObjCount( ptr->ObjUtil, ptr->Unit[i].UtilUnitIdx );
      for (j=0;j<obj_count;j++){
        GFL_G3D_OBJ* pObj;

        //非表示状態ならば描画処理をスルー
        if (ptr->Unit[i].ExpObjStatus[j].Vanish) continue;

        pObj = GFL_G3D_UTIL_GetObjHandle(ptr->ObjUtil, obj_idx+j);
        if ( ptr->Unit[i].ExpObjStatus[j].Culling ){
          GFL_G3D_DRAW_DrawObjectCullingON( pObj, &ptr->Unit[i].ExpObjStatus[j].ObjStatus );
        }else{
          GFL_G3D_DRAW_DrawObject( pObj, &ptr->Unit[i].ExpObjStatus[j].ObjStatus );
        }
      }
    }
  }
}

//--------------------------------------------------------------------------------------------
/**
 *  アニメーション再生コントロール
 *
 * @param   ptr               モジュールポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_PlayAnime( FLD_EXP_OBJ_CNT_PTR ptr )
{
  u8 i,j,k;
  //ユニット数分ループ
  for (i=0;i<FLD_EXP_OBJ_UNIT_MAX;i++){
    if (ptr->Unit[i].Valid == TRUE){
      u16 obj_count = GFL_G3D_UTIL_GetUnitObjCount( ptr->ObjUtil, ptr->Unit[i].UtilUnitIdx );
      for (j=0;j<obj_count;j++){
        GFL_G3D_OBJ *pObj = FLD_EXP_OBJ_GetUnitObj(ptr, i, j);
        int anm_num = ptr->Unit[i].AnmList[j].AnmNum;
        for (k=0;k<anm_num;k++){
          EXP_OBJ_ANM_CNT *anm = &ptr->Unit[i].AnmList[j].AnmCnt[k];
          if ( !anm->Valid ){
            continue;
          }
          if ( !anm->Stop ){
            if (anm->Loop){      //ループ再生
              anm->Continue = GFL_G3D_OBJECT_LoopAnimeFrame( pObj, k, anm->AddFrm );
            }else{          //1回再生
              anm->Continue = GFL_G3D_OBJECT_IncAnimeFrame( pObj, k, anm->AddFrm );
            }
          }
        }
      }
    }
  }  
}

static void AnmCntInit(ANM_LIST *lst, const int inAnmNum, const int inHeapID)
{
  int i;
  EXP_OBJ_ANM_CNT *anm_cnt;
  lst->AnmCnt = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(EXP_OBJ_ANM_CNT)*inAnmNum);
  anm_cnt = lst->AnmCnt;
  lst->AnmNum = inAnmNum;
  for (i=0;i<inAnmNum;i++){
    anm_cnt[i].Stop = 0;
    anm_cnt[i].Loop = 1;
    anm_cnt[i].Valid = 0;
    anm_cnt[i].AddFrm = FX32_ONE;
  }  
}

EXP_OBJ_ANM_CNT_PTR FLD_EXP_OBJ_GetAnmCnt(
    FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx, const u16 inObjIdx, const u16 inAnmIdx)
{
  EXP_OBJ_ANM_CNT *anm_cnt;
  if ( !ptr->Unit[inUnitIdx].Valid ){
    GF_ASSERT(0);
    return NULL;
  }
  if (inObjIdx >= ptr->Unit[inUnitIdx].ObjNum){
    GF_ASSERT(0);
    return NULL;
  }
  anm_cnt = ptr->Unit[inUnitIdx].AnmList[inObjIdx].AnmCnt;

  if (inAnmIdx >= ptr->Unit[inUnitIdx].AnmList[inObjIdx].AnmNum){
    GF_ASSERT(0);
    return NULL;
  }
  return &anm_cnt[inAnmIdx];
}

void FLD_EXP_OBJ_ChgAnmStopFlg(EXP_OBJ_ANM_CNT_PTR ptr, const u8 inStop)
{
  ptr->Stop = inStop;
}

u8 FLD_EXP_OBJ_GetAnmStopFlg(EXP_OBJ_ANM_CNT_PTR ptr)
{
  return ptr->Stop;
}

void FLD_EXP_OBJ_ChgAnmLoopFlg(EXP_OBJ_ANM_CNT_PTR ptr, const u8 inLoop)
{
  ptr->Loop = inLoop;
}

BOOL FLD_EXP_OBJ_ChkAnmEnd(EXP_OBJ_ANM_CNT_PTR ptr)
{
  return (!ptr->Continue);
}

void FLD_EXP_OBJ_SetObjAnmFrm(
                        FLD_EXP_OBJ_CNT_PTR ptr,
                        const u16 inUnitIdx,
                        const u16 inObjIdx,
                        const u16 inAnmIdx,
                        const fx32 inFrm  )
{
  int anmFrm = inFrm;
  GFL_G3D_OBJ *g3Dobj = FLD_EXP_OBJ_GetUnitObj(ptr, inUnitIdx, inObjIdx);
  GFL_G3D_OBJECT_SetAnimeFrame( g3Dobj, inAnmIdx, &anmFrm );
}

fx32 FLD_EXP_OBJ_GetObjAnmFrm(
                        FLD_EXP_OBJ_CNT_PTR ptr,
                        const u16 inUnitIdx,
                        const u16 inObjIdx,
                        const u16 inAnmIdx )
{
  int anmFrm;
  GFL_G3D_OBJ *g3Dobj = FLD_EXP_OBJ_GetUnitObj(ptr, inUnitIdx, inObjIdx);
  GFL_G3D_OBJECT_GetAnimeFrame( g3Dobj, inAnmIdx, &anmFrm );
  return anmFrm;
}


void FLD_EXP_OBJ_ValidCntAnm(
                        FLD_EXP_OBJ_CNT_PTR ptr,
                        const u16 inUnitIdx,
                        const u16 inObjIdx,
                        const u16 inAnmIdx,
                        const BOOL inValid)
{
  GFL_G3D_OBJ *g3Dobj = FLD_EXP_OBJ_GetUnitObj(ptr, inUnitIdx, inObjIdx);
  EXP_OBJ_ANM_CNT_PTR anm = FLD_EXP_OBJ_GetAnmCnt( ptr, inUnitIdx, inObjIdx, inAnmIdx );
  
  if (anm == NULL){
    return;
  }

  if (inValid){
    if (anm->Valid){
      return;
    }else{
      GFL_G3D_OBJECT_EnableAnime( g3Dobj, inAnmIdx );
    }
  }else{
    if (!anm->Valid){
      return;
    }
    GFL_G3D_OBJECT_DisableAnime( g3Dobj, inAnmIdx );
  }

  anm->Valid = inValid;
}

fx32 FLD_EXP_OBJ_GetAnimeLastFrame(EXP_OBJ_ANM_CNT_PTR ptr )
{
  fx32 frame = 0;
  NNSG3dAnmObj* anm_obj;
  anm_obj = GFL_G3D_ANIME_GetAnmObj( ptr->g3Danm ) ;

  frame = NNS_G3dAnmObjGetNumFrame( anm_obj ) - FX32_ONE;

  return frame;

}


