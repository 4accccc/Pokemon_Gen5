//============================================================================================
/**
 * @file	camera_adjust_view.h
 * @brief	カメラ調整用ビューワー
 * @author	
 * @date	
 */
//============================================================================================
#ifndef __DEBUG_CAMADJUST_H__
#define __DEBUG_CAMADJUST_H__

//
//	使用の際にはgraphicフォルダ内のcamera_adjust_view.narcをルートに配置してください
//	（"camera_adjust_view.narc"パス指定で取得できる場所）
//
//	使用BG面のコントロールレジスタは呼び出し元で設定された状態に依存します。
//	
typedef struct _GFL_CAMADJUST GFL_CAMADJUST;

//　設定定義
typedef struct {
	u32					cancelKey;		//中断キー判定トリガ(0でキー中断不可)

	GFL_DISPUT_BGID		bgID;			//使用BGVRAMID
	GFL_DISPUT_PALID	bgPalID;		//使用BGパレットID

	u16*				pCameraAngleV;
	u16*				pCameraAngleH;
	fx32*				pCameraLength;
}GFL_CAMADJUST_SETUP;

//============================================================================================
/**
 *
 * @brief	システム起動&終了
 *	
 */
//============================================================================================
extern GFL_CAMADJUST*	GFL_CAMADJUST_Create( const GFL_CAMADJUST_SETUP* setup, HEAPID heapID );
extern void				GFL_CAMADJUST_Delete( GFL_CAMADJUST* gflCamAdjust );
extern BOOL				GFL_CAMADJUST_Main( GFL_CAMADJUST* gflCamAdjust );	//FALSEで終了

extern void	GFL_CAMADJUST_SetCameraParam( GFL_CAMADJUST* gflCamAdjust, 
																		u16* pAngleV, u16* pAngleH, 
																		fx32* pLength, u16* pFovy, fx32* pFar );
extern void	GFL_CAMADJUST_SetWipeParam( GFL_CAMADJUST* gflCamAdjust, fx32* pWipeSize );

#endif

