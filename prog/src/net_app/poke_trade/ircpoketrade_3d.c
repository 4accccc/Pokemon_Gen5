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
#if PM_DEBUG
#include "debug_obata.naix"
#endif


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
●trade2_2.mb(交換ですれ違うシーン)
●trade4.mb（モンスターボールが帰ってくるシーン）
●trade5.mb（モンスターボールが落ちるシーン）
●trade3.mb（モンスターボールからポケモンになるシーン）
  
 */
//============================================================================================

static const GFL_G3D_UTIL_RES res_table_reel[] =
{
  { ARCID_POKETRADE,   NARC_trade_box_001_nsbmd,   GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_RES res_table_trade1[] =
{
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_matome3_nsbmd,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_matome3_nsbca,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_matome3_nsbta,    GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_RES res_table_trade_trade[] =
{
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade2_2_nsbmd,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade2_2_nsbca,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade2_2_nsbta,    GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_RES res_table_trade_return[] =
{
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade4_nsbmd,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade4_nsbca,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade4_nsbta,    GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_RES res_table_trade_splash[] =
{
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade5_nsbmd,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade5_nsbca,    GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_RES res_table_trade_end[] =
{
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade3_nsbmd,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade3_nsbca,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_trade3_nsbta,    GFL_G3D_UTIL_RESARC },
};


static const GFL_G3D_UTIL_ANM anm_table_trade1[] =
{
  { 1, 0 },
  { 2, 0 },
};

static const GFL_G3D_UTIL_ANM anm_table_trade_nomal[] =
{
  { 1, 0 },
  { 2, 0 },
};

static const GFL_G3D_UTIL_ANM anm_table_trade_splash[] =
{
  { 1, 0 },
};

static const GFL_G3D_UTIL_ANM anm_table_trade2[] =
{
  { 4, 0 },
  { 5, 0 },
};


static const GFL_G3D_UTIL_OBJ obj_table_reel[] =
{
  {
    0,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    0,                         // テクスチャリソースID
    0,           // アニメテーブル(複数指定のため)
    NULL,   // アニメリソース数
  },
};


static const GFL_G3D_UTIL_OBJ obj_table_trade1[] =
{
  {
    0,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    0,                         // テクスチャリソースID
    anm_table_trade1,           // アニメテーブル(複数指定のため)
    NELEMS(anm_table_trade1),   // アニメリソース数
  },
  {
    0,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    0,                         // テクスチャリソースID
    NULL,           // アニメテーブル(複数指定のため)
    0,   // アニメリソース数
  },
};

static const GFL_G3D_UTIL_OBJ obj_table_trade_normal[] =
{
  {
    0,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    0,                         // テクスチャリソースID
    anm_table_trade_nomal,           // アニメテーブル(複数指定のため)
    NELEMS(anm_table_trade_nomal),   // アニメリソース数
  },
};

static const GFL_G3D_UTIL_OBJ obj_table_trade_splash[] =
{
  {
    0,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    0,                         // テクスチャリソースID
    anm_table_trade_splash,           // アニメテーブル(複数指定のため)
    NELEMS(anm_table_trade_splash),   // アニメリソース数
  },
};


// セットアップデータ
static const GFL_G3D_UTIL_SETUP setup[] =
{
  { res_table_reel, NELEMS(res_table_reel), obj_table_reel, NELEMS(obj_table_reel) },
  { res_table_trade1, NELEMS(res_table_trade1), obj_table_trade1, NELEMS(obj_table_trade1) },
  { res_table_trade_trade, NELEMS(res_table_trade_trade), obj_table_trade_normal, NELEMS(obj_table_trade_normal) },
  { res_table_trade_return, NELEMS(res_table_trade_return), obj_table_trade_normal, NELEMS(obj_table_trade_normal) },
  { res_table_trade_splash, NELEMS(res_table_trade_splash), obj_table_trade_splash, NELEMS(obj_table_trade_splash) },
  { res_table_trade_end, NELEMS(res_table_trade_end), obj_table_trade_normal, NELEMS(obj_table_trade_normal) },
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
  fx32 far= 2000*FX32_ONE;
  fx32 near= FX32_ONE;

  campos.x = 0;
  campos.y = 3*FX32_ONE;
  campos.z = 15*FX32_ONE;
  tarpos.x = 0;
  tarpos.y = 0;
  tarpos.z = 0;

  GFL_G3D_CAMERA_SetPos( pWork->camera, &campos );
  GFL_G3D_CAMERA_SetTarget( pWork->camera, &tarpos );
  GFL_G3D_CAMERA_SetNear( pWork->camera, &near );
  GFL_G3D_CAMERA_SetFar( pWork->camera, &far );
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
  //  ICA_ANIME_SetCameraStatus( pWork->icaAnime, pWork->camera );
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

static void _moveSetTrade01(IRC_POKEMON_TRADE* pWork,GFL_G3D_OBJSTATUS* pStatus)
{
  int a;

  VEC_Set( &pStatus->trans, 0, 0, 0 );
  VEC_Set( &pStatus->scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &pStatus->rotate );

#if 1
  {

    VecFx32 campos;

    GFL_G3D_CAMERA_GetPos( pWork->camera, &campos );
    
    if(GFL_UI_KEY_GetCont() == PAD_KEY_UP){
      campos.y += FX32_ONE/2;
    }
    if(GFL_UI_KEY_GetCont() == PAD_KEY_DOWN){
      campos.y -= FX32_ONE/2;
    }
    if(GFL_UI_KEY_GetCont() == PAD_KEY_LEFT){
      campos.z += FX32_ONE/2;
    }
    if(GFL_UI_KEY_GetCont() == PAD_KEY_RIGHT){
      campos.z -= FX32_ONE/2;
    }
    GFL_G3D_CAMERA_SetPos( pWork->camera, &campos );
  }
#endif
  
  // カメラ更新
  //  ICA_ANIME_SetCameraStatus( pWork->icaAnime, pWork->camera );
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
  { _cameraSetTrade01,_moveSetTrade01 },
  { _cameraSetTrade01,_moveSetTrade01 },
  { _cameraSetTrade01,_moveSetTrade01 },
  { _cameraSetTrade01,_moveSetTrade01 },
  { _cameraSetTrade01,_moveSetTrade01 },
  { _cameraSetTrade01,_moveSetTrade01 },
  { _cameraSetTrade01,_moveSetTrade01 },
};





//============================================================================================
/**
 * @brief プロトタイプ宣言
 */
//============================================================================================
static void Initialize( IRC_POKEMON_TRADE* pWork,int no );
static void Finalize( IRC_POKEMON_TRADE* pWork );
static void Draw( IRC_POKEMON_TRADE* pWork );


//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_Init( IRC_POKEMON_TRADE* pWork )
{
  VecFx32 campos;
  VecFx32 tarpos;
    int i;


  // 初期化処理
  _demoInit( pWork->heapID );
  // 3D管理ユーティリティーのセットアップ
  pWork->g3dUtil = GFL_G3D_UTIL_Create( 20, 20, pWork->heapID );

	//パーティクルシステムワーク初期化
	GFL_PTC_Init(pWork->heapID);

  {
    void* heap;
    heap = GFL_HEAP_AllocMemory(pWork->heapID, PARTICLE_LIB_HEAP_SIZE);
    pWork->ptc = GFL_PTC_Create(heap, PARTICLE_LIB_HEAP_SIZE, TRUE, pWork->heapID);
  }
  // カメラ作成
  if(pWork->camera==NULL)
  {
    VecFx32    pos = { 0, 0, 0 };
    VecFx32 target = { 0, 0, 0 };
    pWork->camera   = GFL_G3D_CAMERA_CreateDefault( &pos, &target, pWork->heapID );
  }

  {
    Initialize( pWork, REEL_PANEL_OBJECT);
  }
  modelset[pWork->modelno].setCamera(pWork);


	//リソース読み込み＆登録
  {
  	void *resource;
    resource = GFL_PTC_LoadArcResource(
      ARCID_POKETRADEDEMO, NARC_tradedemo_balloon_spa, pWork->heapID);
    GFL_PTC_SetResource(pWork->ptc, resource, TRUE, NULL);
  }
  
}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_Main( IRC_POKEMON_TRADE* pWork )
{
  Draw( pWork );
}


//============================================================================================
/**
 * @brief 終了関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_End( IRC_POKEMON_TRADE* pWork )
{
  {
  	void *heap;
	
    heap = GFL_PTC_GetHeapPtr(pWork->ptc);
    GFL_PTC_Exit();
    GFL_HEAP_FreeMemory(heap);
  }

  // カメラ破棄
  if(pWork->camera)
  {
    GFL_G3D_CAMERA_Delete( pWork->camera );
  }

  Finalize( pWork );
  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( pWork->g3dUtil );

  _demoExit();

}

//============================================================================================
/**
 * @brief モデルをセットしなおす関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_SetModel( IRC_POKEMON_TRADE* pWork, int modelno)
{
  
  Initialize( pWork, modelno);


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
static void Initialize( IRC_POKEMON_TRADE* pWork, int modelno )
{

  // ユニット追加
  {
    pWork->unitIndex = GFL_G3D_UTIL_AddUnit( pWork->g3dUtil, &setup[modelno] );
    pWork->objCount = setup[modelno].objCount;
  }

  // アニメーション有効化
  {
    int i;
    for( i=0; i<pWork->objCount; i++ )
    {
      int j;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( pWork->g3dUtil, pWork->unitIndex+i );
      int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      for( j=0; j<anime_count; j++ )
      {
        OS_TPrintf("アニメーション有効化%d\n",j);
        GFL_G3D_OBJECT_EnableAnime( obj, j );
      }
    }
  }
  pWork->modelno = modelno;

  // icaデータをロード
  //  pWork->icaAnime = ICA_ANIME_CreateStreamingAlloc(
  //      pWork->heapID, ARCID_POKETRADEDEMO, NARC_debug_obata_ica_test_data2_bin, 10 );

}

//--------------------------------------------------------------------------------------------
/**
 * @breif 終了
 */
//--------------------------------------------------------------------------------------------
static void Finalize( IRC_POKEMON_TRADE* pWork )
{

  // ユニット破棄
  {
    int i;
    for( i=0; i<pWork->objCount; i++ )
    {
      GFL_G3D_UTIL_DelUnit( pWork->g3dUtil, pWork->unitIndex+i );
      break;
    }
  }
  pWork->objCount = 0;

}


//--------------------------------------------------------------------------------------------
/**
 * @breif 描画
 */
//--------------------------------------------------------------------------------------------

static void Draw( IRC_POKEMON_TRADE* pWork )
{
  static fx32 frame = 0;
  static fx32 anime_speed = FX32_ONE/2;  // 1/60での動作の為
  GFL_G3D_OBJSTATUS status;


  modelset[pWork->modelno].setMove(pWork, &status);

  // TEMP: カメラ設定
  //  {
  //    fx32 far = FX32_ONE * 0;
  //    GFL_G3D_CAMERA_SetFar( pWork->camera, &far );
  //  }

  // アニメーション更新
  {
    int i;
    for( i=0; i<pWork->objCount; i++ )
    {
      {
        int j;
        GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( pWork->g3dUtil, pWork->unitIndex+i );
        int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
        for( j=0; j<anime_count; j++ )
        {
          GFL_G3D_OBJECT_LoopAnimeFrame( obj, j, anime_speed );
        }
      }
    }
  }

  // 描画
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();

  GFL_PTC_DrawAll();	//パーティクル描画
  GFL_PTC_CalcAll();	//パーティクル計算

  {
    int i;
    for( i=0; i<pWork->objCount; i++ )
    {
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( pWork->g3dUtil, pWork->unitIndex+i );
      GFL_G3D_DRAW_DrawObject( obj, &status );
    }
  }
//  GFL_G3D_DRAW_End();

//  frame += anime_speed;
  //  ICA_ANIME_IncAnimeFrame( pWork->icaAnime, anime_speed );
}



