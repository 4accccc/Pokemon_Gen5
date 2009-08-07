//============================================================================================
/**
 * @file	snd_viewer.c
 * @brief	サウンドステータスビューワー
 * @author	
 * @date	
 */
//============================================================================================
#ifndef __SND_VIEWER_H__
#define __SND_VIEWER_H__

//
//	使用の際にはgraphicフォルダ内のsndstatus.narcをルートに配置してください
//	（"sndstatus.narc"パス指定で取得できる場所）
//
//	使用BG面のコントロールレジスタは呼び出し元で設定された状態に依存します。
//	
typedef struct _GFL_SNDVIEWER GFL_SNDVIEWER;
typedef NNSSndHandle*	GFL_SNDVIEWER_GETSNDHANDLE_FUNC(void);
typedef u32				GFL_SNDVIEWER_GETSOUNDNO_FUNC(void);
typedef u32				GFL_SNDVIEWER_GETPLAYERNO_FUNC(void);
typedef BOOL			GFL_SNDVIEWER_GETREVERBFLAG_FUNC(void);

typedef enum {
	GFL_SNDVIEWER_CONTROL_NONE		= 0x0000,	// コントロールなし
	GFL_SNDVIEWER_CONTROL_ENABLE	= 0x0001,	// 操作あり
	GFL_SNDVIEWER_CONTROL_EXIT		= 0x0002,	// ＥＸＩＴ受付あり

}GFL_SNDVIEWER_CONTROL;

//　設定定義
typedef struct {
	u32					cancelKey;		//中断キー判定トリガ(0でキー中断不可)

	GFL_DISPUT_BGID		bgID;			//使用BGVRAMID
	GFL_DISPUT_PALID	bgPalID;		//使用BGパレットID

	GFL_SNDVIEWER_GETSNDHANDLE_FUNC*	getSndHandleFunc;
	GFL_SNDVIEWER_GETSOUNDNO_FUNC*		getSoundNoFunc;
	GFL_SNDVIEWER_GETPLAYERNO_FUNC*		getPlayerNoFunc;
	GFL_SNDVIEWER_GETREVERBFLAG_FUNC*	getReverbFlagFunc;

	u16					controlFlag;	//GFL_SNDVIEWER_CONTROLの組み合わせ

}GFL_SNDVIEWER_SETUP;

#define GFL_SNDVIEWER_PLAYERNO_UNKNOWN	(0xffffffff)
//============================================================================================
/**
 *
 * @brief	システム起動&終了
 *	
 */
//============================================================================================
extern GFL_SNDVIEWER*	GFL_SNDVIEWER_Create( const GFL_SNDVIEWER_SETUP* setup, HEAPID heapID );
extern void				GFL_SNDVIEWER_Delete( GFL_SNDVIEWER* gflSndViewer );
extern BOOL				GFL_SNDVIEWER_Main( GFL_SNDVIEWER* gflSndViewer );	//FALSEで終了

extern u16	GFL_SNDVIEWER_GetControl( GFL_SNDVIEWER* gflSndViewer );
extern void	GFL_SNDVIEWER_SetControl( GFL_SNDVIEWER* gflSndViewer, u16 flag );
extern u16	GFL_SNDVIEWER_GetTrackSt( GFL_SNDVIEWER* gflSndViewer );
#endif
