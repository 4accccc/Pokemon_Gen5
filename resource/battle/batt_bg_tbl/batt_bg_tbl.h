
//============================================================================================
/**
 * @file	batt_bg_tbl.h
 * @bfief	戦闘背景テーブル
 * @author	BattBGConverter
 * 戦闘背景テーブルコンバータから生成されました
*/
//============================================================================================

#pragma once

#define  ZONE_SPEC_ATTR_MAX  ( 17 )
#define  BATT_BG_TBL_NO_FILE     ( 0xffffffff )
#define  BATT_BG_TBL_FILE_MAX  ( 4 )

#define  BATT_BG_TBL_SEASON_MAX  ( 4 )

typedef enum
{
	BATT_BG_TBL_FILE_NSBMD = 0,
	BATT_BG_TBL_FILE_NSBCA,
	BATT_BG_TBL_FILE_NSBTA,
	BATT_BG_TBL_FILE_NSBMA,
}BATT_BG_TBL_FILE;

typedef struct
{
	u8 time_zone;
	u8 season;
	u8 bg_file[ ZONE_SPEC_ATTR_MAX ];
	u8 stage_file[ ZONE_SPEC_ATTR_MAX ];
}BATT_BG_TBL_ZONE_SPEC_TABLE;

typedef struct
{
	u16  edge_color;
	u16  padding;
	ARCDATID file[BATT_BG_TBL_FILE_MAX ][ BATT_BG_TBL_SEASON_MAX ];
}BATT_BG_TBL_STAGE_TABLE;

typedef struct
{
	ARCDATID file[BATT_BG_TBL_FILE_MAX ][ BATT_BG_TBL_SEASON_MAX ];
}BATT_BG_TBL_BG_TABLE;

typedef enum{
	BATT_BG_OBONID_00 = 0,		//地面（四季なし）
	BATT_BG_OBONID_01,				//地面（四季あり）
	BATT_BG_OBONID_02,				//床（屋外）
	BATT_BG_OBONID_03,				//床（屋内）
	BATT_BG_OBONID_04,				//洞窟
	BATT_BG_OBONID_05,				//芝生（四季なし）
	BATT_BG_OBONID_06,				//芝生（四季あり）
	BATT_BG_OBONID_07,				//山肌
	BATT_BG_OBONID_08,				//水上（屋外）
	BATT_BG_OBONID_09,				//水上（屋内）
	BATT_BG_OBONID_10,				//砂漠（屋外）
	BATT_BG_OBONID_11,				//砂漠（屋内）
	BATT_BG_OBONID_12,				//浅い湿地
	BATT_BG_OBONID_13,				//氷上（屋内）
	BATT_BG_OBONID_14,				//パレス専用
	BATT_BG_OBONID_15,				//四天王（ゴースト）専用
	BATT_BG_OBONID_16,				//四天王（格闘）専用
	BATT_BG_OBONID_17,				//四天王（悪）専用
	BATT_BG_OBONID_18,				//四天王（エスパー）専用
	BATT_BG_OBONID_19,				//N専用
	BATT_BG_OBONID_20,				//ゲーチス専用
	BATT_BG_OBONID_21,				//チャンピオン専用
	BATT_BG_OBONID_22,				//エンカウント草（四季あり）
	BATT_BG_OBONID_23,				//エンカウント草（四季なし）
	BATT_BG_OBONID_24,				//アスファルト
}BATT_BG_OBONID;

