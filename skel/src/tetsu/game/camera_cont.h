//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _CAMERA_CONTROL	CAMERA_CONTROL;

// カメラコントロールセット
extern CAMERA_CONTROL* AddCameraControl( GAME_SYSTEM* gs, int targetAct, HEAPID heapID );
// カメラコントロールメイン
extern void MainCameraControl( CAMERA_CONTROL* cc );
// カメラコントロール終了
extern void RemoveCameraControl( CAMERA_CONTROL* cc );
// カメラ回転値の取得と設定
extern void GetCameraControlDirection( CAMERA_CONTROL* cc, u16* value );
extern void SetCameraControlDirection( CAMERA_CONTROL* cc, u16* value );

