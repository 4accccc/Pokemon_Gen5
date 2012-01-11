//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_overlay.c
 *	@brief  スクリプトコマンドのオーバーレイ管理
 *	@author	Toru=Nagihashi
 *	@data		2010.10.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
#include "gamesystem/playerwork.h"  //typedef ZONEID

//script
#include "system/vm.h"
#include "fieldmap/zone_id.h"

//mine
#include "scrcmd_overlay.h"

//スクリプトオーバーレイ定義
#include "scrcmd_bsubway_table.h"
#include "scrcmd_pl_boat_table.h"
#include "scrcmd_fourkings_table.h"
#include "scrcmd_palpark_table.h"
#include "scrcmd_seatemple_table.h"
#include "scrcmd_wbt_table.h"
#include "scrcmd_resort_table.h"
#include "scrcmd_infinity_dungeon_table.h"
#include "scrcmd_pokewood_table.h"
#include "scrcmd_bgate_table.h"
#include "scrcmd_hide_ship_table.h"
#include "scrcmd_pokecen_table.h"
#include "scrcmd_demo_gimmick_table.h"
#include "scrcmd_gimmick_out58_table.h"
#include "scrcmd_gimmick_sppoke_table.h"

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define SCRCMD_OV_Printf(...) NAGI_Printf(__VA_ARGS__)
#else
#define SCRCMD_OV_Printf(...) ((void)0)
#endif //PM_DEBUG

//-------------------------------------
///	オーバーレイエクスターン
//=====================================
FS_EXTERN_OVERLAY( scrcmd_bsubway );
FS_EXTERN_OVERLAY( scrcmd_pl_boat );
FS_EXTERN_OVERLAY( scrcmd_fourkings );
FS_EXTERN_OVERLAY( scrcmd_palpark );
FS_EXTERN_OVERLAY( scrcmd_seatemple );
FS_EXTERN_OVERLAY( scrcmd_wbt );
FS_EXTERN_OVERLAY( scrcmd_wbt_lobby );
FS_EXTERN_OVERLAY( scrcmd_wbt_hall );
FS_EXTERN_OVERLAY( scrcmd_resort );
FS_EXTERN_OVERLAY( scrcmd_resort_common );
FS_EXTERN_OVERLAY( scrcmd_infinity_dungeon );
FS_EXTERN_OVERLAY( scrcmd_pokewood );
FS_EXTERN_OVERLAY( scrcmd_bgate );
FS_EXTERN_OVERLAY( scrcmd_hide_ship );
FS_EXTERN_OVERLAY( scrcmd_pokecen );
FS_EXTERN_OVERLAY( scrcmd_demo_gimmick );
FS_EXTERN_OVERLAY( scrcmd_gimmick_out58 );
FS_EXTERN_OVERLAY( scrcmd_gimmick_sppoke );

//=============================================================================
/**
 *					ゾーン管理テーブル
*/
//=============================================================================
//-------------------------------------
///	バトルサブウェイオーバーレイを読込むゾーンID
//=====================================
static const ZONEID UseZoneIDTbl_bsubway[] =
{
  ZONE_ID_C04R0101,
  ZONE_ID_C04R0102,
  ZONE_ID_C04R0103,
  ZONE_ID_C04R0104,
  ZONE_ID_C04R0105,
  ZONE_ID_C04R0106,
  ZONE_ID_C04R0107,
  ZONE_ID_C04R0108,
  ZONE_ID_C04R0109,
  ZONE_ID_C04R0110,
  ZONE_ID_C04R0111,
};

//-------------------------------------
///	遊覧船
//=====================================
static const ZONEID UseZoneIDTbl_pl_boat[] =
{
  ZONE_ID_C03R0801,
};

//-------------------------------------
///	四天王部屋　と　チャンピオン部屋
//=====================================
static const ZONEID UseZoneIDTbl_fourkings[]  =
{
  ZONE_ID_C09R0101,
  ZONE_ID_C09R0201,
  ZONE_ID_C09R0301,
  ZONE_ID_C09R0401,
  ZONE_ID_C09R0501,   // チャンピオン部屋
};

//-------------------------------------
///	ポケシフター（名前は共通化させるためパルパークです）
//=====================================
static const ZONEID UseZoneIDTbl_palpark[]  =
{
  ZONE_ID_R15R0301,
};

//-------------------------------------
///	海底神殿
//=====================================
static const ZONEID UseZoneIDTbl_seatemple[]  =
{
  ZONE_ID_D20R0101,
  ZONE_ID_D20R0201,
  ZONE_ID_D20R0301,
  ZONE_ID_D20R0401,
  ZONE_ID_D20R0501,
};

//-------------------------------------
///	WBT
//=====================================
static const ZONEID UseZoneIDTbl_wbt_lobby[] = 
{
  ZONE_ID_D05R0101,
};

static const ZONEID UseZoneIDTbl_wbt_hall[] = 
{
  ZONE_ID_D05R0201,
};

//-------------------------------------
///	レゾナンスリゾート
//=====================================
static const ZONEID UseZoneIDTbl_resort[] = 
{
  ZONE_ID_C16,
  ZONE_ID_C16R0101,
};

//-------------------------------------
///	無限ダンジョン
//=====================================
static const ZONEID UseZoneIDTbl_infinitydungeon[] = 
{
  ZONE_ID_D31R0101,
  ZONE_ID_D31R0201,
  ZONE_ID_D31R0301,
  ZONE_ID_D31R0401,
  ZONE_ID_D31R0501,
  ZONE_ID_D31R0601,
  ZONE_ID_D31R0701,
  ZONE_ID_D31R0801,
  ZONE_ID_D31R0901,
  ZONE_ID_D31R1001,
  ZONE_ID_D31R1101,
  ZONE_ID_D31R1201,
};
//-------------------------------------
///	無限ダンジョン
//=====================================
static const ZONEID UseZoneIDTbl_pokewood[] = 
{
  ZONE_ID_C15,
  ZONE_ID_C15R0101,
  ZONE_ID_C15R0201,
  ZONE_ID_C15R0301,
};
//-------------------------------------
///	バッジチェックゲート
//=====================================
static const ZONEID UseZoneIDTbl_badge_gate[] = 
{
  ZONE_ID_D26R0101,
};
//-------------------------------------
///	アジト船
//=====================================
static const ZONEID UseZoneIDTbl_hide_ship[] = 
{
  ZONE_ID_D27R1001,   // エンジン部屋
  ZONE_ID_D27R1301,   // エンジン部屋
  ZONE_ID_D27R0201,   // 船首
  ZONE_ID_D27R1201,   // 船首
  ZONE_ID_D27R0701,   // パスワードID取得に使用
  ZONE_ID_D27R1801,   // パスワードID取得に使用
  ZONE_ID_D27R1901,   // パスワードID取得に使用
  ZONE_ID_D27R2001,   // パスワードID取得に使用
  ZONE_ID_D27R2101,   // パスワードID取得に使用
  ZONE_ID_D27R2201,   // パスワードID取得に使用
};
//-------------------------------------
///	ポケモンセンター
//=====================================
static const ZONEID UseZoneIDTbl_pokecen[] = 
{
  ZONE_ID_BC10PCR0101,
  ZONE_ID_C01PC0101,
  ZONE_ID_C02PC0101,
  ZONE_ID_C03PC0101,
  ZONE_ID_C04PC0101,
  ZONE_ID_C05PC0101,
  ZONE_ID_C06PC0101,
  ZONE_ID_C07PC0101,
  ZONE_ID_C08PC0101,
  ZONE_ID_T02PC0101,
  ZONE_ID_T03PC0101,
  ZONE_ID_T04PC0101,
  ZONE_ID_WC10PCR0101,
  ZONE_ID_C12PC0101,
  ZONE_ID_T06PC0101,
  ZONE_ID_C13PC0101,
  ZONE_ID_T07PC0101,
  ZONE_ID_C14PC0101,
  ZONE_ID_D26PC0101,
  ZONE_ID_C09R0701,
};
//-------------------------------------
///	デモギミック関係
//=====================================
static const ZONEID UseZoneIDTbl_demo_gimmick[] = 
{
  ZONE_ID_D10R0501,   // ジャイアントホール
  ZONE_ID_C12,        // ヒオウギシティ
  ZONE_ID_C09P03,     // エンディングでのポケモンリーグ
  ZONE_ID_D08R0901,   // リュウラセンの塔
};
//-------------------------------------
///	OUT58のギミック関係
//=====================================
static const ZONEID UseZoneIDTbl_gimmick_out58[] = 
{
  ZONE_ID_C14,        // セイガイハシティ
  ZONE_ID_W21,        // 21番水路(セイガイハと地続きの為含める)
  ZONE_ID_R22,        // 22番道路(セイガイハと地続きの為含める)
};
//-------------------------------------
///	特殊ポケのギミック関係
//=====================================
static const ZONEID UseZoneIDTbl_gimmick_sppoke[] = 
{
  ZONE_ID_D29R0401,   // レジ３体の地底遺跡
  ZONE_ID_T06R0401,   // 【配布】ケルディオ
  ZONE_ID_C03R0901,   // 【配布】メロエッタ
};



//=============================================================================
/**
 *					管理テーブル
*/
//=============================================================================
//-------------------------------------
///	スクリプトコマンドオーバーレイ管理テーブル
//=====================================
static const struct
{
  const VMCMD_FUNC *scriptCmdTbl;   //スクリプトコマンドテーブル
  const ZONEID  *zoneIDTbl;         //テーブルが読み込まれるゾーンIDのテーブル
  u32 zoneIDTblMax;                 //ゾーンIDテーブルの要素数
  FSOverlayID overlayID;            //テーブルとコマンドが属するオーバーレイID
  FSOverlayID sub_overlayID;        //テーブルとコマンドが属するオーバーレイIDサブ
} ScrCmd_Overlay_table[] =
{
  //NONE
  {
    NULL,
    NULL,
    0,
    GFL_OVERLAY_BLANK_ID,
  },
  //BSUBWAY
  {
    ScriptCmdTbl_bsubway,
    UseZoneIDTbl_bsubway,
    NELEMS( UseZoneIDTbl_bsubway ),
    FS_OVERLAY_ID( scrcmd_bsubway ),
    GFL_OVERLAY_BLANK_ID,
  },
  //PL_BOAT
  {
    ScriptCmdTbl_pl_boat,
    UseZoneIDTbl_pl_boat,
    NELEMS(UseZoneIDTbl_pl_boat),
    FS_OVERLAY_ID( scrcmd_pl_boat ),
    GFL_OVERLAY_BLANK_ID,
  },
  //FOURKINGS
  {
    ScriptCmdTbl_fourkings,
    UseZoneIDTbl_fourkings,
    NELEMS( UseZoneIDTbl_fourkings ),
    FS_OVERLAY_ID( scrcmd_fourkings ),
    GFL_OVERLAY_BLANK_ID,
  },
  //PALPARK
  {
    ScriptCmdTbl_palpark,
    UseZoneIDTbl_palpark,
    NELEMS( UseZoneIDTbl_palpark ),
    FS_OVERLAY_ID( scrcmd_palpark ),
    GFL_OVERLAY_BLANK_ID,
  },
  //SEATEMPLE
  {
    ScriptCmdTbl_seatemple,
    UseZoneIDTbl_seatemple,
    NELEMS(UseZoneIDTbl_seatemple),
    FS_OVERLAY_ID( scrcmd_seatemple ),
    GFL_OVERLAY_BLANK_ID,
  },
  //WBT（ロビー）
  {
    ScriptCmdTbl_wbt,
    UseZoneIDTbl_wbt_lobby,
    NELEMS(UseZoneIDTbl_wbt_lobby),
    FS_OVERLAY_ID( scrcmd_wbt ),
    FS_OVERLAY_ID( scrcmd_wbt_lobby ),
  },
  //WBT（会場内）
  {
    ScriptCmdTbl_wbt,
    UseZoneIDTbl_wbt_hall,
    NELEMS(UseZoneIDTbl_wbt_hall),
    FS_OVERLAY_ID( scrcmd_wbt ),
    FS_OVERLAY_ID( scrcmd_wbt_hall ),
  },
  //RESONANCE_RESORT
  {
    ScriptCmdTbl_resort,
    UseZoneIDTbl_resort,
    NELEMS(UseZoneIDTbl_resort),
    FS_OVERLAY_ID( scrcmd_resort ),
    FS_OVERLAY_ID( scrcmd_resort_common ),
  },
  //INFINITY_DUNGEON
  {
    ScriptCmdTbl_infinity_dungeon,
    UseZoneIDTbl_infinitydungeon,
    NELEMS(UseZoneIDTbl_infinitydungeon),
    FS_OVERLAY_ID( scrcmd_infinity_dungeon ),
    GFL_OVERLAY_BLANK_ID,
  },
  //POKEWOOD
  {
    ScriptCmdTbl_pokewood,
    UseZoneIDTbl_pokewood,
    NELEMS(UseZoneIDTbl_pokewood),
    FS_OVERLAY_ID( scrcmd_pokewood ),
    GFL_OVERLAY_BLANK_ID,
  },
  //BADGE_CHECK_GATE
  {
    ScriptCmdTbl_bgate,
    UseZoneIDTbl_badge_gate,
    NELEMS(UseZoneIDTbl_badge_gate),
    FS_OVERLAY_ID( scrcmd_bgate ),
    GFL_OVERLAY_BLANK_ID,
  },
  //HIDE_SHIP
  {
    ScriptCmdTbl_hide_ship,
    UseZoneIDTbl_hide_ship,
    NELEMS(UseZoneIDTbl_hide_ship),
    FS_OVERLAY_ID( scrcmd_hide_ship ),
    GFL_OVERLAY_BLANK_ID,
  },
  //POKECEN
  {
    ScriptCmdTbl_pokecen,
    UseZoneIDTbl_pokecen,
    NELEMS(UseZoneIDTbl_pokecen),
    FS_OVERLAY_ID( scrcmd_pokecen ),
    GFL_OVERLAY_BLANK_ID,
  },
  //DEMO_GIMMICK
  {
    ScriptCmdTbl_demo_gimmick,
    UseZoneIDTbl_demo_gimmick,
    NELEMS(UseZoneIDTbl_demo_gimmick),
    FS_OVERLAY_ID( scrcmd_demo_gimmick ),
    GFL_OVERLAY_BLANK_ID,
  },
  //GIMMICK_OUT58
  {
    ScriptCmdTbl_gimmick_out58,
    UseZoneIDTbl_gimmick_out58,
    NELEMS(UseZoneIDTbl_gimmick_out58),
    FS_OVERLAY_ID( scrcmd_gimmick_out58 ),
    GFL_OVERLAY_BLANK_ID,
  },
  //GIMMICK_SPPOKE
  {
    ScriptCmdTbl_gimmick_sppoke,
    UseZoneIDTbl_gimmick_sppoke,
    NELEMS(UseZoneIDTbl_gimmick_sppoke),
    FS_OVERLAY_ID( scrcmd_gimmick_sppoke ),
    GFL_OVERLAY_BLANK_ID,
  },
};
//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ゾーン切り替わり時をチェック
 *  
 *  @param  GAMEDATA  gamedata ゲームデータ
 *	@param	ZONEID zoneID   現在のゾーンID
 */
//-----------------------------------------------------------------------------
void SCRCMD_OVERLAY_CheckMapChange( GAMEDATA *p_gamedata, ZONEID zoneID )
{
  int i, j;

  //読み込むオーバーレイがあるかチェック
  for( i = 0; i < NELEMS(ScrCmd_Overlay_table); i++ )
  {
    if( i == 0 )
    {
      continue;
    }

    for( j = 0; j < ScrCmd_Overlay_table[i].zoneIDTblMax; j++ )
    {
      if( ScrCmd_Overlay_table[i].zoneIDTbl[j] == zoneID )
      {
        GF_ASSERT_MSG( GAMEDATA_GetFieldScriptCommandOverlayID( p_gamedata ) == 0, "スクリプトオーバーレイの二重登録です\n"  );
        GAMEDATA_SetFieldScriptCommandOverlayID( p_gamedata, i );
        SCRCMD_OV_Printf( "スクリプトオーバーレイに対応 zoneID=%d scrCmdOvID%d\n", zoneID, i );
        return;
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドを監査してスクリプト用オーバーレイを読み込む
 *
 *	@param	GAMEDATA  gamedata ゲームデータ
 */
//-----------------------------------------------------------------------------
void SCRCMD_OVERLAY_SetUp( GAMEDATA  *p_gamedata )
{
  u32 scrCmdTableID = GAMEDATA_GetFieldScriptCommandOverlayID( p_gamedata );

  //ゾーンチェンジ指定があるならば、
  //オーバーレイ読込み
  if( scrCmdTableID == 0 )
  {
    return;
  }
  
  if( ScrCmd_Overlay_table[ scrCmdTableID ].overlayID != GFL_OVERLAY_BLANK_ID )
  {
    SCRCMD_OV_Printf( "スクリプトコマンドオーバーレイ読込み=%d\n", scrCmdTableID );
    GFL_OVERLAY_Load( ScrCmd_Overlay_table[ scrCmdTableID ].overlayID );
  }
  if( ScrCmd_Overlay_table[ scrCmdTableID ].sub_overlayID != GFL_OVERLAY_BLANK_ID )
  {
    SCRCMD_OV_Printf( "スクリプトコマンドオーバーレイ読込み【サブ】=%d\n", scrCmdTableID );
    GFL_OVERLAY_Load( ScrCmd_Overlay_table[ scrCmdTableID ].sub_overlayID );
  }
}
//----------------------------------------------------------------------------
/**
 *	@briefスクリプト用オーバーレイを解放
 *
 *	@param	FIELDMAP_WORK *fieldWork  フィールドワーク
 */
//-----------------------------------------------------------------------------
void SCRCMD_OVERLAY_CleanUp( GAMEDATA  *p_gamedata )
{
  u32 scrCmdTableID = GAMEDATA_GetFieldScriptCommandOverlayID( p_gamedata );

  //オーバーレイが読み込まれているならば
  //指定されてオーバーレイを解放
  if( scrCmdTableID == 0 )
  {
    return;
  }
  if( ScrCmd_Overlay_table[ scrCmdTableID ].sub_overlayID != GFL_OVERLAY_BLANK_ID )
  {
    SCRCMD_OV_Printf( "スクリプトコマンドオーバーレイ破棄【サブ】=%d\n", scrCmdTableID );
    GFL_OVERLAY_Unload( ScrCmd_Overlay_table[ scrCmdTableID ].sub_overlayID );
  }
  if( ScrCmd_Overlay_table[ scrCmdTableID ].overlayID != GFL_OVERLAY_BLANK_ID )
  {
    SCRCMD_OV_Printf( "スクリプトコマンドオーバーレイ破棄=%d\n", scrCmdTableID );
    GFL_OVERLAY_Unload( ScrCmd_Overlay_table[ scrCmdTableID ].overlayID );
  }

  //読込み終了したので、IDを消去
  GAMEDATA_SetFieldScriptCommandOverlayID( p_gamedata, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  読み込んだテーブルを取得
 *	@retval 読み込んだテーブルのアドレス  NULLならば読み込んでいない
 */
//-----------------------------------------------------------------------------
const VMCMD_FUNC *VMCMD_OVERLAY_GetTable( const GAMEDATA  *cp_gamedata )
{
  u32 scrCmdTableID = GAMEDATA_GetFieldScriptCommandOverlayID( cp_gamedata );

  //オーバーレイが読み込まれているならば、
  //指定されたテーブルを返す
  if( scrCmdTableID == 0 )
  {
    return NULL;
  }
  else
  {
    return ScrCmd_Overlay_table[ scrCmdTableID ].scriptCmdTbl;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  読み込んだテーブルの要素数を取得
 *	@retval 読み込んだテーブルの要素数  0ならば読み込んでいない
 */
//-----------------------------------------------------------------------------
u32 VMCMD_OVERLAY_GetTableMax( const GAMEDATA  *cp_gamedata )
{
  u32 scrCmdTableID = GAMEDATA_GetFieldScriptCommandOverlayID( cp_gamedata );

  //オーバーレイが読み込まれているならば、
  //番兵法にてテーブルの要素数をカウントし返す
  if( scrCmdTableID == 0
      || VMCMD_OVERLAY_GetTable( cp_gamedata ) == NULL )
  {
    return 0;
  }
  else
  {
    int i;
    for( i = 0; (u32)(VMCMD_OVERLAY_GetTable( cp_gamedata )[i]) != 0xFFFFFFFF; i++ )
    {
      /* count */
    }
    return i;
  }
}
