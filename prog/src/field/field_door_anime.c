//============================================================================================
/**
 * @file field_door_anime.c
 * @brief
 * @date  2009.06.13
 * @author  tamada GAME FREAK inc.
 *
 */
//============================================================================================

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "fieldmap.h"
#include "field_player.h"
#include "field_buildmodel.h"

#include "field/field_const.h"
#include "field_door_anime.h"

//============================================================================================
//============================================================================================
static GMEVENT_RESULT FieldDoorAnimeEvent(GMEVENT * event, int *seq, void * work);

static void makeRect(FLDHIT_RECT * rect, const VecFx32 * pos);
static void getPlayerFrontPos(FIELDMAP_WORK * fieldmap, VecFx32 * pos);
static void * searchDoorObject(FIELDMAP_WORK *fieldmap, const VecFx32 *pos);


//============================================================================================
//============================================================================================
enum {
  //アニメデータの定義順に依存している。
  ANM_INDEX_DOOR_OPEN = 0,
  ANM_INDEX_DOOR_CLOSE,
};
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA * gamedata;
  FIELDMAP_WORK * fieldmap;
  HEAPID heapID;
  FIELD_BMODEL * entry;
  int anm_idx;
  VecFx32 pos;
}FIELD_DOOR_ANIME_WORK;

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT FieldDoorAnimeEvent(GMEVENT * event, int *seq, void * work)
{
  FIELD_DOOR_ANIME_WORK * fdaw = work;
  
  FLDMAPPER * fldmapper = FIELDMAP_GetFieldG3Dmapper(fdaw->fieldmap);
  FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager(fldmapper);

  switch (*seq) {
  case 0:
    fdaw->entry = searchDoorObject(fdaw->fieldmap, &fdaw->pos);
    if (fdaw->entry == NULL) return GMEVENT_RES_FINISH;
    FIELD_BMODEL_MAN_SetAnime(bmodel_man, fdaw->entry, fdaw->anm_idx, BMANM_REQ_START);
    ++ *seq;
    break;

  case 1:
    if (fdaw->entry == NULL)
    {
      ++ *seq;
      break;
    }
    if (FIELD_BMODEL_MAN_GetAnimeStatus(bmodel_man, fdaw->entry, fdaw->anm_idx) == TRUE)
    {
      if (fdaw->anm_idx == ANM_INDEX_DOOR_OPEN) {
        FIELD_BMODEL_MAN_SetAnime(bmodel_man, fdaw->entry, fdaw->anm_idx, BMANM_REQ_STOP);
      } else {
        FIELD_BMODEL_MAN_SetAnime(bmodel_man, fdaw->entry, fdaw->anm_idx, BMANM_REQ_END);
      }
      fdaw->entry = NULL;
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;

    ++ *seq;
    break;
  }
    //自機の目の前にドアを探す
    //  自機の目の前の領域を取得
    //  配置モデルを検索
    //  ヒットした配置モデルの中にドアがあるかどうかをチェック
    //
    //ドアを開くアニメを実行
    //  そのドアに適合したアニメを取得
    //自機がドアに入るアニメを実行
    //自機を消去
  return GMEVENT_RES_CONTINUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldDoorOpenAnime(GAMESYS_WORK * gsys, FIELDMAP_WORK *fieldmap)
{
  GMEVENT * event;
  FIELD_DOOR_ANIME_WORK * fdaw;
  event = GMEVENT_Create( gsys, NULL, FieldDoorAnimeEvent, sizeof(FIELD_DOOR_ANIME_WORK) );
  fdaw = GMEVENT_GetEventWork(event);
  fdaw->gsys = gsys;
  fdaw->gamedata = GAMESYSTEM_GetGameData(gsys);
  fdaw->fieldmap = fieldmap;
  fdaw->heapID = FIELDMAP_GetHeapID(fieldmap);
  fdaw->anm_idx = ANM_INDEX_DOOR_OPEN;
  getPlayerFrontPos(fieldmap, &fdaw->pos);
  return event;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldDoorClose(GAMESYS_WORK * gsys, FIELDMAP_WORK *fieldmap)
{
  GMEVENT * event;
  FIELD_DOOR_ANIME_WORK * fdaw;
  event = GMEVENT_Create(gsys, NULL, FieldDoorAnimeEvent, sizeof(FIELD_DOOR_ANIME_WORK));
  fdaw = GMEVENT_GetEventWork(event);
  fdaw->gsys = gsys;
  fdaw->gamedata = GAMESYSTEM_GetGameData(gsys);
  fdaw->fieldmap = fieldmap;
  fdaw->heapID = FIELDMAP_GetHeapID(fieldmap);
  fdaw->anm_idx = ANM_INDEX_DOOR_CLOSE;
  {
    FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
    FIELD_PLAYER_GetPos(player, &fdaw->pos);
  }
  return event;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------

//============================================================================================
//============================================================================================
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
//------------------------------------------------------------------
static void getPlayerFrontPos(FIELDMAP_WORK * fieldmap, VecFx32 * pos)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  u16 dir;

  FIELD_PLAYER_GetPos(player, pos);
  {
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( player );
    dir = MMDL_GetDirDisp( fmmdl );
  }
	switch( dir ) {
	case DIR_UP:		pos->z -= FIELD_CONST_GRID_FX32_SIZE; break;
	case DIR_DOWN:	pos->z += FIELD_CONST_GRID_FX32_SIZE; break;
	case DIR_LEFT:	pos->x -= FIELD_CONST_GRID_FX32_SIZE; break;
	case DIR_RIGHT:	pos->x += FIELD_CONST_GRID_FX32_SIZE; break;
	default:
                  GF_ASSERT(0);
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void * searchDoorObject(FIELDMAP_WORK *fieldmap, const VecFx32 *pos)
{
  FLDHIT_RECT rect;
  void * entry = NULL;

  makeRect(&rect, pos);

  {
    FLDMAPPER * fldmapper = FIELDMAP_GetFieldG3Dmapper(fieldmap);
    FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager(fldmapper);
    const GFL_G3D_MAP_GLOBALOBJ_ST * st;
    u32 num;
    int i;

    st = FLDMAPPER_CreateObjStatusList( fldmapper, &rect, &num);
    if (st == NULL) return NULL;

    for (i = 0; i < num; i++)
    {
      if (FIELD_BMODEL_MAN_IsDoor(bmodel_man, &st[i]) == TRUE)
      {
        entry = FIELD_BMODEL_MAN_GetObjHandle(bmodel_man, &st[i]);
        break;
      }
    }
    GFL_HEAP_FreeMemory((void*)st);
  }
  return entry;
}




