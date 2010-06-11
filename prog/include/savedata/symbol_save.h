//==============================================================================
/**
 * @file    symbol_save.h
 * @brief   シンボルエンカウント用セーブデータ
 * @author  matsuda
 * @date    2010.01.06(水)
 */
//==============================================================================
#pragma once


//==============================================================================
//  型定義
//==============================================================================
//シンボルエンカウントセーブデータ
typedef struct _SYMBOL_SAVE_WORK SYMBOL_SAVE_WORK;


//==============================================================================
//  定数定義
//==============================================================================
enum {
  SYMBOL_POKE_MAX         = 530,         ///<シンボルポケモン最大数

  SYMBOL_SPACE_NONE       = (0xffff),     ///<対象ゾーンに空きが無い事を示す値
};

///1マップ(32x32)でのストック数
#define SYMBOL_MAP_STOCK_SMALL    (20)
///1マップ(64x64)でのストック数
#define SYMBOL_MAP_STOCK_LARGE    (10)
///キープゾーン、フリーゾーンの中で最も1マップ中、多くストック出来る数
#define SYMBOL_MAP_STOCK_MAX      (SYMBOL_MAP_STOCK_SMALL)


//--------------------------------------------------------------
/**
 * @brief シンボルポケモン保持領域指定IDの定義
 *
 * ※SymbolZoneTypeDataNoと並びを同じにしておくこと！
 */
//--------------------------------------------------------------
typedef enum{
  SYMBOL_ZONE_TYPE_KEEP_LARGE,      ///<キープゾーン(64x64)専用エリア
  SYMBOL_ZONE_TYPE_KEEP_SMALL,      ///<キープゾーン(32x32)専用エリア
  SYMBOL_ZONE_TYPE_FREE_LARGE,      ///<フリーゾーン(64x64)
  SYMBOL_ZONE_TYPE_FREE_SMALL,      ///<フリーゾーン(32x32)
  
  SYMBOL_ZONE_TYPE_KEEP_ALL,        ///<キープゾーンLARGEとSMALL両方
}SYMBOL_ZONE_TYPE;

//--------------------------------------------------------------
///マップ番号
//--------------------------------------------------------------
typedef enum{
  SYMBOL_MAP_ID_KEEP,               ///<MAP ID キープエリア(LARGE & SMALL)
  
  SYMBOL_MAP_ID_FREE_LARGE_START,   ///<MAP ID フリー(64x64)開始No
  SYMBOL_MAP_ID_FREE_LARGE_END = SYMBOL_MAP_ID_FREE_LARGE_START + 3,
  
  SYMBOL_MAP_ID_FREE_SMALL_START = SYMBOL_MAP_ID_FREE_LARGE_END,  ///<MAP ID フリー(32x32)開始No

  SYMBOL_MAP_ID_ENTRANCE  = 4,  ///< MAP ID 入った最初のところ
}SYMBOL_MAP_ID;

//--------------------------------------------------------------
///シンボルマップ(64x64)のマップレベル(数字が大きいほど広い)
//--------------------------------------------------------------
typedef enum{
  SYMBOL_MAP_LEVEL_LARGE_NONE,      ///<マップが存在しない
  SYMBOL_MAP_LEVEL_LARGE_LEVEL_1,
  
  SYMBOL_MAP_LEVEL_LARGE_MAX = SYMBOL_MAP_LEVEL_LARGE_LEVEL_1,
}SYMBOL_MAP_LEVEL_LARGE;

//--------------------------------------------------------------
///シンボルマップ(32x32)のマップレベル(数字が大きいほど広い)
//--------------------------------------------------------------
typedef enum{
  SYMBOL_MAP_LEVEL_SMALL_1,
  SYMBOL_MAP_LEVEL_SMALL_2,
  SYMBOL_MAP_LEVEL_SMALL_3,
  SYMBOL_MAP_LEVEL_SMALL_4,
  SYMBOL_MAP_LEVEL_SMALL_5,
  SYMBOL_MAP_LEVEL_SMALL_6,
  SYMBOL_MAP_LEVEL_SMALL_7,
  
  SYMBOL_MAP_LEVEL_SMALL_MAX = SYMBOL_MAP_LEVEL_SMALL_7,
}SYMBOL_MAP_LEVEL_SMALL;


//==============================================================================
//  構造体定義
//==============================================================================
//--------------------------------------------------------------
///シンボルエンカウント1匹分のセーブ
//--------------------------------------------------------------
typedef struct _SYMBOL_POKEMON{
  u32 monsno:11;        ///<ポケモン番号
  u32 wazano:10;        ///<技番号
  u32 sex:2;            ///<性別(PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN)
  u32 form_no:6;        ///<フォルム番号
  u32 move_type:3;      ///<動作タイプ
}SYMBOL_POKEMON;


//--------------------------------------------------------------
///SYMBOL_ZONE_TYPE毎の配置Noの開始位置と終端位置を管理する構造体
//--------------------------------------------------------------
typedef struct{
  u16 start;
  u16 end;
}SYMBOL_ZONE_TYPE_DATA_NO;


//==============================================================================
//  外部関数宣言
//==============================================================================
//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
extern SYMBOL_SAVE_WORK* SymbolSave_GetSymbolData(SAVE_CONTROL_WORK* pSave);
extern u32 SymbolSave_GetWorkSize( void );
extern void SymbolSave_WorkInit(void *work);


//--------------------------------------------------------------
//  アクセス関数
//--------------------------------------------------------------
extern BOOL SymbolSave_GetSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, u32 no, SYMBOL_POKEMON* pSymbol);

extern u32 SymbolSave_CheckSpace(
  SYMBOL_SAVE_WORK *symbol_save, SYMBOL_ZONE_TYPE zone_type);
extern void SymbolSave_DataShift(SYMBOL_SAVE_WORK *symbol_save, u32 no);
extern void SymbolSave_SetFreeZone(SYMBOL_SAVE_WORK *symbol_save,
    u16 monsno, u16 wazano, u8 sex, u8 form_no, u8 move_type, SYMBOL_ZONE_TYPE zone_type);

static inline u32 SymbolSave_CheckFreeZoneSpace(
    SYMBOL_SAVE_WORK *symbol_save, SYMBOL_ZONE_TYPE zone_type)
{ return SymbolSave_CheckSpace( symbol_save, zone_type ); }

//--------------------------------------------------------------
//  ツール関数
//--------------------------------------------------------------
extern SYMBOL_ZONE_TYPE SYMBOLZONE_GetZoneTypeFromNumber(u32 no);
extern int SymbolSave_CheckFlashLoad(SAVE_CONTROL_WORK *ctrl, const SYMBOL_POKEMON *spoke, HEAPID heap_id, BOOL pp_mode);

//==============================================================================
//  外部データ
//==============================================================================
extern const SYMBOL_ZONE_TYPE_DATA_NO SymbolZoneTypeDataNo[];



//==============================================================================
//  外部データアクセスのためのインライン関数
//==============================================================================
//--------------------------------------------------------------
/**
 * SYMBOL_ZONE_TYPEごとの保持データ最大数を取得
 * @param zone_type
 * @return  u16     保持データ最大数
 */
//--------------------------------------------------------------
static inline u16 SYMBOLZONE_GetStockMax( SYMBOL_ZONE_TYPE zone_type )
{
  return SymbolZoneTypeDataNo[zone_type].end - SymbolZoneTypeDataNo[zone_type].start;
}

//--------------------------------------------------------------
/// SYMBOL_ZONE_TYPEごとの保持データ開始ナンバーを取得
//--------------------------------------------------------------
static inline u16 SYMBOLZONE_GetStartNo( SYMBOL_ZONE_TYPE zone_type )
{
  return SymbolZoneTypeDataNo[zone_type].start;
}

//--------------------------------------------------------------
/// SYMBOL_ZONE_TYPEごとの保持データ終了ナンバーを取得
//--------------------------------------------------------------
static inline u16 SYMBOLZONE_GetEndNo( SYMBOL_ZONE_TYPE zone_type )
{
  return SymbolZoneTypeDataNo[zone_type].end;
}


