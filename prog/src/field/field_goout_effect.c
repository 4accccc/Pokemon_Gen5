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
#include "system/gfl_use.h"
#include "field/zonedata.h"

#include "field_goout_effect.naix"
#include "arc_def.h"

#include "field_goout_effect.h"

#include "debug/debugwin_sys.h"
#include "field_goout_effect_data.h"


#ifdef PM_DEBUG
#define DEBUGMENU_GOOUT_EFFECT
#endif //PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM (3)
#define FIELD_GOOUT_EFFECT_CAMERA_DISTANCE      (60)

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
  BOOL is_enable;
  s32 seq;
  FIELD_GOOUT_DATA *p_data;
  GOOUT_EFFECT_ID effectID;
  GAMEDATA *p_gamedata;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void Field_Goout_Effect_AttachCamera( FIELD_GOOUT_EFFECT *p_wk, const GFL_G3D_CAMERA* cp_camera );
static void Field_Goout_Effect_LoadResource( FLD_EXP_OBJ_CNT_PTR p_ex_obj, const FIELD_GOOUT_DATA *cp_data, GOOUT_EFFECT_ID effectID );


static BOOL Field_Goout_Effect_IsOutZone( const GAMEDATA *cp_gamedata, ZONEID zoneID );
static FIELD_GOOUT_TIMEZONE Field_Goout_Effect_GetTimeZone( u32 timezone );

//-------------------------------------
///	デバッグ
//=====================================
#ifdef DEBUGMENU_GOOUT_EFFECT
static void GOOUT_DEBUG_Init( FIELD_GOOUT_EFFECT *p_wk, GAMEDATA *p_gamedata, HEAPID heapID );
static void GOOUT_DEBUG_Exit( void );
static void GOOUT_DEBUG_Main( void );
static BOOL GOOUT_DEBUG_GetForceVisible( void );
static int  GOOUT_DEBUG_GetCameraDistance( void );
static int  GOOUT_DEBUG_GetCameraDistance( void );
#else
#define GOOUT_DEBUG_Init( x, y, z )   /*  */
#define GOOUT_DEBUG_Exit()      /*  */
#define GOOUT_DEBUG_Main()      /*  */
#define GOOUT_DEBUG_GetForceVisible() 0
#define GOOUT_DEBUG_GetCameraDistance() FIELD_GOOUT_EFFECT_CAMERA_DISTANCE
#endif //DEBUGMENU_GOOUT_EFFECT

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  外出表現  作成
 *
 *  @param  フィールド3D拡張オブジェクト
 *  @param  zoneID  ゾーンID
 *	@param	HEAPID heapID ヒープID
 *
 *	@return ワークハンドル
 */
//-----------------------------------------------------------------------------
FIELD_GOOUT_EFFECT * FIELD_GOOUT_EFFECT_Init( GAMEDATA *p_gamedata, FLD_EXP_OBJ_CNT_PTR p_ex_obj, ZONEID zoneID, u8 season, u32 timezone, HEAPID heapID )
{
  FIELD_GOOUT_EFFECT  *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_GOOUT_EFFECT) );
  p_wk->p_data      = FIELD_GOOUT_DATA_Init( heapID );
  p_wk->p_ex_obj    = p_ex_obj;
  p_wk->p_gamedata  = p_gamedata;

  //リソース読込み
  {
    FIELD_GOOUT_TIMEZONE goout_tz = Field_Goout_Effect_GetTimeZone( timezone );
    GOOUT_COMBI_ID combiID  = FIELD_GOOUT_DATA_GetCombiIDByZoneID( p_wk->p_data, zoneID, season, goout_tz );
    u8  max =  FIELD_GOOUT_DATA_GetEffectNum( p_wk->p_data, combiID );
    if( max == 0 )
    {
      p_wk->effectID = GOOUT_EFF_NULL;
    }
    else
    {
      p_wk->effectID  = FIELD_GOOUT_DATA_GetEffectID( p_wk->p_data, combiID, GFUser_GetPublicRand(max) );
    }

    NAGI_Printf( "室外表現：組み合わせ=%d 演出=%d\n　ゾーンID=%d 季節=%d 時刻=%d\n", combiID, p_wk->effectID, zoneID, season, timezone );

    if( p_wk->effectID != GOOUT_EFF_NULL )
    {
      Field_Goout_Effect_LoadResource( p_wk->p_ex_obj, p_wk->p_data, p_wk->effectID );
    }
  }

  //使用チェック
  {
    p_wk->is_enable = GAMEDATA_GetGooutEffectFlag( p_wk->p_gamedata );
    NAGI_Printf( "室外表現許可=%d\n", p_wk->is_enable );
  }

  //デバッグ開始
  GOOUT_DEBUG_Init( p_wk, p_gamedata, heapID );

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

  if( p_wk->effectID != GOOUT_EFF_NULL )
  {
    FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM );
  }

  FIELD_GOOUT_DATA_Exit( p_wk->p_data );
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

#ifdef PM_DEBUG
  GOOUT_DEBUG_Main();
#endif

  //室内カウント
  {
    ZONEID  zoneID  = PLAYERWORK_getZoneID( GAMEDATA_GetMyPlayerWork(p_wk->p_gamedata) );
    if( !Field_Goout_Effect_IsOutZone( p_wk->p_gamedata, zoneID ) )
    {
      u32 count = GAMEDATA_GetGooutEffectRoomCount( p_wk->p_gamedata );
      if( count != 0xffffffff )
      {
        count+=2; //1/30なので
      }
      GAMEDATA_SetGooutEffectRoomCount( p_wk->p_gamedata, count );
    }
  }

  //演出なしならなにもしない
  if( p_wk->effectID == GOOUT_EFF_NULL )
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

  if( p_wk->is_setup )
  {
    switch( p_wk->seq )
    {
    case SEQ_INIT:
      {
        int i;
        EXP_OBJ_ANM_CNT_PTR p_anm_cnt;
        for( i = 0; i < FIELD_GOOUT_RESOURCE_MAX-1; i++ )
        {
          FLD_EXP_OBJ_ValidCntAnm(p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, 0, i, TRUE);
          p_anm_cnt = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, 0, i);
          FLD_EXP_OBJ_ChgAnmStopFlg(p_anm_cnt, TRUE );
          //頭出し
          FLD_EXP_OBJ_SetObjAnmFrm( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, 0, i, 0 );
          FLD_EXP_OBJ_ChgAnmLoopFlg(p_anm_cnt, 0);
        }
      }
      FLD_EXP_OBJ_SetVanish
        (p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, 0, FALSE );
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
        for( i = 0; i < FIELD_GOOUT_RESOURCE_MAX-1; i++ )
        {
          p_anm_cnt = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, 0, i);
          FLD_EXP_OBJ_ChgAnmStopFlg(p_anm_cnt, 0);
        }
      }
      p_wk->seq++;
      break;
    case SEQ_ANM_WAIT:
      {
        int i;
        BOOL result = TRUE;
        for( i = 0; i < FIELD_GOOUT_RESOURCE_MAX-1; i++ )
        {
          EXP_OBJ_ANM_CNT_PTR p_anm_cnt = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, 0, i);

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
          (p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, 0, TRUE );
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
  if( p_wk->is_enable )
  {
    p_wk->is_setup  = TRUE;
    p_wk->seq       = 0;
  }
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
  if( p_wk->is_enable )
  {
    p_wk->is_start  = TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンの切り替わりをチェック
 *
 *	@param	FIELD_GOOUT_EFFECT *p_wk  ワーク
 *	@param	zoneID  ゾーンID
 */
//-----------------------------------------------------------------------------
void FIELD_GOOUT_EFFECT_CheckZone( FIELD_GOOUT_EFFECT *p_wk, ZONEID zoneID )
{
  BOOL is_enable  = FALSE;
  if( Field_Goout_Effect_IsOutZone( p_wk->p_gamedata, zoneID) )
  {
    //もし規定時間以上かつ外ならば、外出表現開始してもOK
    if( FIELD_GOOUT_ROOM_COUNT_MAX*60 < GAMEDATA_GetGooutEffectRoomCount( p_wk->p_gamedata ) )
    {
      is_enable = TRUE;
    }

    //もし外ならカウンタリセット
    GAMEDATA_SetGooutEffectRoomCount( p_wk->p_gamedata, 0 );
  }

  GAMEDATA_SetGooutEffectFlag( p_wk->p_gamedata, is_enable );
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
    GFL_G3D_OBJSTATUS *p_status;
    p_status = FLD_EXP_OBJ_GetUnitObjStatus(p_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, 0 );
    p_status->trans = pos;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リソース読込み
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR p_ex_obj  拡張オブジェクト
 *	@param	FIELD_GOOUT_DATA *cp_data     データ
 *	@param  GOOUT_EFFECT_ID effectID      エフェクトID
 */
//-----------------------------------------------------------------------------
static void Field_Goout_Effect_LoadResource( FLD_EXP_OBJ_CNT_PTR p_ex_obj, const FIELD_GOOUT_DATA *cp_data, GOOUT_EFFECT_ID effectID )
{
  GFL_G3D_UTIL_SETUP  setup = {0};
  GFL_G3D_UTIL_RES    res_tbl[FIELD_GOOUT_RESOURCE_MAX] = {0};
  GFL_G3D_UTIL_OBJ    obj = {0};
  GFL_G3D_UTIL_ANM    anm_tbl[FIELD_GOOUT_RESOURCE_MAX-1] = {0};
  u32 res_cnt = 0;
  FIELD_GOOUT_RESOURCE res_index_tbl[FIELD_GOOUT_RESOURCE_MAX]= {0};

  //リソース
  {
    int i;
    for( i = 0; i < FIELD_GOOUT_RESOURCE_MAX; i++ )
    {
      ARCDATID  datID = FIELD_GOOUT_DATA_GetResource( cp_data, effectID, i );
      if( datID != GOOUT_EFF_RESOUCE_NONE )
      {
        res_tbl[res_cnt].arcive   = ARCID_FIELD_GOOUT_EFFECT;
        res_tbl[res_cnt].datID    = datID;
        res_tbl[res_cnt].arcType  = GFL_G3D_UTIL_RESARC;
        res_index_tbl[res_cnt]    = i;
        res_cnt++;
      }
    }
  }
  
  //アニメ
  {
    int i;
    for( i = 0; i < (res_cnt-1); i++ )
    {
      anm_tbl[i].anmresID  = res_index_tbl[i+1];
      anm_tbl[i].anmdatID  = 0;
    }
  }

  //OBJ
  obj.mdlresID  = FIELD_GOOUT_RESOURCE_IMD;
  obj.mdldatID  = 0;
  obj.texresID  = FIELD_GOOUT_RESOURCE_IMD;
  obj.anmTbl    = anm_tbl;
  obj.anmCount  = (res_cnt-1);

  //設定
  setup.resTbl    = res_tbl;
  setup.resCount  = res_cnt;
  setup.objTbl    = &obj;
  setup.objCount  = 1;

  FLD_EXP_OBJ_AddUnit( p_ex_obj,
      &setup,
      FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM );

  FLD_EXP_OBJ_SetVanish
        (p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM, 0, TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  外判定
 *
 *	@param	const GAMEDATA *cp_gamedata ゲームデータ
 *	@param	zoneID  ゾーンID
 *
 *	@return TRUE外  FALSE中
 */
//-----------------------------------------------------------------------------
static BOOL Field_Goout_Effect_IsOutZone( const GAMEDATA *cp_gamedata, ZONEID zoneID )
{
  return ZONEDATA_FlyEnable( zoneID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  共通タイムゾーンから室外表現内のタイムゾーンへ変換
 *
 *	@param	u32 timezone  タイムゾーン
 *
 *	@return 室外表現のタイムゾーン
 */
//-----------------------------------------------------------------------------
static FIELD_GOOUT_TIMEZONE Field_Goout_Effect_GetTimeZone( u32 timezone )
{
  switch( timezone )
  {
  case TIMEZONE_MORNING:
  case TIMEZONE_NOON:
    return FIELD_GOOUT_TIMEZONE_MORNING;
  case TIMEZONE_EVENING:
    return FIELD_GOOUT_TIMEZONE_EVENING;
  case TIMEZONE_NIGHT:
  case TIMEZONE_MIDNIGHT:
    return FIELD_GOOUT_TIMEZONE_NIGHT;
  default:
    GF_ASSERT(0);
    return 0;
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
extern int g_debug_force_visible;
extern int g_debug_camera_distance;
extern int g_debug_use_obj_id;
static BOOL s_debug_obj_change  = FALSE;
static FIELD_GOOUT_EFFECT *s_debug_wk  = NULL;
//-------------------------------------
///	プロトタイプ
//=====================================
static BOOL DebugWin_Util_ChangeValue( int *p_param, int min, int max );
static void Goout_Debug_Update_ForceVisible( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Draw_ForceVisible( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Update_CameraDistance( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Draw_CameraDistance( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );

static void Goout_Debug_Update_UseObjID( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Draw_UseObjID( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Update_RoomCount( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void Goout_Debug_Draw_RoomCount( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグメニュー  作成
 *
 *	@param	HEAPID heapID 
 */
//-----------------------------------------------------------------------------
static void GOOUT_DEBUG_Init( FIELD_GOOUT_EFFECT *p_wk, GAMEDATA *p_gamedata, HEAPID heapID )
{
  s_debug_wk  = p_wk;
  DEBUGWIN_AddGroupToTop( DEBUGWIN_GOTOU_EFFECT, "しつがいエフェクト", heapID );

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
  //室内カウンタ
  DEBUGWIN_AddItemToGroupEx( Goout_Debug_Update_RoomCount, 
                                Goout_Debug_Draw_RoomCount,
                                p_gamedata,
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
 *	@brief  デバッグメニューメイン処理
 */
//-----------------------------------------------------------------------------
static void GOOUT_DEBUG_Main( void )
{
  if( s_debug_obj_change )
  {
    if( s_debug_wk->effectID != GOOUT_EFF_NULL )
    {
      FLD_EXP_OBJ_DelUnit( s_debug_wk->p_ex_obj, FIELD_GOOUT_EFFECT_EXP_OBJ_REGISTER_NUM );
    }
    s_debug_wk->effectID  = g_debug_use_obj_id;
    if( s_debug_wk->effectID != GOOUT_EFF_NULL )
    {
      Field_Goout_Effect_LoadResource( s_debug_wk->p_ex_obj, s_debug_wk->p_data, s_debug_wk->effectID );
    }
    s_debug_obj_change  = FALSE;
  }
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
 *	@brief  デバッグメニュー  汎用値更新関数
 *
 *	@param	*p_param  更新する値
 *	@param	min 最小値
 *	@param	max 最大値
 */
//-----------------------------------------------------------------------------
static BOOL DebugWin_Util_ChangeValue( int *p_param, int min, int max )
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
    return TRUE;
  }

  return FALSE;
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
  if( DebugWin_Util_ChangeValue( &g_debug_force_visible, 0, 1 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
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
  if( DebugWin_Util_ChangeValue( &g_debug_camera_distance, 0, 0xFFFFFFFF/2 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
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
  if( DebugWin_Util_ChangeValue( &g_debug_use_obj_id, 0, GOOUT_EFF_MAX-1 ) )
  {
    s_debug_obj_change  = TRUE;
    DEBUGWIN_RefreshScreen();
  }
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
  DEBUGWIN_ITEM_SetNameV( p_item , "えんしゅつ[%d]", g_debug_use_obj_id );
}
//----------------------------------------------------------------------------
/**
 *	@brief  室内カウンタ 更新
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Update_RoomCount( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  GAMEDATA *p_gamedata  = p_wk_adrs;
  int cnt = GAMEDATA_GetGooutEffectRoomCount( p_gamedata );
  if( DebugWin_Util_ChangeValue( &cnt, 0, 0xFFFFFFFF/2 ) )
  {
    GAMEDATA_SetGooutEffectRoomCount( p_gamedata, cnt );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  室内カウンタ 描画
 *
 *	@param	void* p_wk_adrs ワーク
 *	@param	p_item          アイテム
 */
//-----------------------------------------------------------------------------
static void Goout_Debug_Draw_RoomCount( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  GAMEDATA *p_gamedata  = p_wk_adrs;
  u32 cnt = GAMEDATA_GetGooutEffectRoomCount( p_gamedata );
  DEBUGWIN_ITEM_SetNameV( p_item , "へやカウンタ[%d]", cnt  );
}
#endif //DEBUGMENU_GOOUT_EFFECT
