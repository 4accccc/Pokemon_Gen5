//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _CAMERA_CONTROL	CAMERA_CONTROL;

// カメラコントロールセット
extern CAMERA_CONTROL* AddCameraControl( GAME_SYSTEM* gs, HEAPID heapID );
// カメラコントロールメイン
extern void MainCameraControl( CAMERA_CONTROL* cc );
// カメラコントロール終了
extern void RemoveCameraControl( CAMERA_CONTROL* cc );
// カメラ座標の取得と設定
extern void GetCameraControlTrans( CAMERA_CONTROL* cc, VecFx32* trans );
extern void SetCameraControlTrans( CAMERA_CONTROL* cc, VecFx32* trans );
// カメラ回転値の取得と設定
extern void GetCameraControlDirection( CAMERA_CONTROL* cc, u16* value );
extern void SetCameraControlDirection( CAMERA_CONTROL* cc, u16* value );
// カメラ方向と対象物の内積取得
extern void GetCameraControlDotProduct( CAMERA_CONTROL* cc, VecFx32* trans, int* scalar );
// カリングチェック：カメラとのスカラー値によるカリング
extern BOOL CullingCameraScalar( CAMERA_CONTROL* cc, VecFx32* trans, int scalarLimit );
// カリングチェック：カメラの視界によるカリング（ＸＺのみ）
extern BOOL CullingCameraBitween( CAMERA_CONTROL* cc, VecFx32* trans, u16 theta );
// カリングチェック：カメラ対象物の周囲位置によるカリング（ＸＺのみ）
extern BOOL CullingCameraAround( CAMERA_CONTROL* cc, VecFx32* trans, fx32 len );


