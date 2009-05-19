//============================================================================================
/**
 * @file  field_rail.c
 * @brief ノーグリッドでの自機・カメラ制御構造
 * @author  tamada, tomoya
 * @date  2009.05.18
 *
 */
//============================================================================================

#include <gflib.h>

#include "field_rail.h"


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------


//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
typedef enum {  
  FIELD_RAIL_TYPE_POINT = 0,
  FIELD_RAIL_TYPE_LINE,

  FIELD_RAIL_TYPE_MAX
}FIELD_RAIL_TYPE;


//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
typedef struct {  
  FIELD_RAIL_TYPE type;
  union { 
    void * dummy;
    const RAIL_POINT * point;
    const RAIL_LINE * line;
  };
}FIELD_RAIL;


//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
struct _FIELD_RAIL_MAN{
  HEAPID heapID;

  u32 line_ofs;     ///<LINEにいる間の、LINE上でのオフセット位置
  RAIL_KEY last_active_key;
  FIELD_RAIL nowRail;

  //いずれ必要になるが今考えると混乱する
  //u32 width_ofs;    ///<LINEにいる間の幅の動きオフセット位置

};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY keyContToRailKey(int cont);
static RAIL_KEY getReverseKey(RAIL_KEY key);

//------------------------------------------------------------------
//------------------------------------------------------------------
static void initRail(FIELD_RAIL * rail);
static BOOL isValidRail(const FIELD_RAIL * rail);
static const char * getRailName(const FIELD_RAIL * rail);


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief レール制御システムの生成
 */
//------------------------------------------------------------------
FIELD_RAIL_MAN * FIELD_RAIL_MAN_Create(HEAPID heapID)
{ 
  FIELD_RAIL_MAN * man = GFL_HEAP_AllocMemory(heapID, sizeof(FIELD_RAIL_MAN));

  man->heapID = heapID;
  man->line_ofs = 0;
  man->last_active_key = RAIL_KEY_NULL;
  initRail(&man->nowRail);

  return man;
}

//------------------------------------------------------------------
/**
 * @brief レール制御システムの削除
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_Delete(FIELD_RAIL_MAN * man)
{ 
  GFL_HEAP_FreeMemory(man);
}

//------------------------------------------------------------------
/**
 * @brief レールデータの読み込み
 *
 * とりあえず、最初はPOINTから始まると仮定する
 *
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_Load(FIELD_RAIL_MAN * man, const RAIL_POINT * railPointData)
{ 
  FIELD_RAIL * rail = &man->nowRail;
  rail->type = FIELD_RAIL_TYPE_POINT;
  rail->point = railPointData;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_Update(FIELD_RAIL_MAN * man, int key_cont)
{ 
  FIELD_RAIL_TYPE type = man->nowRail.type;
  RAIL_KEY set_key = RAIL_KEY_NULL;
  RAIL_KEY before_key = man->last_active_key;
  RAIL_KEY key = keyContToRailKey(key_cont);

  if (isValidRail(&man->nowRail) == FALSE)
  {
    return;
  }
  if (key == RAIL_KEY_NULL)
  {
    return;
  }

  if(man->nowRail.type == FIELD_RAIL_TYPE_POINT)
  { //POINT上のときの移動処理
    int i;
    const RAIL_POINT * nPoint = man->nowRail.point;
    for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
    {
      if (nPoint->keys[i] == key)
      { // POINT --> LINE[i] への移行処理
        const RAIL_LINE * nLine = nPoint->line[i];
        if (nLine->point[0] == nPoint)
        { 
          man->line_ofs = 1;  //最初は１から！
        }
        else if (nLine->point[1] == nPoint)
        {
          man->line_ofs = nLine->line_divider - 1; //最後ー1から！
        }
        //つながったLINEを次のRAILとしてセット
        man->nowRail.type = FIELD_RAIL_TYPE_LINE;
        man->nowRail.line = nLine;
        set_key = key;
        
      }
    }
  }
  else if(man->nowRail.type == FIELD_RAIL_TYPE_LINE)
  { //LINE上のときの移動処理
    const RAIL_LINE * nLine = man->nowRail.line;
    if (key == nLine->key)
    {
      set_key = key;
      man->line_ofs ++;
      if (man->line_ofs == nLine->line_divider)
      { // LINE --> POINT[1] への移行処理
        man->nowRail.type = FIELD_RAIL_TYPE_POINT;
        man->nowRail.point = nLine->point[1];
      }
    }
    else if (key == getReverseKey(nLine->key))
    {
      set_key = key;
      man->line_ofs --;
      if (man->line_ofs == 0)
      { // LINE --> POINT[0] への移行処理
        man->nowRail.type = FIELD_RAIL_TYPE_POINT;
        man->nowRail.point = nLine->point[0];
      }
    }
  }

  if (set_key != RAIL_KEY_NULL)
  {
    TAMADA_Printf("RAIL:active key ofs=%d\n", man->line_ofs);
    man->last_active_key = set_key;
  }

  if (type != man->nowRail.type)
  {
    TAMADA_Printf("RAIL:change to %d(%s:%08x)\n",
        man->nowRail.type,
        getRailName(&man->nowRail),
        man->nowRail.dummy);
  }
}


//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_GetPos(const FIELD_RAIL_MAN * man, VecFx32 * pos)
{
  RAIL_POS_FUNC * func = NULL;

  if(man->nowRail.type == FIELD_RAIL_TYPE_POINT)
  {
    //POINTにいるときはPOINTの座標
    *pos = man->nowRail.point->pos;
  }
  else if(man->nowRail.type == FIELD_RAIL_TYPE_LINE)
  {
    func = man->nowRail.line->line_pos_set->func;
    if (func == NULL)
    {
      func = FIELD_RAIL_POSFUNC_StraitLine;
    }
    func(man, pos);
  }
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_GetCameraPos(const FIELD_RAIL_MAN * man, VecFx32 * CamPos)
{
  const FIELD_RAIL * rail = &man->nowRail;
  RAIL_POS_FUNC * func = NULL;
  switch (rail->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    if (rail->point->camera_set)
    {
      func = rail->point->camera_set->func;
    }
    break;
  case FIELD_RAIL_TYPE_LINE:
    if (rail->line->camera_set)
    {
      func = rail->line->camera_set->func;
    }
    break;
  default:
    GF_ASSERT(0); //NO TYPE;
  }
  if (func)
  {
    func(man, CamPos);
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_RAIL_MAN_GetDirection(const FIELD_RAIL_MAN * man, VecFx32 * dir)
{
  const FIELD_RAIL * rail = &man->nowRail;
  switch (rail->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    break;
  case FIELD_RAIL_TYPE_LINE:
    break;
  default:
    break;
  }
}

//============================================================================================
/**
 */
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void initRail(FIELD_RAIL * rail)
{
  rail->type = FIELD_RAIL_TYPE_MAX;
  rail->dummy = NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL isValidRail(const FIELD_RAIL * rail)
{
  if (rail->type < FIELD_RAIL_TYPE_MAX && rail->dummy != NULL)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static const char * getRailName(const FIELD_RAIL * rail)
{
  switch (rail->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    return rail->point->name;
  case FIELD_RAIL_TYPE_LINE:
    return rail->line->name;
  }
  return "NO ENTRY";
}
//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
static RAIL_KEY keyContToRailKey(int cont)
{
  if (cont & PAD_KEY_UP) return RAIL_KEY_UP;
  if (cont & PAD_KEY_LEFT) return RAIL_KEY_LEFT;
  if (cont & PAD_KEY_RIGHT) return RAIL_KEY_RIGHT;
  if (cont & PAD_KEY_DOWN) return RAIL_KEY_DOWN;
  return RAIL_KEY_NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY getReverseKey(RAIL_KEY key)
{
  switch (key) {
  case RAIL_KEY_UP: 
    return RAIL_KEY_DOWN;
  case RAIL_KEY_DOWN:
    return RAIL_KEY_UP;
  case RAIL_KEY_LEFT:
    return RAIL_KEY_RIGHT;
  case RAIL_KEY_RIGHT:
    return RAIL_KEY_LEFT;
  }
  return RAIL_KEY_NULL;
}

//============================================================================================
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_RAIL_POSFUNC_StraitLine(const FIELD_RAIL_MAN * man, VecFx32 * pos)
{
    //LINEにいるときはPOINTとPOINTの間の座標
    //pos = p0 + (p1 - p0) * (ofs / div )
    const RAIL_LINE * nLine = man->nowRail.line;
    fx32 ofs = (man->line_ofs * FX32_ONE) / nLine->line_divider;
    VecFx32 val;
    const VecFx32 * p0 = &nLine->point[0]->pos;
    const VecFx32 * p1 = &nLine->point[1]->pos;
    VEC_Subtract(p1, p0, &val);
    VEC_MultAdd(ofs, &val, p0, pos);
}

const RAIL_CAMERA_SET RAIL_CAMERA_SET_Default =
{
  NULL,
  0,
  0,
  0,
  0,
};

const RAIL_LINEPOS_SET RAIL_LINEPOS_SET_Default =
{
  NULL,
  0,
  0,
  0,
  0,
};



