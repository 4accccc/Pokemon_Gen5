//======================================================================
/*
 * @file	enceff.c
 * @brief	エンカウントエフェクト
 * @author saito
 */
//======================================================================
#include "enceff.h"
#include "gamesystem/gamesystem.h"

#include "fieldmap.h"

typedef GMEVENT* (*CREATE_FUNC)(GAMESYS_WORK *, FIELDMAP_WORK *);
typedef void (*DRAW_FUNC)(ENCEFF_CNT_PTR);


typedef struct ENCEFF_CNT_tag
{
  void *Work;
  DRAW_FUNC DrawFunc;
  FIELDMAP_WORK *FieldMapWork;
}ENCEFF_CNT;

typedef struct {
  CREATE_FUNC CreateFunc;
  DRAW_FUNC DrawFunc;
}ENCEFF_TBL;

static const ENCEFF_TBL EncEffTbl[] = {
  {ENCEFF_CreateEff1, ENCEFF_DrawEff1},
  {ENCEFF_CreateEff2, ENCEFF_DrawEff2},
  {ENCEFF_CreateEff3, ENCEFF_DrawEff3},
};

//--------------------------------------------------------------
/**
 * @brief エンカウントエフェクトコントローラ作成
 * @param inHeapID    ヒープＩＤ
 * @retval  ptr   コントローラポインタ
 */
//--------------------------------------------------------------
ENCEFF_CNT_PTR ENCEFF_CreateCntPtr(const HEAPID inHeapID, FIELDMAP_WORK * fieldmap)
{
  ENCEFF_CNT_PTR ptr;
  ptr = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(ENCEFF_CNT));
  ptr->FieldMapWork = fieldmap;
  return ptr;
}

//--------------------------------------------------------------
/**
 * @brief   エンカウントエフェクトコントローラ破棄
 * @param ptr     コントローラポインタ
 * @retval  none
 */
//--------------------------------------------------------------
void ENCEFF_DeleteCntPtr(ENCEFF_CNT_PTR ptr)
{
  GFL_HEAP_FreeMemory( ptr );
}

//--------------------------------------------------------------
/**
 * @brief   エンカウントエフェクトセット
 * @param   ptr       コントローラポインタ
 * @param   event     イベントポインタ
 * @param   inID      エフェクトＩＤ
 * @retval  none
 */
//--------------------------------------------------------------
void ENCEFF_SetEncEff(ENCEFF_CNT_PTR ptr, GMEVENT * event, const ENCEFF_ID inID)
{
  GMEVENT *call_event;
  FIELDMAP_WORK * fieldmap = ptr->FieldMapWork;
  GAMESYS_WORK * gsys = FIELDMAP_GetGameSysWork( ptr->FieldMapWork );

  int no;
  no = GFUser_GetPublicRand(2);

  call_event = EncEffTbl[no].CreateFunc(gsys,fieldmap);
  //イベントコール
  GMEVENT_CallEvent( event, call_event );
  //ワークポインタセット
  ptr->Work = GMEVENT_GetEventWork( call_event );
  //描画関数セット
  ptr->DrawFunc = EncEffTbl[no].DrawFunc;
}

//--------------------------------------------------------------
/**
 * @brief   ワークポインタを返す
 * @param ptr   コントローラポインタ
 * @retval  void*     ワークポインタ
 */
//--------------------------------------------------------------
void *ENCEFF_GetWork(ENCEFF_CNT_PTR ptr)
{
  if (ptr->Work == NULL) GF_ASSERT(0);

  return ptr->Work;
}

//--------------------------------------------------------------
/**
 * @brief   描画関数
 * @param   ptr     コントローラポインタ
 * @retval  none
 */
//--------------------------------------------------------------
void ENCEFF_Draw(ENCEFF_CNT_PTR ptr)
{
  if (ptr->DrawFunc != NULL)
  {
    ptr->DrawFunc(ptr);
  }
}

