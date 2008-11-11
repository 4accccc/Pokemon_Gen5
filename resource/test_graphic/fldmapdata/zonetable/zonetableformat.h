
//NitroSDK/include/nitro/types.hよりコピペ
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long u32;

#ifndef	TRUE
#define	TRUE	1
#endif
#ifndef	FALSE
#define	FALSE	(!TRUE)
#endif

//===========================================================================
//===========================================================================
//------------------------------------------------------------------
/**
 * @brief	ゾーン毎に持つデータの構成
 */
//------------------------------------------------------------------
typedef struct{
	u8 area_id;							///<AREA_IDの指定
	u8 movemodel_id;					///<動作モデルの指定
	u16 matrix_id;						///<マップマトリックスの指定
	u16 script_id;						///<スクリプトファイルのアーカイブID
	u16 sp_script_id;					///<特殊スクリプトファイルのアーカイブID
	u16 msg_id;							///<メッセージファイルのアーカイブID
	u16 bgm_day_id;						///<BGM指定（昼）のID
	u16 bgm_night_id;					///<BGM指定（夜）のID
	u16 enc_data_id;					///<エンカウント指定
	u16 event_data_id;					///<イベントデータファイルのアーカイブID
	u16 placename_id:8;					///<地名メッセージのＩＤ
	u16 placename_win:8;				///<地名メッセージ用のウィンドウ
	u8 weather_id;						///<天候指定のID
	u8 camera_id;						///<カメラ指定のID
	u16 maptype:7;							///<マップタイプの指定(MAPTYPE定義参照）
	u16 battle_bg_type:5;				///<戦闘背景の指定
	u16 bicycle_flag:1;					///<自転車に乗れるかどうか
	u16 dash_flag:1;						///<ダッシュできるかどうか
	u16 escape_flag:1;					///<あなぬけできるかどうか
	u16 fly_flag:1;
}ZONE_DATA;
//------------------------------------------------------------------
//------------------------------------------------------------------

enum {
	//sp_scr_dummy = NARC_scr_seq_sp_dummy_bin,
	sp_scr_dummy = 0xf000,
	//scr_dummy = NARC_scr_seq_dummy_bin,
	scr_dummy = 0xe000,
	//event_dummy = NARC_zone_event_zone_dummy_total_bin,
	event_dummy = 0xd000,
	//sp_scr_dummy = NARC_scr_seq_sp_c01_bin,
	//scr_dummy = NARC_scr_seq_c01_bin,
	//msg_dummy = NARC_msg_c01_dat,
	msg_dummy = 0xcccc,
	enc_dummy = 0xffff
};

enum {
	AREA_ID_FIELD	=	0,
	MATRIX_ID_SINOU	=	1,
	BG_ID_FOREST	=	3,
	MMLID_NOENTRY	=	5,
	MAPTYPE_NOWHERE	=	0xff,
	MAPNAME_NOTHING	=	0x33,
	WEATHER_SYS_SUNNY	=	0x11,
	PLACE_WIN_SEE	=	0x80,
	PLACE_WIN_CITY	=	0x81
};

