//============================================================================================
/**
 * @file field_bmanime_tool.c
 * @brief 配置モデルアニメコントローラー
 * @date  2009.10.25
 * @author  tamada GAME FREAK inc.
 *
 * @todo
 * event --> field_bmanime_tool --> field_buildmodel(FIELD_BMODEL-->OBJHND) --> GFL_G3D_OBJECT
 * と、階層が深くなっているのをどうするか考える。
 */
//============================================================================================

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/field_const.h"

#include "fieldmap.h"
#include "field_player.h"
#include "field_buildmodel.h"

#include "field_bmanime_tool.h"

#include "script_def.h"
//============================================================================================
//============================================================================================
struct _BMANIME_CONTROL_WORK {
  FIELD_BMODEL_MAN * man;
  FIELD_BMODEL * entry;
  G3DMAPOBJST * obj;
  u16 anm_idx;
};

//============================================================================================
//============================================================================================
static void makeRect(FLDHIT_RECT * rect, const VecFx32 * pos);
static G3DMAPOBJST * searchDoorObject(FIELD_BMODEL_MAN * bmodel_man, const VecFx32 * pos);
//------------------------------------------------------------------
//------------------------------------------------------------------
static void makeRect(FLDHIT_RECT * rect, const VecFx32 * pos)
{
  enum { RECT_SIZE = (FIELD_CONST_GRID_SIZE * 2) << FX32_SHIFT };
  rect->top = pos->z - RECT_SIZE;
  rect->bottom = pos->z + RECT_SIZE;
  rect->left = pos->x - RECT_SIZE;
  rect->right = pos->x + RECT_SIZE;
}

//------------------------------------------------------------------
/**
 * @brief 指定位置付近のドア配置モデルを検索する
 * @param bmodel_man
 * @param pos
 * @return NULL   見つからなかった
 * @return G3DMAPOBJST *  見つけたドア配置モデルへの参照
 */
//------------------------------------------------------------------
static G3DMAPOBJST * searchDoorObject(FIELD_BMODEL_MAN * bmodel_man, const VecFx32 * pos)
{
  G3DMAPOBJST * entry = NULL;
  G3DMAPOBJST ** array;
  u32 result_num;

  {
    //検索矩形を作成
    FLDHIT_RECT rect;
    makeRect(&rect, pos);
    //矩形範囲内の配置モデルリストを生成する
    array = FIELD_BMODEL_MAN_CreateObjStatusList(bmodel_man, &rect, &result_num);
  }
  {
    int i;
    for (i = 0; i < result_num; i++)
    {
      if (FIELD_BMODEL_MAN_G3DMAPOBJSTisDoor(bmodel_man, array[i]) == TRUE)
      {
        //取得した配置モデルリストから、ドアであるかチェック
        entry = array[i];
        break;
      }
    }
  }
  //矩形範囲内の配置モデルリストを解放する
  GFL_HEAP_FreeMemory(array);
  return entry;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BMANIME_CONTROL_WORK * BMANIME_CTRL_Create(FIELD_BMODEL_MAN * bmodel_man, const VecFx32 * pos)
{
  BMANIME_CONTROL_WORK * ctrl;
  G3DMAPOBJST * obj;
  FIELD_BMODEL * entry;

  GF_ASSERT( SCR_BMANM_DOOR_OPEN == ANM_INDEX_DOOR_OPEN );
  GF_ASSERT( SCR_BMANM_DOOR_CLOSE == ANM_INDEX_DOOR_CLOSE );

  obj = searchDoorObject(bmodel_man, pos);
  if (obj == NULL) return NULL;
  entry = FIELD_BMODEL_Create( bmodel_man, obj );
  if (entry == NULL) return NULL;
  FIELD_BMODEL_MAN_EntryBuildModel( bmodel_man, entry );
  G3DMAPOBJST_changeViewFlag(obj, FALSE);

  ctrl = GFL_HEAP_AllocClearMemory(
      FIELD_BMODEL_MAN_GetHeapID(bmodel_man), sizeof(BMANIME_CONTROL_WORK) );
  ctrl->man = bmodel_man;
  ctrl->obj = obj;
  ctrl->entry = entry;
  ctrl->anm_idx = 0xffff;
  return ctrl;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void BMANIME_CTRL_Delete(BMANIME_CONTROL_WORK * ctrl)
{
  if (ctrl == NULL) return;
  if (ctrl->entry)
  {
    FIELD_BMODEL_MAN_releaseBuildModel( ctrl->man, ctrl->entry );
    FIELD_BMODEL_Delete( ctrl->entry );
    ctrl->entry = NULL;
  }
  if (ctrl->obj)
  {
    G3DMAPOBJST_changeViewFlag(ctrl->obj, TRUE);
    ctrl->obj = NULL;
  }
  GFL_HEAP_FreeMemory( ctrl );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void BMANIME_CTRL_SetAnime(BMANIME_CONTROL_WORK * ctrl, u32 anm_idx)
{
  if (ctrl == NULL) return;
  if (ctrl->anm_idx != 0xffff) {
    FIELD_BMODEL_SetAnime( ctrl->entry, ctrl->anm_idx, BMANM_REQ_END);
  }
  ctrl->anm_idx = anm_idx;
  FIELD_BMODEL_SetAnime( ctrl->entry, anm_idx, BMANM_REQ_START);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL BMANIME_CTRL_WaitAnime(BMANIME_CONTROL_WORK * ctrl)
{
  if (ctrl == NULL) return TRUE;
  if (ctrl->anm_idx == 0xffff) return TRUE;
  if ( FIELD_BMODEL_GetAnimeStatus( ctrl->entry, ctrl->anm_idx) == TRUE)
  {
    FIELD_BMODEL_SetAnime( ctrl->entry, ctrl->anm_idx, BMANM_REQ_STOP);
    return TRUE;
  }
  return FALSE;
}

