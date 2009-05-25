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

#include "field/fieldmap_proc.h"

#include "field_rail.h"

#include "field_camera.h"

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
struct _FIELD_RAIL{  
  FIELD_RAIL_TYPE type;
  union { 
    void * dummy;
    const RAIL_POINT * point;
    const RAIL_LINE * line;
  };
  /// LINEにいる間の、LINE上でのオフセット位置
  s32 line_ofs;
  /// LINEにいる間の幅オフセット位置(XZ平面上でLINEに垂直)
  s32 width_ofs;

  RAIL_KEY key;

  /// 幅情報
  RAIL_WIDTH width;
};

//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
struct _FIELD_RAIL_MAN{
  HEAPID heapID;

  FIELD_CAMERA * field_camera;

  BOOL active_flag;

  /// 移動が起きた最新のキーバリュー
  RAIL_KEY last_active_key;

  FIELD_RAIL now_rail;

};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY keyContToRailKey(int cont);
static RAIL_KEY getReverseKey(RAIL_KEY key);
static RAIL_KEY getClockwiseKey(RAIL_KEY key);
static RAIL_KEY getAntiClockwiseKey(RAIL_KEY key);

//------------------------------------------------------------------
//------------------------------------------------------------------
static void initRail(FIELD_RAIL * rail);
static BOOL isValidRail(const FIELD_RAIL * rail);
static const RAIL_CAMERA_SET * getCameraSet(const FIELD_RAIL * rail);
static const char * getRailName(const FIELD_RAIL * rail);
static void getRailPosition(const FIELD_RAIL * rail, VecFx32 * pos);


//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL debugCheckLineData(const RAIL_LINE * line);
static BOOL debugCheckPointData(const RAIL_POINT * point);
static void debugPrintPoint(const char * before, const RAIL_POINT * point, const char * after);
static const char * debugGetRailKeyName(RAIL_KEY key);
static void debugPrintRailInfo(const FIELD_RAIL * rail);


static RAIL_KEY updateLineMove_new(FIELD_RAIL * rail, RAIL_KEY key);
//static RAIL_KEY updateLineMove(FIELD_RAIL * rail, RAIL_KEY key);
static RAIL_KEY updatePointMove(FIELD_RAIL * rail, RAIL_KEY key);

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief レール制御システムの生成
 */
//------------------------------------------------------------------
FIELD_RAIL_MAN * FIELD_RAIL_MAN_Create(HEAPID heapID, FIELD_CAMERA * camera)
{ 
  FIELD_RAIL_MAN * man = GFL_HEAP_AllocMemory(heapID, sizeof(FIELD_RAIL_MAN));

  man->heapID = heapID;
  man->active_flag = FALSE;
  man->field_camera = camera;
  //man->line_ofs = 0;
  man->last_active_key = RAIL_KEY_NULL;
  initRail(&man->now_rail);

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
void FIELD_RAIL_MAN_Load(FIELD_RAIL_MAN * man, const RAIL_POINT * railPointData, const RAIL_WIDTH * railWidth)
{ 
  FIELD_RAIL * rail = &man->now_rail;
  rail->type = FIELD_RAIL_TYPE_POINT;
  rail->point = railPointData;
  rail->width = *railWidth;
  man->active_flag = TRUE;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_SetActiveFlag(FIELD_RAIL_MAN * man, BOOL flag)
{
  man->active_flag = flag;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_GetActiveFlag(const FIELD_RAIL_MAN *man)
{
  return man->active_flag;
}

//------------------------------------------------------------------
/**
 * @brief 現在位置の取得
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_GetPos(const FIELD_RAIL_MAN * man, VecFx32 * pos)
{
  getRailPosition(&man->now_rail, pos);
}

//------------------------------------------------------------------
/**
 * @brief カメラ状態のアップデート
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_UpdateCamera(const FIELD_RAIL_MAN * man)
{
  const FIELD_RAIL * rail = &man->now_rail;
  RAIL_CAMERA_FUNC * func = NULL;
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
    func(man);
  }
}

//------------------------------------------------------------------
/**
 * @brief 現在位置のアップデート
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_Update(FIELD_RAIL_MAN * man, int key_cont)
{ 
  FIELD_RAIL_TYPE type = man->now_rail.type;
  RAIL_KEY set_key = RAIL_KEY_NULL;
  RAIL_KEY key = keyContToRailKey(key_cont);

  if (!man->active_flag)
  {
    return;
  }

  if (isValidRail(&man->now_rail) == FALSE)
  {
    return;
  }

  if (key == RAIL_KEY_NULL)
  {
    return;
  }

  if(man->now_rail.type == FIELD_RAIL_TYPE_POINT)
  { //POINT上のときの移動処理
    set_key = updatePointMove(&man->now_rail, key);
  }
  else if(man->now_rail.type == FIELD_RAIL_TYPE_LINE)
  { //LINE上のときの移動処理
    set_key = updateLineMove_new(&man->now_rail, key);
  }

  if (set_key != RAIL_KEY_NULL)
  {
    OS_TPrintf("RAIL:%s :line_ofs=%d width_ofs=%d\n",
        debugGetRailKeyName(set_key), man->now_rail.line_ofs, man->now_rail.width_ofs);
    man->last_active_key = set_key;
  }

  if (type != man->now_rail.type)
  {
    OS_TPrintf("RAIL:change to ");
    debugPrintRailInfo(&man->now_rail);
  }
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static const RAIL_LINE * RAILPOINT_getLineByKey(const RAIL_POINT * point, RAIL_KEY key)
{
  int i;
  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
    if (point->keys[i] == key)
    {
      return point->lines[i];
    }
  }
  return NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY setLine(FIELD_RAIL * rail, const RAIL_LINE * line, RAIL_KEY key, int l_ofs, int w_ofs)
{
  GF_ASSERT(rail->line != line);
  GF_ASSERT(rail->line);
  GF_ASSERT(line->name);
  if (rail->type == FIELD_RAIL_TYPE_LINE)
  {
    TAMADA_Printf("RAIL: LINE \"%s\" to %s \"%s\"\n",
        rail->line->name, debugGetRailKeyName(key), line->name);
  }
  else
  {
    TAMADA_Printf("RAIL: POINT \"%s\" to %s \"%s\"\n",
        rail->point->name, debugGetRailKeyName(key), line->name);
  }
  rail->type = FIELD_RAIL_TYPE_LINE;
  rail->line = line;
  rail->key = key;
  rail->line_ofs = l_ofs;
  rail->width_ofs = w_ofs;

  debugCheckLineData(line);
  return key;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY updateLineMove_new(FIELD_RAIL * rail, RAIL_KEY key)
{ //LINE上のときの移動処理
  const RAIL_LINE * nLine = rail->line;

  if (key == nLine->key)
  {//正方向キーの場合
    const RAIL_POINT* nPoint = nLine->point_e;
    const RAIL_LINE * front = RAILPOINT_getLineByKey(nPoint, key);
    const RAIL_LINE * left = RAILPOINT_getLineByKey(nPoint, getAntiClockwiseKey(key));
    const RAIL_LINE * right = RAILPOINT_getLineByKey(nPoint, getClockwiseKey(key));
    //const RAIL_LINE * back = RAILPOINT_getLineByKey(nPoint, getReverseKey(key));
    TAMADA_Printf("↑");
    rail->line_ofs ++;
    if (rail->line_ofs <= nLine->ofs_max) return key;
    if (front)
    { //正面移行処理
      debugCheckPointData(nPoint);
      return setLine(rail, front, key,
          /*l_ofs*/1,
          /*w_ofs*/rail->width_ofs); //そのまま
    }
    if (left && rail->width_ofs < 0)
    { //左側移行処理
      if (left->point_s == nPoint)
      { //始端の場合
        debugCheckPointData(nPoint);
        return setLine(rail, left, key,
            /*l_ofs*/rail->width_ofs,
            /*w_ofs*/0);
      }
      else if (left->point_e == nPoint)
      { //終端の場合
        debugCheckPointData(nPoint);
        return setLine(rail, left, key,
            /*l_ofs*/left->ofs_max + rail->width_ofs, //width_ofs < 0なので実質減算
            /*w_ofs*/0);
      }
      else GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
    }
    if (right && rail->width_ofs > 0)
    { //右側移行処理
      if (right->point_s == nPoint)
      { //始端の場合
        debugCheckPointData(nPoint);
        return setLine(rail, right, key,
            /*l_ofs*/rail->width_ofs,
            /*w_ofs*/0);
      }
      else if (right->point_e == nPoint)
      { //終端の場合
        debugCheckPointData(nPoint);
        return setLine(rail, right, key,
            /*l_ofs*/right->ofs_max - rail->width_ofs,
            /*w_ofs*/0);
      }
      else GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
    }

    //行くあてがない…LINEそのまま、加算をとりけし
    rail->line_ofs = nLine->ofs_max;
    return key;
  }
  else if (key == getReverseKey(nLine->key))
  {//逆方向キーの場合
    const RAIL_POINT* nPoint = nLine->point_s;
    //const RAIL_LINE * front = RAILPOINT_getLineByKey(nPoint, key);
    const RAIL_LINE * left = RAILPOINT_getLineByKey(nPoint, getClockwiseKey(key));
    const RAIL_LINE * right = RAILPOINT_getLineByKey(nPoint, getAntiClockwiseKey(key));
    const RAIL_LINE * back = RAILPOINT_getLineByKey(nPoint, key);
    TAMADA_Printf("↓");
    rail->line_ofs --;
    if (rail->line_ofs >= 0) return key;
    if (back)
    {//背面移行処理
      debugCheckPointData(nPoint);
      return setLine(rail, back, key,
          /*l_ofs*/back->ofs_max - 1,
          /*w_ofs*/rail->width_ofs);
    }
    if (left && rail->width_ofs < 0)
    { //左側移行処理
      if (left->point_s == nPoint)
      { //始端の場合
        debugCheckPointData(nPoint);
        return setLine(rail, left, key,
            /*l_ofs*/rail->width_ofs,
            /*w_ofs*/0);
      }
      else if (left->point_e == nPoint)
      { //終端の場合
        debugCheckPointData(nPoint);
        return setLine(rail, right, key,
            /*l_ofs*/left->ofs_max + rail->width_ofs, //width_ofs < 0なので実質減算
            /*w_ofs*/0);
      }
      else GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
    }
    if (right && rail->width_ofs > 0)
    { //右側移行処理
      if (right->point_s == nPoint)
      { //始端の場合
        debugCheckPointData(nPoint);
        return setLine(rail, right, key,
            /*l_ofs*/rail->width_ofs,
            /*w_ofs*/0);
      }
      if (right->point_e == nPoint)
      { //終端の場合
        debugCheckPointData(nPoint);
        return setLine(rail, right, key,
            /*l_ofs*/right->ofs_max - rail->width_ofs,
            /*w_ofs*/0);
      }
      else GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, right->name);
    }
    //行くあてがない…LINEそのまま、減算を取り消し
    rail->line_ofs = 0;
    return key;
  }
  else if (key == getClockwiseKey(nLine->key))
  {//時計回り隣方向キーの場合
    TAMADA_Printf("→");
    rail->width_ofs ++;
    if (rail->width_ofs < rail->width.ofs_max)
    {//範囲内の場合、終了
      return key;
    }
    else if (rail->line_ofs < rail->width.ofs_max)
    { //始端に近い場合
      const RAIL_POINT* nPoint = nLine->point_s;
      const RAIL_LINE *right = RAILPOINT_getLineByKey(nPoint, key);
      if (right)
      {
        if (right->point_s == nPoint)
        {//右側LINEは始端からの場合
          debugCheckPointData(nPoint);
          return setLine(rail, right, key,
              rail->width_ofs,
              - rail->line_ofs);
        }
        if (right->point_e == nPoint)
        {//右側LINEは終端からの場合
          debugCheckPointData(nPoint);
          return setLine(rail, right, key,
              right->ofs_max - rail->width_ofs,
              rail->line_ofs);
        }
        GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, right->name);
      }
    }
    else if (rail->line_ofs >= rail->line->ofs_max - rail->width.ofs_max)
    { //終端に近い場合
      const RAIL_POINT* nPoint = nLine->point_e;
      const RAIL_LINE *right = RAILPOINT_getLineByKey(nPoint, key);
      if (right)
      {
        if (right->point_s == nPoint)
        {//右側LINEは始端からの場合
          debugCheckPointData(nPoint);
          return setLine(rail, right, key,
              rail->width_ofs,
              nLine->ofs_max - rail->line_ofs);
        }
        if (right->point_e == nPoint)
        {//右側LINEは終端からの場合
          debugCheckPointData(nPoint);
          return setLine(rail, right, key,
              right->ofs_max - rail->width_ofs,
              - (nLine->ofs_max - rail->line_ofs));
        }
        GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, right->name);
      }
    }
    //中間地点の場合、行くあてがない場合…LINEそのまま、加算を取り消し
    rail->width_ofs = rail->width.ofs_max - 1;
    return key;
  }
  else if (key == getAntiClockwiseKey(nLine->key))
  {//反時計回り隣方向キーの場合
    TAMADA_Printf("←");
    rail->width_ofs --;
    if (MATH_ABS(rail->width_ofs) < rail->width.ofs_max)
    {//範囲内の場合、終了
      return key;
    }
    else if (rail->line_ofs < rail->width.ofs_max)
    {//始端に近い場合
      const RAIL_POINT* nPoint = nLine->point_s;
      const RAIL_LINE* left = RAILPOINT_getLineByKey(nPoint, key);
      if (left)
      {
        if (left->point_s == nPoint)
        {//左側LINEが始端からの場合
          debugCheckPointData(nPoint);
          return setLine(rail, left, key,
              MATH_ABS(rail->width_ofs), 
              rail->line_ofs );
        }
        if (left->point_e == nPoint)
        {//左側LINEが終端からの場合
          debugCheckPointData(nPoint);
          return setLine(rail, left, key,
              left->ofs_max - MATH_ABS(rail->width_ofs),
              - rail->line_ofs );
        }
        GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
      }
    }
    if (rail->line_ofs >= nLine->ofs_max - rail->width.ofs_max)
    {//終端に近い場合
      const RAIL_POINT* nPoint = nLine->point_e;
      const RAIL_LINE* left = RAILPOINT_getLineByKey(nPoint, key);
      if (left)
      {
        if (left->point_s == nPoint)
        {//左側LINEが始端からの場合
          debugCheckPointData(nPoint);
          return setLine(rail, left, key,
              MATH_ABS(rail->width_ofs),
              - (nLine->ofs_max - rail->line_ofs));
        }
        if (left->point_e == nPoint)
        {//左側LINEが終端からの場合
          debugCheckPointData(nPoint);
          return setLine(rail, left, key,
              left->ofs_max - MATH_ABS(rail->width_ofs),
              nLine->ofs_max - rail->line_ofs);
        }
        GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
      }
    }
    //中間地点の場合、行くあてがない場合…LINEそのまま、減算を取り消し
    rail->width_ofs ++;
    return key;
  }
  return RAIL_KEY_NULL;
}

#if 0
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY updateLineMove(FIELD_RAIL * rail, RAIL_KEY key)
{ //LINE上のときの移動処理
  const RAIL_LINE * nLine = rail->line;
  if (key == nLine->key)
  {//正方向キーの場合
    rail->line_ofs ++;
    if (rail->line_ofs == nLine->ofs_max)
    { // LINE --> point_e への移行処理
      rail->type = FIELD_RAIL_TYPE_POINT;
      rail->point = nLine->point_e;
      debugCheckPointData(rail->point);
    }
    return key;
  }
  else if (key == getReverseKey(nLine->key))
  {//逆方向キーの場合
    rail->line_ofs --;
    if (rail->line_ofs == 0)
    { // LINE --> POINT_S への移行処理
      rail->type = FIELD_RAIL_TYPE_POINT;
      rail->point = nLine->point_s;
      debugCheckPointData(rail->point);
    }
    return key;
  }
  else if (key == getClockwiseKey(nLine->key))
  {//時計回りとなり方向キーの場合
    if (rail->width_ofs < rail->width.ofs_max)
    {
      rail->width_ofs ++;
    }
    return key;
  }
  else if (key == getAntiClockwiseKey(nLine->key))
  {//反時計回りとなり方向キーの場合
    if (rail->width_ofs > - rail->width.ofs_max)
    {
      rail->width_ofs --;
    }
    return key;
  }
  return RAIL_KEY_NULL;
}
#endif

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY updatePointMove(FIELD_RAIL * rail, RAIL_KEY key)
{//POINT上のときの移動処理
  int i;
  const RAIL_POINT * nPoint = rail->point;
  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
    if (nPoint->keys[i] == key)
    { // POINT --> LINE[i] への移行処理
      const RAIL_LINE * nLine = nPoint->lines[i];
      if (nLine->point_s == nPoint)
      { 
        setLine(rail, nLine, key, 1, 0);
        //rail->line_ofs = 1;  //最初は１から！
      }
      else if (nLine->point_e == nPoint)
      {
        setLine(rail, nLine, key, nLine->ofs_max - 1, 0);
        //rail->line_ofs = nLine->ofs_max - 1; //最後ー1から！
      }
      //つながったLINEを次のRAILとしてセット
      //rail->type = FIELD_RAIL_TYPE_LINE;
      //rail->line = nLine;
      //debugCheckLineData(rail->line);
      return key;
    }
  }
  return RAIL_KEY_NULL;
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
  rail->line_ofs = 0;
  rail->width_ofs = 0;
  rail->key = RAIL_KEY_NULL;
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
static const RAIL_CAMERA_SET * getCameraSet(const FIELD_RAIL * rail)
{
  switch (rail->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    return rail->point->camera_set;
    break;
  case FIELD_RAIL_TYPE_LINE:
    return rail->line->camera_set;
    break;
  }
  GF_ASSERT(0);
  return &RAIL_CAMERA_SET_Default;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void getRailPosition(const FIELD_RAIL * rail, VecFx32 * pos)
{
  RAIL_POS_FUNC * func = NULL;

  if(rail->type == FIELD_RAIL_TYPE_POINT)
  {
    //POINTにいるときはPOINTの座標
    *pos = rail->point->pos;
  }
  else if(rail->type == FIELD_RAIL_TYPE_LINE)
  {
    func = rail->line->line_pos_set->func;
    if (func == NULL)
    {
      func = FIELD_RAIL_POSFUNC_StraitLine;
    }
    func(rail, pos);
  }
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
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY getClockwiseKey(RAIL_KEY key)
{
  switch (key) {
  case RAIL_KEY_UP: 
    return RAIL_KEY_RIGHT;
  case RAIL_KEY_DOWN:
    return RAIL_KEY_LEFT;
  case RAIL_KEY_LEFT:
    return RAIL_KEY_UP;
  case RAIL_KEY_RIGHT:
    return RAIL_KEY_DOWN;
  }
  return RAIL_KEY_NULL;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY getAntiClockwiseKey(RAIL_KEY key)
{
  switch (key) {
  case RAIL_KEY_UP: 
    return RAIL_KEY_LEFT;
  case RAIL_KEY_DOWN:
    return RAIL_KEY_RIGHT;
  case RAIL_KEY_LEFT:
    return RAIL_KEY_DOWN;
  case RAIL_KEY_RIGHT:
    return RAIL_KEY_UP;
  }
  return RAIL_KEY_NULL;
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
//------------------------------------------------------------------
static RAIL_KEY searchLineInPoint(const RAIL_POINT *point, const RAIL_LINE * line)
{
  int i;
  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
    if (point->lines[i] == line) return point->keys[i];
  }
  return RAIL_KEY_NULL;
}

//============================================================================================
//
//
//    デバッグ用関数
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL debugCheckLineData(const RAIL_LINE * line)
{

  RAIL_KEY key;

  if(line->point_s == NULL || line->point_e == NULL)
  {
    OS_TPrintf("%sはPOINTが正しくありません\n", line->name);
    return FALSE;
  }
  key = searchLineInPoint(line->point_s, line);
  if (key == RAIL_KEY_NULL)
  {
    OS_TPrintf("%sは%sと正しく接続できていません\n", line->name, line->point_s->name);
    return FALSE;
  }
  if (key != line->key)
  {
    OS_TPrintf("%sは始端%sとキーがあっていません\n", line->name, line->point_s->name);
    return FALSE;
  }
  key = searchLineInPoint(line->point_e, line);
  if (key == RAIL_KEY_NULL)
  {
    OS_TPrintf("%sは%sと正しく接続できていません\n", line->name, line->point_e->name);
    return FALSE;
  }
  if (key != getReverseKey(line->key))
  {
    OS_TPrintf("%sは終端%sとキーがあっていません\n", line->name, line->point_e->name);
    return FALSE;
  }
  return TRUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL debugCheckPointData(const RAIL_POINT * point)
{
  int i, j;
  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
    if ((point->keys[i] == RAIL_KEY_NULL) != (point->lines[i] == NULL) )
    {
      OS_TPrintf("%s:LINEとRAIL_KEYの指定に矛盾があります\n", point->name);
      return FALSE;
    }

    if (point->lines[i] == NULL) continue;

    if (point->lines[i]->point_s != point && point->lines[i]->point_e != point)
    {
      OS_TPrintf("%sは%sと接続していません\n",point->name, point->lines[i]->name);
      return FALSE;
    }

    if (point->keys[i] != RAIL_KEY_NULL)
    {
      for (j = i+1; j < RAIL_CONNECT_LINE_MAX; j++)
      {
        if (point->keys[i] == getReverseKey(point->keys[j]))
        {
          if (point->lines[i]->key != point->lines[j]->key)
          {
            OS_Printf("%s から %s 経由で %s へ向かうキーに矛盾があります\n",
                point->lines[i]->name, point->name, point->lines[j]->name);
            return FALSE;
          }
        }
      }
    }
  }
  return TRUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintWholeVector(const char * before, const VecFx32 * vec, const char * after)
{
  if (before)
  {
    OS_TPrintf("%s",before);
  }
  OS_TPrintf("( %d, %d, %d)", FX_Whole(vec->x), FX_Whole(vec->y), FX_Whole(vec->z));
  if (after)
  {
    OS_TPrintf("%s",after);
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintHexVector(const VecFx32 * vec)
{
  OS_TPrintf("(%08x, %08x, %08x)\n", (vec->x), (vec->y), (vec->z));
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintPoint(const char * before, const RAIL_POINT * point, const char * after)
{
  if (point != NULL)
  {
    debugPrintWholeVector(before, &point->pos, after);
  }
  else
  {
    if (before) OS_TPrintf("%s", before);
    OS_TPrintf("( NULL )");
    if (after) OS_TPrintf("%s", after);
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintDegree(const VecFx32 * p0, const VecFx32 * p1)
{
    VecFx32 s,e;
    fx32 angle;
    VEC_Normalize( p0, &s );
    VEC_Normalize( p1, &e );
    angle = FX_AcosIdx( VEC_DotProduct( &s, &e ) );
    OS_TPrintf("Degree: %08x\n",angle);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static const char * debugGetRailKeyName(RAIL_KEY key)
{
    static const char * const names[] = {
      "RAIL_KEY_NULL","RAIL_KEY_UP","RAIL_KEY_RIGHT","RAIL_KEY_DOWN","RAIL_KEY_LEFT"
    };
    GF_ASSERT(key < RAIL_KEY_MAX);
    return names[key];
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintRailInfo(const FIELD_RAIL * rail)
{
  FIELD_RAIL_TYPE type = rail->type;
  OS_TPrintf("%d(%s:%08x)\n",
      type,
      getRailName(rail),
      rail->dummy);
  if (type == FIELD_RAIL_TYPE_POINT)
  {
    debugPrintPoint(NULL, rail->point ,"\n");
  } else if (type == FIELD_RAIL_TYPE_LINE)
  {
    debugPrintPoint("start", rail->line->point_s, NULL);
    debugPrintPoint("-end", rail->line->point_e, "\n");
  }
}

//============================================================================================
//
//    座標演算関連
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void getVectorFromAngleValue(VecFx32 * vec, u16 yaw, u16 pitch, fx32 len)
{
  enum {  PITCH_LIMIT = 0x200 };
  VecFx32 cameraPos;

	fx16 sinYaw = FX_SinIdx(yaw);
	fx16 cosYaw = FX_CosIdx(yaw);
	fx16 sinPitch = FX_SinIdx(pitch);
	fx16 cosPitch = FX_CosIdx(pitch);

  // 裏周りするとカメラ上位置も変更する必要がある！！
  // 裏まわりしないようにマイナス値はプラス値に補正
	if(cosPitch < 0){ cosPitch = -cosPitch; }
  // 0値近辺は不正表示になるため補正
	if(cosPitch < PITCH_LIMIT){ cosPitch = PITCH_LIMIT; }

	VEC_Set( &cameraPos, FX_Mul(sinYaw, cosPitch), sinPitch, FX_Mul(cosYaw, cosPitch) );
	VEC_Normalize(&cameraPos, &cameraPos);
  {
    const VecFx32 zeroVec = {0, 0, 0};
    VEC_MultAdd(len, &cameraPos, &zeroVec, &cameraPos);
  }
  *vec = cameraPos;
}


//------------------------------------------------------------------
/**
 * @brief ベクトルとベクトルの間の球面線形補完をおこなう
 *
 * @param out     生成したベクトル
 * @param start   開始ベクトル
 * @param end     終了ベクトル
 * @param t       0～FX32_ONE
 *
 * 生成されるベクトルは正規化されているので注意
 */
//------------------------------------------------------------------
static void getInterNormalVector(VecFx32 * out, const VecFx32 * start, const VecFx32 * end, fx32 t)
{
  VecFx32 s, e;
  fx32 angle;   //-1～1
  u16 sinTheta; //radian index
  u16 Ps, Pe;   //radian index

  VEC_Normalize( start, &s );
  VEC_Normalize( end, &e );
  angle = FX_AcosIdx( VEC_DotProduct( &s, &e ) );
  sinTheta = FX_SinIdx( angle );
  Ps = FX_SinIdx( FX_Mul(angle, FX32_ONE - t) );
  Pe = FX_SinIdx( FX_Mul(angle, t) );

  // out = ( Ps * s + Pe * e ) / sinTheta
  GFL_CALC3D_VEC_MulScalar(&s, FX32_ONE * Ps / sinTheta, &s);
  GFL_CALC3D_VEC_MulScalar(&e, FX32_ONE * Pe / sinTheta, &e);
  //GFL_CALC3D_VEC_MulScalar(&s, FX32_ONE * Ps / 0x10000, &s);
  //GFL_CALC3D_VEC_MulScalar(&e, FX32_ONE * Pe / 0x10000, &e);
  VEC_Add(&s, &e, out);
  //GFL_CALC3D_VEC_DivScalar(out, FX32_ONE * sinTheta / 0x10000, out);

  VEC_Normalize(out, out);
}

//------------------------------------------------------------------
/**
 * @brief ベクトルとベクトルの間の線形補完（長さも補完）
 *
 * @param out     生成したベクトル
 * @param start   開始ベクトル
 * @param end     終了ベクトル
 * @param t       0～FX32_ONE
 */
//------------------------------------------------------------------
static void getIntermediateVector(VecFx32 * out, const VecFx32 * start, const VecFx32 * end, fx32 t)
{
  const VecFx32 zeroVec = {0, 0, 0};
  fx32 l_s, l_e, l_i;
  getInterNormalVector(out, start, end, t);

  // out = (|start| + (|end| - |start|) * t) * out
  l_s = VEC_Mag(start);
  l_e = VEC_Mag(end);
  l_i = l_s + FX_Mul((l_e - l_s) , t);
  VEC_MultAdd(l_i, out, &zeroVec, out);
}

//============================================================================================
//
//
//
//    レール表現を実現するためのツール
//
//
//
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
const RAIL_LINEPOS_SET RAIL_LINEPOS_SET_Default =
{
  NULL,
  0,
  0,
  0,
  0,
};

//------------------------------------------------------------------
/**
 * @brief レール移動：点と点の間の直線補完
 */
//------------------------------------------------------------------
void FIELD_RAIL_POSFUNC_StraitLine(const FIELD_RAIL * rail, VecFx32 * pos)
{
  //LINEにいるときはPOINTとPOINTの間の座標
  //pos = p0 + (p1 - p0) * (ofs / div )
  const RAIL_LINE * nLine = rail->line;
  fx32 ofs = (rail->line_ofs * FX32_ONE) / nLine->ofs_max;
  VecFx32 val;
  const VecFx32 * p0 = &nLine->point_s->pos;
  const VecFx32 * p1 = &nLine->point_e->pos;
  VEC_Subtract(p1, p0, &val);
  VEC_MultAdd(ofs, &val, p0, pos);

  {
    VecFx32 xzNormal = {0,FX32_ONE, 0}; //XZ平面に垂直な単位ベクトル
    VecFx32 width;
    fx32 w_ofs = rail->width_ofs * rail->width.ofs_unit;
    VEC_CrossProduct(&val, &xzNormal, &width);
    VEC_Normalize(&width, &width);
    VEC_MultAdd(w_ofs, &width, pos, pos);
  }

  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)
  {
    debugPrintWholeVector("start ", p0, "\n");
    //debugPrintHexVector(p0);
    debugPrintWholeVector("now   ", pos, "\n");
    //debugPrintHexVector(pos);
    debugPrintWholeVector("end   ", p1, "\n");
    //debugPrintHexVector(p1);
  }
}

//------------------------------------------------------------------
/**
 *
 * @brief レール移動：点と点の間を曲線補完
 *
 * （中心点-->スタート位置）、（中心点-->終了位置）の二つのベクトルを
 * 球面線形補完している。中心点の位置はパラメータで受け取る
 */
//------------------------------------------------------------------
void FIELD_RAIL_POSFUNC_CurveLine(const FIELD_RAIL * rail, VecFx32 * pos)
{
  const RAIL_LINE * nLine = rail->line;
  const VecFx32 * p_s = &nLine->point_s->pos;
  const VecFx32 * p_e = &nLine->point_e->pos;
  VecFx32 center, vec_s, vec_e, vec_i;
  fx32 ofs = (rail->line_ofs * FX32_ONE) / nLine->ofs_max;

  center.x = nLine->line_pos_set->param0;
  center.y = nLine->line_pos_set->param1;
  center.z = nLine->line_pos_set->param2;

  VEC_Subtract(p_s, &center, &vec_s);
  VEC_Subtract(p_e, &center, &vec_e);
  
  getIntermediateVector(&vec_i, &vec_s, &vec_e, ofs);
  VEC_Add(&center, &vec_i, pos);
  {
    VecFx32 w_vec;
    VEC_Normalize(&vec_i, &w_vec);
    VEC_MultAdd(-(rail->width_ofs) * rail->width.ofs_unit, &w_vec, pos, pos);
  }

  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)
  {
    debugPrintWholeVector("Center:", &center, "\n");
    debugPrintWholeVector("start: ", p_s, "\n");
    debugPrintWholeVector("end:   ", p_e, "\n");
    debugPrintWholeVector("v_s:   ", &vec_s, "\n");
    OS_TPrintf("v_s:len=%08x\n",VEC_Mag(&vec_s));
    debugPrintWholeVector("v_e:   ", &vec_e, "\n");
    OS_TPrintf("v_e:len=%08x\n",VEC_Mag(&vec_e));
    debugPrintWholeVector("v_i:   ", &vec_i, "\n");
    debugPrintHexVector(&vec_i);
    OS_TPrintf("v_i:len=%08x\n",VEC_Mag(&vec_i));
    debugPrintWholeVector("pos:   ", pos, "\n");
  }
}


//============================================================================================
//
//
//      カメラ指定のためのツール
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
const RAIL_CAMERA_SET RAIL_CAMERA_SET_Default =
{
  NULL,
  0,
  0,
  0,
  0,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static void calcAngleCamera(FIELD_CAMERA * field_camera)
{
  {
    VecFx32 camPos, target, before;
    u16 yaw = FIELD_CAMERA_GetAngleYaw(field_camera);
    u16 pitch = FIELD_CAMERA_GetAnglePitch(field_camera);
    fx32 len = FIELD_CAMERA_GetAngleLen(field_camera);

    FIELD_CAMERA_GetCameraPos(field_camera, &before);

    FIELD_CAMERA_GetTargetPos(field_camera, &target);
    getVectorFromAngleValue(&camPos, yaw, pitch, len);
    VEC_Add(&camPos, &target, &camPos);
    FIELD_CAMERA_SetCameraPos(field_camera, &camPos);

    if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y)
    {
      debugPrintWholeVector("CamTgt:", &target, "\n");
      debugPrintWholeVector("CamPos:", &camPos, "\n");
      OS_TPrintf("yaw:%04x pitch:%04x len:%08x\n",yaw, pitch, len);
    }
  }
}

//------------------------------------------------------------------
/**
 * @brief カメラ指定：パラメータをFx32３つとして単純に足すだけ
 */
//------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_FixPosCamera(const FIELD_RAIL_MAN* man)
{
  VecFx32 pos;
  const RAIL_CAMERA_SET * cam_set = getCameraSet(&man->now_rail);
  FIELD_RAIL_MAN_GetPos(man, &pos);

  pos.x += cam_set->param0;
  pos.y += cam_set->param1;
  pos.z += cam_set->param2;
  FIELD_CAMERA_SetCameraPos(man->field_camera, &pos);
}

//------------------------------------------------------------------
/**
 * @brief カメラ指定：パラメータをAngle指定に使用する
 */
//------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_FixAngleCamera(const FIELD_RAIL_MAN* man)
{
  FIELD_CAMERA * cam = man->field_camera;
  const RAIL_CAMERA_SET * cam_set = getCameraSet(&man->now_rail);
  FIELD_CAMERA_SetAnglePitch(cam, (u16)cam_set->param0);
  FIELD_CAMERA_SetAngleYaw(cam, (u16)cam_set->param1);
  FIELD_CAMERA_SetAngleLen(cam, (fx32)cam_set->param2);

  calcAngleCamera(man->field_camera);
}

//------------------------------------------------------------------
/**
 * @brief カメラ指定：保持する二つのPOINTのangleバリューを補完する
 */
//------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_OfsAngleCamera(const FIELD_RAIL_MAN* man)
{
  VecFx32 before;
  VecFx32 c_s, c_e, c_now, target;

  const FIELD_RAIL * rail = &man->now_rail;
  const RAIL_CAMERA_SET * cs;
  const RAIL_CAMERA_SET * ce;
  GF_ASSERT(rail->type == FIELD_RAIL_TYPE_LINE);

  cs = rail->line->point_s->camera_set;
  GF_ASSERT(cs->func == FIELD_RAIL_CAMERAFUNC_FixAngleCamera);
  ce = rail->line->point_e->camera_set;
  GF_ASSERT(ce->func == FIELD_RAIL_CAMERAFUNC_FixAngleCamera);

  FIELD_CAMERA_GetCameraPos(man->field_camera, &before);

  {
    u32 div = rail->line->ofs_max;
    fx32 t = FX32_ONE * man->now_rail.line_ofs / div;

    getVectorFromAngleValue(&c_s, cs->param1, cs->param0, cs->param2);
    getVectorFromAngleValue(&c_e, ce->param1, ce->param0, ce->param2);
    getIntermediateVector(&c_now, &c_s, &c_e, t);

    FIELD_CAMERA_GetTargetPos(man->field_camera, &target);
    VEC_Add(&c_now, &target, &c_now);
    FIELD_CAMERA_SetCameraPos(man->field_camera, &c_now);
  }

  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y)
  {
    debugPrintWholeVector("CamTgt:", &target, "\n");
    debugPrintWholeVector("CamPos:", &c_now, "\n");
  }
}




//----------------------------------------------------------------------------
/**
 *	@brief  円カメラ動作
 *
 *	@param	man 
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_POSFUNC_CircleCamera( const FIELD_RAIL_MAN * man )
{
  const RAIL_CAMERA_SET * cs;
  const RAIL_CAMERA_SET * ce;
  VecFx32 pos, target, n0, camera_pos;
  FIELD_CAMERA* p_camera;
  fx32 xz_dist;
  fx32 target_y;
  u16 pitch;
  fx32 len;
  s32 div;
  s32 ofs;

  if( man->now_rail.type == FIELD_RAIL_TYPE_LINE )
  {
    cs = man->now_rail.line->point_s->camera_set;
    ce = man->now_rail.line->point_e->camera_set;
    div = man->now_rail.line->ofs_max;
    ofs = man->now_rail.line_ofs;
  }
  else
  {
    cs = man->now_rail.point->camera_set;
    ce = man->now_rail.point->camera_set;
    div = 1;
    ofs = 0;
  }

  // pitch len の計算
  pitch = cs->param0 + ((((int)ce->param0 - (int)cs->param0) * ofs) / div);
  len   = cs->param1 + FX_Div( FX_Mul(ce->param1 - cs->param1, ofs<<FX32_SHIFT), div<<FX32_SHIFT );

  p_camera = man->field_camera;
	FIELD_CAMERA_GetTargetPos( p_camera, &target);
  target_y  = target.y;
  target.y  = 0;
  
  FIELD_RAIL_MAN_GetPos( man, &pos );
  pos.y   = 0;

  VEC_Subtract( &pos, &target, &pos );
  VEC_Normalize( &pos, &n0 );

  // 方向ベクトルから、カメラangleを求める
  camera_pos.y = FX_Mul( FX_SinIdx( pitch ), len );
  xz_dist      = FX_Mul( FX_CosIdx( pitch ), len );
  camera_pos.x = FX_Mul( n0.x, xz_dist );
  camera_pos.z = FX_Mul( n0.z, xz_dist );
  camera_pos.x += target.x;
  camera_pos.y += target_y;
  camera_pos.z += target.z;
  
	FIELD_CAMERA_SetCameraPos( p_camera, &camera_pos );
}

