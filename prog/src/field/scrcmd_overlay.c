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
#include "scrcmd_un_table.h"
#include "scrcmd_pl_boat_table.h"
#include "scrcmd_fourkings_table.h"
#include "scrcmd_palpark_table.h"
#include "scrcmd_seatemple_table.h"
#include "scrcmd_wbt_table.h"
#include "scrcmd_resort_table.h"

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define SCRCMD_OV_Printf(...) NAGI_Printf(__VA_ARGS__)
#endif //PM_DEBUG

//-------------------------------------
///	オーバーレイエクスターン
//=====================================
FS_EXTERN_OVERLAY( scrcmd_bsubway );
FS_EXTERN_OVERLAY( scrcmd_un );
FS_EXTERN_OVERLAY( scrcmd_pl_boat );
FS_EXTERN_OVERLAY( scrcmd_fourkings );
FS_EXTERN_OVERLAY( scrcmd_palpark );
FS_EXTERN_OVERLAY( scrcmd_seatemple );
FS_EXTERN_OVERLAY( scrcmd_wbt );
FS_EXTERN_OVERLAY( scrcmd_resort );

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
///	国連を読込むゾーンID
//=====================================
static const ZONEID UseZoneIDTbl_un[] =
{
  ZONE_ID_C11,
  ZONE_ID_C11R0101,
  ZONE_ID_C11R0201,
  ZONE_ID_C03P14,     //国連行き船で使用
  ZONE_ID_C02R0701,   //性格を設定
  ZONE_ID_C03R0101,    //趣味を設定
  ZONE_ID_C03PC0101,  //国コードをセットしているかどうか
};

//-------------------------------------
///	遊覧船
//=====================================
static const ZONEID UseZoneIDTbl_pl_boat[] =
{
  ZONE_ID_C03R0801,
};

//-------------------------------------
///	四天王部屋
//=====================================
static const ZONEID UseZoneIDTbl_fourkings[]  =
{
  ZONE_ID_C09R0101,
  ZONE_ID_C09R0201,
  ZONE_ID_C09R0301,
  ZONE_ID_C09R0401,
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

static const ZONEID UseZoneIDTbl_wbt[] = 
{
  ZONE_ID_D05R0101,
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
} ScrCmd_Overlay_table[] =
{
  //NONE
  {
    NULL,
    NULL,
    0,
  },
  //BSUBWAY
  {
    ScriptCmdTbl_bsubway,
    UseZoneIDTbl_bsubway,
    NELEMS( UseZoneIDTbl_bsubway ),
    FS_OVERLAY_ID( scrcmd_bsubway ),
  },
  //UN
  {
    ScriptCmdTbl_un,
    UseZoneIDTbl_un,
    NELEMS( UseZoneIDTbl_un ),
    FS_OVERLAY_ID( scrcmd_un ),
  },
  //PL_BOAT
  {
    ScriptCmdTbl_pl_boat,
    UseZoneIDTbl_pl_boat,
    NELEMS(UseZoneIDTbl_pl_boat),
    FS_OVERLAY_ID( scrcmd_pl_boat ),
  },
  //FOURKINGS
  {
    ScriptCmdTbl_fourkings,
    UseZoneIDTbl_fourkings,
    NELEMS( UseZoneIDTbl_fourkings ),
    FS_OVERLAY_ID( scrcmd_fourkings ),
  },
  //PALPARK
  {
    ScriptCmdTbl_palpark,
    UseZoneIDTbl_palpark,
    NELEMS( UseZoneIDTbl_palpark ),
    FS_OVERLAY_ID( scrcmd_palpark ),
  },
  //SEATEMPLE
  {
    ScriptCmdTbl_seatemple,
    UseZoneIDTbl_seatemple,
    NELEMS(UseZoneIDTbl_seatemple),
    FS_OVERLAY_ID( scrcmd_seatemple ),
  },
  {
    ScriptCmdTbl_wbt,
    UseZoneIDTbl_wbt,
    NELEMS(UseZoneIDTbl_wbt),
    FS_OVERLAY_ID( scrcmd_wbt ),
  },
  //RESONANCE_RESORT
  {
    ScriptCmdTbl_resort,
    UseZoneIDTbl_resort,
    NELEMS(UseZoneIDTbl_resort),
    FS_OVERLAY_ID( scrcmd_resort ),
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
  
  if( ScrCmd_Overlay_table[ scrCmdTableID ].overlayID != NULL )
  {
    SCRCMD_OV_Printf( "スクリプトコマンドオーバーレイ読込み=%d\n", scrCmdTableID );
    GFL_OVERLAY_Load( ScrCmd_Overlay_table[ scrCmdTableID ].overlayID );
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
  if( ScrCmd_Overlay_table[ scrCmdTableID ].overlayID != NULL )
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
