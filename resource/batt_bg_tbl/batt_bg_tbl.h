
//============================================================================================
/**
 * @file	batt_bg_tbl.h
 * @bfief	戦闘背景テーブル
 * @author	BattBGConverter
 * 戦闘背景テーブルコンバータから生成されました
*/
//============================================================================================

#pragma once

#define  ZONE_SPEC_ATTR_MAX  ( 13 )
#define  BATT_BG_TBL_NO_FILE     ( 0xffff )
#define  BATT_BG_TBL_SEASON_MAX  ( 4 )

typedef struct
{
	BOOL time_zone;
	BOOL season;
	u8   bg_file[ ZONE_SPEC_ATTR_MAX ];
	u8   stage_file[ ZONE_SPEC_ATTR_MAX ];
}BATT_BG_TBL_ZONE_SPEC_TABLE;

typedef struct
{
	ARCDATID nsbmd_file[ BATT_BG_TBL_SEASON_MAX ];
	ARCDATID nsbca_file[ BATT_BG_TBL_SEASON_MAX ];
	ARCDATID nsbta_file[ BATT_BG_TBL_SEASON_MAX ];
	ARCDATID nsbma_file[ BATT_BG_TBL_SEASON_MAX ];
}BATT_BG_TBL_FILE_TABLE;
