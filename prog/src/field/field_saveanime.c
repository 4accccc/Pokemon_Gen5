//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_saveaime.c
 *	@brief  セーブアニメ    VBlankを使用したBGアニメ
 *	@author	tomoya takahashi
 *	@date		2010.02.26
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "arc_def.h"
#include "fieldmap/save_icon.naix"


#include "system/gfl_use.h"

#include "field_saveanime.h"
#include "field_camera.h"


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
// VBlankTCB優先順位
#define FIELD_SAVEANIME_TCB_PRI (128)


// 見た目
#define FIELD_SAVEANIME_PAL_NO  ( 9 ) // 使用パレット
#define FIELD_SAVEANIME_PAL_USE (1)   // 使用本数
#define FIELD_SAVEANIME_BG_FRAME  ( GFL_BG_FRAME3_M ) // 使用BGフレーム

#define FIELD_SAVEANIME_BG_CHAR_SIZX  ( 4 )
#define FIELD_SAVEANIME_BG_CHAR_SIZY  ( 4 )

#define FIELD_SAVEANIME_ONE_INDEX_CHAR_SIZ  ( FIELD_SAVEANIME_BG_CHAR_SIZX*FIELD_SAVEANIME_BG_CHAR_SIZY*GFL_BG_1CHRDATASIZ )

// ビットマップ描画位置オフセット
#define FIELD_SAVEANIME_BMP_POS_X_OFS ( -16 )
#define FIELD_SAVEANIME_BMP_POS_Y_OFS ( -24 )
#define FIELD_SAVEANIME_POS_Y_OFS ( 28*FX32_ONE )

// 性別別パレットインデックス
static const int sc_PLTT_INDEX[ PM_NEUTRAL ] = 
{
  0, 1,
};


// フレーム情報
typedef struct {
  u16 index;
  u16 draw_frame;
}ANIME_FRAME;
#define FIELD_SAVEANIME_FRAME_MAX (80)
static const ANIME_FRAME sc_FIELD_SAVEANIME_FRAME[] = 
{
  { 0, 20 },
  { 1, 40 },
  { 0, 60 },
  { 2, FIELD_SAVEANIME_FRAME_MAX },
};

#define FIELD_SAVEANIME_TBL_MAX (NELEMS(sc_FIELD_SAVEANIME_FRAME))


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	FIELD_SAVEANIME
//=====================================
struct _FIELD_SAVEANIME{

  FIELDMAP_WORK* p_fieldmap;

  GFL_BMPWIN* p_win;

  GFL_TCB* p_tcb;

  void* p_charbuf;
  NNSG2dCharacterData* p_char;


  int frame;
  int anime_index;
} ;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	画像
//=====================================
static void SAVEANIME_LoadResource( FIELD_SAVEANIME* p_wk, HEAPID heapID );
static void SAVEANIME_ReleaseResource( FIELD_SAVEANIME* p_wk );

//-------------------------------------
///	ビットマップ
//=====================================
static void SAVEANIME_CreateBmpWin( FIELD_SAVEANIME* p_wk );
static void SAVEANIME_DeleteBmpWin( FIELD_SAVEANIME* p_wk );
static void SAVEANIME_OnBmpWin( FIELD_SAVEANIME* p_wk );
static void SAVEANIME_OffBmpWin( FIELD_SAVEANIME* p_wk );
static void SAVEANIME_TransIndex( FIELD_SAVEANIME* p_wk, int index );



//-------------------------------------
///	VBlank関数
//=====================================
static void SAVEANIME_VBlank( GFL_TCB* p_tcb, void* p_work );


//----------------------------------------------------------------------------
/**
 *	@brief  セーブアニメーションの生成
 *
 *	@param	heapID        ヒープID
 *	@param	p_fieldmap    フィールドマップ
 *
 *	@return
 */
//-----------------------------------------------------------------------------
FIELD_SAVEANIME* FIELD_SAVEANIME_Create( HEAPID heapID, FIELDMAP_WORK* p_fieldmap )
{
  FIELD_SAVEANIME* p_wk;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_SAVEANIME) );

  p_wk->p_fieldmap = p_fieldmap;

  // 画像読み込み
  SAVEANIME_LoadResource( p_wk, heapID );

  // ビットマップ生成
  SAVEANIME_CreateBmpWin( p_wk );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  セーブアニメ　破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_SAVEANIME_Delete( FIELD_SAVEANIME* p_wk )
{
  // 一応読んでおく
  FIELD_SAVEANIME_End( p_wk );
  
  // ビットマップ破棄
  SAVEANIME_DeleteBmpWin( p_wk );

  // 画像破棄
  SAVEANIME_ReleaseResource( p_wk );

  // ワーク破棄
  GFL_HEAP_FreeMemory( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  アニメーション開始
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_SAVEANIME_Start( FIELD_SAVEANIME* p_wk )
{
  GF_ASSERT( p_wk->p_tcb == NULL );

  GFL_BG_ClearFrame( FIELD_SAVEANIME_BG_FRAME );
  
  p_wk->frame = 0;
  // WinOn
  SAVEANIME_OnBmpWin( p_wk );

  //Vblank登録
  p_wk->p_tcb = GFUser_VIntr_CreateTCB( SAVEANIME_VBlank, p_wk, FIELD_SAVEANIME_TCB_PRI );

}


//----------------------------------------------------------------------------
/**
 *	@brief  アニメーション終了
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_SAVEANIME_End( FIELD_SAVEANIME* p_wk )
{
  if(p_wk->p_tcb){
    
    // VBlank破棄
    GFL_TCB_DeleteTask( p_wk->p_tcb );
    p_wk->p_tcb = NULL;

    // WinOff
    SAVEANIME_OffBmpWin( p_wk );
  }
}





//-----------------------------------------------------------------------------
/**
 *      private
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	画像
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  リソース読み込み
 *
 *	@param	p_wk      ワーク
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void SAVEANIME_LoadResource( FIELD_SAVEANIME* p_wk, HEAPID heapID )
{
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_FIELD_SAVE_ICON, heapID );
  GAMESYS_WORK * p_gsys = FIELDMAP_GetGameSysWork( p_wk->p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
  const MYSTATUS* cp_mystatus = GAMEDATA_GetMyStatus( p_gdata );
  u32 sex = MyStatus_GetMySex( cp_mystatus );
  
  // 自分が男か女か？
  GF_ASSERT( sex < NELEMS(sc_PLTT_INDEX) );

  // BGR
  p_wk->p_charbuf = GFL_ARCHDL_UTIL_LoadBGCharacter( p_handle, NARC_save_icon_report_icon_NCGR, FALSE,
      &p_wk->p_char, heapID );

  // PLTT転送
  GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_save_icon_report_icon_NCLR,
      PALTYPE_MAIN_BG, sc_PLTT_INDEX[sex]*32, FIELD_SAVEANIME_PAL_NO*32, 
      FIELD_SAVEANIME_PAL_USE*32, heapID );

  GFL_ARC_CloseDataHandle( p_handle );
}

//----------------------------------------------------------------------------
/**
 *	@brief  リソース破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void SAVEANIME_ReleaseResource( FIELD_SAVEANIME* p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_charbuf );
  p_wk->p_charbuf = NULL;
}


//-------------------------------------
///	ビットマップ
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  ビットマップ生成
 *
 *	@param	p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SAVEANIME_CreateBmpWin( FIELD_SAVEANIME* p_wk )
{
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_wk->p_fieldmap );
  const GFL_G3D_CAMERA *cp_g3Dcamera = FIELD_CAMERA_GetCameraPtr( p_camera ); //g3Dcamera Lib ハンドル
  VecFx32 pos;
  int  pos_x, pos_y;
  
  // ビットマップ描画位置を求める
  {
    VecFx32 camera_way;
    VecFx32 camera_way_xz;
    VecFx32 camera_side;
    VecFx32 camera_up;
    VecFx32 camera_pos;
    VecFx32 camera_target;
    static const VecFx32 up_way = { 0,FX32_ONE,0 };

	  GFL_G3D_CAMERA_Switching( cp_g3Dcamera );

    GFL_G3D_CAMERA_GetTarget( cp_g3Dcamera, &camera_target );
    GFL_G3D_CAMERA_GetPos( cp_g3Dcamera, &camera_pos );

    VEC_Subtract( &camera_target, &camera_pos, &camera_way );
    // XZ平面横方向取得
    camera_way_xz = camera_way;
    camera_way_xz.y = 0;
    VEC_Normalize( &camera_way_xz, &camera_way_xz );

    // 横方向を外積で求める
    VEC_CrossProduct( &camera_way_xz, &up_way, &camera_side );
    // 横とカメラ方向から上を求める
    VEC_CrossProduct( &camera_way, &camera_side, &camera_up );
    VEC_Normalize( &camera_up, &camera_up );

    FIELD_PLAYER_GetPos( p_player, &pos );
    // 上方向に移動
    pos.x += FX_Mul( camera_up.x, -FIELD_SAVEANIME_POS_Y_OFS );
    pos.y += FX_Mul( camera_up.y, -FIELD_SAVEANIME_POS_Y_OFS );
    pos.z += FX_Mul( camera_up.z, -FIELD_SAVEANIME_POS_Y_OFS );
    NNS_G3dWorldPosToScrPos( &pos, &pos_x, &pos_y );
    pos_x += FIELD_SAVEANIME_BMP_POS_X_OFS; // Xはスクリーン座標であわせる
    pos_y += FIELD_SAVEANIME_BMP_POS_Y_OFS; // Xはスクリーン座標であわせる
    
  }

  // 位置決定
  {

    pos_x /= 8;
    pos_y /= 8;

    if( pos_x < 0 ){
      pos_x = 0;
    }
    if( pos_y < 0 ){
      pos_y = 0;
    }
    if( pos_x > 31 ){
      pos_x = 31;
    }
    if( pos_y > 24 ){
      pos_y = 24;
    }
  }
  OS_TPrintf( "pos_x %d pos_y %d\n", pos_x, pos_y );

  // ビットマップ生成
  p_wk->p_win = GFL_BMPWIN_Create(
    FIELD_SAVEANIME_BG_FRAME,
    pos_x, pos_y, 
    FIELD_SAVEANIME_BG_CHAR_SIZX,FIELD_SAVEANIME_BG_CHAR_SIZY,
    FIELD_SAVEANIME_PAL_NO, GFL_BMP_CHRAREA_GET_B );

  // 画面反映
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(p_wk->p_win), 15);
  GFL_BMPWIN_TransVramCharacter(p_wk->p_win);

  // 1インデックス書き込み
  SAVEANIME_TransIndex( p_wk, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビットマップ破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void SAVEANIME_DeleteBmpWin( FIELD_SAVEANIME* p_wk )
{
  GFL_BMPWIN_Delete( p_wk->p_win );
  p_wk->p_win = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BmpWinOn
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void SAVEANIME_OnBmpWin( FIELD_SAVEANIME* p_wk )
{
  // スクリーン書き込み
  GFL_BMPWIN_MakeScreen(p_wk->p_win);
  GFL_BG_LoadScreenV_Req( FIELD_SAVEANIME_BG_FRAME );
  GFL_BG_SetVisible( FIELD_SAVEANIME_BG_FRAME, VISIBLE_ON );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BmpWinClear
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void SAVEANIME_OffBmpWin( FIELD_SAVEANIME* p_wk )
{
  // スクリーンクリア
  GFL_BMPWIN_ClearScreen( p_wk->p_win );
  GFL_BG_LoadScreenV_Req( FIELD_SAVEANIME_BG_FRAME );
  GFL_BG_SetVisible( FIELD_SAVEANIME_BG_FRAME, VISIBLE_OFF );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ビットマップ転送
 *
 *	@param	p_wk    ワーク
 *	@param	frame   フレーム
 */
//-----------------------------------------------------------------------------
static void SAVEANIME_TransIndex( FIELD_SAVEANIME* p_wk, int index )
{
  u32 char_ofs;
  u32 res_ofs;
  int i;
  u8* p_data = (u8*)p_wk->p_char->pRawData;

  // 転送先オフセット
  char_ofs = GFL_BMPWIN_GetChrNum( p_wk->p_win );

  // リソースのオフセット
  res_ofs = FIELD_SAVEANIME_ONE_INDEX_CHAR_SIZ * index;

  // ウィンドウのキャラクタオフセットに転送。
  GFL_BG_LoadCharacter( FIELD_SAVEANIME_BG_FRAME, &p_data[ res_ofs ],
      FIELD_SAVEANIME_ONE_INDEX_CHAR_SIZ, char_ofs );
}




//-------------------------------------
///	VBlank関数
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  VBlankメイン
 *
 *	@param	p_tcb   TCB
 *	@param	p_work  ワーク
 */
//-----------------------------------------------------------------------------
static void SAVEANIME_VBlank( GFL_TCB* p_tcb, void* p_work )
{
  FIELD_SAVEANIME* p_wk = p_work;
  int i;
  int nex_index;
  
  // 出来るだけ早くしたいのでべた書き

  // フレームカウント
  p_wk->frame ++;

  // 今のフレームの限界値を超えたら次へ
  if( sc_FIELD_SAVEANIME_FRAME[ p_wk->anime_index ].draw_frame <= p_wk->frame ){
    p_wk->anime_index ++;

    // ループチェック
    if( p_wk->anime_index >= FIELD_SAVEANIME_TBL_MAX ){
      p_wk->anime_index = 0;
      p_wk->frame = 0;
    }

    // 転送
    SAVEANIME_TransIndex( p_wk, sc_FIELD_SAVEANIME_FRAME[ p_wk->anime_index ].index );
  }
}



