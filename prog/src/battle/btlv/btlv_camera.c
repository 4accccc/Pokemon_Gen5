
//============================================================================================
/**
 * @file	btlv_camera.c
 * @brief	戦闘画面カメラ制御
 * @author	soga
 * @date	2008.11.20
 */
//============================================================================================

#include <gflib.h>

#include "btlv_effect.h"

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================

#define	PHI_MAX				( ( 65536 / 360 ) * 89 )

#define	CAMERA_POS_MOVE_FLAG		( 0x00000001 )
#define	CAMERA_TARGET_MOVE_FLAG	( 0x00000002 )
#define	CAMERA_SHAKE_FLAG       ( 0x00000004 )

#define	CAMERA_POS_MOVE_FLAG_OFF	  ( 0x00000001 ^ 0xffffffff )
#define	CAMERA_TARGET_MOVE_FLAG_OFF	( 0x00000002 ^ 0xffffffff )
#define	CAMERA_SHAKE_FLAG_OFF       ( 0x00000004 ^ 0xffffffff )

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

struct _BTLV_CAMERA_WORK
{
	GFL_TCBSYS			  *tcb_sys;
	GFL_G3D_CAMERA	  *camera;
	int							  phi;
	int							  theta;
	fx32						  radius;
	int							  move_flag;
	VecFx32					  now_pos;
	VecFx32					  now_target;
	VecFx32					  move_pos;
	VecFx32					  move_target;
	VecFx32					  vec_pos;
	VecFx32					  vec_target;
	VecFx32					  shake_ofs;
	int							  wait;
	int							  wait_tmp;
	int							  brake_frame;
  EFFTOOL_MOVE_WORK emw;
	HEAPID					  heapID;
};

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================
static	void		BTLV_CAMERA_Move( BTLV_CAMERA_WORK *bcw );
static	void		BTLV_CAMERA_UpdateCameraAngle( BTLV_CAMERA_WORK *bcw );

//============================================================================================
/**
 *	カメラ初期位置
 */
//============================================================================================
static const VecFx32 cam_pos	= { FX_F32_TO_FX32( 6.7f ), FX_F32_TO_FX32( 6.7f ), FX_F32_TO_FX32( 17.3f ) };
static const VecFx32 cam_target = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 2.6f ), FX_F32_TO_FX32( 0.0f ) };
static const VecFx32 cam_up		= { 0,						FX32_ONE,				0 };

//============================================================================================
/**
 *	システム初期化
 *
 * @param[in]	tcb_sys	システム内で使用するTCBSYS構造体へのポインタ
 * @param[in]	heapID	ヒープID
 */
//============================================================================================
BTLV_CAMERA_WORK	*BTLV_CAMERA_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID )
{
	BTLV_CAMERA_WORK *bcw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_CAMERA_WORK ) );

	bcw->heapID = heapID;
	bcw->tcb_sys = tcb_sys;

	bcw->camera = GFL_G3D_CAMERA_Create( GFL_G3D_PRJPERS, 
										 FX32_SIN13,
										 FX32_COS13,
										 FX_F32_TO_FX32( 1.35f ),
										 NULL,
										 FX32_ONE,
										 FX32_ONE * 512, //FX32_ONE * 256,
										 NULL,
										 &cam_pos,
										 &cam_up,
										 &cam_target,
										 heapID );

	BTLV_CAMERA_UpdateCameraAngle( bcw );

	GFL_G3D_CAMERA_Switching( bcw->camera );

	BTLV_CAMERA_GetDefaultCameraPosition( &bcw->now_pos, &bcw->now_target );

	return bcw;
}

//============================================================================================
/**
 *	システム終了
 *
 * @param[in]	bcw	BTLV_CAMERA管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_CAMERA_Exit( BTLV_CAMERA_WORK *bcw )
{
	GFL_G3D_CAMERA_Delete( bcw->camera );
	GFL_HEAP_FreeMemory( bcw );
}

//============================================================================================
/**
 *	システムメイン
 *
 * @param[in]	bcw	BTLV_CAMERA管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_CAMERA_Main( BTLV_CAMERA_WORK *bcw )
{
	BTLV_CAMERA_Move( bcw );
}

//============================================================================================
/**
 *	カメラ移動（位置、ターゲット指定）
 *
 * @param[in]	bcw		BTLV_CAMERA管理ワークへのポインタ
 * @param[in]	pos		移動先カメラ位置
 * @param[in]	target	移動先カメラターゲット
 */
//============================================================================================
void	BTLV_CAMERA_MoveCameraPosition( BTLV_CAMERA_WORK *bcw, VecFx32 *pos, VecFx32 *target )
{
	if( pos != NULL ){
    bcw->now_pos.x = pos->x;
    bcw->now_pos.y = pos->y;
    bcw->now_pos.z = pos->z;
		GFL_G3D_CAMERA_SetPos( bcw->camera, pos );
	}
	if( target != NULL ){
    bcw->now_target.x = target->x;
    bcw->now_target.y = target->y;
    bcw->now_target.z = target->z;
		GFL_G3D_CAMERA_SetTarget( bcw->camera, target );
	}
	GFL_G3D_CAMERA_Switching( bcw->camera );
	BTLV_CAMERA_UpdateCameraAngle( bcw );
}

//============================================================================================
/**
 *	カメラ移動（角度指定）
 *
 * @param[in]	bcw		BTLV_CAMERA管理ワークへのポインタ
 * @param[in]	phi		X軸方向の移動量
 * @param[in]	theta	Y軸方向の移動量
 */
//============================================================================================
void	BTLV_CAMERA_MoveCameraAngle( BTLV_CAMERA_WORK *bcw, int phi, int theta )
{
	BTLV_CAMERA_GetCameraPositionAngle( bcw, phi, theta, &bcw->now_pos, &bcw->now_target );

	GFL_G3D_CAMERA_SetPos( bcw->camera, &bcw->now_pos );
	GFL_G3D_CAMERA_Switching( bcw->camera );
}

//============================================================================================
/**
 *	カメラ移動（移動先を指定して、間を補間する）
 *
 * @param[in]	bcw		BTLV_CAMERA管理ワークへのポインタ
 * @param[in]	pos		移動先カメラ位置
 * @param[in]	target	移動先カメラターゲット
 * @param[in]	flame	移動フレーム数
 * @param[in]	wait	移動ウエイト
 * @param[in]	brake	移動にブレーキをかけるフレーム数
 */
//============================================================================================
void	BTLV_CAMERA_MoveCameraInterpolation( BTLV_CAMERA_WORK *bcw, VecFx32 *pos, VecFx32 *target, int flame, int wait, int brake )
{
	bcw->brake_frame = brake;
	bcw->wait = wait;
	bcw->wait_tmp = wait;

	if( pos != NULL ){
		bcw->move_pos.x = pos->x;
		bcw->move_pos.y = pos->y;
		bcw->move_pos.z = pos->z;
		BTLV_EFFTOOL_CalcMoveVector( &bcw->now_pos, pos, &bcw->vec_pos, FX32_CONST( flame ) );
		bcw->move_flag |= CAMERA_POS_MOVE_FLAG;
	}
	if( target != NULL ){
		bcw->move_target.x = target->x;
		bcw->move_target.y = target->y;
		bcw->move_target.z = target->z;
		BTLV_EFFTOOL_CalcMoveVector( &bcw->now_target, target, &bcw->vec_target, FX32_CONST( flame ) );
		bcw->move_flag |= CAMERA_TARGET_MOVE_FLAG;
	}
}

//============================================================================================
/**
 *	カメラゆれ
 *
 * @param[in]	bcw     BTLV_CAMERA管理ワークへのポインタ
 * @param[in] dir     ゆれ方向
 * @param[in] value   ゆれ幅
 * @param[in] offset	ゆれ幅オフセット
 * @param[in] frame   １ゆれにかかるフレーム
 * @param[in] wait    １フレーム毎のウエイト
 * @param[in] count   ゆれ回数
 */
//============================================================================================
void  BTLV_CAMERA_Shake( BTLV_CAMERA_WORK *bcw, int dir, fx32 value, fx32 offset, int frame, int wait, int count )
{ 
  bcw->emw.move_type     = EFFTOOL_CALCTYPE_ROUNDTRIP_LONG;
  bcw->emw.vec_time      = frame;
  bcw->emw.vec_time_tmp  = frame;
  bcw->emw.wait          = 0;
  bcw->emw.wait_tmp      = wait;
  bcw->emw.count         = count * 4;
  bcw->emw.start_value.x = 0;
  bcw->emw.start_value.y = 0;
  bcw->emw.start_value.z = 0;

  if( dir == BTLEFF_CAMERA_SHAKE_HORIZON )
  { 
    bcw->emw.end_value.x   = value;
    bcw->emw.end_value.y   = 0;

    bcw->emw.vector.x = FX_Div( value, FX32_CONST( frame ) );
    bcw->emw.vector.y = 0;
  }
  else
  { 
    bcw->emw.end_value.x   = 0;
    bcw->emw.end_value.y   = value;

    bcw->emw.vector.x = 0;
    bcw->emw.vector.y = FX_Div( value, FX32_CONST( frame ) );
  }

  bcw->emw.end_value.z   = 0;
  bcw->emw.vector.z = 0;

	bcw->move_flag |= CAMERA_SHAKE_FLAG;
}

//============================================================================================
/**
 *	カメラ位置を取得
 *
 * @param[in]	bcw		BTLV_CAMERA管理ワークへのポインタ
 * @param[in]	pos		取得したカメラ位置を格納するワークへのポインタ
 * @param[in]	target	取得したカメラターゲットを格納するワークへのポインタ
 */
//============================================================================================
void	BTLV_CAMERA_GetCameraPosition( BTLV_CAMERA_WORK *bcw, VecFx32 *pos, VecFx32 *target )
{
	GFL_G3D_CAMERA_GetPos( bcw->camera, pos );
	GFL_G3D_CAMERA_GetTarget( bcw->camera, target );
}

//============================================================================================
/**
 *	指定された角度のカメラ位置を取得
 *
 * @param[in]		bcw			BTLV_CAMERA管理ワークへのポインタ
 * @param[in]		phi			取得したいカメラ角度
 * @param[in]		theta		取得したいカメラ角度
 * @param[out]	pos			取得したカメラ位置を格納するワークへのポインタ
 * @param[out]	target	取得したカメラターゲットを格納するワークへのポインタ
 */
//============================================================================================
void	BTLV_CAMERA_GetCameraPositionAngle( BTLV_CAMERA_WORK *bcw, int phi, int theta, VecFx32 *pos, VecFx32 *target )
{
	GFL_G3D_CAMERA_GetTarget( bcw->camera, target );

	bcw->phi += phi;
	bcw->theta += theta;

//	if( now_phi <= -PHI_MAX ){
//		now_phi = -PHI_MAX;
//	}
//	if( now_phi >= PHI_MAX ){
//		now_phi = PHI_MAX;
//	}

	//phiとthetaとscaleからcamPosを計算
	pos->x = FX_MUL( FX_CosIdx( bcw->theta ), FX_CosIdx( bcw->phi ) );
	pos->y = FX_SinIdx( bcw->phi );
	pos->z = FX_MUL( FX_SinIdx( bcw->theta ), FX_CosIdx( bcw->phi ) );
	pos->x = FX_MUL( pos->x, bcw->radius );
	pos->y = FX_MUL( pos->y, bcw->radius );
	pos->z = FX_MUL( pos->z, bcw->radius );

	pos->x += target->x;
	pos->y += target->y;
	pos->z += target->z;
}

//============================================================================================
/**
 *	カメラのデフォルト位置を取得
 *
 * @param[in]	pos		取得したカメラ位置を格納するワークへのポインタ
 * @param[in]	target	取得したカメラターゲットを格納するワークへのポインタ
 */
//============================================================================================
void	BTLV_CAMERA_GetDefaultCameraPosition( VecFx32 *pos, VecFx32 *target )
{
	pos->x = cam_pos.x;
	pos->y = cam_pos.y;
	pos->z = cam_pos.z;

	target->x = cam_target.x;
	target->y = cam_target.y;
	target->z = cam_target.z;
}

//============================================================================================
/**
 *	カメラ移動が実行されているかチェック
 *
 * @param[in]	bcw		BTLV_CAMERA管理ワークへのポインタ
 *
 * @retval	FALSE:移動していない　TRUE:移動中
 */
//============================================================================================
BOOL	BTLV_CAMERA_CheckExecute( BTLV_CAMERA_WORK *bcw )
{
	return ( bcw->move_flag != 0 );
}

//============================================================================================
/**
 *	カメラ移動処理
 *
 * @param[in]	bcw		BTLV_CAMERA管理ワークへのポインタ
 */
//============================================================================================
static	void	BTLV_CAMERA_Move( BTLV_CAMERA_WORK *bcw )
{
	BOOL ret = TRUE;
	VecFx32	pos, target;

	if( bcw->move_flag == 0 ) return;

	if( bcw->wait == 0 )
  {
		bcw->wait = bcw->wait_tmp;
		if( bcw->brake_frame )
    {
			bcw->brake_frame--;
			if( bcw->brake_frame == 0 )
      {
				bcw->vec_pos.x = bcw->vec_pos.x >> 1;
				bcw->vec_pos.y = bcw->vec_pos.y >> 1;
				bcw->vec_pos.z = bcw->vec_pos.z >> 1;
				bcw->vec_target.x = bcw->vec_target.x >> 1;
				bcw->vec_target.y = bcw->vec_target.y >> 1;
				bcw->vec_target.z = bcw->vec_target.z >> 1;
			}
		}

		if( bcw->move_flag & CAMERA_POS_MOVE_FLAG )
    {
			BTLV_EFFTOOL_CheckMove( &bcw->now_pos.x, &bcw->vec_pos.x, &bcw->move_pos.x, &ret );
			BTLV_EFFTOOL_CheckMove( &bcw->now_pos.y, &bcw->vec_pos.y, &bcw->move_pos.y, &ret );
			BTLV_EFFTOOL_CheckMove( &bcw->now_pos.z, &bcw->vec_pos.z, &bcw->move_pos.z, &ret );
			if( ret == TRUE )
      {
				bcw->move_flag &= CAMERA_POS_MOVE_FLAG_OFF;
			}
		}
		if( bcw->move_flag & CAMERA_TARGET_MOVE_FLAG )
    {
			BTLV_EFFTOOL_CheckMove( &bcw->now_target.x, &bcw->vec_target.x, &bcw->move_target.x, &ret );
			BTLV_EFFTOOL_CheckMove( &bcw->now_target.y, &bcw->vec_target.y, &bcw->move_target.y, &ret );
			BTLV_EFFTOOL_CheckMove( &bcw->now_target.z, &bcw->vec_target.z, &bcw->move_target.z, &ret );
			if( ret == TRUE )
      {
				bcw->move_flag &= CAMERA_TARGET_MOVE_FLAG_OFF;
			}
		}
		if( bcw->move_flag & CAMERA_SHAKE_FLAG )
    { 
      ret = BTLV_EFFTOOL_CalcParam( &bcw->emw, &bcw->shake_ofs );
      if( ret == TRUE ){
				bcw->move_flag &= CAMERA_SHAKE_FLAG_OFF;
      }
    }
    pos.x = bcw->now_pos.x + bcw->shake_ofs.x;
    pos.y = bcw->now_pos.y + bcw->shake_ofs.y;
    pos.z = bcw->now_pos.z + bcw->shake_ofs.z;
    target.x = bcw->now_target.x + bcw->shake_ofs.x;
    target.y = bcw->now_target.y + bcw->shake_ofs.y;
    target.z = bcw->now_target.z + bcw->shake_ofs.z;
		GFL_G3D_CAMERA_SetPos( bcw->camera, &pos );
		GFL_G3D_CAMERA_SetTarget( bcw->camera, &target );
		GFL_G3D_CAMERA_Switching( bcw->camera );
		BTLV_CAMERA_UpdateCameraAngle( bcw );
	}
	else
  {
		bcw->wait--;
	}
}

//============================================================================================
/**
 *	現在のカメラ位置とターゲット位置からカメラ角度を計算して更新
 *
 * @param[in]	bcw		BTLV_CAMERA管理ワークへのポインタ
 */
//============================================================================================
static	void	BTLV_CAMERA_UpdateCameraAngle( BTLV_CAMERA_WORK *bcw )
{
	VecFx32	pos, target;

	GFL_G3D_CAMERA_GetPos( bcw->camera, &pos );
	GFL_G3D_CAMERA_GetTarget( bcw->camera, &target );

	pos.x -= target.x;
	pos.y -= target.y;
	pos.z -= target.z;

	bcw->phi    = FX_Atan2Idx( pos.y, pos.z );
	bcw->theta  = FX_Atan2Idx( pos.z, pos.x );
	bcw->radius = VEC_Mag( &pos );
}

//カメラのポジションから、φ、θを求める式（参考のためにコメントで残しておく）
#if 0
	cw->phi		= FX_Atan2Idx( cam_pos.y, cam_pos.z );
	cw->theta	= FX_Atan2Idx( cam_pos.y, cam_pos.x );
	cw->scale	= VEC_Mag( &cam_pos );
//======================================================================
//	カメラの制御
//======================================================================
static	void	MoveCamera( SOGA_WORK *wk )
{
	CAMERA_WORK	*cw = &wk->cw;
	int pad = GFL_UI_KEY_GetCont();
	VecFx32	pos,ofsx,ofsz;
	fx32	xofs=0,yofs=0,zofs=0;		

	if( pad & PAD_BUTTON_Y ){
		if( pad & PAD_KEY_LEFT ){
			cw->theta -= ROTATE_SPEED;
		}
		if( pad & PAD_KEY_RIGHT ){
			cw->theta += ROTATE_SPEED;
		}
		if( pad & PAD_KEY_UP ){
			cw->phi -= ROTATE_SPEED;
			if( cw->phi <= -PHI_MAX ){
				cw->phi = -PHI_MAX;
			}
		}
		if( pad & PAD_KEY_DOWN ){
			cw->phi += ROTATE_SPEED;
			if( cw->phi >= PHI_MAX ){
				cw->phi = PHI_MAX;
			}
		}
	}
	else{
		if( pad & PAD_KEY_LEFT ){
			xofs = -MOVE_SPEED;
		}
		if( pad & PAD_KEY_RIGHT ){
			xofs = MOVE_SPEED;
		}
		if( pad & PAD_KEY_UP ){
			yofs = MOVE_SPEED;
		}
		if( pad & PAD_KEY_DOWN ){
			yofs = -MOVE_SPEED;
		}
		if( pad & PAD_BUTTON_X ){
			zofs = MOVE_SPEED;
		}
		if( pad & PAD_BUTTON_B ){
			zofs = -MOVE_SPEED;
		}
		if( wk->mw.pos_x ){
			xofs = MOVE_SPEED * wk->mw.pos_x;
			wk->mw.pos_x = 0;
		}
		if( wk->mw.pos_y ){
			yofs = MOVE_SPEED * wk->mw.pos_y;
			wk->mw.pos_y = 0;
		}
		if( wk->mw.distance ){
			zofs = MOVE_SPEED * (wk->mw.distance / 50);
			wk->mw.distance = 0;
		}
		pos.x = cw->camPos.x - cw->target.x;
		pos.y = 0;
		pos.z = cw->camPos.z - cw->target.z;
		VEC_Normalize( &pos, &pos );

		ofsx.x = FX_MUL( pos.z, xofs );
		ofsx.y = 0;
		ofsx.z = -FX_MUL( pos.x, xofs );

		ofsz.x = -FX_MUL( pos.x, zofs );
		ofsz.y = yofs;
		ofsz.z = -FX_MUL( pos.z, zofs );

		cw->target.x += ofsx.x + ofsz.x;
		cw->target.y += ofsx.y + ofsz.y;
		cw->target.z += ofsx.z + ofsz.z;
	}
	if( pad & PAD_BUTTON_L ){
		if( cw->scale > 0 ){
			cw->scale -= SCALE_SPEED;
		}
	}
	if( pad & PAD_BUTTON_R ){
		if( cw->scale < SCALE_MAX ){
			cw->scale += SCALE_SPEED;
		}
	}
	if( wk->mw.theta ){
		cw->theta -= wk->mw.theta * 32 ;
		wk->mw.theta = 0;
	}

	if( wk->mw.phi ){
		cw->phi -= wk->mw.phi * 32 ;
		if( cw->phi <= -PHI_MAX ){
			cw->phi = -PHI_MAX;
		}
		if( cw->phi >= PHI_MAX ){
			cw->phi = PHI_MAX;
		}
		wk->mw.phi = 0;
	}

	CalcCamera( cw );
}

//======================================================================
//	カメラ位置の計算
//======================================================================
static	void	CalcCamera( CAMERA_WORK *cw )
{
	MtxFx43	scale,trans;
	VecFx32	pos;

	//phiとthetaとscaleからcamPosを計算
	pos.x = FX_MUL( FX_CosIdx( cw->theta & 0xffff ), FX_CosIdx( cw->phi & 0xffff ) );
	pos.y = FX_SinIdx( cw->phi & 0xffff );
	pos.z = FX_MUL( FX_SinIdx( cw->theta & 0xffff ), FX_CosIdx( cw->phi & 0xffff ) );
	MTX_Scale43( &scale, cw->scale, cw->scale, cw->scale );
	MTX_MultVec43( &pos, &scale, &pos );
	MTX_Identity43( &trans );
//	MTX_TransApply43( &trans, &trans, cw->target.x,	cw->target.y, cw->target.z );
	MTX_MultVec43( &pos, &trans, &cw->camPos );
}
#endif
