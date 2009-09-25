//=============================================================================
/**
 * @file	  ircpoketrade_3d.c
 * @bfief	  ポケモン交換３Ｄ部分
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/09/18
 */
//=============================================================================


#include <gflib.h>
#include "arc_def.h"
#include "poke_tool/monsno_def.h"

#include "trade.naix"

#include "ircpokemontrade.h"
#include "ircpokemontrade_local.h"
#include "msg/msg_poke_trade.h"
#include "gamesystem/msgspeed.h"  //MSGSPEED_GetWait
#include "font/font.naix"    // NARC_font_default_nclr

#include "system/bmp_winframe.h"
#include "savedata/box_savedata.h"
#include "pokeicon/pokeicon.h"

#include "tradedemo.naix"



#define cameraPerspway  ( 0x0b60 )
#define cameraAspect    ( FX32_ONE * 4 / 3 )
#define cameraNear	    ( 1 << FX32_SHIFT )
#define cameraFar       ( 400 << FX32_SHIFT )


//============================================================================================
/**
 * @brief BGデータ
 */
//============================================================================================
// 優先順位
enum
{
  PRIORITY_MAIN_BG1,
  PRIORITY_MAIN_BG0,
};




//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light_data[] = {
  { 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 1, {{  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
};
static const GFL_G3D_LIGHTSET_SETUP light_setup = { light_data, NELEMS(light_data) };
static GFL_G3D_LIGHTSET* g_lightSet;

typedef void (MODEL3D_CAMERA_FUNC)(IRC_POKEMON_TRADE* pWork);
typedef void (MODEL3D_MOVE_FUNC)(IRC_POKEMON_TRADE* pWork,GFL_G3D_OBJSTATUS* pStatus);


typedef struct 
{
  MODEL3D_CAMERA_FUNC* setCamera;
  MODEL3D_MOVE_FUNC* setMove;
} MODEL3D_SET_FUNCS;


//-------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 */
//-------------------------------------------------------------------------------------------
static void _demoInit( HEAPID heap_id )
{
  //VRAM設定
  //	GFL_DISP_SetBank(&sc_VRAM_param );

  // BGL初期化
  //	GFL_BG_Init( heap_id );	// システム初期化
  //	GFL_BG_SetBGMode( &sc_BG_mode );	  // BGモード設定
  //	GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &sc_BGCNT1_M, GFL_BG_MODE_TEXT );	// MAIN_BG1の設定
  //	GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );						// MAIN_BG0を表示ON
  //	GFL_BG_SetBGControl3D( PRIORITY_MAIN_BG0 );		              // 3D面の表示優先順位を設定

  // 3Dシステムを初期化
  // GFL_G3D_Init(
  //   GFL_G3D_VMANLNK, GFL_G3D_TEX128K,
  //    GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0x1000, heap_id, NULL );

  // ライト作成
  g_lightSet = GFL_G3D_LIGHT_Create( &light_setup, heap_id );
  GFL_G3D_LIGHT_Switching( g_lightSet );

  // カメラ初期設定
  {
    GFL_G3D_PROJECTION proj = { GFL_G3D_PRJPERS, 0, 0, cameraAspect, 0, cameraNear, cameraFar, 0 };
    proj.param1 = FX_SinIdx( cameraPerspway );
    proj.param2 = FX_CosIdx( cameraPerspway );
    GFL_G3D_SetSystemProjection( &proj );
  }
}

//-------------------------------------------------------------------------------------------
/**
 * @brief 終了処理
 */
//-------------------------------------------------------------------------------------------
static void _demoExit()
{
  // ライトセット破棄
  GFL_G3D_LIGHT_Delete( g_lightSet );

  // 3Dシステム終了
  GFL_G3D_Exit();

  // BGL破棄
  GFL_BG_Exit();
}


//============================================================================================
/**
 * @brief 3Dデータ
 */
//============================================================================================

static const GFL_G3D_UTIL_RES res_table_reel[] =
{
  { ARCID_POKETRADE,
    NARC_trade_box_001_nsbmd,
    GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_RES res_table_trade1[] =
{
  { ARCID_POKETRADEDEMO,
    NARC_tradedemo_trade_nsbmd,
    GFL_G3D_UTIL_RESARC },
};
static const GFL_G3D_UTIL_ANM anm_table_ball[] =
{
  { 1, 0 },
  { 2, 0 },
};
static const GFL_G3D_UTIL_OBJ obj_table_ball[] =
{
  {
    0,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    0,                         // テクスチャリソースID
    NULL,
    0,
    //   anm_table_ball,           // アニメテーブル(複数指定のため)
    //   NELEMS(anm_table_ball),   // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP setup[] =
{
  { res_table_reel, NELEMS(res_table_reel), obj_table_ball, NELEMS(obj_table_ball) },
  { res_table_trade1, NELEMS(res_table_trade1), obj_table_ball, NELEMS(obj_table_ball) },
};


static void _cameraSetReel(IRC_POKEMON_TRADE* pWork)
{
  VecFx32 campos;
  VecFx32 tarpos;

  campos.x = 0*FX32_ONE;
  campos.y = FX32_ONE*43;
  campos.z = 241*FX32_ONE;
  tarpos.x = 0;
  tarpos.y = FX32_ONE*43;
  tarpos.z = 0;

  GFL_G3D_CAMERA_SetPos( pWork->camera, &campos );
  GFL_G3D_CAMERA_SetTarget( pWork->camera, &tarpos );
  GFL_G3D_CAMERA_SetFar( pWork->camera, &campos.z );
  tarpos.x = 0;
  tarpos.y = FX32_ONE;
  tarpos.z = 0;

  GFL_G3D_CAMERA_SetCamUp( pWork->camera, &tarpos );
}

static void _cameraSetTrade01(IRC_POKEMON_TRADE* pWork)
{
  VecFx32 campos;
  VecFx32 tarpos;

  campos.x = 0*FX32_ONE;
  campos.y = 0;
  campos.z = 241*FX32_ONE;
  tarpos.x = 0;
  tarpos.y = 0;
  tarpos.z = 0;

  GFL_G3D_CAMERA_SetPos( pWork->camera, &campos );
  GFL_G3D_CAMERA_SetTarget( pWork->camera, &tarpos );
  GFL_G3D_CAMERA_SetFar( pWork->camera, &campos.z );
  tarpos.x = 0;
  tarpos.y = FX32_ONE;
  tarpos.z = 0;

  GFL_G3D_CAMERA_SetCamUp( pWork->camera, &tarpos );
}


static void _moveSetReel(IRC_POKEMON_TRADE* pWork,GFL_G3D_OBJSTATUS* pStatus)
{
  int a;

  VEC_Set( &pStatus->trans, 0, 0, 0 );
  VEC_Set( &pStatus->scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &pStatus->rotate );

  a = -pWork->FriendBoxScrollNum * 65535 / 2976;

  MTX_RotY33(	&pStatus->rotate,FX_SinIdx((u16)a),FX_CosIdx((u16)a));
  // カメラ更新
  //  ICA_ANIME_SetCameraStatus( work->icaAnime, work->camera );
  GFL_G3D_CAMERA_Switching(pWork->camera );



  {
    // 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
    G3X_SetShading( GX_SHADING_TOON); //GX_SHADING_HIGHLIGHT );
    G3X_AntiAlias( FALSE );
    G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
    G3X_AlphaBlend( FALSE );		// アルファブレンド　オン
    G3X_EdgeMarking( FALSE );
    G3X_SetFog( TRUE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

    // クリアカラーの設定
    G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog
    // ビューポートの設定
    G3_ViewPort(0, 0, 255, 191);

    // ライト設定
    {
      static const GFL_G3D_LIGHT sc_GFL_G3D_LIGHT[] =
      {
        {
          { 0, -FX16_ONE, 0 },
          GX_RGB( 16,16,16),
        },
        {
          { 0, FX16_ONE, 0 },
          GX_RGB( 16,16,16),
        },
        {
          { 0, -FX16_ONE, 0 },
          GX_RGB( 16,16,16),
        },
        {
          { 0, -FX16_ONE, 0 },
          GX_RGB( 16,16,16),
        },
      };
      int i;

      for( i=0; i<NELEMS(sc_GFL_G3D_LIGHT); i++ ){
        GFL_G3D_SetSystemLight( i, &sc_GFL_G3D_LIGHT[i] );
      }
    }
  }
}


static const MODEL3D_SET_FUNCS modelset[] =
{
  { _cameraSetReel,_moveSetReel },
  { _cameraSetTrade01,_moveSetReel },
};





//============================================================================================
/**
 * @brief プロトタイプ宣言
 */
//============================================================================================
static void Initialize( IRC_POKEMON_TRADE* work,int no );
static void Finalize( IRC_POKEMON_TRADE* work );
static void Draw( IRC_POKEMON_TRADE* work );


//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_Init( IRC_POKEMON_TRADE* pWork )
{
  VecFx32 campos;
  VecFx32 tarpos;

  // 初期化処理
  _demoInit( pWork->heapID );
  // 3D管理ユーティリティーのセットアップ
  pWork->g3dUtil = GFL_G3D_UTIL_Create( 10, 10, pWork->heapID );

  Finalize(pWork);

  Initialize( pWork, REEL_PANEL_OBJECT);
  pWork->modelno = REEL_PANEL_OBJECT;

  modelset[pWork->modelno].setCamera(pWork);

}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_Main( IRC_POKEMON_TRADE* pWork )
{
  IRC_POKEMON_TRADE* work = pWork;
  BOOL end = FALSE;
  VecFx32 campos;
  VecFx32 tarpos;



  Draw( work );
}


//============================================================================================
/**
 * @brief 終了関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_End( IRC_POKEMON_TRADE* pWork )
{
  IRC_POKEMON_TRADE* work = pWork;

  // 終了処理
  //  ICA_ANIME_Delete( work->icaAnime );
  Finalize( work );
  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( work->g3dUtil );

  _demoExit();

}

//============================================================================================
/**
 * @brief モデルをセットしなおす関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_SetModel( IRC_POKEMON_TRADE* pWork, int modelno)
{

  Finalize(pWork);
  Initialize( pWork, REEL_PANEL_OBJECT);

  pWork->modelno = modelno;

  modelset[pWork->modelno].setCamera(pWork);

}

//============================================================================================
/**
 * @brief モデルを消す関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_RemoveModel( IRC_POKEMON_TRADE* pWork)
{
  Finalize(pWork);
}



//============================================================================================
/**
 * @brief 非公開関数の定義
 */
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @breif 初期化
 */
//--------------------------------------------------------------------------------------------
static void Initialize( IRC_POKEMON_TRADE* work, int modelno )
{

  // ユニット追加
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      if(work->unitIndex[i]==NULL)
      {
        work->unitIndex[i] = GFL_G3D_UTIL_AddUnit( work->g3dUtil, &setup[modelno] );
        break;
      }
    }
  }

  // アニメーション有効化
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      int j;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[i] );
      int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      for( j=0; j<anime_count; j++ )
      {
        GFL_G3D_OBJECT_EnableAnime( obj, j );
      }
    }
  }

  // icaデータをロード
  //  work->icaAnime = ICA_ANIME_CreateStreamingAlloc(
  //      work->heapID, ARCID_POKETRADEDEMO, NARC_debug_obata_ica_test_data2_bin, 10 );

  // カメラ作成
  if(work->camera==NULL)
  {
    VecFx32    pos = { 0, 0, 0 };
    VecFx32 target = { 0, 0, 0 };
    work->camera   = GFL_G3D_CAMERA_CreateDefault( &pos, &target, work->heapID );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 終了
 */
//--------------------------------------------------------------------------------------------
static void Finalize( IRC_POKEMON_TRADE* work )
{
  // カメラ破棄
  if(work->camera)
  {
    GFL_G3D_CAMERA_Delete( work->camera );
  }

  // ユニット破棄
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      if(work->unitIndex[i])
      {
        GFL_G3D_UTIL_DelUnit( work->g3dUtil, work->unitIndex[i] );
        work->unitIndex[i] = NULL;
      }
    }
  }

}


//--------------------------------------------------------------------------------------------
/**
 * @breif 描画
 */
//--------------------------------------------------------------------------------------------
static void Draw( IRC_POKEMON_TRADE* work )
{
  static fx32 frame = 0;
  static fx32 anime_speed = FX32_ONE;
  GFL_G3D_OBJSTATUS status;


  modelset[work->modelno].setMove(work, &status);

  // TEMP: カメラ設定
  //  {
  //    fx32 far = FX32_ONE * 0;
  //    GFL_G3D_CAMERA_SetFar( work->camera, &far );
  //  }

  // アニメーション更新
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      int j;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[i] );
      int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      for( j=0; j<anime_count; j++ )
      {
        GFL_G3D_OBJECT_LoopAnimeFrame( obj, j, anime_speed );
      }
    }
  }

  // 描画
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[i] );
      GFL_G3D_DRAW_DrawObject( obj, &status );
    }
  }
  GFL_G3D_DRAW_End();

  frame += anime_speed;
  //  ICA_ANIME_IncAnimeFrame( work->icaAnime, anime_speed );
}
