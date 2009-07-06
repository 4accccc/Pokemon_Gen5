//==============================================================================
/**
 * @file    union_char.c
 * @brief   ユニオンルームでの人物関連
 * @author  matsuda
 * @date    2009.07.06(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "union_types.h"
#include "net_app/union/union_main.h"
#include "net_app/union/union_beacon_tool.h"


//==============================================================================
//  データ
//==============================================================================
///キャラクタの配置座標
static const struct{
  u16 x;    //グリッド座標：X
  u16 z;    //グリッド座標：Y
}UnionCharPosTbl[] = {
  {10, 20},   //index 0
  {10, 20},   //index 1
  {10, 20},   //index 2
  {10, 20},   //index 3
  {10, 20},   //index 4
  {10, 20},   //index 5
  {10, 20},   //index 6
  {10, 20},   //index 7
  {10, 20},   //index 8
  {10, 20},   //index 9
  {10, 20},   //index 10
  {10, 20},   //index 11
  {10, 20},   //index 12
  {10, 20},   //index 13
  {10, 20},   //index 14
  {10, 20},   //index 15
  {10, 20},   //index 16
  {10, 20},   //index 17
  {10, 20},   //index 18
  {10, 20},   //index 19
  {10, 20},   //index 20
};

///ユニオンキャラクターのMMDLヘッダーデータ
static const MMDL_HEADER UnionChar_MMdlHeader = 
{
  0,
  0,
  MV_DIR_RND,
  0,	///<イベントタイプ
  0,	///<イベントフラグ
  0,	///<イベントID
  DIR_DOWN,	///<指定方向
  0,	///<指定パラメタ 0
  0,	///<指定パラメタ 1
  0,	///<指定パラメタ 2
  0,	///<X方向移動制限
  0,	///<Z方向移動制限
  0,	///<グリッドX
  0,	///<グリッドZ
  0,	///<Y値 fx32型
};



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ユニオンキャラクター：登録
 *
 * @param   unisys		      
 * @param   gdata		        ゲームデータへのポインタ
 * @param   trainer_view		見た目
 * @param   sex		          性別
 * @param   char_index		  キャラクターIndex
 *
 * @retval  MMDL *		      
 */
//==================================================================
MMDL * UNION_CHAR_AddOBJ(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata, u8 trainer_view, u8 sex, u16 char_index)
{
  MMDL *addmdl;
  MMDLSYS *mdlsys;
  MMDL_HEADER head;
  
  mdlsys = GAMEDATA_GetMMdlSys( gdata );
  
  head = UnionChar_MMdlHeader;
  head.id = char_index;
  head.obj_code = UnionView_GetCharaNo(sex, trainer_view);
  head.gx = UnionCharPosTbl[char_index].x;
  head.gz = UnionCharPosTbl[char_index].z;
  
  addmdl = MMDLSYS_AddMMdl(mdlsys, &head, ZONE_ID_PALACETEST);
  return addmdl;
}

