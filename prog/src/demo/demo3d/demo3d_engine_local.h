//==========================================================================
/*
 *  @file   demo3d_engine_local.h
 *  @brief  Demo3D エンジン　ローカルヘッダ
 *  @author Miyuki Iwasawa
 *  @date   10.03.17
 */
//==========================================================================

#pragma once

struct _DEMO3D_ENGINE_WORK {
  // [IN]
  DEMO3D_GRAPHIC_WORK*    graphic;
  DEMO3D_ID               demo_id;
//  u32                     start_frame;

  // [PRIVATE]
  HEAPID        heapID;
  HEAPID        tmpHeapID;
  BOOL          is_double;
  fx32          anime_speed;  ///< アニメーションスピード
  ICA_ANIME*    ica_anime;
  GFL_G3D_UTIL*   g3d_util;
  GFL_G3D_CAMERA* camera;
  fx32 def_top;
  fx32 def_bottom;
  DEMO3D_CMD_WORK*  cmd;

  u16* unit_idx; // unit_idx保持（ALLOC)
  

  //シーンパラメータ保持
  const DEMO3D_SCENE_DATA* scene;
  DEMO3D_SCENE_ENV        env;
};


