//============================================================================================
/**
 * @file	camera_cont.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"

#include "camera_cont.h"
//============================================================================================
//
//
//	カメラコントロール
//
//
//============================================================================================
#define	MAINCAMERA_POFS_LENGTH	(40)
#define	MAINCAMERA_POFS_HEIGHT	(24)
#define	MAINCAMERA_TOFS_LENGTH	(0)
#define	MAINCAMERA_TOFS_HEIGHT	(16)

#define	VIEWCAMERA_POFS_LENGTH	(24)
#define	VIEWCAMERA_POFS_HEIGHT	(24)
#define	VIEWCAMERA_TOFS_LENGTH	(0)
#define	VIEWCAMERA_TOFS_HEIGHT	(10)

#define	VIEWCAMERA_AUTO_ROTATEVALUE	(0x0080)
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
struct _CAMERA_CONTROL {
	GAME_SYSTEM*			gs;
	int						targetObj;

	u16						seq;
	u16						mainCameraDirection;
	u16						autoCameraDirection;
	HEAPID					heapID;
};

//------------------------------------------------------------------
/**
 * @brief	カメラコントロールセット
 */
//------------------------------------------------------------------
CAMERA_CONTROL* AddCameraControl( GAME_SYSTEM* gs, int targetObj, HEAPID heapID )
{
	CAMERA_CONTROL* cc = GFL_HEAP_AllocClearMemory( heapID, sizeof(CAMERA_CONTROL) );
	cc->heapID = heapID;
	cc->gs = gs;
	cc->targetObj = targetObj;
	cc->mainCameraDirection = 0;
	cc->autoCameraDirection = 0;

	return cc;
}

//------------------------------------------------------------------
/**
 * @brief	カメラコントロール終了
 */
//------------------------------------------------------------------
void RemoveCameraControl( CAMERA_CONTROL* cc )
{
	GFL_HEAP_FreeMemory( cc ); 
}

//------------------------------------------------------------------
/**
 * @brief	カメラ方向の取得と設定
 */
//------------------------------------------------------------------
void GetCameraControlDirection( CAMERA_CONTROL* cc, u16* value )
{
	*value = cc->mainCameraDirection;
}

void SetCameraControlDirection( CAMERA_CONTROL* cc, u16* value )
{
	cc->mainCameraDirection = *value;
}

//------------------------------------------------------------------
/**
 * @brief	カメラコントロールメイン
 */
//------------------------------------------------------------------
void MainCameraControl( CAMERA_CONTROL* cc )
{
	GFL_G3D_CAMERA* g3Dcamera;
	VecFx32 targetTrans, targetRotate;
	VecFx32 cameraPos, cameraTarget;

	Get3DobjTrans( Get_GS_SceneObj( cc->gs ), cc->targetObj, &targetTrans );
	Get3DobjRotate( Get_GS_SceneObj( cc->gs ), cc->targetObj, &targetRotate );

	//下カメラ制御
	cameraPos.x = targetTrans.x + MAINCAMERA_POFS_LENGTH * FX_SinIdx(cc->mainCameraDirection);
	cameraPos.y = targetTrans.y + MAINCAMERA_POFS_HEIGHT * FX32_ONE;
	cameraPos.z = targetTrans.z + MAINCAMERA_POFS_LENGTH * FX_CosIdx(cc->mainCameraDirection);

	cameraTarget.x = targetTrans.x + MAINCAMERA_TOFS_LENGTH * FX_SinIdx(cc->mainCameraDirection);
	cameraTarget.y = targetTrans.y + MAINCAMERA_TOFS_HEIGHT * FX32_ONE;
	cameraTarget.z = targetTrans.z + MAINCAMERA_TOFS_LENGTH * FX_CosIdx(cc->mainCameraDirection);

	g3Dcamera = Get_GS_G3Dcamera( cc->gs, MAINCAMERA_ID );
	GFL_G3D_CAMERA_SetPos( g3Dcamera, &cameraPos );
	GFL_G3D_CAMERA_SetTarget( g3Dcamera, &cameraTarget );

	//上カメラ制御
	cameraPos.x = targetTrans.x + VIEWCAMERA_POFS_LENGTH * FX_SinIdx(cc->autoCameraDirection);
	cameraPos.y = targetTrans.y + VIEWCAMERA_POFS_HEIGHT * FX32_ONE;
	cameraPos.z = targetTrans.z + VIEWCAMERA_POFS_LENGTH * FX_CosIdx(cc->autoCameraDirection);

	cameraTarget.x = targetTrans.x + VIEWCAMERA_TOFS_LENGTH * FX_SinIdx(cc->autoCameraDirection);
	cameraTarget.y = targetTrans.y + VIEWCAMERA_TOFS_HEIGHT * FX32_ONE;
	cameraTarget.z = targetTrans.z + VIEWCAMERA_TOFS_LENGTH * FX_CosIdx(cc->autoCameraDirection);

	g3Dcamera = Get_GS_G3Dcamera( cc->gs, SUBCAMERA_ID );
	GFL_G3D_CAMERA_SetPos( g3Dcamera, &cameraPos );
	GFL_G3D_CAMERA_SetTarget( g3Dcamera, &cameraTarget );

	cc->autoCameraDirection += VIEWCAMERA_AUTO_ROTATEVALUE;
}



