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
#include "../resource/fldmapdata/build_model/buildmodel_outdoor.naix"


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  GAMEDATA * gamedata;
  FIELDMAP_WORK * fieldmap;
  HEAPID heapID;
  GFL_G3D_MAP_GLOBALOBJ_ST * st;
  FIELD_BMODEL * entry;
}FIELD_DOOR_ANIME_WORK;


static void makeRect(FLDHIT_RECT * rect, VecFx32 * pos);
static void getPlayerFrontPos(FIELDMAP_WORK * fieldmap, VecFx32 * pos);
//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT_RESULT FieldDoorInAnime(GMEVENT * event, int *seq, void * work)
{
  FIELD_DOOR_ANIME_WORK * fdaw = work;
  VecFx32 pos;
  FLDHIT_RECT rect;
  u32 num;
  FLDMAPPER * fldmapper = FIELDMAP_GetFieldG3Dmapper(fdaw->fieldmap);
  FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager(fldmapper);
  //u32 idx = FIELD_BMODEL_MAN_GetEntryIndex(bmodel_man, NARC_buildmodel_outdoor_p_door_nsbmd);
  u32 idx = FIELD_BMODEL_MAN_GetEntryIndex(bmodel_man, NARC_buildmodel_outdoor_door01_nsbmd);
  int i;

  switch (*seq) {
  case 0:
    fdaw->entry = NULL;
    getPlayerFrontPos(fdaw->fieldmap, &pos);
    makeRect(&rect, &pos);
    (const GFL_G3D_MAP_GLOBALOBJ_ST *)fdaw->st =
      FLDMAPPER_CreateObjStatusList( fldmapper, &rect, fdaw->heapID, &num);
    if (fdaw->st == NULL) return GMEVENT_RES_FINISH;
    for (i = 0; i < num; i++)
    {
      FIELD_BMODEL * bmodel;
      if (fdaw->st[i].id == idx)
      {
        fdaw->st[i].trans.z += 16 * FX32_ONE;
        bmodel = FIELD_BMODEL_Create(bmodel_man, fldmapper, &fdaw->st[i]);
        FIELD_BMODEL_MAN_EntryBuildModel(bmodel_man, bmodel);
        FIELD_BMODEL_SetAnime(bmodel, 0);
        fdaw->entry = bmodel;
        TAMADA_Printf("id:%2d rotate:%04x\n", fdaw->st[i].id, fdaw->st[i].rotate);
        TAMADA_Printf("x,y,z (%d,%d,%d)\n",
            FX_Whole(fdaw->st[i].trans.x),
            FX_Whole(fdaw->st[i].trans.y),
            FX_Whole(fdaw->st[i].trans.z) );
        break;
      }
    }
    

    ++ *seq;
    break;
  case 1:
    if (fdaw->entry == NULL)
    {
      ++ *seq;
      break;
    }
    if (FIELD_BMODEL_GetAnimeStatus(fdaw->entry) == TRUE)
    {
      FIELD_BMODEL_MAN_releaseBuildModel(bmodel_man, fdaw->entry);
      FIELD_BMODEL_Delete(fdaw->entry);
      fdaw->entry = NULL;
    }
    break;
  case 2:
    GFL_HEAP_FreeMemory((void*)fdaw->st);
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
GMEVENT * EVENT_FieldDoorInAnime(GAMESYS_WORK * gsys, FIELDMAP_WORK *fieldmap)
{
  GMEVENT * event;
  FIELD_DOOR_ANIME_WORK * fdaw;
  event = GMEVENT_Create( gsys, NULL, FieldDoorInAnime, sizeof(FIELD_DOOR_ANIME_WORK) );
  fdaw = GMEVENT_GetEventWork(event);
  fdaw->gsys = gsys;
  fdaw->gamedata = GAMESYSTEM_GetGameData(gsys);
  fdaw->fieldmap = fieldmap;
  fdaw->heapID = FIELDMAP_GetHeapID(fieldmap);
  return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void makeRect(FLDHIT_RECT * rect, VecFx32 * pos)
{
  rect->top = pos->z - 16 * FX32_ONE;
  rect->bottom = pos->z + 16 * FX32_ONE;
  rect->left = pos->x - 16 * FX32_ONE;
  rect->right = pos->x + 16 * FX32_ONE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void getPlayerFrontPos(FIELDMAP_WORK * fieldmap, VecFx32 * pos)
{
  enum { GRIDSIZE = 16 * FX32_ONE };
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldmap);
  u16 dir;

  FIELD_PLAYER_GetPos(player, pos);
  {
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( player );
    dir = MMDL_GetDirDisp( fmmdl );
  }
	switch( dir ) {
	case DIR_UP:		pos->z -= GRIDSIZE; break;
	case DIR_DOWN:	pos->z += GRIDSIZE; break;
	case DIR_LEFT:	pos->x -= GRIDSIZE; break;
	case DIR_RIGHT:	pos->x += GRIDSIZE; break;
	default:
                  GF_ASSERT(0);
  }
}



