//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		prog\src\field\field_camera_debug.c
 *	@brief
 *	@author	 
 *	@date		2010.04.19
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include <calctool.h>

#include "field/field_const.h"

#include "field_g3d_mapper.h"
#include "field_common.h"
#include "field_camera_debug.h"
	
#include "arc_def.h"


// デバック機能を違うソースにするために、定義を別ヘッダーに移しました。
#include "field_camera_local.h"

#include "msg/debug/msg_d_tomoya.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


#ifdef  PM_DEBUG
#include "test/camera_adjust_view.h"
static fx32	fldWipeScale = 0;
//------------------------------------------------------------------
//  デバッグ用：下画面操作とのバインド
//------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_BindSubScreen(FIELD_CAMERA * camera, void * param, FIELD_CAMERA_DEBUG_BIND_TYPE type, HEAPID heapID)
{ 
  GFL_CAMADJUST * gflCamAdjust = param;

  // ワイプ表示初期化
  camera->p_debug_wipe = FLD_WIPEOBJ_Create( heapID );

  // ファー座標を取得
  GFL_G3D_CAMERA_GetFar( camera->g3Dcamera, &camera->debug_far );

  camera->debug_subscreen_type = type;
  camera->debug_trace_off = TRUE;
  if( type == FIELD_CAMERA_DEBUG_BIND_CAMERA_POS )
  {
    camera->debug_save_camera_mode = camera->mode;
	  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    GFL_CAMADJUST_SetCameraParam(	gflCamAdjust,
																	&camera->angle_yaw, 
																	&camera->angle_pitch, 
																	&camera->angle_len, 
																	&camera->fovy, 
																	&camera->debug_far);  
    GFL_CAMADJUST_SetWipeParam(		gflCamAdjust, &fldWipeScale );
  }
  else if( type == FIELD_CAMERA_DEBUG_BIND_TARGET_POS )
  {
    camera->debug_save_camera_mode = camera->mode;
	  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_TARGET_POS );
    camera->debug_target_yaw    = camera->angle_yaw;
    camera->debug_target_pitch  = camera->angle_pitch;
    camera->debug_target_len    = camera->angle_len;
    GFL_CAMADJUST_SetCameraParam(	gflCamAdjust,
																	&camera->debug_target_yaw, 
																	&camera->debug_target_pitch, 
																	&camera->debug_target_len, 
																	&camera->fovy, 
																	&camera->debug_far);  
    GFL_CAMADJUST_SetWipeParam(		gflCamAdjust, &fldWipeScale );
  }
}

void FIELD_CAMERA_DEBUG_ReleaseSubScreen(FIELD_CAMERA * camera)
{ 

  if( camera->debug_subscreen_type == FIELD_CAMERA_DEBUG_BIND_TARGET_POS ){

    OS_TPrintf( "DEBUG Camera Target x[0x%x] y[0x%x] z[0x%x]\n", camera->debug_target.x, camera->debug_target.y, camera->debug_target.z  );
    OS_TPrintf( "DEBUG Camera Pos x[0x%x] y[0x%x] z[0x%x]\n", camera->camPos.x, camera->camPos.y, camera->camPos.z  );

    VEC_Subtract( &camera->debug_target, &camera->target, &camera->target_offset );
    OS_TPrintf( "DEBUG Scroll Camera Target Offset x[0x%x] y[0x%x] z[0x%x]\n", camera->target_offset.x, camera->target_offset.y, camera->target_offset.z  );
  }

  camera->debug_subscreen_type = FIELD_CAMERA_DEBUG_BIND_NONE;
  camera->debug_trace_off = FALSE;

  FIELD_CAMERA_ChangeMode( camera, camera->debug_save_camera_mode );
  if(camera->debug_save_camera_mode == FIELD_CAMERA_MODE_CALC_CAMERA_POS)
  {
    FIELD_CAMERA_BindDefaultTarget( camera );
  }

  // ワイプ表示破棄
  FLD_WIPEOBJ_Delete( camera->p_debug_wipe );
  camera->p_debug_wipe = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	操作しているターゲットを取得
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_GetBindSubScreenTarget( const FIELD_CAMERA * camera, VecFx32* p_target )
{
	*p_target = camera->debug_target;
}



//----------------------------------------------------------------------------
/**
 *	@brief  カメラデバック３D描画
 *
 *	@param	camera
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_Draw( const FIELD_CAMERA* camera)
{ 
  if(camera->p_debug_wipe)
  {
    FLD_WIPEOBJ_Main( camera->p_debug_wipe, fldWipeScale, 20 );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  デバックコントロール　情報管理
 */
//-----------------------------------------------------------------------------
static void DEBUG_ControlWork( FIELD_CAMERA* camera )
{
  // 現状のfield_cameraを維持したまま、カメラの平行移動などを行うために、
  // 実際に、gflibのカメラに渡したいカメラ座標から、
  // FIELD_CAMERA_MODE_CALC_CAMERA_POS用のアングルや、
  // ターゲットオフセットを割り出す。
  
  // アングルからデバックカメラ座標を計算
  FIELD_CAMERA_LOCAL_calcAnglePos( &camera->debug_target, &camera->debug_camera, camera->debug_target_yaw, camera->debug_target_pitch, camera->debug_target_len );
  
  // デバックターゲット
  // デバックカメラの位置から、
  // ターゲットオフセットと
  // アングルを求める
  {
    VecFx32 move_camera;
    
    // ターゲットオフセット
    if( camera->watch_target != NULL )
    {
      VEC_Subtract( &camera->debug_target, &camera->target_write, &camera->target_offset );
    }
    else
    {
      
#ifdef DEBUG_CAMERA_ALLCONTROL_MODE_2
      VEC_Subtract( &camera->debug_target, &camera->target_offset, &camera->target );
#else
      VEC_Set( &camera->target, camera->debug_target.x, camera->debug_target.y, camera->debug_target.z );
      VEC_Set( &camera->target_offset, 0,0,0 );
#endif
    }

    // カメラアングル
    VEC_Subtract( &camera->debug_camera, &camera->target_write, &move_camera ); 
    FIELD_CAMERA_LOCAL_modeChange_CalcVecAngel( &move_camera, &camera->angle_pitch, &camera->angle_yaw, &camera->angle_len );
  }
  
  // カメラ情報を設定
  FIELD_CAMERA_LOCAL_ControlParameter( camera, 0 );
}




//-----------------------------------------------------------------------------
/**
 *
 */
//-----------------------------------------------------------------------------
#define CAMERA_DEBUG_ANGLE_MOVE  (64)
#define CAMERA_DEBUG_PAERS_MOVE  (64)
#define CAMERA_DEBUG_NEARFAR_MOVE  (FX32_CONST(2))
#define CAMERA_DEBUG_POS_MOVE  (FX32_ONE)
#define CAMERA_DEBUG_LEN_MOVE  (FX32_ONE)
enum
{
  CAMERA_DEBUG_BUFFER_MODE_ZBUFF_MANUAL,
  CAMERA_DEBUG_BUFFER_MODE_WBUFF_MANUAL,
  CAMERA_DEBUG_BUFFER_MODE_ZBUFF_AUTO,
  CAMERA_DEBUG_BUFFER_MODE_WBUFF_AUTO,

  CAMERA_DEBUG_BUFFER_MODE_MAX,
};
//----------------------------------------------------------------------------
/**
 *	@brief  デバックコントロール初期化
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_InitControl( FIELD_CAMERA* camera, HEAPID heapID )
{
  GF_ASSERT( !camera->p_debug_wordset );
  GF_ASSERT( !camera->p_debug_msgdata );

  // trace OFF
  camera->debug_trace_off = TRUE;

  // ワードセット作成
  camera->p_debug_wordset = WORDSET_Create( heapID );
  camera->p_debug_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_tomoya_dat, heapID );

  camera->p_debug_strbuff   = GFL_STR_CreateBuffer( 512, heapID );
  camera->p_debug_strbuff_tmp = GFL_STR_CreateBuffer( 512, heapID );

  // フォントデータ
  camera->p_debug_font = GFL_FONT_Create(
    ARCID_FONT, NARC_font_large_gftr,
    GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

  GFL_UI_KEY_SetRepeatSpeed( 1,8 );

  // 今の設定カメラパラメータを取得
  {
    VecFx32 move_camera;
    GFL_G3D_CAMERA_GetTarget( camera->g3Dcamera, &camera->debug_target );
    GFL_G3D_CAMERA_GetPos( camera->g3Dcamera, &camera->debug_camera );

    // アングルを求る
    VEC_Subtract( &camera->debug_camera, &camera->debug_target, &move_camera );
    FIELD_CAMERA_LOCAL_modeChange_CalcVecAngel( &move_camera, &camera->debug_target_pitch, &camera->debug_target_yaw, &camera->debug_target_len );
  }
  
  DEBUG_ControlWork( camera );

}

//----------------------------------------------------------------------------
/**
 *	@brief  デバックコントロール破棄
 *
 *	@param	camera 
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_ExitControl( FIELD_CAMERA* camera )
{
  // フォントデータ
  GFL_FONT_Delete( camera->p_debug_font );
  camera->p_debug_font = NULL;


  GFL_MSG_Delete( camera->p_debug_msgdata );
  camera->p_debug_msgdata = NULL;

  WORDSET_Delete( camera->p_debug_wordset );
  camera->p_debug_wordset = NULL;

  GFL_STR_DeleteBuffer( camera->p_debug_strbuff );
  camera->p_debug_strbuff = NULL;
  GFL_STR_DeleteBuffer( camera->p_debug_strbuff_tmp );
  camera->p_debug_strbuff_tmp = NULL;

  GFL_UI_KEY_SetRepeatSpeed( 8,15 );

  // trace ON
  camera->debug_trace_off = FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  アングル動作の有無チェック
 *
 *	@param	pitch 
 *
 *	@retval TRUE    ＯＫ
 *	@retval FALSE   ＮＧ
 */
//-----------------------------------------------------------------------------
static BOOL DEBUG_is_PitchAngleMove( u16 pitch )
{
  if( (((pitch - CAMERA_DEBUG_ANGLE_MOVE) > GFL_CALC_GET_ROTA_NUM(270)) && ((pitch - CAMERA_DEBUG_ANGLE_MOVE) < 0x10000)) ||
      (((pitch - CAMERA_DEBUG_ANGLE_MOVE) >= GFL_CALC_GET_ROTA_NUM(0)) && ((pitch - CAMERA_DEBUG_ANGLE_MOVE) < GFL_CALC_GET_ROTA_NUM(90))) )
  {
    return TRUE;
  }
  return FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  フィールドカメラ  コントロール詳細
 *
 *	@param	camera      カメラ  
 *	@param	trg         キー情報
 *	@param	cont        
 *	@param	repeat      
 *
 *  NONE：ターゲット、カメラ平行移動
 *	B：カメラ公転（ターゲット座標が変わる）
 *	Y：カメラ自転（カメラ座標が変わる）
 *	A：パース操作
 *	X：ターゲットカメラバインドのON・OFF
 *	
 *	START：バッファリングモード変更
 */
//-----------------------------------------------------------------------------
BOOL FIELD_CAMERA_DEBUG_Control( FIELD_CAMERA* camera, int trg, int cont, int repeat )
{
  BOOL ret = FALSE;
  
  FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

  // カメラ公転（ターゲット座標変更　カメラオフセットを動かす）
  if( cont & PAD_BUTTON_B )
  {
    u16 pitch, yaw;

    // ターゲットを動かすためのアングルに変換
    pitch = camera->debug_target_pitch;
    yaw   = camera->debug_target_yaw;
     
    if( repeat & PAD_KEY_UP )
    {
      if( DEBUG_is_PitchAngleMove(pitch - CAMERA_DEBUG_ANGLE_MOVE) )
      {
        pitch -= CAMERA_DEBUG_ANGLE_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      if( DEBUG_is_PitchAngleMove(pitch + CAMERA_DEBUG_ANGLE_MOVE) )
      {
        pitch += CAMERA_DEBUG_ANGLE_MOVE;
        ret = TRUE;
      }
    }

    if( repeat & PAD_KEY_LEFT )
    {
      yaw += CAMERA_DEBUG_ANGLE_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_KEY_RIGHT )
    {
      yaw -= CAMERA_DEBUG_ANGLE_MOVE;
      ret = TRUE;
    }

    if( repeat & PAD_BUTTON_L )
    {
      camera->debug_target_len -= CAMERA_DEBUG_LEN_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_BUTTON_R )
    {
      camera->debug_target_len += CAMERA_DEBUG_LEN_MOVE;
      ret = TRUE;
    }


    // 変更地を保存
    camera->debug_target_pitch  = pitch;
    camera->debug_target_yaw    = yaw;

    // 変更座標を求める
    if( ret )
    {
      pitch += 0x8000;  // 方向を反転
      yaw   += 0x8000;
      
      // ターゲットの座標を求める
      FIELD_CAMERA_LOCAL_calcAnglePos( &camera->debug_camera, &camera->debug_target, yaw, pitch, camera->debug_target_len );
    }
  }
  // カメラ自転（カメラ座標変更）
  else if( cont & PAD_BUTTON_Y )
  {

    if( repeat & PAD_KEY_UP )
    {
      if( DEBUG_is_PitchAngleMove(camera->debug_target_pitch + CAMERA_DEBUG_ANGLE_MOVE) )
      {
        camera->debug_target_pitch += CAMERA_DEBUG_ANGLE_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      if( DEBUG_is_PitchAngleMove(camera->angle_pitch - CAMERA_DEBUG_ANGLE_MOVE) )
      {
        camera->debug_target_pitch -= CAMERA_DEBUG_ANGLE_MOVE;
        ret = TRUE;
      }
    }

    if( repeat & PAD_KEY_LEFT )
    {
      camera->debug_target_yaw -= CAMERA_DEBUG_ANGLE_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_KEY_RIGHT )
    {
      camera->debug_target_yaw += CAMERA_DEBUG_ANGLE_MOVE;
      ret = TRUE;
    }

  }
  // パース操作
  else if( cont & PAD_BUTTON_A )
  {
    if( repeat & PAD_KEY_UP )
    {
      if( (camera->fovy + CAMERA_DEBUG_PAERS_MOVE) < GFL_CALC_GET_ROTA_NUM(90) )
      {
        camera->fovy += CAMERA_DEBUG_PAERS_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      if( (camera->fovy - CAMERA_DEBUG_PAERS_MOVE) > 0 )
      {
        camera->fovy -= CAMERA_DEBUG_PAERS_MOVE;
        ret = TRUE;
      }
    }
    // 左右では、ドット崩れのない座標を求める 
    else if( (repeat & PAD_KEY_LEFT) || (repeat & PAD_KEY_RIGHT) )
    {
      fx32 dist;

      if( repeat & PAD_KEY_LEFT )
      {
        if( (camera->fovy + CAMERA_DEBUG_PAERS_MOVE) < GFL_CALC_GET_ROTA_NUM(90) )
        {
          camera->fovy += CAMERA_DEBUG_PAERS_MOVE;
          ret = TRUE;
        }
      }
      else if( repeat & PAD_KEY_RIGHT )
      {  
        if( (camera->fovy - CAMERA_DEBUG_PAERS_MOVE) > 0 )
        {
          camera->fovy -= CAMERA_DEBUG_PAERS_MOVE;
          ret = TRUE;
        }
      }

      //カメラの中心にある2D画像が、崩れずに表示できる距離を求める
      //つぶれずに表示するためには、1Dot　＝　1Unitで計算すればよい
      //カメラが画面中心を見つめるとき、画面半分のサイズは、96Dot
      //表示させたい、カメラのパースを考え、96Dot　＝　96Unitになる距離にすればよい
      dist = FX_Div( FX_Mul( FX_CosIdx( camera->fovy ), FX_F32_TO_FX32(96) ),	
                 FX_SinIdx( camera->fovy ));					

      camera->debug_target_len = dist; 
    }

    if( ret )
    {
      GFL_G3D_CAMERA_SetfovySin( camera->g3Dcamera, FX_SinIdx( camera->fovy ) );
      GFL_G3D_CAMERA_SetfovyCos( camera->g3Dcamera, FX_CosIdx( camera->fovy ) );
    }
  }
  // Near Far操作
  // カメラエリア　ＯＮ・ＯＦＦ
  else if( cont & PAD_BUTTON_DEBUG )
  {
    fx32 near, far;

    GFL_G3D_CAMERA_GetNear(camera->g3Dcamera, &near);
    GFL_G3D_CAMERA_GetFar(camera->g3Dcamera, &far);

    if( repeat & PAD_KEY_UP )
    {
      if( (near + CAMERA_DEBUG_NEARFAR_MOVE) < far )
      {
        near += CAMERA_DEBUG_NEARFAR_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      if( (near - CAMERA_DEBUG_NEARFAR_MOVE) > 0 )
      {
        near -= CAMERA_DEBUG_NEARFAR_MOVE;
        ret = TRUE;
      }
    }

    if( repeat & PAD_KEY_LEFT )
    {
      if( (far - CAMERA_DEBUG_NEARFAR_MOVE) > near )
      {
        far -= CAMERA_DEBUG_NEARFAR_MOVE;
        ret = TRUE;
      }
    }
    else if( repeat & PAD_KEY_RIGHT )
    {
      far += CAMERA_DEBUG_NEARFAR_MOVE;
      ret = TRUE;
    }

    if( ret )
    {
      GFL_G3D_CAMERA_SetNear(camera->g3Dcamera, &near);
      GFL_G3D_CAMERA_SetFar(camera->g3Dcamera, &far);
    }

    if( trg & PAD_BUTTON_X ){
      if( camera->camera_area_active ){
        camera->camera_area_active = FALSE;
      }else{
        camera->camera_area_active = TRUE;
      }
      ret = TRUE;
    }
  }
  // 平行移動
  else 
  {
    // ターゲットオフセットを変更
    if( repeat & PAD_KEY_UP )
    {
      camera->debug_target.z -= CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_KEY_DOWN )
    {
      camera->debug_target.z += CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    if( repeat & PAD_KEY_LEFT )
    {
      camera->debug_target.x -= CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_KEY_RIGHT )
    {
      camera->debug_target.x += CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    if( repeat & PAD_BUTTON_L )
    {
      camera->debug_target.y -= CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }
    else if( repeat & PAD_BUTTON_R )
    {
      camera->debug_target.y += CAMERA_DEBUG_POS_MOVE;
      ret = TRUE;
    }

    //  ターゲットのバインドOn・Off
    if( trg & PAD_BUTTON_X )
    {
      if( camera->watch_target )
      {
        FIELD_CAMERA_FreeTarget( camera );
      }
      else
      {
        FIELD_CAMERA_BindDefaultTarget( camera );
      }
      ret = TRUE;
    }

  }
   

  // バッファリングモード変更
  if( trg & PAD_BUTTON_START )
  {
    camera->debug_save_buffer_mode = (camera->debug_save_buffer_mode + 1) % CAMERA_DEBUG_BUFFER_MODE_MAX;
    
    switch( camera->debug_save_buffer_mode )
    {
    case CAMERA_DEBUG_BUFFER_MODE_ZBUFF_MANUAL:
      GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
      ret = TRUE;
      break;
    case CAMERA_DEBUG_BUFFER_MODE_WBUFF_MANUAL:
      GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_W );
      ret = TRUE;
      break;
    case CAMERA_DEBUG_BUFFER_MODE_ZBUFF_AUTO:
      GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
      ret = TRUE;
      break;
    case CAMERA_DEBUG_BUFFER_MODE_WBUFF_AUTO:
      GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
      ret = TRUE;
      break;
    }
  }

  // 再表示
  if( trg & PAD_BUTTON_DEBUG )
  {
    ret = TRUE;
  }

  DEBUG_ControlWork( camera );

  return ret;
}


//----------------------------------------------------------------------------
/**
 *	@brief  カメラ情報を書き込む
 *
 *	@param	camera    カメラ
 *	@param	p_win     ウィンドウ
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_DrawInfo( FIELD_CAMERA* camera, GFL_BMPWIN* p_win, fx32 map_size_x, fx32 map_size_z )
{
  //クリア
  
  // カメラアングル
  {
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 0, camera->angle_pitch, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 1, camera->angle_yaw, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 2, FX_Whole(camera->angle_len), 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_ANGLE, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 0, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // 座標
  {
    VecFx32 camerapos, target;

    GFL_G3D_CAMERA_GetTarget( camera->g3Dcamera, &target );
    GFL_G3D_CAMERA_GetPos( camera->g3Dcamera, &camerapos );
    
    // カメラ
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 0, FX_Whole(camerapos.x), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 1, FX_Whole(camerapos.y), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 2, FX_Whole(camerapos.z), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 3, FX_Whole(target.x), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 4, FX_Whole(target.y), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 5, FX_Whole(target.z), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 32, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // ターゲットオフセット
  {
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 0, FX_Whole(camera->target_offset.x), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 1, FX_Whole(camera->target_offset.y), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 2, FX_Whole(camera->target_offset.z), 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_OFFSET, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 64, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // パース
  // Near Far
  {
    fx32 near, far;
    
    GFL_G3D_CAMERA_GetNear(camera->g3Dcamera, &near);
    GFL_G3D_CAMERA_GetFar(camera->g3Dcamera, &far);

    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 0, camera->fovy, 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 1, FX_Whole(near), 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterHexNumber( camera->p_debug_wordset, 2, FX_Whole(far), 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_PARS, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 96, camera->p_debug_strbuff, camera->p_debug_font );
  }
  

  // バッファリング
  {
    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_BUFF + camera->debug_save_buffer_mode, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 112, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // 主人公追尾
  {
    if( camera->watch_target )
    {
      GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_BIND, camera->p_debug_strbuff_tmp );
    }
    else
    {
      GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_BIND0002, camera->p_debug_strbuff_tmp );
    }

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 128, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // カメラエリアのＯＮ・ＯＦＦ
  {
    if( camera->camera_area_active )
    {
      GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_BIND0003, camera->p_debug_strbuff_tmp );
    }
    else
    {
      GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_BIND0004, camera->p_debug_strbuff_tmp );
    }

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 128, 128, camera->p_debug_strbuff, camera->p_debug_font );
  }

  // Maya Camera Anime Trans
  // 小数点　第2位まで表示
  {
    f32 f_x, f_y, f_z;
    s32 x, y, z;
    u16 rot_x, rot_y, rot_z;
    fx32 xz_dist;
    VecFx32 camerapos, target, xz_vec;

    GFL_G3D_CAMERA_GetTarget( camera->g3Dcamera, &target );
    GFL_G3D_CAMERA_GetPos( camera->g3Dcamera, &camerapos );

    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG )
    {
      fx32 half_x, half_z;
      half_x = FX_Div( map_size_x, 2<<FX32_SHIFT );
      half_z = FX_Div( map_size_z, 2<<FX32_SHIFT );
      OS_Printf( "\n--------------Maya Param-------------\nMaya Trans x[%f] y[%f] z[%f]\n", FX_FX32_TO_F32(camerapos.x - half_x), FX_FX32_TO_F32(camerapos.y), FX_FX32_TO_F32(camerapos.z - half_z) );
    }
    
    // カメラ座標を表示
    // 小数点以下無視
    x = FX_Whole( camerapos.x ) - (FX_Whole(map_size_x)/2);
    y = FX_Whole( camerapos.y );
    z = FX_Whole( camerapos.z ) - (FX_Whole(map_size_z)/2);

    f_x = FX_FX32_TO_F32(camerapos.x - FX_Floor(camerapos.x));
    f_y = FX_FX32_TO_F32(camerapos.y - FX_Floor(camerapos.y));
    f_z = FX_FX32_TO_F32(camerapos.z - FX_Floor(camerapos.z));

    /// 整数部
    WORDSET_RegisterNumber( camera->p_debug_wordset, 0, x, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 1, y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 2, z, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

    /// 小数部
    x = (s32)(f_x * 100.0f);
    y = (s32)(f_y * 100.0f);
    z = (s32)(f_z * 100.0f);
    WORDSET_RegisterNumber( camera->p_debug_wordset, 3, x, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 4, y, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 5, z, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_MAYA_TRANS, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 144, camera->p_debug_strbuff, camera->p_debug_font );

    // カメラからターゲットへのベクトルを生成
    VEC_Subtract( &target, &camerapos, &target );

    // XZ平面の距離を求める
    VEC_Set( &xz_vec, target.x, 0, target.z );
    xz_dist = VEC_Mag( &xz_vec );

    // 角度を求める
    // XZ平面の角度
    // 平面とY方向の角度
    rot_y = FX_Atan2Idx( -target.x, -target.z );
    rot_x = FX_Atan2Idx( target.y, xz_dist );
    rot_z = 0;

    f_x = (f32)(rot_x / (f32)0x10000) * 360.0f;
    f_y = (f32)(rot_y / (f32)0x10000) * 360.0f;

    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG )
    {
      OS_Printf( "Maya Rotate x[%f] y[%f]\n", f_x, f_y );
    }
 
    x = (s32)f_x;
    y = (s32)f_y;
    z = 0;
    
    /// 整数部
    WORDSET_RegisterNumber( camera->p_debug_wordset, 0, x, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 1, y, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 2, z, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

    /// 小数部
    f_x -= x;
    f_y -= y;
    x = (s32)(f_x * 100.0f);
    y = (s32)(f_y * 100.0f);
    WORDSET_RegisterNumber( camera->p_debug_wordset, 3, x, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 4, y, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    WORDSET_RegisterNumber( camera->p_debug_wordset, 5, z, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

    GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_MAYA_ROTATE, camera->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 160, camera->p_debug_strbuff, camera->p_debug_font );
  }

  GFL_BMPWIN_TransVramCharacter( p_win );
}


//----------------------------------------------------------------------------
/**
 *	@brief  カメラ操作方法を書き込む
 */
//-----------------------------------------------------------------------------
void FIELD_CAMERA_DEBUG_DrawControlHelp( FIELD_CAMERA* camera, GFL_BMPWIN* p_win )
{
  GFL_MSG_GetString( camera->p_debug_msgdata, CAMERA_DELICATE_HELP, camera->p_debug_strbuff_tmp );

  WORDSET_ExpandStr( camera->p_debug_wordset, camera->p_debug_strbuff, camera->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 0, 0, camera->p_debug_strbuff, camera->p_debug_font );

  GFL_BMPWIN_TransVramCharacter( p_win );
}



#endif  //PM_DEBUG
