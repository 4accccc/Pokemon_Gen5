
//============================================================================================
/**
 * @file	batt_bg_tbl.h
 * @bfief	戦闘背景テーブル
 * @author	BattBGConverter
 * 戦闘背景テーブルコンバータから生成されました
*/
//============================================================================================

#pragma once

#define  ZONE_SPEC_ATTR_MAX  ( 15 )
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
