//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_goout_effect.c
 *	@brief  フィールドの外出表現（空がまぶしかったり、雨があがったり…）
 *	@author	Toru=Nagihashi
 *	@data		2010.12.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

#include "field_goout_effect.naix"
#include "arc_def.h"

#include "field_goout_effect.h"

#include "debug/debugwin_sys.h"


#ifdef PM_DEBUG
#define DEBUGMENU_GOOUT_EFFECT
#endif //PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM (3)
#define FIELD_GOOUT_EFFECT_CAMERA_DISTANCE      (30)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
struct _FIELD_GOOUT_EFFECT
{
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
  BOOL is_setup;
  BOOL is_start;
  s32 seq;
  s32 use_obj_id;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void Field_Goout_Effect_AttachCamera( FIELD_GOOUT_EFFECT *p_wk, const GFL_G3D_CAMERA* cp_camera );

//-------------------------------------
///	デバッグ
//=====================================
#ifdef DEBUGMENU_GOOUT_EFFECT
static void GOOUT_DEBUG_Init( HEAPID heapID );
static void GOOUT_DEBUG_Exit( void );
static BOOL GOOUT_DEBUG_GetUseFlag( void );
static BOOL GOOUT_DEBUG_GetForceVisible( void );
static int  GOOUT_DEBUG_GetCameraDistance( void );
static int  GOOUT_DEBUG_GetCameraDistance( void );
static int  GOOUT_DEBUG_GetUseObjID( void );
#else
#define GOOUT_DEBUG_Init( x )   /*  */
#define GOOUT_DEBUG_Exit()      /*  */
#define GOOUT_DEBUG_GetUseFlag()  1
#define GOOUT_DEBUG_GetForceVisible() 0
#define GOOUT_DEBUG_GetCameraDistance() FIELD_GOOUT_EFFECT_CAMERA_DISTANCE
#define GOOUT_DEBUG_GetUseObjID() OBJ_ID_NORMAL
#endif //DEBUGMENU_GOOUT_EFFECT

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//リソースの並び順番
enum 
{
  RES_ID_NORMAL_MDL = 0,
  RES_ID_NORMAL_ICA,
  RES_ID_NORMAL_ITA,
  RES_ID_NORMAL_IMA,
  RES_ID_Z_MDL,
  RES_ID_Z_ICA,
  RES_ID_Z_IMA,
  RES_ID_NORMAL_ITP,
  RES_ID_AUTUMN_IMD,
  RES_ID_AUTUMN_ICA,
  RES_ID_AUTUMN_ITA,
  RES_ID_AUTUMN_IMA,
  RES_ID_MAX
};
//読み込む3Dリソース
static const GFL_G3D_UTIL_RES sc_g3d_util_res_tbl[RES_ID_MAX] = 
{
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_nsbmd, GFL_G3D_UTIL_RESARC },      //RES_ID_NORMAL_MDL
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_nsbca, GFL_G3D_UTIL_RESARC },      //RES_ID_NORMAL_ICA,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_nsbta, GFL_G3D_UTIL_RESARC },      //RES_ID_NORMAL_ITA,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_nsbma, GFL_G3D_UTIL_RESARC },      //RES_ID_NORMAL_IMA,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_z_nsbmd, GFL_G3D_UTIL_RESARC },    //RES_ID_Z_MDL,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_z_nsbca, GFL_G3D_UTIL_RESARC },    //RES_ID_Z_ICA,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_z_nsbma, GFL_G3D_UTIL_RESARC },    //RES_ID_Z_IMA,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_nsbtp, GFL_G3D_UTIL_RESARC },      //RES_ID_NORMAL_ITP,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_aki_nsbmd, GFL_G3D_UTIL_RESARC },  //RES_ID_AUTUMN_IMD,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_aki_nsbca, GFL_G3D_UTIL_RESARC },  //RES_ID_AUTUMN_ICA,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_aki_nsbta, GFL_G3D_UTIL_RESARC },  //RES_ID_AUTUMN_ITA,
  { ARCID_FIELD_GOOUT_EFFECT, NARC_field_goout_effect_d_effect_aki_nsbma, GFL_G3D_UTIL_RESARC },  //RES_ID_AUTUMN_IMA,
};

//3Dアニメ
static const GFL_G3D_UTIL_ANM sc_g3d_util_normal_anm_tbl[] = 
{
	{ RES_ID_NORMAL_ICA, 0 },
  { RES_ID_NORMAL_ITA, 0 },
  { RES_ID_NORMAL_ITP, 0 },
  { RES_ID_NORMAL_IMA, 0 },
};
static const GFL_G3D_UTIL_ANM sc_g3d_util_z_anm_tbl[] = 
{
	{ RES_ID_Z_ICA, 0 },
  { RES_ID_Z_IMA, 0 },
};
static const GFL_G3D_UTIL_ANM sc_g3d_util_autumn_anm_tbl[] = 
{
	{ RES_ID_AUTUMN_ICA, 0 },
  { RES_ID_AUTUMN_ITA, 0 },
  { RES_ID_AUTUMN_IMA, 0 },
};

enum
{
  OBJ_ID_NORMAL,
  OBJ_ID_Z,
  OBJ_ID_AUTUMN,
  OBJ_ID_MAX,
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ sc_g3d_util_obj_tbl[OBJ_ID_MAX] = {
	{
		RES_ID_NORMAL_MDL, 	      //モデルリソースID
		0, 							          //モデルデータID(リソース内部INDEX)
		RES_ID_NORMAL_MDL, 	      //テクスチャリソースID
		sc_g3d_util_normal_anm_tbl,			//アニメテーブル(複数指定のため)
		NELEMS(sc_g3d_util_normal_anm_tbl),	//アニメリソース数
	},
	{
		RES_ID_Z_MDL, 	      //モデルリソースID
		0, 							          //モデルデータID(リソース内部INDEX)
		RES_ID_Z_MDL, 	      //テクスチャリソースID
		sc_g3d_util_z_anm_tbl,			//アニメテーブル(複数指定のため)
		NELEMS(sc_g3d_util_z_anm_tbl),	//アニメリソース数
	},
  {
    RES_ID_AUTUMN_IMD,
    0,
    RES_ID_AUTUMN_IMD,
    sc_g3d_util_autumn_anm_tbl,
    NELEMS(sc_g3d_util_autumn_anm_tbl),
  },
};

static const GFL_G3D_UTIL_SETUP sc_g3d_util_setup =
{
  sc_g3d_util_res_tbl,				    //リソーステーブル
	RES_ID_MAX,		//リソース数
	sc_g3d_util_obj_tbl,				    //オブジェクト設定テーブル
	OBJ_ID_MAX,		//オブジェクト数
};
//----------------------------------------------------------------------------
/**
 *	@brief  外出表現  作成
 *
 *  @param  フィールド3D拡張オブジェクト
 *	@param	HEAPID heapID ヒープID
 *
 *	@return ワークハンドル
 */
//-----------------------------------------------------------------------------
FIELD_GOOUT_EFFECT * FIELD_GOOUT_EFFECT_Init( FLD_EXP_OBJ_CNT_PTR p_ex_obj, HEAPID heapID )
{
  FIELD_GOOUT_EFFECT  *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_GOOUT_EFFECT) );
  p_wk->p_ex_obj  = p_ex_obj;
  p_wk->use_obj_id  = OBJ_ID_NORMAL;

  {
    int i;
    FLD_EXP_OBJ_AddUnit( p_wk->p_ex_obj,
                          &sc_g3d_util_setup,
                          FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM );

    for( i = 0; i < OBJ_ID_MAX; i++ )
    {

      FLD_EXP_OBJ_SetVanish
          (p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, i, TRUE );
    }
  }

  GOOUT_DEBUG_Init( heapID );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  外出表現  破棄
 *
 *	@param	FIELD_GOOUT_EFFECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_GOOUT_EFFECT_Exit( FIELD_GOOUT_EFFECT *p_wk )
{
  GOOUT_DEBUG_Exit();

  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM );

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  外出表現  メイン
 *
 *	@param	FIELD_GOOUT_EFFECT *p_wk ワーク
 *	@param  FIELD_CAMERA  *p_camera カメラワーク
 */
//-----------------------------------------------------------------------------
void FIELD_GOOUT_EFFECT_Main( FIELD_GOOUT_EFFECT *p_wk, const FIELD_CAMERA * cp_camera )
{
  enum
  {
    SEQ_INIT,
    SEQ_WAIT,
    SEQ_ANM_START,
    SEQ_ANM_WAIT,
    SEQ_END,
  };

  if( !GOOUT_DEBUG_GetUseFlag() )
  {
    return;
  }

  if( GOOUT_DEBUG_GetForceVisible() )
  {
    p_wk->is_setup  = TRUE;
    p_wk->is_start  = TRUE;
    if( p_wk->seq > SEQ_ANM_WAIT )
    {
      p_wk->seq = 0;
    }
  }

  p_wk->use_obj_id  = GOOUT_DEBUG_GetUseObjID();

  if( p_wk->is_setup )
  {
    switch( p_wk->seq )
    {
    case SEQ_INIT:
      {
        int i;
        EXP_OBJ_ANM_CNT_PTR p_anm_cnt;
        for( i = 0; i < sc_g3d_util_obj_tbl[p_wk->use_obj_id].anmCount; i++ )
        {
          FLD_EXP_OBJ_ValidCntAnm(p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, p_wk->use_obj_id, i, TRUE);
          p_anm_cnt = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, p_wk->use_obj_id, i);
          FLD_EXP_OBJ_ChgAnmStopFlg(p_anm_cnt, TRUE );
          //頭出し
          FLD_EXP_OBJ_SetObjAnmFrm( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, p_wk->use_obj_id, i, 0 );
          FLD_EXP_OBJ_ChgAnmLoopFlg(p_anm_cnt, 0);
        }
      }
      FLD_EXP_OBJ_SetVanish
      (p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, p_wk->use_obj_id, FALSE );
      p_wk->seq++;
      break;
    case SEQ_WAIT:
      if( p_wk->is_start )
      {
        p_wk->seq++;
      }
      break;
    case SEQ_ANM_START:
      {
        int i;
        EXP_OBJ_ANM_CNT_PTR p_anm_cnt;
        for( i = 0; i < sc_g3d_util_obj_tbl[p_wk->use_obj_id].anmCount; i++ )
        {
          p_anm_cnt = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, p_wk->use_obj_id, i);
          FLD_EXP_OBJ_ChgAnmStopFlg(p_anm_cnt, 0);
        }
      }
      p_wk->seq++;
      break;
    case SEQ_ANM_WAIT:
      {
        int i;
        BOOL result = TRUE;
        for( i = 0; i < sc_g3d_util_obj_tbl[p_wk->use_obj_id].anmCount; i++ )
        {
          EXP_OBJ_ANM_CNT_PTR p_anm_cnt = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, p_wk->use_obj_id, i);

          if( !FLD_EXP_OBJ_ChkAnmEnd(p_anm_cnt) )
          {
            result  &= FALSE;
          }
          else
          {
            FLD_EXP_OBJ_ChgAnmStopFlg(p_anm_cnt, TRUE);
          }
        }
        if( result )
        {
          p_wk->seq++;
          FLD_EXP_OBJ_SetVanish
          (p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, p_wk->use_obj_id, TRUE );
        }
      }
      break;
    case SEQ_END:
      p_wk->is_start  = FALSE;
      p_wk->is_setup  = FALSE;
      break;
    default:
      GF_ASSERT(0);
    }
    Field_Goout_Effect_AttachCamera( p_wk, FIELD_CAMERA_GetCameraPtr(cp_camera) );
    FLD_EXP_OBJ_PlayAnime( p_wk->p_ex_obj );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  外出表現  表示設定
 *
 *	@param	FIELD_GOOUT_EFFECT *p_wk  ワーク
 *	@param	is_visible  TRUE表示  FALSE非表示
 */
//-----------------------------------------------------------------------------
void FIELD_GOOUT_EFFECT_Setup( FIELD_GOOUT_EFFECT *p_wk )
{
  p_wk->is_setup  = TRUE;
  p_wk->seq       = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  外出表現  開始
 *
 *	@param	FIELD_GOOUT_EFFECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_GOOUT_EFFECT_Start( FIELD_GOOUT_EFFECT *p_wk )
{
  p_wk->is_start  = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデルをカメラからの距離の位置におく
 *
 *	@param	FIELD_GOOUT_EFFECT *p_wk    ワーク
 *	@param	GFL_G3D_CAMERA* cp_camera   カメラ
 */
//-----------------------------------------------------------------------------
static void Field_Goout_Effect_AttachCamera( FIELD_GOOUT_EFFECT *p_wk, const GFL_G3D_CAMERA* cp_camera )
{
  VecFx32 cam_pos, up, target;
  VecFx32 pos;

  //カメラ設定取得
  GFL_G3D_CAMERA_GetPos( cp_camera, &cam_pos );
  GFL_G3D_CAMERA_GetCamUp( cp_camera, &up );
  GFL_G3D_CAMERA_GetTarget( cp_camera, &target );

  //カメラの先を計算
  VEC_Subtract( &target, &cam_pos, &pos );
  VEC_Normalize(&pos, &pos);
  VEC_MultAdd( FX32_CONST(GOOUT_DEBUG_GetCameraDistance()), &pos, &cam_pos, &pos );

  {
    int i;
    GFL_G3D_OBJSTATUS *p_status;
    for( i = 0; i < OBJ_ID_MAX; i++ )
    {
      p_status = FLD_EXP_OBJ_GetUnitObjStatus(p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, i );
      p_status->trans = pos;
    }
  }
}

#ifdef DEBUGMENU_GOOUT_EFFECT
//=============================================================================
/**
 *  デバッグ機能
 */
//=============================================================================
//-------------------------------------
///	定数
//=====================================
#define DEBUGWIN_GOTOU_EFFECT (16)

//-------------------------------------
///	グローバル
//    実体はdebug/debug_nagihashi.cにあります。
//    （staticにしてしまうとマップ遷移のとき情報が消えてしまうので
//    常駐にあります。）
//=====================================
extern int g_debug_use_flag;
extern int g_debug_force_visible;
extern int g_debug_camera_distance;
extern int g_debug_use_obj_id;

//-------------------------------------
///	プロトタイプ
//=====================================
static void DebugWin_Util_ChangeValue( DEBUGWIN_ITEM* p_item, int *p_param, int min, int max );
static void Goout_Debug_Update_UserFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Draw_UserFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Update_ForceVisible( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Draw_ForceVisible( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Update_CameraDistance( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Draw_CameraDistance( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );

static void Goout_Debug_Update_UseObjID( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Draw_UseObjID( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー  作成
 *
 *	@param	HEAPID heapID 
 */
//-----------------------------------------------------------------------------
static void GOOUT_DEBUG_Init( HEAPID heapID )
{
  DEBUGWIN_AddGroupToTop( DEBUGWIN_GOTOU_EFFECT, "しつがいエフェクト", heapID );

  //使う、使わない設定
  DEBUGWIN_AddItemToGroupEx( Goout_Debug_Update_UserFlag , 
                                Goout_Debug_Draw_UserFlag , 
                                NULL, 
                                DEBUGWIN_GOTOU_EFFECT,
                                heapID );
  //強制表示
  DEBUGWIN_AddItemToGroupEx( Goout_Debug_Update_ForceVisible , 
                                Goout_Debug_Draw_ForceVisible , 
                                NULL, 
                                DEBUGWIN_GOTOU_EFFECT,
                                heapID );
  //座標調整
  DEBUGWIN_AddItemToGroupEx( Goout_Debug_Update_CameraDistance , 
                                Goout_Debug_Draw_CameraDistance , 
                                NULL, 
                                DEBUGWIN_GOTOU_EFFECT,
                                heapID );
  //使用モデル
  DEBUGWIN_AddItemToGroupEx( Goout_Debug_Update_UseObjID, 
                                Goout_Debug_Draw_UseObjID, 
                                NULL, 
                                DEBUGWIN_GOTOU_EFFECT,
                                heapID );

}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー破棄
 */
//-----------------------------------------------------------------------------
static void GOOUT_DEBUG_Exit( void )
{
  DEBUGWIN_RemoveGroup(DEBUGWIN_GOTOU_EFFECT);
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー  使用フラグ設定
 *	@return TRUE使用  FALSE不使用
 */
//-----------------------------------------------------------------------------
static BOOL GOOUT_DEBUG_GetUseFlag( void )
{
  return g_debug_use_flag != 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー  強制表示フラグ取得
 *	@return TRUE強制表示ON  FALSE強制表示OFF
 */
//-----------------------------------------------------------------------------
static BOOL GOOUT_DEBUG_GetForceVisible( void )
{
  return g_debug_force_visible != 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー  カメラからの距離を取得
 *	@return カメラからの距離
 */
//-----------------------------------------------------------------------------
static int  GOOUT_DEBUG_GetCameraDistance( void )
{
  return g_debug_camera_distance;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー  使用OBJIDの取得
 *	@return 使用OBJID
 */
//-----------------------------------------------------------------------------
static int  GOOUT_DEBUG_GetUseObjID( void )
{
  return g_debug_use_obj_id;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー  汎用値更新関数
 *
 *	@param	DEBUGWIN_ITEM* p_item アイテム
 *	@param	*p_param  更新する値
 *	@param	min 最小値
 *	@param	max 最大値
 */
//-----------------------------------------------------------------------------
static void DebugWin_Util_ChangeValue( DEBUGWIN_ITEM* p_item, int *p_param, int min, int max )
{ 
  int value = 0;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  { 
    value = -1;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    value = +1;
  }

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A )
  { 
    value *= 10;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  { 
    value *= 100;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y )
  { 
    value *= 1000;
  }
  
  (*p_param)  +=  value;
  (*p_param)  = MATH_CLAMP( (*p_param), min, max );


  if( value != 0 )
  { 
    DEBUGWIN_RefreshScreen();
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  使用フラグ  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Update_UserFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DebugWin_Util_ChangeValue( p_item, &g_debug_use_flag, 0, 1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  使用フラグ  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Draw_UserFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char * sc_label[]  =
  {
    "OFF",
    "ON",
  };

  DEBUGWIN_ITEM_SetNameV( p_item , "しようフラグ[%s]", sc_label[g_debug_use_flag] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  強制表示  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Update_ForceVisible( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DebugWin_Util_ChangeValue( p_item, &g_debug_force_visible, 0, 1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  強制表示  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Draw_ForceVisible( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char * sc_label[]  =
  {
    "OFF",
    "ON",
  };

  DEBUGWIN_ITEM_SetNameV( p_item , "きょうせい[%s]", sc_label[g_debug_force_visible] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ距離  更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Update_CameraDistance( void* p_wk_adrs, DEBUGWIN_ITEM* p_item ){
  DebugWin_Util_ChangeValue( p_item, &g_debug_camera_distance, 0, 0xFFFFFFFF/2 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ距離  描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Draw_CameraDistance( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DEBUGWIN_ITEM_SetNameV( p_item , "きょり[%d]", g_debug_camera_distance );
}
//----------------------------------------------------------------------------
/**
 *	@brief  使用OBJID 更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Update_UseObjID( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DebugWin_Util_ChangeValue( p_item, &g_debug_use_obj_id, 0, OBJ_ID_MAX-1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  使用OBJID 描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Draw_UseObjID( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char * sc_label[OBJ_ID_MAX]  =
  {
    "げんてん",
    "Zずらし",
    "あき",
  };
  DEBUGWIN_ITEM_SetNameV( p_item , "モデル[%s]", sc_label[g_debug_use_obj_id] );
}
#endif //DEBUGMENU_GOOUT_EFFECT
