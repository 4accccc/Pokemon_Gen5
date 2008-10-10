//============================================================================================
/**
 * @file	skb.h
 * @brief	ソフトウエアキーボード
 * @author	
 * @date	
 */
//============================================================================================
typedef struct _GFL_SKB GFL_SKB;

//グラフィックアーカイブ内容ＩＤ定義
enum {
	NARC_skb_skb_NCGR = 0,
	NARC_skb_skb_NCLR = 1,
	NARC_skb_skb_1_NSCR = 2,
	NARC_skb_skb_2_NSCR = 3,
	NARC_skb_skb_3_NSCR = 4
};

typedef enum {
	GFL_SKB_BGID_M0 = 0,
	GFL_SKB_BGID_M1,
	GFL_SKB_BGID_M2,
	GFL_SKB_BGID_M3,
	GFL_SKB_BGID_S0,
	GFL_SKB_BGID_S1,
	GFL_SKB_BGID_S2,
	GFL_SKB_BGID_S3,
}GFL_SKB_BGID;

typedef enum {
	GFL_SKB_PALID_0 = 0,
	GFL_SKB_PALID_1,
	GFL_SKB_PALID_2,
	GFL_SKB_PALID_3,
	GFL_SKB_PALID_4,
	GFL_SKB_PALID_5,
	GFL_SKB_PALID_6,
	GFL_SKB_PALID_7,
	GFL_SKB_PALID_8,
	GFL_SKB_PALID_9,
	GFL_SKB_PALID_10,
	GFL_SKB_PALID_11,
	GFL_SKB_PALID_12,
	GFL_SKB_PALID_13,
	GFL_SKB_PALID_14,
	GFL_SKB_PALID_15,
	GFL_SKB_PALID_NONE = 255,
}GFL_SKB_PALID;

typedef enum {
	GFL_SKB_MODE_HIRAGANA = 0,
	GFL_SKB_MODE_KATAKANA,
	GFL_SKB_MODE_ENGNUM,
}GFL_SKB_MODE;

typedef struct {
	void*			strings;		//文字列格納ポインタ
	u32				strlen;			//文字列格納長さ

	GFL_SKB_MODE	mode;			//初期モード
	BOOL			modeChange;		//モード変更可否フラグ
	u32				cancelKey;		//中断キー判定トリガ

	GFL_SKB_BGID	bgID;			//使用BGVRAMID
	GFL_SKB_PALID	bgPalID;		//使用BGパレットID
	GFL_SKB_PALID	bgPalID_on;		//使用BGパレットID

}GFL_SKB_SETUP;

//============================================================================================
/**
 *
 * @brief	システム起動&終了
 *	
 */
//============================================================================================
extern GFL_SKB*		GFL_SKB_Boot( HEAPID heapID, const GFL_SKB_SETUP* setup );
extern void			GFL_SKB_Exit( GFL_SKB* gflSkb );	//強制終了したい場合使用
extern BOOL			GFL_SKB_Main( GFL_SKB* gflSkb );	//FALSEで終了

