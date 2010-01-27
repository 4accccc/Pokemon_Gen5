//=============================================================================
/**
 * @file	  pokemontrade_3d.c
 * @bfief	  ポケモン交換３Ｄ部分
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/09/18
 */
//=============================================================================


#include <gflib.h>
#include "arc_def.h"
#include "poke_tool/monsno_def.h"

#include "trade.naix"

#include "net_app/pokemontrade.h"
#include "pokemontrade_local.h"
#include "msg/msg_poke_trade.h"
#include "gamesystem/msgspeed.h"  //MSGSPEED_GetWait
#include "font/font.naix"    // NARC_font_default_nclr

#include "system/bmp_winframe.h"
#include "savedata/box_savedata.h"
#include "pokeicon/pokeicon.h"

#include "tradedemo.naix"
#include "tradeirdemo.naix"
#include "system/ica_anime.h"
#include "system/ica_camera.h"

#define UNIT_NULL		(0xffff)
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

typedef void (MODEL3D_CAMERA_FUNC)(POKEMON_TRADE_WORK* pWork);
typedef void (MODEL3D_MOVE_FUNC)(POKEMON_TRADE_WORK* pWork,GFL_G3D_OBJSTATUS* pStatus);


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
}



//============================================================================================
/**
 * @brief 3Dデータ

 */
//============================================================================================

static const GFL_G3D_UTIL_RES res_table_reel[] =
{
  { ARCID_POKETRADE,   NARC_trade_box_001_nsbmd,   GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_RES res_table_trade1[] =
{
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_matome_test_nsbmd,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_matome_test_nsbca,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO,    NARC_tradedemo_matome_test_nsbta,    GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_RES res_table_trade_ir[] =
{
  { ARCID_POKETRADEDEMO_IR,    NARC_tradeirdemo_ir_sean_nsbmd,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO_IR,    NARC_tradeirdemo_ir_sean_nsbca,    GFL_G3D_UTIL_RESARC },
  { ARCID_POKETRADEDEMO_IR,    NARC_tradeirdemo_ir_sean_nsbta,    GFL_G3D_UTIL_RESARC },
};


static const GFL_G3D_UTIL_ANM anm_table_trade1[] =
{
  { 1, 0 },
  { 2, 0 },
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
  }
};


// セットアップデータ
static const GFL_G3D_UTIL_SETUP setup[] =
{
//  { res_table_reel, NELEMS(res_table_reel), obj_table_reel, NELEMS(obj_table_reel) },
  { NULL, 0, NULL, 0},
  { res_table_trade1, NELEMS(res_table_trade1), obj_table_trade1, NELEMS(obj_table_trade1) },
  { res_table_trade_ir, NELEMS(res_table_trade_ir), obj_table_trade1, NELEMS(obj_table_trade1) },

};


static void _cameraSetReel(POKEMON_TRADE_WORK* pWork)
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
  campos.z-=FX32_ONE;
  GFL_G3D_CAMERA_SetFar( pWork->camera, &campos.z );
  tarpos.x = 0;
  tarpos.y = FX32_ONE;
  tarpos.z = 0;

  GFL_G3D_CAMERA_SetCamUp( pWork->camera, &tarpos );
}

static void _cameraSetTrade01(POKEMON_TRADE_WORK* pWork)
{
  VecFx32 campos;
  VecFx32 tarpos;
  fx32 far= 8000*FX32_ONE;
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


static void _moveSetReel(POKEMON_TRADE_WORK* pWork,GFL_G3D_OBJSTATUS* pStatus)
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
    G3X_AntiAlias( TRUE );
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

static void _moveSetTrade01(POKEMON_TRADE_WORK* pWork,GFL_G3D_OBJSTATUS* pStatus)
{
  int a;

  VEC_Set( &pStatus->trans, 0, 0, 0 );
  VEC_Set( &pStatus->scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &pStatus->rotate );

  if(pWork->pPokemonTradeDemo){
    ICA_CAMERA_SetCameraPos( pWork->camera ,pWork->pPokemonTradeDemo->icaCamera );
    ICA_CAMERA_SetTargetPos( pWork->camera ,pWork->pPokemonTradeDemo->icaTarget );
  }

#if 0  //カメラ位置確認用
  {
    VecFx32 pos;
    fx32 far;;
    GFL_G3D_CAMERA_GetPos( pWork->camera , &pos );
    OS_TPrintf("pos %d %d %d\n",pos.x/FX32_ONE,pos.y/FX32_ONE,pos.z/FX32_ONE);
    //  GFL_G3D_CAMERA_GetTarget( pWork->camera , &pos );
    ///OS_TPrintf("target %d %d %d\n",pos.x/FX32_ONE,pos.y/FX32_ONE,pos.z/FX32_ONE);

    GFL_G3D_CAMERA_GetFar(pWork->camera ,&far);
    OS_TPrintf("far %d \n",far/FX32_ONE);


    pos.x = 0;
    pos.y = 200*FX32_ONE;
    pos.z = 200*FX32_ONE;
    GFL_G3D_CAMERA_SetPos( pWork->camera , &pos );
  }
#endif

  GFL_G3D_CAMERA_Switching(pWork->camera );


  {
    // 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
    G3X_SetShading( GX_SHADING_TOON); //GX_SHADING_HIGHLIGHT );
    G3X_AntiAlias( TRUE );
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

  if(pWork->pPokemonTradeDemo){
    if(pWork->pPokemonTradeDemo->pBallInPer){
      VecFx32 pos;
      ICA_ANIME_GetTranslate( pWork->pPokemonTradeDemo->icaBallin, &pos );
      GFL_PTC_SetEmitterPosition(pWork->pPokemonTradeDemo->pBallInPer, &pos);
    }
    if(pWork->pPokemonTradeDemo->pBallOutPer){
      VecFx32 pos;
      ICA_ANIME_GetTranslate( pWork->pPokemonTradeDemo->icaBallout, &pos );
      GFL_PTC_SetEmitterPosition(pWork->pPokemonTradeDemo->pBallOutPer, &pos);
    }
  }


  // カメラ位置を求める



}


static const MODEL3D_SET_FUNCS modelset[] =
{
  { _cameraSetReel,_moveSetReel },
  { _cameraSetTrade01,_moveSetTrade01 },
  { _cameraSetTrade01,_moveSetTrade01 },
};


//============================================================================================
/**
 * @brief ICA初期化
 */
//============================================================================================

void POKEMONTRADE_DEMO_ICA_Init(POKEMONTRADE_DEMO_WORK* pWork)
{
  pWork->icaCamera = ICA_ANIME_CreateStreamingAlloc(
    pWork->heapID, ARCID_POKETRADEDEMO, NARC_tradedemo_icacamera_bin, 10 );
  pWork->icaTarget = ICA_ANIME_CreateStreamingAlloc(
    pWork->heapID, ARCID_POKETRADEDEMO, NARC_tradedemo_icatarget_bin, 10 );
  pWork->icaBallin = ICA_ANIME_CreateStreamingAlloc(
    pWork->heapID, ARCID_POKETRADEDEMO, NARC_tradedemo_ball_001_bin, 10 );
  pWork->icaBallout = ICA_ANIME_CreateStreamingAlloc(
    pWork->heapID, ARCID_POKETRADEDEMO, NARC_tradedemo_ball_002_bin, 10 );
}

//============================================================================================
/**
 * @brief ICA消去
 */
//============================================================================================

void POKEMONTRADE_DEMO_ICA_Delete(POKEMONTRADE_DEMO_WORK* pWork)
{
  if(pWork->icaCamera){
    ICA_ANIME_Delete(pWork->icaCamera);
    pWork->icaCamera=NULL;
  }
  if(pWork->icaTarget){
    ICA_ANIME_Delete(pWork->icaTarget);
    pWork->icaTarget=NULL;
  }
  if(pWork->icaBallin){
    ICA_ANIME_Delete(pWork->icaBallin);
    pWork->icaBallin=NULL;
  }
  if(pWork->icaBallout){
    ICA_ANIME_Delete(pWork->icaBallout);
    pWork->icaBallout=NULL;
  }
}


//============================================================================================
/**
 * @brief ICA初期化
 */
//============================================================================================

void POKEMONTRADE_DEMO_IRICA_Init(POKEMONTRADE_DEMO_WORK* pWork)
{
  pWork->icaCamera = ICA_ANIME_CreateStreamingAlloc(
    pWork->heapID, ARCID_POKETRADEDEMO_IR, NARC_tradeirdemo_icacamera_bin, 10 );
  pWork->icaTarget = ICA_ANIME_CreateStreamingAlloc(
    pWork->heapID, ARCID_POKETRADEDEMO_IR, NARC_tradeirdemo_icatarget_bin, 10 );
}

//============================================================================================
/**
 * @brief ICA消去
 */
//============================================================================================

void POKEMONTRADE_DEMO_IRICA_Delete(POKEMONTRADE_DEMO_WORK* pWork)
{
  if(pWork->icaCamera){
    ICA_ANIME_Delete(pWork->icaCamera);
    pWork->icaCamera=NULL;
  }
  if(pWork->icaTarget){
    ICA_ANIME_Delete(pWork->icaTarget);
    pWork->icaTarget=NULL;
  }
}




//============================================================================================
/**
 * @brief パーティクル初期化 読み込み
 */
//============================================================================================
void POKEMONTRADE_DEMO_PTC_Init( POKEMONTRADE_DEMO_WORK* pWork )
{
  VecFx32 campos;
  VecFx32 tarpos;
  int i;

  //パーティクルシステムワーク初期化
  GFL_PTC_Init(pWork->heapID);

  {
    int i;
    BOOL bCamera;
    for(i=0;i<PTC_KIND_NUM_MAX;i++){
      pWork->ptcheap[i] = GFL_HEAP_AllocClearMemory(pWork->heapID, PARTICLE_LIB_HEAP_SIZE);
      switch(i){
      case PTC_KIND_DEMO1:
      case PTC_KIND_DEMO2:
      case PTC_KIND_DEMO3:
      case PTC_KIND_DEMO6:
      case PTC_KIND_DEMO7:
      case PTC_KIND_ORG:
        bCamera=FALSE;
        break;
      default:
        bCamera=TRUE;
        break;
      }
      pWork->ptc[i] = GFL_PTC_Create(pWork->ptcheap[i], PARTICLE_LIB_HEAP_SIZE, bCamera, pWork->heapID);
    }

  }
  //リソース読み込み＆登録
  {
    void *resource;
    int i;
    resource = GFL_PTC_LoadArcResource(
      ARCID_POKETRADEDEMO, NARC_tradedemo_demo_tex001_spa, pWork->heapID);
    for(i=0;i<PTC_KIND_NUM_MAX;i++){
      GFL_PTC_SetResource(pWork->ptc[i], resource, TRUE, NULL);
    }
  }
}

//============================================================================================
/**
 * @brief パーティクル終了関数
 */
//============================================================================================
void POKEMONTRADE_DEMO_PTC_End( POKEMONTRADE_DEMO_WORK* pWork ,int num)
{
  int i;

  GFL_PTC_Exit();
  for(i=0;i<num;i++){
    GFL_HEAP_FreeMemory(pWork->ptcheap[i]);
  }
}


//============================================================================================
/**
 * @brief IRパーティクル初期化 読み込み
 */
//============================================================================================
void POKEMONTRADE_DEMO_IRPTC_Init( POKEMONTRADE_DEMO_WORK* pWork )
{
  VecFx32 campos;
  VecFx32 tarpos;
  int i;

  //パーティクルシステムワーク初期化
  GFL_PTC_Init(pWork->heapID);

  {
    int i;
    BOOL bCamera;
    for(i = 0 ; i < IRPTC_KIND_NUM_MAX ; i++){
      pWork->ptcheap[i] = GFL_HEAP_AllocClearMemory(pWork->heapID, PARTICLE_LIB_HEAP_SIZE);
      bCamera = FALSE;
      pWork->ptc[i] = GFL_PTC_Create(pWork->ptcheap[i], PARTICLE_LIB_HEAP_SIZE, bCamera, pWork->heapID);
    }

  }
  //リソース読み込み＆登録
  {
    void *resource;
    int i;
    resource = GFL_PTC_LoadArcResource(
      ARCID_POKETRADEDEMO_IR, NARC_tradeirdemo_irdemo_spa, pWork->heapID);
    for(i=0;i<IRPTC_KIND_NUM_MAX;i++){
      GFL_PTC_SetResource(pWork->ptc[i], resource, TRUE, NULL);
    }
  }

}

static void _panelLoad(POKEMON_TRADE_WORK* pWork,int boxnum);
static void _panelRelease(POKEMON_TRADE_WORK *pWork);



//============================================================================================
/**
 * @brief プロトタイプ宣言
 */
//============================================================================================
static void Initialize( POKEMON_TRADE_WORK* pWork,int no );
static void Finalize( POKEMON_TRADE_WORK* pWork );
static void Draw( POKEMON_TRADE_WORK* pWork );


//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_Init( POKEMON_TRADE_WORK* pWork )
{
  VecFx32 campos;
  VecFx32 tarpos;
  int i;

  pWork->unitIndex = UNIT_NULL;
  pWork->modelno = -1;

  // 初期化処理
  _demoInit( GetHeapLowID(pWork->heapID) );
  // 3D管理ユーティリティーのセットアップ
  pWork->g3dUtil = GFL_G3D_UTIL_Create( 20, 20, GetHeapLowID(pWork->heapID) );

  // カメラ作成
  if(pWork->camera==NULL)
  {
    VecFx32    pos = { 0, 0, 0 };
    VecFx32 target = { 0, 0, 0 };
    pWork->camera   = GFL_G3D_CAMERA_CreateDefault( &pos, &target, pWork->heapID );
  }
  if(pWork->modelno!=-1){
//    modelset[pWork->modelno].setCamera(pWork);
  }

   _panelLoad(pWork,25);

  
}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_Main( POKEMON_TRADE_WORK* pWork )
{
  Draw( pWork );
}


//============================================================================================
/**
 * @brief 終了関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_End( POKEMON_TRADE_WORK* pWork )
{

  // カメラ破棄
  if(pWork->camera)
  {
    GFL_G3D_CAMERA_Delete( pWork->camera );
  }
  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( pWork->g3dUtil );

  _demoExit();
  _panelRelease(pWork);

}

//============================================================================================
/**
 * @brief モデルをセットしなおす関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_SetModel( POKEMON_TRADE_WORK* pWork, int modelno)
{
  //  if(modelno != TRADE01_OBJECT){
  //    return;
  //  }


  Initialize( pWork, modelno);

  if( pWork->modelno != -1){
    modelset[pWork->modelno].setCamera(pWork);
  }


}

//============================================================================================
/**
 * @brief モデルを消す関数
 */
//============================================================================================
void IRC_POKETRADEDEMO_RemoveModel( POKEMON_TRADE_WORK* pWork)
{
  if(pWork->modelno == -1){
    return;
  }
  Finalize(pWork);
  pWork->modelno = -1;

}



//============================================================================================
/**
 * @brief 非公開関数の定義
 */
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief 初期化
 */
//--------------------------------------------------------------------------------------------
static void Initialize( POKEMON_TRADE_WORK* pWork, int modelno )
{

  // ユニット追加
  GF_ASSERT(modelno < elementof(setup));
  {
    if(setup[modelno].objCount!=0){
      pWork->unitIndex = GFL_G3D_UTIL_AddUnit( pWork->g3dUtil, &setup[modelno] );
    }
    else{
      pWork->unitIndex = UNIT_NULL;
    }
    //    pWork->objCount = setup[modelno].objCount;
  }


  // アニメーション有効化
  if(pWork->unitIndex != UNIT_NULL){
    int i;
    {
      int j;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( pWork->g3dUtil, pWork->unitIndex );
      int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      for( j=0; j<anime_count; j++ )
      {
        OS_TPrintf("アニメーション有効化%d\n",j);
        GFL_G3D_OBJECT_EnableAnime( obj, j );
      }
    }
  }
  pWork->modelno = modelno;


}

//--------------------------------------------------------------------------------------------
/**
 * @brief 終了
 */
//--------------------------------------------------------------------------------------------
static void Finalize( POKEMON_TRADE_WORK* pWork )
{

  // ユニット破棄
  if(pWork->unitIndex != UNIT_NULL){
    GFL_G3D_UTIL_DelUnit( pWork->g3dUtil, pWork->unitIndex );
  }
  pWork->unitIndex = UNIT_NULL;
  //  pWork->objCount = 0;

}

static void _polygondraw( POKEMON_TRADE_WORK* pWork);

//--------------------------------------------------------------------------------------------
/**
 * @brief 描画
 */
//--------------------------------------------------------------------------------------------


static void Draw( POKEMON_TRADE_WORK* pWork )
{
  static fx32 anime_speed = FX32_ONE;  // 1/60での動作の為
  GFL_G3D_OBJSTATUS status;

  _polygondraw(pWork);

  
  if(pWork->modelno==-1){
    return;
  }



//  modelset[pWork->modelno].setMove(pWork, &status);

  // TEMP: カメラ設定
  //  {
  //    fx32 far = FX32_ONE * 0;
  //    GFL_G3D_CAMERA_SetFar( pWork->camera, &far );
  //  }



  if(UNIT_NULL!=pWork->unitIndex){
    int i;
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( pWork->g3dUtil, pWork->unitIndex );
    int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
    for( i=0; i<anime_count; i++ )
    {
      GFL_G3D_OBJECT_LoopAnimeFrame( obj, i, anime_speed );
    }
  }
  // 描画
  GFL_G3D_DRAW_Start();
  GFL_G3D_CAMERA_Switching( pWork->camera );
  GFL_G3D_DRAW_SetLookAt();

  if(pWork->pPokemonTradeDemo){
    GFL_PTC_DrawAll();	//パーティクル描画
    GFL_PTC_CalcAll();	//パーティクル計算
  }
  {
    if(UNIT_NULL!=pWork->unitIndex){
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( pWork->g3dUtil, pWork->unitIndex );
      GFL_G3D_DRAW_DrawObject( obj, &status );
    }
  }
  //  GFL_G3D_DRAW_End();

  if(pWork->pPokemonTradeDemo){
    if(pWork->pPokemonTradeDemo->icaCamera){
      ICA_ANIME_IncAnimeFrame( pWork->pPokemonTradeDemo->icaCamera, anime_speed );
      ICA_ANIME_IncAnimeFrame( pWork->pPokemonTradeDemo->icaTarget, anime_speed );
    }
    if(pWork->pPokemonTradeDemo->icaBallout){
      ICA_ANIME_IncAnimeFrame( pWork->pPokemonTradeDemo->icaBallout, anime_speed );
    }
    if(pWork->pPokemonTradeDemo->icaBallin){
      ICA_ANIME_IncAnimeFrame( pWork->pPokemonTradeDemo->icaBallin, anime_speed );
    }
  }
}



//--------------------------------------------------------------
//	ポケモンMCSS作成
//--------------------------------------------------------------
void IRCPOKETRADE_PokeCreateMcss( POKEMON_TRADE_WORK *pWork ,int no, int bFront, const POKEMON_PARAM *pp )
{
  MCSS_ADD_WORK addWork;
  VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};
  int xpos[] = { PSTATUS_MCSS_POS_X1 , PSTATUS_MCSS_POS_X2 , PSTATUS_MCSS_POS_X1 , PSTATUS_MCSS_POS_X2};
  int z;

  GF_ASSERT( pWork->pokeMcss[no] == NULL );

  if(bFront){
    MCSS_TOOL_MakeMAWPP( pp , &addWork , MCSS_DIR_FRONT );
    z=PSTATUS_MCSS_POS_MYZ;
  }
  else{
    MCSS_TOOL_MakeMAWPP( pp , &addWork , MCSS_DIR_BACK );
    z=PSTATUS_MCSS_POS_YOUZ;
  }
  pWork->pokeMcss[no] = MCSS_Add( pWork->mcssSys , xpos[no] , PSTATUS_MCSS_POS_Y , z , &addWork );
  MCSS_SetScale( pWork->pokeMcss[no] , &scale );

}

//--------------------------------------------------------------
//	ポケモンMCSS削除
//--------------------------------------------------------------
void IRCPOKETRADE_PokeDeleteMcss( POKEMON_TRADE_WORK *pWork,int no  )
{
  if( pWork->pokeMcss[no] == NULL ){
    return;
  }

  MCSS_SetVanishFlag( pWork->pokeMcss[no] );
  MCSS_Del(pWork->mcssSys,pWork->pokeMcss[no]);
  pWork->pokeMcss[no] = NULL;
}

#if 1

static const u16 pal_16plett[] = {
  GX_RGB(4,4,4), GX_RGB(31,0,0), GX_RGB(0,31,0), GX_RGB(0,0,31),
  GX_RGB(31,31,0), GX_RGB(31,0,31), GX_RGB(0,31,31), GX_RGB(15,0,0),
  GX_RGB(0,15,0), GX_RGB(0,0,15), GX_RGB(15,31,0), GX_RGB(31,15,0),
  GX_RGB(0,31,15), GX_RGB(0,15,31), GX_RGB(31,0,15), GX_RGB(15,0,31),
};




static s16     gCubeGeometry[3 * 8] = {
  FX16_ONE, FX16_ONE, FX16_ONE,
  FX16_ONE, FX16_ONE, -FX16_ONE,
  FX16_ONE, -FX16_ONE, FX16_ONE,
  FX16_ONE, -FX16_ONE, -FX16_ONE,
  -FX16_ONE, FX16_ONE, FX16_ONE,
  -FX16_ONE, FX16_ONE, -FX16_ONE,
  -FX16_ONE, -FX16_ONE, FX16_ONE,
  -FX16_ONE, -FX16_ONE, -FX16_ONE
  };

static VecFx10 gCubeNormal[6] = {
  GX_VECFX10(0, 0, FX32_ONE - 1),
  GX_VECFX10(0, FX32_ONE - 1, 0),
  GX_VECFX10(FX32_ONE - 1, 0, 0),
  GX_VECFX10(0, 0, -FX32_ONE + 1),
  GX_VECFX10(0, -FX32_ONE + 1, 0),
  GX_VECFX10(-FX32_ONE + 1, 0, 0)
  };

static GXSt    gCubeTexCoord[] = {
  GX_ST(0, 0),
  GX_ST(0, 64 * FX32_ONE),
  GX_ST(64 * FX32_ONE, 0),
  GX_ST(64 * FX32_ONE, 64 * FX32_ONE)
  };

static void vtx(int idx)
{
  G3_Vtx(gCubeGeometry[idx * 3], gCubeGeometry[idx * 3 + 1], gCubeGeometry[idx * 3 + 2]);
}

static void normal(int idx)
{
  G3_Direct1(G3OP_NORMAL, gCubeNormal[idx]);
  // use G3_Normal(x, y, z) if not packed yet
}

static void tex_coord(int idx)
{
  G3_Direct1(G3OP_TEXCOORD, gCubeTexCoord[idx]);
  // use G3_TexCoord if not packed yet
}


static const  u32     myTexAddr = 0x03000;       // a texture image at 0x1000 of the texture image slots
static const  u32     myTexPlttAddr = 0x02000;   // a texture palette at 0x1000 of the texture palette slots



static void _printColorSquear(u8* tempBuff, int x, int y, int colno)
{
  int i,j;
  int xstart = x*10+2;
  int ystart = y*10+4;
    
  for(i = ystart ; i < (ystart + 8); i++){  //y座標
    for(j = xstart; j < (xstart + 8); j++){  //x座標
      int pos = j/2+i*32;

      if((j % 2)==0){
        tempBuff[pos] = (0xf0 & tempBuff[pos]) + colno;
      }
      else{
        tempBuff[pos] = (0x0f & tempBuff[pos]) + (colno<<4);
      }
    }
  }
}



static void _panelLoad(POKEMON_TRADE_WORK *pWork,int num)
{
  u8* tempBuff;
  //---------------------------------------------------------------------------
  // Download the texture images:
  //
  // Transfer the texture data on the main memory to the texture image slots.
  //---------------------------------------------------------------------------
  GX_BeginLoadTex();                 // map the texture image slots onto LCDC address space

  tempBuff = GFL_HEAP_AllocClearMemory(pWork->heapID, 2048);
  {
    int i,num;
    for(i=0;i<5;i++){
      GFL_STD_MemFill(tempBuff, 0x0, 2048);

      for( num=0;num<30;num++){
        _printColorSquear(tempBuff, num%6, num/6, num&0xf);
      }
        
      DC_FlushRange(tempBuff,2048);

      GX_LoadTex((void *)tempBuff,        // a pointer to the texture data on the main memory(4 bytes aligned)
                 myTexAddr+2048*i,          // an offset address in the texture image slots
                 2048                // the size of the texture(s)(in bytes)
                 );
    }
  }
  GFL_HEAP_FreeMemory(tempBuff);
  GX_EndLoadTex();                   // restore the texture image slots

  //---------------------------------------------------------------------------
  // Download the texture palettes:
  //
  // Transfer the texture palette data on the main memory to the texture palette slots.
  //---------------------------------------------------------------------------

  GX_BeginLoadTexPltt();             // map the texture palette slots onto LCDC address space
  {
    GX_LoadTexPltt((void *)&pal_16plett[0], // a pointer to the texture data on the main memory(4 bytes aligned)
                   myTexPlttAddr,  // an offset address in the texture palette slots
                   32);            // the size of the texture palette(s)(in bytes)
  }
  GX_EndLoadTexPltt();               // restore the texture palette slots

}


static void _panelRelease(POKEMON_TRADE_WORK *pWork)
{

 int i;
  for(i=0;i<5;i++){
//    GFL_HEAP_FreeMemory(pWork->pTexBoard[i]);
  }
}



#define DEMO_INTENSITY_DF   23
#define DEMO_INTENSITY_AM   8
#define DEMO_INTENSITY_SP   31

static const GXRgb DEMO_DIFFUSE_COL =
GX_RGB(DEMO_INTENSITY_DF, DEMO_INTENSITY_DF, DEMO_INTENSITY_DF);
static const GXRgb DEMO_AMBIENT_COL =
GX_RGB(DEMO_INTENSITY_AM, DEMO_INTENSITY_AM, DEMO_INTENSITY_AM);
static const GXRgb DEMO_SPECULAR_COL =
GX_RGB(DEMO_INTENSITY_SP, DEMO_INTENSITY_SP, DEMO_INTENSITY_SP);
static const GXRgb DEMO_EMISSION_COL = GX_RGB(0, 0, 0);

static const u32 LIGHTING_L_DOT_S_SHIFT = 8;


static void DEMO_Set3DDefaultMaterial(BOOL bUsediffuseAsVtxCol, BOOL bUseShininessTbl)
{
    G3_MaterialColorDiffAmb(DEMO_DIFFUSE_COL,   // diffuse
                            DEMO_AMBIENT_COL,   // ambient
                            bUsediffuseAsVtxCol // use diffuse as vtx color if TRUE
        );

    G3_MaterialColorSpecEmi(DEMO_SPECULAR_COL,  // specular
                            DEMO_EMISSION_COL,  // emission
                            bUseShininessTbl    // use shininess table if TRUE
        );
}



static float posfx = -1;

static void _createBoard(float pos, int index)
{
  u16     Rotate = 182*(360-70);                // for rotating cubes(0-65535)
  fx32 height;
  
 G3_PushMtx();
  {
    fx16    s = FX_SinIdx(Rotate);
    fx16    c = FX_CosIdx(Rotate);

    G3_Scale(FX32_ONE*0.15,FX32_ONE*0.15,FX32_ONE*0.15);

    G3_Translate(0, 0, -5.5 * FX32_ONE); //-5.5 * FX32_ONE);

    G3_Translate(0, -4.3*FX32_ONE , 0);
    G3_Translate(pos * FX32_ONE, 0, 0);

    G3_RotX(s, c);

  }

  {
    G3_MtxMode(GX_MTXMODE_TEXTURE);
    G3_Identity();
    // Use an identity matrix for the texture matrix for simplicity
    G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
  }

  // Set the material color( diffuse, ambient , specular ) as basic white
  DEMO_Set3DDefaultMaterial(TRUE, TRUE);
  //DEMO_Set3DDefaultShininessTable();

  G3_TexImageParam(GX_TEXFMT_PLTT16,      // use 16 colors palette texture
                   GX_TEXGEN_TEXCOORD,    // use texcoord
                   GX_TEXSIZE_S64,        // 64 pixels
                   GX_TEXSIZE_T64,        // 64 pixels
                   GX_TEXREPEAT_NONE,     // no repeat
                   GX_TEXFLIP_NONE,       // no flip
                   GX_TEXPLTTCOLOR0_USE,  // use color 0 of the palette
                   myTexAddr+index*2048     // the offset of the texture image
                   );

  G3_TexPlttBase(myTexPlttAddr,  // the offset of the texture palette
                 GX_TEXFMT_PLTT16 // 16 colors palette texture
                 );

  G3_PolygonAttr(GX_LIGHTMASK_0, // Light #0 is on
                 GX_POLYGONMODE_MODULATE, // modulation mode
                 GX_CULL_NONE,   // cull none
                 0,              // polygon ID(0 - 63)
                 31,             // alpha(0 - 31)
                 0               // OR of GXPolygonAttrMisc's value
                 );

  G3_Begin(GX_BEGIN_QUADS);
  {
    tex_coord(1);
    normal(0);
    vtx(2);
    tex_coord(0);
    normal(0);
    vtx(0);
    tex_coord(2);
    normal(0);
    vtx(4);
    tex_coord(3);
    normal(0);
    vtx(6);

  }
  G3_End();
  G3_PopMtx(1);
}



static void _polygondraw(POKEMON_TRADE_WORK *pWork)
{
  u16     Rotate = 0;                // for rotating cubes(0-65535)

//  return;
  
  G3X_Reset();

  //---------------------------------------------------------------------------
  // Set up a camera matrix
  //---------------------------------------------------------------------------
  {
    VecFx32 Eye = { 0, 0, FX32_ONE };   // Eye position
    VecFx32 at = { 0, 0, 0 };  // Viewpoint
    VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

    G3_LookAt(&Eye, &vUp, &at, NULL);
  }

  //---------------------------------------------------------------------------
  // Set up light colors and direction.
  // Notice that light vector is transformed by the current vector matrix
  // immediately after LightVector command is issued.
  //
  // GX_LIGHTID_0: white, downward
  //---------------------------------------------------------------------------
  G3_LightVector(GX_LIGHTID_0, FX16_SQRT1_3, -FX16_SQRT1_3, -FX16_SQRT1_3);
  G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));





  {
    int i;
    float num = pWork->FriendBoxScrollNum;

    num = num / 127;
  
    for(i=0;i<25;i++){
      num = num - i;
      _createBoard(  3.7 * num ,   i  );
    }
  }

  
}

#endif
