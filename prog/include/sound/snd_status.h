//============================================================================================
/**
 * @file	snd_status.c
 * @brief	サウンドステータスビューワー
 * @author	
 * @date	
 */
//============================================================================================
//
//	使用の際にはgraphicフォルダ内のsndstatus.narcをルートに配置してください
//	（"sndstatus.narc"パス指定で取得できる場所）
//
//	使用BG面のコントロールレジスタは呼び出し元で設定された状態に依存します。
//	
typedef struct _GFL_SNDSTATUS GFL_SNDSTATUS;

typedef enum {
	GFL_SNDSTATUS_CONTROL_NONE		= 0x0000,	// コントロールなし
	GFL_SNDSTATUS_CONTROL_ENABLE	= 0x0001,	// 操作あり
	GFL_SNDSTATUS_CONTROL_EXIT		= 0x0002,	// ＥＸＩＴ受付あり

}GFL_SNDSTATUS_CONTROL;

//　設定定義
typedef struct {
	u32					cancelKey;		//中断キー判定トリガ(0でキー中断不可)

	GFL_DISPUT_BGID		bgID;			//使用BGVRAMID
	GFL_DISPUT_PALID	bgPalID;		//使用BGパレットID

	NNSSndHandle*		pBgmHandle;

	u16					controlFlag;	//GFL_SNDSTATUS_CONTROLの組み合わせ

}GFL_SNDSTATUS_SETUP;

//============================================================================================
/**
 *
 * @brief	システム起動&終了
 *	
 */
//============================================================================================
extern GFL_SNDSTATUS*	GFL_SNDSTATUS_Create( const GFL_SNDSTATUS_SETUP* setup, HEAPID heapID );
extern void				GFL_SNDSTATUS_Delete( GFL_SNDSTATUS* gflSndStatus );
extern BOOL				GFL_SNDSTATUS_Main( GFL_SNDSTATUS* gflSndStatus );	//FALSEで終了

extern void	GFL_SNDSTATUS_InitControl( GFL_SNDSTATUS* gflSndStatus );
extern void	GFL_SNDSTATUS_InitReverbControl( GFL_SNDSTATUS* gflSndStatus );
extern u16	GFL_SNDSTATUS_GetControl( GFL_SNDSTATUS* gflSndStatus );
extern void	GFL_SNDSTATUS_SetControl( GFL_SNDSTATUS* gflSndStatus, u16 flag );
extern void	GFL_SNDSTATUS_ChangeSndHandle( GFL_SNDSTATUS* gflSndStatus, NNSSndHandle* pBgmHandle );
