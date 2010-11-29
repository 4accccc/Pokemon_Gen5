//////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   field_place_name.c
 * @brief  地名表示ウィンドウ
 * @author obata_toshihiro
 * @date   2009.07   
 */
//////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "field_place_name.h"
#include "place_name_letter.h"
#include "system/main.h"

#include "system/bmp_oam.h"
#include "print/wordset.h"
#include "field/zonedata.h"
#include "field/field_status.h"
#include "field/intrude_common.h"
#include "field/intrude_comm.h"
#include "field_comm/intrude_work.h"
#include "field_comm/intrude_main.h"
#include "savedata/situation.h"
#include "savedata/save_control.h"

#include "buflen.h"
#include "field_oam_pal.h"  // for FLDOAM_PALNO_PLACENAME

#include "arc/arc_def.h"
#include "arc/place_name.naix"
#include "arc/message.naix"
#include "msg/msg_place_name.h"

#include "../../../resource/fldmapdata/zonetable/zone_id.h"

#ifdef PM_DEBUG
#include "debug/debug_flg.h"
#endif 

#define BACKOBJ_MOVE_UD 2

//===================================================================================
// ■デバッグ
//=================================================================================== 
//#define DEBUG_PRINT_ON // デバッグ出力スイッチ
#define DEBUG_PRINT_TARGET (3) // デバッグ出力先

#ifdef PM_DEBUG
BOOL PlaceNameEnable    = TRUE; // 地名表示の有効フラグ
BOOL PlaceNameLong      = FALSE;  //地名表示長さチェックフラグ
u32 PlaceNameMoveType   = 0;  //動きの種類 0,上下同時1,上、下2下、上
BOOL PlaceNameShakeFlag = TRUE;  //動きの揺れ追加
BOOL PlaceNameAlphaFlag = FALSE;  //アルファの種類  
u32 PROCESS_TIME_FADEIN2  = 10; //入ってくるときの速度
u32 PROCESS_TIME_WAIT_LAUNCH  = 30; //待つときの時間
u32 PROCESS_TIME_WAIT_FADEOUT = 12; //出て行く時の速度
u32 ALPHA_VALUE_1             = (14); // 第1対象面のαブレンディング係数
u32 ALPHA_VALUE_2             = (8); // 第2対象面のαブレンディング係数
BOOL  PlaceNameVersionFlag    = 0;    //0がブラック１がホワイト
#endif

//===================================================================================
// ■ 定数・マクロ
//=================================================================================== 
#define MAX_STATE_COUNT (0xffff) // 状態カウンタ最大値
#define MAX_NAME_LENGTH (28)
// ↑パレス侵入中は侵入相手の名前を表示するため, 大きめに設定する.2010.05.11
// 現時点の最大長「ホワイトフォレスト」+ 名前5文字 = 14文字 
// ==> ローカライズ考慮で 28文字
#define MAX_BACK_OBJ  (2)//  背景OBJ数

//-----------
// 表示設定
//-----------
// OBJ
#define OBJ_CLUNIT_WK_NUM      (32)
// BG
#define BG_PALETTE_NO          (0) // BGパレット番号
#define BG_FRAME (GFL_BG_FRAME3_M) // 使用するBGフレーム
#define BG_FRAME_PRIORITY      (1) // BGフレームのプライオリティ
// αブレンド
#define ALPHA_PLANE_1 (GX_BLEND_PLANEMASK_BG3) // αブレンドの第1対象面
#define ALPHA_PLANE_2 (GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_OBJ) // αブレンドの第2対象面
//#define ALPHA_VALUE_1                     (14) // 第1対象面のαブレンディング係数
//#define ALPHA_VALUE_2                      (8) // 第2対象面のαブレンディング係数
#define CHAR_SIZE                    (8) // 1キャラ = 8ドット

//-----------
// 動作設定
//-----------
// 各状態の動作設定
#define PROCESS_TIME_FADEIN1  (7)
//#define PROCESS_TIME_FADEIN2  (10)
//#define PROCESS_TIME_WAIT_LAUNCH   (30)
//#define PROCESS_TIME_WAIT_FADEOUT   (12)
#define PROCESS_TIME_FADEOUT (5)
static int  PROCESS_TIME_FADEIN = PROCESS_TIME_FADEIN1;

//--------------
// インデックス
//--------------
// パレットリソース
typedef enum {
	PLTT_RES_PLATE,	// プレートオブジェクトで使用するパレット
	PLTT_RES_NUM      // 総数
} PLTT_RES_INDEX;

// セルアクターユニット
typedef enum {
	CLUNIT_PLATE, // プレートオブジェクト
	CLUNIT_NUM,    // 総数
} CLUNIT_INDEX;

// システムの状態
typedef enum {
	SYSTEM_STATE_HIDE,			   // 非表示
  SYSTEM_STATE_SETUP,        // 準備
	SYSTEM_STATE_FADEIN,	     // フェード・イン
	SYSTEM_STATE_WAIT_LAUNCH,	 // 発射待ち
	SYSTEM_STATE_LAUNCH,		   // 文字発射
	SYSTEM_STATE_WAIT_FADEOUT, // フェードアウト待ち
	SYSTEM_STATE_FADEOUT,	     // フェード・アウト
	SYSTEM_STATE_NUM,          // 総数
} SYSTEM_STATE; 

//プレートオブジェクト
typedef enum
{
  PLATE_TOP,
  PLATE_BOTTOM,
  PLATE_MAX,
} PLATE_INDEX;

// BG コントロール
static const GFL_BG_BGCNT_HEADER BGCntHeader = 
{
  0, 0,					          // 初期表示位置
  0x800,						      // スクリーンバッファサイズ
  0,							        // スクリーンバッファオフセット
  GFL_BG_SCRSIZ_256x256,	// スクリーンサイズ
  GX_BG_COLORMODE_16,			// カラーモード
  GX_BG_SCRBASE_0x1000,		// スクリーンベースブロック
  GX_BG_CHARBASE_0x04000,	// キャラクタベースブロック
  GFL_BG_CHRSIZ_256x256,	// キャラクタエリアサイズ
  GX_BG_EXTPLTT_01,			  // BG拡張パレットスロット選択
  1,							        // 表示プライオリティー
  0,							        // エリアオーバーフラグ
  0,							        // DUMMY
  FALSE,						      // モザイク設定
}; 

// レンダラーのインデックス
typedef enum {
  RENDERER_PLATE, // プレートオブジェクト用
  RENDERER_NUM
} RENDERER_INDEX;

// セルアクターのレンダラー
static const GFL_REND_SURFACE_INIT RendererSurface[ RENDERER_NUM ] = 
{
  {
    0,                                  // サーフェース左上ｘ座標
    0,                                  // サーフェース左上ｙ座標
    256,                                // サーフェース幅
    192,                                // サーフェース高さ
    CLSYS_DRAW_MAIN,                    // サーフェースタイプ(CLSYS_DRAW_TYPE)
    CLSYS_REND_CULLING_TYPE_NOT_AFFINE, // カリングタイプ
  },
};
//===================================================================================
// ■地名表示システム
//===================================================================================
struct _FIELD_PLACE_NAME { 

  GAMESYS_WORK* gameSystem;
  GAMEDATA*     gameData;
  WORDSET*      wordset;
	HEAPID        heapID;
  ARCHANDLE*    arcHandle;
	GFL_MSGDATA*  message;
  GFL_FONT*     font;

	// 地名・季節
	STRBUF* nameBuf; // 表示中の地名文字列
	u8      nameLen; // 表示中の地名文字数
  u8      seasonID;
  u16     zoneID;

	// 帯
  u32           nullCharPos; // NULLキャラクタのキャラNo.

	// OBJ
  u32             resChar;
  u32             resCell;
	u32             resPltt[ PLTT_RES_NUM ]; // パレットリソース
	BMPOAM_SYS_PTR  bmpOamSys;               // BMPOAMシステム
	GFL_CLUNIT*     clunit[ CLUNIT_NUM ];    // セルアクターユニット
  GFL_CLSYS_REND* renderer;                // レンダラーシステム

  // システム状態
	SYSTEM_STATE state;	
  u8           stateSeq;   // 状態内シーケンス番号
	u16	         stateCount; // 状態カウンタ

  // 表示するゾーン
  u16  prevZoneID;    // 前回のゾーンID
  u16  nowZoneID;     // 現在のゾーンID
  BOOL dispFlag;      // 表示フラグ
  BOOL forceDispFlag; // 強制表示フラグ

  //プレート
  PN_PLATE* plate[PLATE_MAX];

  // BG パレット
  void* BGPalBinary; // バイナリデータ
	NNSG2dPaletteData* BGPalData; // バイナリ展開データ
};


//===================================================================================
// ■prototype
//===================================================================================
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// BG
static void SetupBG( FIELD_PLACE_NAME* system );
static void CleanBG( FIELD_PLACE_NAME* system );
static void ResetBG( FIELD_PLACE_NAME* system );	
static void LoadBGPaletteData( FIELD_PLACE_NAME* system, ARCDATID pltID ); // BG パレットデータを読み込む
static void UnloadBGPaletteData( FIELD_PLACE_NAME* system ); // BG パレットデータを解放する
static void TransBGPalette( FIELD_PLACE_NAME* system ); // BG パレットを転送する
static void AllocBGNullCharArea( FIELD_PLACE_NAME* system ); // 空白キャラエリアを確保する
static void FreeBGNullCharaArea( FIELD_PLACE_NAME* system ); // 空白キャラエリアを解放する
// OBJ
static void CreateClactRenderer( FIELD_PLACE_NAME* system ); // セルアクターのレンダラーを作成する
static void DeleteClactRenderer( FIELD_PLACE_NAME* system ); // セルアクターのレンダラーを破棄する
static void LoadOBJResource( FIELD_PLACE_NAME* system ); // OBJ リソースを読み込む
static void UnloadOBJResource( FIELD_PLACE_NAME* system ); // OBJ リソースを解放する
static void CreateBmpOamSystem( FIELD_PLACE_NAME* system ); // BMPOAM システムを生成する
static void DeleteBmpOamSystem( FIELD_PLACE_NAME* system ); // BMPOAM システムを破棄する
static void CreateClactUnit( FIELD_PLACE_NAME* system ); // セルアクターユニットを生成する
static void DeleteClactUnit( FIELD_PLACE_NAME* system ); // セルアクターユニットを破棄する
static void SetUserRenderer( FIELD_PLACE_NAME* system, CLUNIT_INDEX unitIdx ); // セルアクターユニットにレンダラーを設定する
// プレートオブジェクト
static void CreatePlate( FIELD_PLACE_NAME* system ); //プレートを作成する
static void DeletePlate( FIELD_PLACE_NAME* system ); //プレートを破棄する
static void SetupPlate( FIELD_PLACE_NAME* system ); //プレートをセットアップする
static void HidePlate( FIELD_PLACE_NAME* system ); //プレート全体を非表示にする
static void MovePlate( FIELD_PLACE_NAME* system ); //プレートオブジェクトを動かす
static BOOL CheckPlate( FIELD_PLACE_NAME* system ); //プレートの動作をチェック
static void EnterPlate( FIELD_PLACE_NAME* system ); //プレード出現開始
static void OutPlate( FIELD_PLACE_NAME* system ); //プレード出現開始
// 状態の変更
static void ChangeState( FIELD_PLACE_NAME* system, SYSTEM_STATE next_state );
static void Cancel( FIELD_PLACE_NAME* system ); 
// 各状態時の動作
static void Process_HIDE( FIELD_PLACE_NAME* system );
static void Process_SETUP( FIELD_PLACE_NAME* system );
static void Process_FADEIN_Both( FIELD_PLACE_NAME* system );
static void Process_FADEIN_UpDown( FIELD_PLACE_NAME* system );
static void Process_FADEIN_DownUp( FIELD_PLACE_NAME* system );
static void Process_WAIT_LAUNCH( FIELD_PLACE_NAME* system );
static void Process_LAUNCH( FIELD_PLACE_NAME* system );
static void Process_WAIT_FADEOUT( FIELD_PLACE_NAME* system );
static void Process_FADEOUT( FIELD_PLACE_NAME* system ); 
// 各状態時の描画処理
static void Draw_HIDE( const FIELD_PLACE_NAME* system );
static void Draw_SETUP( const FIELD_PLACE_NAME* system );
static void Draw_FADEIN( const FIELD_PLACE_NAME* system );
static void Draw_WAIT_LAUNCH( const FIELD_PLACE_NAME* system );
static void Draw_LAUNCH( const FIELD_PLACE_NAME* system );
static void Draw_WAIT_FADEOUT( const FIELD_PLACE_NAME* system );
static void Draw_FADEOUT( const FIELD_PLACE_NAME* system );
//-----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------- 
// システム
static FIELD_PLACE_NAME* CreatePNSystem( HEAPID heapID ); // 地名表示システムを生成する
static void InitPNSystem( FIELD_PLACE_NAME* system, GAMESYS_WORK* gameSystem ); // 地名表示システムを初期化する
static void DeletePNSystem( FIELD_PLACE_NAME* system ); // 地名表示システムを破棄する
// 生成
static void CreateWordset( FIELD_PLACE_NAME* system ); // ワードセットを生成する
static void CreateMessageData( FIELD_PLACE_NAME* system ); // メッセージデータを生成する
static void CreatePlaceNameBuffer( FIELD_PLACE_NAME* system ); // 地名バッファを生成する
static void OpenDataHandle( FIELD_PLACE_NAME* system ); // アーカイブデータハンドルを開く
// 破棄
static void DeleteWordset( FIELD_PLACE_NAME* system ); // ワードセットを破棄する
static void DeleteMessageData( FIELD_PLACE_NAME* system ); // メッセージデータを破棄する
static void DeletePlaceNameBuffer( FIELD_PLACE_NAME* system ); // 地名バッファを破棄する
static void CloseDataHandle( FIELD_PLACE_NAME* system ); // アーカイブデータハンドルを閉じる
// 各種ゲッター関数
static HEAPID GetHeapID( const FIELD_PLACE_NAME* system ); // ヒープIDを取得する
static ARCHANDLE* GetArcHandle( const FIELD_PLACE_NAME* system ); // アーカイブデータハンドルを取得する
static GFL_FONT* GetFont( const FIELD_PLACE_NAME* system ); // フォントを取得する
static GFL_MSGDATA* GetMessageData( const FIELD_PLACE_NAME* system ); // メッセージデータを取得する
// 各種セッター関数
static void SetHeapID( FIELD_PLACE_NAME* system, HEAPID heapID ); // ヒープIDを設定する
static void SetFont( FIELD_PLACE_NAME* system, GFL_FONT* font ); // フォントを設定する
// システム状態
static void SetState( FIELD_PLACE_NAME* system, SYSTEM_STATE state ); // システムの状態を設定する
static SYSTEM_STATE GetState( const FIELD_PLACE_NAME* system ); // システムの状態を取得する
static u16 GetStateCount( const FIELD_PLACE_NAME* system ); // 状態カウンタを取得する
static void ResetStateCount( FIELD_PLACE_NAME* system ); // 状態カウンタをリセットする
static void IncStateCount( FIELD_PLACE_NAME* system ); // 状態カウンタを更新する
static u8 GetStateSeq( const FIELD_PLACE_NAME* system ); // 状態内シーケンスを取得する
static void ResetStateSeq( FIELD_PLACE_NAME* system ); // 状態内シーケンスをリセットする
static void IncStateSeq( FIELD_PLACE_NAME* system ); // 状態内シーケンスをインクリメントする
// 地名
static void LoadNowZoneID( FIELD_PLACE_NAME* system ); // 現在のゾーンIDを読み込む
static void SaveNowZoneID( const FIELD_PLACE_NAME* system ); // 現在のゾーンIDを記録する
static u16 GetPrevZoneID( const FIELD_PLACE_NAME* system ); // 前回のゾーンIDを取得する
static u16 GetNowZoneID( const FIELD_PLACE_NAME* system ); // 現在のゾーンIDを取得する
static void SetNowZoneID( FIELD_PLACE_NAME* system, u16 zoneID ); // 現在のゾーンIDを設定する
static const STRBUF* GetPlaceName( const FIELD_PLACE_NAME* system ); // 地名バッファを取得する
static u8 GetPlaceNameLength( const FIELD_PLACE_NAME* system ); // 地名の文字数を取得する
// 地名の更新
static BOOL CheckForceDispFlag( const FIELD_PLACE_NAME* system ); // 強制表示フラグをチェックする
static void SetForceDispFlag( FIELD_PLACE_NAME* system ); // 強制表示フラグを立てる
static void ResetForceDispFlag( FIELD_PLACE_NAME* system ); // 強制表示フラグを落とす
static BOOL CheckDispFlag( const FIELD_PLACE_NAME* system ); // 表示フラグをチェックする
static void SetDispFlag( FIELD_PLACE_NAME* system ); // 表示フラグをセットする
static void ResetDispFlag( FIELD_PLACE_NAME* system ); // 表示フラグを落とす
static BOOL CheckPlaceNameIDChange( const FIELD_PLACE_NAME* system ); // 地名が変化したかどうかをチェックする
static void UpdatePlaceName( FIELD_PLACE_NAME* system ); // 表示する地名を更新する
static BOOL CheckIntrudeField( const FIELD_PLACE_NAME* system, u16 zoneID ); // 侵入先かどうかをチェックする
// ユーティリティ
static BOOL CheckPlaceNameFlag( u16 zoneID, BOOL isSaveRecovery ); // 地名表示フラグを取得する
// デバッグ
//#ifdef DEBUG_PRINT_ON
static void DEBUG_PrintSystemState( const FIELD_PLACE_NAME* system ); // システム状態名を出力する
//#endif


//====================================================================================
// ■作成・破棄
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを作成する
 *
 * @param gameSystem
 * @param heapID     使用するヒープID
 * @param msgbg      使用するメッセージ表示システム
 *
 * @return 生成したシステム
 */
//------------------------------------------------------------------------------------
FIELD_PLACE_NAME* FIELD_PLACE_NAME_Create( GAMESYS_WORK* gameSystem, HEAPID heapID, FLDMSGBG* msgbg )
{
	FIELD_PLACE_NAME* system;

	system = CreatePNSystem( heapID );

  InitPNSystem( system, gameSystem );
  LoadNowZoneID( system );
  OpenDataHandle( system );
  CreateWordset( system );
  CreateMessageData( system );
  CreatePlaceNameBuffer( system );
  SetHeapID( system, heapID );
  SetFont( system, FLDMSGBG_GetFontHandle( msgbg ) );

  CreateClactRenderer( system );
	LoadOBJResource( system );
	CreateClactUnit( system );
  SetUserRenderer( system, CLUNIT_PLATE );
  CreateBmpOamSystem( system ); 
  CreatePlate( system );
  GFL_BG_SetBGControl( BG_FRAME, &BGCntHeader, GFL_BG_MODE_TEXT ); 
  LoadBGPaletteData( system, NARC_place_name_place_name_back_NCLR );
  AllocBGNullCharArea( system );

	ChangeState( system, SYSTEM_STATE_HIDE ); 
	return system;
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを破棄する
 *
 * @param system
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Delete( FIELD_PLACE_NAME* system )
{
  SaveNowZoneID( system );

  UnloadBGPaletteData( system ); 

  DeletePlate( system );
  DeletePlaceNameBuffer( system ); 
  DeleteBmpOamSystem( system ); 
  DeleteClactUnit( system );
  UnloadOBJResource( system ); 
  DeleteClactRenderer( system );
  DeleteWordset( system );
  DeleteMessageData( system ); 
  FreeBGNullCharaArea( system ); 
  CloseDataHandle( system );

	GFL_BG_FreeBGControl( BG_FRAME ); 

  DeletePNSystem( system );
} 


//====================================================================================
// ■動作
//====================================================================================

//#define TICK_TEST
//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムの動作処理
 *
 * @param system
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Process( FIELD_PLACE_NAME* system )
{ 
#ifdef TICK_TEST
  OSTick start, end;
#endif

#ifdef PM_DEBUG
  if( PlaceNameEnable == FALSE ) { return; }
#endif

  IncStateCount( system ); // 状態カウンタを更新

  //DEBUG_PrintSystemState( system );
#ifdef TICK_TEST
  OS_TFPrintf( 3, "[%d]: ", GetStateSeq(system) );
  start = OS_GetTick();
#endif

#ifdef PM_DEBUG
  {
    if( PlaceNameShakeFlag )
    {
      PROCESS_TIME_FADEIN = PROCESS_TIME_FADEIN2;
    }
    else
    {
      PROCESS_TIME_FADEIN = PROCESS_TIME_FADEIN1;
    }
  }

  if( PlaceNameAlphaFlag )
  {
    G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, ALPHA_VALUE_1, ALPHA_VALUE_2 );
  }
  else
  {
    G2_BlendNone();
  }
#endif 

  {
    static void (*Process_FADEIN[])( FIELD_PLACE_NAME* system ) =
    {
      Process_FADEIN_Both,
      Process_FADEIN_UpDown,
      Process_FADEIN_DownUp,
    };
    // 状態に応じた動作
    switch( GetState(system) ) {
    case SYSTEM_STATE_HIDE:         Process_HIDE( system );		      break;
    case SYSTEM_STATE_SETUP:        Process_SETUP( system );		    break;
    case SYSTEM_STATE_FADEIN:       Process_FADEIN[PlaceNameMoveType%NELEMS(Process_FADEIN)]( system );	      break;
    case SYSTEM_STATE_WAIT_LAUNCH:  Process_WAIT_LAUNCH( system );	break;
    case SYSTEM_STATE_LAUNCH:       Process_LAUNCH( system );	      break;
    case SYSTEM_STATE_WAIT_FADEOUT: Process_WAIT_FADEOUT( system );	break;
    case SYSTEM_STATE_FADEOUT:      Process_FADEOUT( system );	    break;
    }
  }

	// プレートオブジェクトを動かす
  MovePlate( system );

#ifdef TICK_TEST
  end = OS_GetTick();
  OS_TFPrintf( 3, "tick = %d, micro sec = %d\n", 
      (u32)(end - start),
      (u32)OS_TicksToMicroSeconds( (end - start) ) );
#endif
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示ウィンドウの描画処理
 *
 * @param system
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Draw( const FIELD_PLACE_NAME* system )
{ 
	switch( GetState(system) ) {
  case SYSTEM_STATE_HIDE:		      Draw_HIDE( system );		     break;
  case SYSTEM_STATE_SETUP:		    Draw_SETUP( system );		     break;
  case SYSTEM_STATE_FADEIN:		    Draw_FADEIN( system );	     break;
  case SYSTEM_STATE_WAIT_LAUNCH:	Draw_WAIT_LAUNCH( system );	 break;
  case SYSTEM_STATE_LAUNCH:		    Draw_LAUNCH( system );	     break;
  case SYSTEM_STATE_WAIT_FADEOUT:	Draw_WAIT_FADEOUT( system ); break;
  case SYSTEM_STATE_FADEOUT:	    Draw_FADEOUT( system );	     break;
	} 
}


//====================================================================================
// ■制御
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 基本のルールに従って, 地名を表示する
 *
 * @param system
 * @param zoneID 地名を表示する場所のゾーンID
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_DisplayOnStanderdRule( FIELD_PLACE_NAME* system, u32 zoneID )
{ 
  // リクエストのあったゾーンIDを更新
  SetNowZoneID( system, zoneID ); 

  // 地名表示フラグが立っていない場所では表示しない
  if( CheckPlaceNameFlag( zoneID, FALSE ) == FALSE ) { return; }

  // 表示中のウィンドウを退出させる
	Cancel( system ); 

  // 表示フラグを立てる
  SetDispFlag( system );
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示フラグに従い, 地名を表示する
 *
 * @param system
 * @param zoneID 地名を表示する場所のゾーンID
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_DisplayOnPlaceNameFlag( FIELD_PLACE_NAME* system, u32 zoneID )
{
  // リクエストのあったゾーンIDを更新
  SetNowZoneID( system, zoneID ); 

  // 地名表示フラグが立っていない場所では表示しない
  if( CheckPlaceNameFlag( zoneID, FALSE ) == FALSE ) { return; }

  // 表示中のウィンドウを退出させる
	Cancel( system ); 

  // 強制的に表示
  SetForceDispFlag( system ); 
}

//----------------------------------------------------------------------------
/**
 * @brief セーブからの復帰用 地名表示
 *
 * @param system
 * @param zoneID 地名を表示する場所のゾーンID
 */
//-----------------------------------------------------------------------------
void FIELD_PLACE_NAME_DisplayContinue( FIELD_PLACE_NAME* system, u32 zoneID )
{
  // リクエストのあったゾーンIDを更新
  SetNowZoneID( system, zoneID ); 

  // 地名表示フラグが立っていない場所では表示しない
  if( CheckPlaceNameFlag( zoneID, TRUE ) == FALSE ) { return; }

  // 表示中のウィンドウを退出させる
	Cancel( system ); 

  // 強制的に表示
  SetForceDispFlag( system ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief 強制的に地名を表示する
 *
 * @param system
 * @param zoneID 地名を表示する場所のゾーンID
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_DisplayForce( FIELD_PLACE_NAME* system, u32 zoneID )
{
  // リクエストのあったゾーンIDを更新
  SetNowZoneID( system, zoneID ); 

  // 表示中のウィンドウを退出させる
	Cancel( system ); 

  // 強制的に表示
  SetForceDispFlag( system ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名ウィンドウの表示を強制的に終了する
 *
 * @param system
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Hide( FIELD_PLACE_NAME* system )
{
	ChangeState( system, SYSTEM_STATE_HIDE );

  // 全ての表示リクエストを無効化
  ResetDispFlag( system );
  ResetForceDispFlag( system );
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示ＢＧ復帰
 *
 * @param system
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_RecoverBG( FIELD_PLACE_NAME* system )
{
  ResetBG( system );
}

#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ用にバージョン切り替え
 *
 *	@param	FIELD_PLACE_NAME* system 
 */
//-----------------------------------------------------------------------------
void FIELD_PLACE_NAME_DebugChangeVersion( FIELD_PLACE_NAME* system )
{
  UnloadBGPaletteData( system );
  if( PlaceNameVersionFlag == 1 )
  {
    GFL_ARCHDL_UTIL_TransVramBgCharacter( GetArcHandle(system), NARC_place_name_place_name_back_w_NCGR, BG_FRAME, 0, 0, FALSE, GetHeapID(system) );
    GFL_ARCHDL_UTIL_TransVramScreen( GetArcHandle(system), NARC_place_name_place_name_back_w_NSCR, BG_FRAME, 0, 0, FALSE, GetHeapID(system));

    LoadBGPaletteData( system, NARC_place_name_place_name_back_w_NCLR );
  }
  else
  {
    GFL_ARCHDL_UTIL_TransVramBgCharacter( GetArcHandle(system), NARC_place_name_place_name_back_NCGR, BG_FRAME, 0, 0, FALSE, GetHeapID(system) );
    GFL_ARCHDL_UTIL_TransVramScreen( GetArcHandle(system), NARC_place_name_place_name_back_NSCR, BG_FRAME, 0, 0, FALSE, GetHeapID(system));
    
    LoadBGPaletteData( system, NARC_place_name_place_name_back_NCLR );
  }

  TransBGPalette( system );
}
#endif


//===================================================================================
// ■非公開関数
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief BGのセットアップ
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void SetupBG( FIELD_PLACE_NAME* system )
{
	TransBGPalette( system ); 
  AllocBGNullCharArea( system );

  // BG表示設定
	GFL_BG_SetPriority( BG_FRAME, BG_FRAME_PRIORITY );
	GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF ); 
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, ALPHA_VALUE_1, ALPHA_VALUE_2 );
}

//-----------------------------------------------------------------------------------
/**
 * @brief BGのクリーンアップ処理
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void CleanBG( FIELD_PLACE_NAME* system )
{
	// キャラVRAM・スクリーンVRAMをクリア
	GFL_BG_ClearFrame( BG_FRAME );

  FreeBGNullCharaArea( system ); 
}

//-----------------------------------------------------------------------------------
/**
 * @brief BGの再設定
 *
 * @param system 地名表示システム
 */ 
//-----------------------------------------------------------------------------------
static void ResetBG( FIELD_PLACE_NAME* system )
{ 
  CleanBG( system );
  SetupBG( system );
}

//------------------------------------------------------------------------------------
/**
 * @brief BG パレットデータを読み込む
 *
 * @param system
 */
//------------------------------------------------------------------------------------
static void LoadBGPaletteData( FIELD_PLACE_NAME* system, ARCDATID pltID )
{
  ARCHANDLE* handle;
  HEAPID heapID;
  u32 size;

  GF_ASSERT( system->BGPalBinary == NULL );
  GF_ASSERT( system->BGPalData == NULL );

  heapID = GetHeapID( system ); 
  handle = GetArcHandle( system );

  size = GFL_ARC_GetDataSizeByHandle( handle, pltID ); // データサイズ取得
  system->BGPalBinary = GFL_HEAP_AllocMemory( heapID, size );	// データバッファ確保
  GFL_ARC_LoadDataByHandle( handle, pltID, system->BGPalBinary ); // データ取得
  NNS_G2dGetUnpackedPaletteData( system->BGPalBinary, &system->BGPalData ); // バイナリを展開

}

//------------------------------------------------------------------------------------
/**
 * @brief BG パレットデータを解放する
 *
 * @param system
 */
//------------------------------------------------------------------------------------
static void UnloadBGPaletteData( FIELD_PLACE_NAME* system )
{
  if( system->BGPalBinary ) {
    GFL_HEAP_FreeMemory( system->BGPalBinary );
    system->BGPalBinary = NULL;
    system->BGPalData = NULL;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief BG パレットを転送する
 *
 * @param system
 */
//------------------------------------------------------------------------------------
static void TransBGPalette( FIELD_PLACE_NAME* system )
{
  GFL_BG_LoadPalette( 
      BG_FRAME, system->BGPalData->pRawData, 0x20, BG_PALETTE_NO );
}

//------------------------------------------------------------------------------------
/**
 * @brief 空白キャラエリアを確保する
 *
 * @param system
 */
//------------------------------------------------------------------------------------
static void AllocBGNullCharArea( FIELD_PLACE_NAME* system )
{
  // 未確保
  if( system->nullCharPos == AREAMAN_POS_NOTFOUND ) {
    system->nullCharPos = 
      GFL_BG_AllocCharacterArea( 
          BG_FRAME, CHAR_SIZE * CHAR_SIZE / 2, GFL_BG_CHRAREA_GET_F );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 空白キャラエリアを解放する
 *
 * @param system
 */
//------------------------------------------------------------------------------------
static void FreeBGNullCharaArea( FIELD_PLACE_NAME* system )
{
  // 確保済み
  if( system->nullCharPos != AREAMAN_POS_NOTFOUND ) {
    GFL_BG_FreeCharacterArea( 
        BG_FRAME, system->nullCharPos, CHAR_SIZE * CHAR_SIZE / 2 );
    system->nullCharPos = AREAMAN_POS_NOTFOUND;
  }
}


//-----------------------------------------------------------------------------------
/**
 * @brief セルアクターのレンダラーを作成する
 *
 * @param system 
 */
//-----------------------------------------------------------------------------------
static void CreateClactRenderer( FIELD_PLACE_NAME* system )
{
  GF_ASSERT( system->renderer == NULL );

  system->renderer = GFL_CLACT_USERREND_Create( 
      RendererSurface, NELEMS(RendererSurface), system->heapID );
}

//-----------------------------------------------------------------------------------
/**
 * @brief セルアクターのレンダラーを破棄する
 *
 * @param system 
 */
//-----------------------------------------------------------------------------------
static void DeleteClactRenderer( FIELD_PLACE_NAME* system )
{
  GF_ASSERT( system->renderer );

  if( system->renderer ) {
    GFL_CLACT_USERREND_Delete( system->renderer );
    system->renderer = NULL;
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief OBJ リソースを読み込む
 *
 * @param system 
 */
//-----------------------------------------------------------------------------------
static void LoadOBJResource( FIELD_PLACE_NAME* system )
{
  HEAPID heapID;
	ARCHANDLE* handle;

  heapID = GetHeapID( system );
	handle = GetArcHandle( system );

	system->resPltt[ PLTT_RES_PLATE ] = 
		GFL_CLGRP_PLTT_RegisterEx( 
				handle, NARC_place_name_place_name_string_NCLR,
				CLSYS_DRAW_MAIN, FLDOAM_PALNO_PLACENAME * 32, 0, 1, heapID );

	system->resChar = GFL_CLGRP_CGR_Register( 
      handle, NARC_place_name_place_name_obj_NCGR, 
      FALSE, CLSYS_DRAW_MAIN, heapID );

	system->resCell = GFL_CLGRP_CELLANIM_Register( 
      handle, NARC_place_name_place_name_obj_NCER, 
      NARC_place_name_place_name_obj_NANR, heapID );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: LoadOBJResource\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief OBJ リソースを解放する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void UnloadOBJResource( FIELD_PLACE_NAME* system )
{
  int i;

  GFL_CLGRP_CGR_Release( system->resChar );
  GFL_CLGRP_CELLANIM_Release( system->resCell );
	for( i=0; i<PLTT_RES_NUM; i++ )
	{
		GFL_CLGRP_PLTT_Release( system->resPltt[i] );
	}

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: UnloadOBJResource\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを生成する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void CreateClactUnit( FIELD_PLACE_NAME* system )
{
  HEAPID heapID;

  heapID = GetHeapID( system );
	system->clunit[ CLUNIT_PLATE ] = 
		GFL_CLACT_UNIT_Create( OBJ_CLUNIT_WK_NUM, BG_FRAME_PRIORITY, heapID );

	// 初期設定
	GFL_CLACT_UNIT_SetDrawEnable( system->clunit[ CLUNIT_PLATE ], TRUE );
  
#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CreateClactUnit\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DeleteClactUnit( FIELD_PLACE_NAME* system )
{
  int i;

	for( i=0; i<CLUNIT_NUM; i++ )
	{
		GFL_CLACT_UNIT_Delete( system->clunit[i] );
	}

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: DeleteClactUnit\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットにレンダラーを設定する
 *
 * @param system
 * @param unitIdx   ユニット
 */
//-----------------------------------------------------------------------------------
static void SetUserRenderer( FIELD_PLACE_NAME* system, CLUNIT_INDEX unitIdx )
{
  if( system->renderer ) {
    GFL_CLACT_UNIT_SetUserRend( system->clunit[ unitIdx ], system->renderer );
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: SetUserRenderer\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief アーカイブデータハンドルを開く
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void OpenDataHandle( FIELD_PLACE_NAME* system )
{
  HEAPID heapID;

  GF_ASSERT( system->arcHandle == NULL );

  heapID = GetHeapID( system );
  system->arcHandle = GFL_ARC_OpenDataHandle( ARCID_PLACE_NAME, heapID );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: OpenDataHandle\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief アーカイブデータハンドルを閉じる
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void CloseDataHandle( FIELD_PLACE_NAME* system )
{
  if( system->arcHandle ) {
    GFL_ARC_CloseDataHandle( system->arcHandle );
    system->arcHandle = NULL;
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CloseDataHandle\n" );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレートを破棄する
 *
 *	@param	FIELD_PLACE_NAME* system 
 */
//-----------------------------------------------------------------------------
static void CreatePlate( FIELD_PLACE_NAME* system )
{
  {
    PN_PLATE_TOP_INIT_PARAM param;
    GF_ASSERT( system->plate[PLATE_TOP] == NULL );

    GFL_STD_MemClear( &param, sizeof(PN_PLATE_TOP_INIT_PARAM ) );
    param.bmpOamSys       = system->bmpOamSys;
    param.fontPlttRegIdx  = system->resPltt[ PLTT_RES_PLATE ];

    system->plate[PLATE_TOP]  = PN_PLATE_Create(
     GetHeapID( system ), 
     PN_PLATE_TYPE_TOP, 
     &param, 
     system->clunit[ CLUNIT_PLATE ],
     GetArcHandle( system ),
     CLUNIT_PLATE,
     system->resPltt[ PLTT_RES_PLATE ],
     system->resChar,
     system->resCell
     );
  }
  {
    PN_PLATE_BOTTOM_INIT_PARAM param;
    GF_ASSERT( system->plate[PLATE_BOTTOM] == NULL );

    GFL_STD_MemClear( &param, sizeof(PN_PLATE_BOTTOM_INIT_PARAM ) );

    system->plate[PLATE_BOTTOM]  = PN_PLATE_Create(
     GetHeapID( system ), 
     PN_PLATE_TYPE_BOTTOM, 
     &param, 
     system->clunit[ CLUNIT_PLATE ],
     GetArcHandle( system ),
     CLUNIT_PLATE,
     system->resPltt[ PLTT_RES_PLATE ],
     system->resChar,
     system->resCell
     );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  プレートを破棄する
 *
 *	@param	FIELD_PLACE_NAME* system 
 */
//-----------------------------------------------------------------------------
static void DeletePlate( FIELD_PLACE_NAME* system )
{
  int i;
  for( i = 0; i < PLATE_MAX; i++ )
  {
    if( system->plate[ i ] != NULL )
    {
      PN_PLATE_Delete( system->plate[ i ] );
      system->plate[ i ]  = NULL;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  プレートをセットアップする
 *
 *	@param	FIELD_PLACE_NAME* system 
 */
//-----------------------------------------------------------------------------
static void SetupPlate( FIELD_PLACE_NAME* system )
{
  {
    PN_PLATE_TOP_SETUP_PARAM  setup;
    GFL_STD_MemClear( &setup, sizeof( PN_PLATE_TOP_SETUP_PARAM) );
    setup.font      = GetFont( system );
    setup.zoneID    = system->zoneID;
    setup.placeName = GetPlaceName( system );
    PN_PLATE_SetupReq( system->plate[ PLATE_TOP ], &setup );
  }
  {
    PN_PLATE_BOTTOM_SETUP_PARAM setup;
    GFL_STD_MemClear( &setup, sizeof( PN_PLATE_BOTTOM_SETUP_PARAM) );
    setup.seasonID    = system->seasonID;
    PN_PLATE_SetupReq( system->plate[ PLATE_BOTTOM ], &setup );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレートを非表示にする
 *
 *	@param	FIELD_PLACE_NAME* system 
 */
//-----------------------------------------------------------------------------
static void HidePlate( FIELD_PLACE_NAME* system )
{
  int i;
  for( i = 0; i < PLATE_MAX; i++ )
  {
    PN_PLATE_HideReq( system->plate[ i ] );
  }
  GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF );	// BGを非表示にする
}

//-----------------------------------------------------------------------------------
/**
 * @brief プレートオブジェクトを動かす
 *
 * @param system 
 */
//-----------------------------------------------------------------------------------
static void MovePlate( FIELD_PLACE_NAME* system )
{
  int i;
	for( i=0; i<PLATE_MAX; i++ )
	{
    PN_PLATE_Main( system->plate[i] );
	}
} 
//----------------------------------------------------------------------------
/**
 *	@brief  プレートの動作をチェック
 *
 *	@param	FIELD_PLACE_NAME* system 
 *
 *	@retval TRUE  動いていない
 *	@retval FALSE 動いている
 */
//-----------------------------------------------------------------------------
static BOOL CheckPlate( FIELD_PLACE_NAME* system )
{
  BOOL moveFinish = TRUE;
  int i;
  for( i=0; i<PLATE_MAX; i++ )
  {
    if( !PN_PLATE_IsEnd( system->plate[i] ) ) 
    {
      moveFinish = FALSE;
      break; 
    }
  }

  return moveFinish;
}
//----------------------------------------------------------------------------
/**
 *	@brief  出現開始
 *
 *	@param	FIELD_PLACE_NAME* system 
 */
//-----------------------------------------------------------------------------
static void EnterPlate( FIELD_PLACE_NAME* system )
{
  int i;
  for( i = 0; i < PLATE_MAX; i++ )
  {
    PN_PLATE_EnterReq( system->plate[ i ], system->stateCount, PROCESS_TIME_FADEIN );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  出て行く処理開始
 *
 *	@param	FIELD_PLACE_NAME* system 
 */
//-----------------------------------------------------------------------------
static void OutPlate( FIELD_PLACE_NAME* system )
{
  int i;
  for( i = 0; i < PLATE_MAX; i++ )
  {
    PN_PLATE_OutReq( system->plate[ i ], /*system->stateCount*/0, PROCESS_TIME_WAIT_FADEOUT );
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief BMPOAM システムを生成する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void CreateBmpOamSystem( FIELD_PLACE_NAME* system )
{
  HEAPID heapID;

  GF_ASSERT( system->bmpOamSys == NULL );

  heapID = GetHeapID( system );
	system->bmpOamSys = BmpOam_Init( heapID, system->clunit[ CLUNIT_PLATE ] );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CreateBmpOamSystem\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief BMPOAM システムを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DeleteBmpOamSystem( FIELD_PLACE_NAME* system )
{
  if( system->bmpOamSys ) {
    BmpOam_Exit( system->bmpOamSys );
    system->bmpOamSys = NULL;
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: DeleteBmpOamSystem\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 強制表示フラグを立てる
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void SetForceDispFlag( FIELD_PLACE_NAME* system )
{
  system->forceDispFlag = TRUE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 強制表示フラグを落とす
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void ResetForceDispFlag( FIELD_PLACE_NAME* system )
{
  system->forceDispFlag = FALSE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 表示フラグをチェックする
 *
 * @param system
 *
 * @return 強制表示フラグが立っている場合 TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------
static BOOL CheckDispFlag( const FIELD_PLACE_NAME* system )
{
  return system->dispFlag;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 表示フラグを立てる
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void SetDispFlag( FIELD_PLACE_NAME* system )
{
  system->dispFlag = TRUE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 表示フラグを落とす
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void ResetDispFlag( FIELD_PLACE_NAME* system )
{
  system->dispFlag = FALSE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 強制表示フラグをチェックする
 *
 * @param system
 *
 * @return 強制表示フラグが立っている場合 TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------
static BOOL CheckForceDispFlag( const FIELD_PLACE_NAME* system )
{
  return system->forceDispFlag;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 地名が変化したかどうかをチェックする
 *
 * @param system
 *
 * @return 地名が変化した場合 TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------
static BOOL CheckPlaceNameIDChange( const FIELD_PLACE_NAME* system )
{
  u32 prevID = ZONEDATA_GetPlaceNameID( system->prevZoneID );
  u32 nowID  = ZONEDATA_GetPlaceNameID( system->nowZoneID );

  // 地名に変化がない
  if( prevID == nowID ) {
    return FALSE;
  }
  else {
    return TRUE;
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief 表示する地名を更新する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void UpdatePlaceName( FIELD_PLACE_NAME* system )
{
  u16 zoneID;
	u16 strID;
  
	// ゾーンIDから地名文字列を取得する
  zoneID = GetNowZoneID(system);
	strID = ZONEDATA_GetPlaceNameID( zoneID );

  // エラー回避
	if( (strID < 0) || (msg_place_name_max <= strID) ){ strID = 0; } 

  //「なぞのばしょ」なら表示しない
	if( strID == 0 ) { 
    OS_Printf( "「なぞのばしょ」を検出 ( zoneID = %d )\n", zoneID );
    FIELD_PLACE_NAME_Hide( system );
    return;
  }

  // 侵入先にいる
  if( CheckIntrudeField( system, zoneID ) ) 
  {
    // 侵入先のプレイヤー名を展開
    GAME_COMM_SYS_PTR gameComm = GAMESYSTEM_GetGameCommSysPtr( system->gameSystem );
    INTRUDE_COMM_SYS_PTR intrudeComm= Intrude_Check_CommConnect( gameComm );
    u8 intrudeNetID = Intrude_GetPalaceArea( system->gameData );
    MYSTATUS* status = Intrude_GetMyStatus( system->gameData, intrudeNetID );
    STRBUF* strbuf = GFL_MSG_CreateString( system->message, MAPNAME_INTRUDE );
    GFL_MSG_GetString( system->message,	strID, system->nameBuf );
    WORDSET_RegisterPlayerName( system->wordset, 0, status );
    WORDSET_RegisterPlaceName( system->wordset, 1, strID );
    WORDSET_ExpandStr( system->wordset, system->nameBuf, strbuf );
    GFL_STR_DeleteBuffer( strbuf );
    system->seasonID  = GAMEDATA_GetIntrudeSeasonID( system->gameData, intrudeNetID );
  }
  // 自分のフィールドにいる
  else {
    GFL_MSG_GetString( system->message,	strID, system->nameBuf );
    system->seasonID  = GAMEDATA_GetSeasonID( system->gameData );
  }
  system->nameLen = GFL_STR_GetBufferLength( system->nameBuf );
  system->zoneID  = zoneID;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 侵入先かどうかをチェックする
 *
 * @param system
 * @param zoneID
 *
 * @return 指定したゾーンが侵入先なら TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------
static BOOL CheckIntrudeField( const FIELD_PLACE_NAME* system, u16 zoneID )
{
  GAME_COMM_SYS_PTR gameComm;
  INTRUDE_COMM_SYS_PTR intrudeComm;
  FIELD_STATUS* fieldStatus;
  int myNetID;
  u8 intrudeNetID;

  gameComm = GAMESYSTEM_GetGameCommSysPtr( system->gameSystem );
  intrudeComm = Intrude_Check_CommConnect( gameComm );
  fieldStatus = GAMEDATA_GetFieldStatus( system->gameData );
  myNetID = GAMEDATA_GetIntrudeMyID( system->gameData );

  if( intrudeComm ) {
    intrudeNetID = Intrude_GetPalaceArea( system->gameData );

    // 他人のフィールドにいる
    if( FIELD_STATUS_GetMapMode( fieldStatus ) == MAPMODE_INTRUDE ) { 
      return TRUE;
    }
    // 他人のパレスにいる
    else if( ZONEDATA_IsPalace( zoneID ) && (myNetID != intrudeNetID) ) {
      return TRUE;
    }
  }

  return FALSE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param system
 * @param next_state 設定する状態
 */
//-----------------------------------------------------------------------------------
static void ChangeState( FIELD_PLACE_NAME* system, SYSTEM_STATE next_state )
{
  SetState( system, next_state ); // 状態を設定
  ResetStateCount( system );      // 状態カウンタをリセット
  ResetStateSeq( system );        // 状態内シーケンスをリセット

	// 遷移先の状態に応じた初期化
	switch( next_state ) {
		case SYSTEM_STATE_HIDE:	
			HidePlate( system );
			break;
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 強制的にウィンドウを退出させる
 *
 * @param system 状態を更新するシステム
 */
//-----------------------------------------------------------------------------------
static void Cancel( FIELD_PLACE_NAME* system )
{
	SYSTEM_STATE next_state;
	int   start_count;
	float passed_rate;
	
	// 現在の状態に応じた処理
	switch( system->state ) {
  case SYSTEM_STATE_HIDE:	
  case SYSTEM_STATE_FADEOUT:
    return;
  case SYSTEM_STATE_LAUNCH:
  case SYSTEM_STATE_WAIT_LAUNCH:
  case SYSTEM_STATE_WAIT_FADEOUT:
    next_state  = SYSTEM_STATE_WAIT_FADEOUT;
    start_count = 0;
    break;
  case SYSTEM_STATE_FADEIN:
    next_state  = SYSTEM_STATE_WAIT_FADEOUT;
    // 経過済みフレーム数を算出
    // (stateCountから, 表示位置を算出しているため, 
    //  強制退出させた場合は stateCount を適切に計算する必要がある)
    passed_rate = system->stateCount / (float)PROCESS_TIME_FADEIN;
    start_count = (int)( (1.0f - passed_rate) * PROCESS_TIME_WAIT_FADEOUT );
    break;
  case SYSTEM_STATE_SETUP:
    next_state = SYSTEM_STATE_HIDE;
    start_count = 0;
    break;
	}

	// 状態を更新
	ChangeState( system, next_state );
	system->stateCount = start_count;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 非表示状態時の動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void Process_HIDE( FIELD_PLACE_NAME* system )
{
  BOOL start = FALSE;

  // 強制表示
  if( CheckForceDispFlag(system) ) {
    start = TRUE; 
    ResetForceDispFlag( system );
  } 
  // 表示リクエスト有
  else if( CheckDispFlag(system) ) { 
    // 地名が変化
    if( CheckPlaceNameIDChange(system) ) {
      start = TRUE;
    }
    ResetDispFlag( system );
  }

  // 表示開始
  if( start ) {
    ChangeState( system, SYSTEM_STATE_SETUP );
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief 準備状態時の動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void Process_SETUP( FIELD_PLACE_NAME* system )
{
  switch( GetStateSeq(system) ) {
  case 0:
    UpdatePlaceName( system );
    IncStateSeq( system );
    break;

  case 1:
    GFL_BG_ClearCharacter( BG_FRAME );
    GFL_BG_ClearScreen( BG_FRAME );
    TransBGPalette( system ); 
    IncStateSeq( system );
    break; 

  case 2:

  if( PlaceNameVersionFlag == 1 )
  {
    GFL_ARCHDL_UTIL_TransVramBgCharacter( GetArcHandle(system), NARC_place_name_place_name_back_w_NCGR, BG_FRAME, 0, 0, FALSE, GetHeapID(system) );
    GFL_ARCHDL_UTIL_TransVramScreen( GetArcHandle(system), NARC_place_name_place_name_back_w_NSCR, BG_FRAME, 0, 0, FALSE, GetHeapID(system));
  }
  else
  {
    GFL_ARCHDL_UTIL_TransVramBgCharacter( GetArcHandle(system), NARC_place_name_place_name_back_NCGR, BG_FRAME, 0, 0, FALSE, GetHeapID(system) );
    GFL_ARCHDL_UTIL_TransVramScreen( GetArcHandle(system), NARC_place_name_place_name_back_NSCR, BG_FRAME, 0, 0, FALSE, GetHeapID(system));
  }
    GFL_BG_SetPriority( BG_FRAME, BG_FRAME_PRIORITY );
    GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF ); 
    G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, ALPHA_VALUE_1, ALPHA_VALUE_2 );
    IncStateSeq( system );
    GX_SetVisibleWnd( GX_WNDMASK_OW );
    G2_SetWndOBJInsidePlane(GX_WND_PLANEMASK_BG0
        |GX_WND_PLANEMASK_BG1
        |GX_WND_PLANEMASK_BG2
        |GX_WND_PLANEMASK_BG3
        |GX_WND_PLANEMASK_OBJ, TRUE );
    G2_SetWndOutsidePlane(GX_WND_PLANEMASK_BG0
        |GX_WND_PLANEMASK_BG1
        |GX_WND_PLANEMASK_BG2
        |GX_WND_PLANEMASK_OBJ, TRUE );

    break; 

  case 3:
    SetupPlate(system);
    IncStateSeq( system );
    break;

  case 4:
    if( CheckPlate(system ) )
    {
      ChangeState( system, SYSTEM_STATE_FADEIN );
    }
    break;
  } 
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・イン状態時の動作
 *
 * @param system 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_FADEIN_Both( FIELD_PLACE_NAME* system )
{
  switch( GetStateSeq(system) ) {
  case 0:
    Draw_FADEIN( system ); // α値を初期化
    GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON ); // BGを表示

    EnterPlate( system );
    IncStateSeq( system );
    break;

  case 1:
    // 一定時間が経過したら, 次の状態へ

    if( PROCESS_TIME_FADEIN < system->stateCount
        && CheckPlate( system )) {
      ChangeState( system, SYSTEM_STATE_WAIT_LAUNCH );
    }
    break;
  }
}
static void Process_FADEIN_UpDown( FIELD_PLACE_NAME* system )
{
  switch( GetStateSeq(system) ) {
  case 0:
    Draw_FADEIN( system ); // α値を初期化
    GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON ); // BGを表示

    PN_PLATE_EnterReq( system->plate[ 0 ], 0, PROCESS_TIME_FADEIN );
    IncStateSeq( system );
    break;

  case 1:
    if( PN_PLATE_IsEnd( system->plate[0] ) ) {
      PN_PLATE_EnterReq( system->plate[ 1 ], 0, PROCESS_TIME_FADEIN );
      IncStateSeq( system );
    }
    break;

  case 2:
    // 一定時間が経過したら, 次の状態へ
    if( PROCESS_TIME_FADEIN*2 < system->stateCount
        && PN_PLATE_IsEnd( system->plate[1] ) ) {
      ChangeState( system, SYSTEM_STATE_WAIT_LAUNCH );
    }
    break;
  }
}
static void Process_FADEIN_DownUp( FIELD_PLACE_NAME* system )
{
  switch( GetStateSeq(system) ) {
  case 0:
    Draw_FADEIN( system ); // α値を初期化
    GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON ); // BGを表示

    PN_PLATE_EnterReq( system->plate[ 1 ], 0, PROCESS_TIME_FADEIN );
    IncStateSeq( system );
    break;

  case 1:
    if( PN_PLATE_IsEnd( system->plate[1] ) ) {
      PN_PLATE_EnterReq( system->plate[ 0 ], 0, PROCESS_TIME_FADEIN );
      IncStateSeq( system );
    }
    break;

  case 2:
    // 一定時間が経過したら, 次の状態へ
    if( PROCESS_TIME_FADEIN*2 < system->stateCount
        && PN_PLATE_IsEnd( system->plate[0] ) ) {
      ChangeState( system, SYSTEM_STATE_WAIT_LAUNCH );
    }
    break;
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief 発射待ち状態時の動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT_LAUNCH( FIELD_PLACE_NAME* system )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_WAIT_LAUNCH < system->stateCount )
	{
		ChangeState( system, SYSTEM_STATE_LAUNCH );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 発射状態時の動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void Process_LAUNCH( FIELD_PLACE_NAME* system )
{
  switch( GetStateSeq(system) ) {
  case 0:
    IncStateSeq( system );
    break;

  case 1:
    ChangeState( system, SYSTEM_STATE_WAIT_FADEOUT );
    break;
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェードアウト待ち状態時の動作
 *
 * @param system 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT_FADEOUT( FIELD_PLACE_NAME* system )
{
	int i;

  switch( GetStateSeq(system) ) {
  case 0:
    OutPlate( system );
    IncStateSeq( system );
    break; 

  case 1: 
    // 一定時間が経過
   if( PROCESS_TIME_WAIT_FADEOUT < system->stateCount
       && CheckPlate( system )) { 
      ChangeState( system, SYSTEM_STATE_FADEOUT );
    }
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・アウト状態時の動作
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void Process_FADEOUT( FIELD_PLACE_NAME* system )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_FADEOUT < system->stateCount ) {
		ChangeState( system, SYSTEM_STATE_HIDE );
    HidePlate( system ); // プレートを非表示にする
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 非表示状態時の描画処理
 *
 * @param system 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_HIDE( const FIELD_PLACE_NAME* system )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 準備状態時の描画処理
 *
 * @param system 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_SETUP( const FIELD_PLACE_NAME* system )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・イン状態時の描画処理
 *
 * @param system 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_FADEIN( const FIELD_PLACE_NAME* system )
{
#if 0
	int val1, val2;
	float rate;
	// αブレンディング係数を更新
  if( PlaceNameMoveType == 0 )
  {
    rate  = (float)system->stateCount / (float)PROCESS_TIME_FADEIN;
  }
  else
  {
    rate  = (float)system->stateCount / (float)(PROCESS_TIME_FADEIN*2);
  }
	val1 = (int)( ALPHA_VALUE_1 * rate );
	val2 = (int)( ALPHA_VALUE_2 + (16 - ALPHA_VALUE_2) * (1.0f - rate) );
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, val1, val2 );
#endif 
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の描画処理
 *
 * @param system 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_WAIT_LAUNCH( const FIELD_PLACE_NAME* system )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字発射状態時の描画処理
 *
 * @param system 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_LAUNCH( const FIELD_PLACE_NAME* system )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の描画処理
 *
 * @param system 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_WAIT_FADEOUT( const FIELD_PLACE_NAME* system )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・アウト状態時の描画処理
 *
 * @param system 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_FADEOUT( const FIELD_PLACE_NAME* system )
{
#if 0
	int val1, val2;
	float rate;
	
	// αブレンディング係数を更新
	rate  = (float)system->stateCount / (float)PROCESS_TIME_FADEOUT;
	val1 = (int)( ALPHA_VALUE_1 * (1.0f - rate) );
	val2 = (int)( ALPHA_VALUE_2 + (16 - ALPHA_VALUE_2) * rate );
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, val1, val2 );
#endif
} 


//-----------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを生成する
 * 
 * @param heapID 使用するヒープID
 */
//-----------------------------------------------------------------------------------
static FIELD_PLACE_NAME* CreatePNSystem( HEAPID heapID )
{
	FIELD_PLACE_NAME* system;

	system = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_PLACE_NAME) );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CreatePNSystem\n" );
#endif

  return system;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを初期化する
 * 
 * @param system
 * @param gameSystem
 */
//-----------------------------------------------------------------------------------
static void InitPNSystem( FIELD_PLACE_NAME* system, GAMESYS_WORK* gameSystem )
{
  // ゼロクリア
  GFL_STD_MemClear( system, sizeof(FIELD_PLACE_NAME) );

  // 初期化
  system->gameSystem    = gameSystem;
  system->gameData      = GAMESYSTEM_GetGameData( gameSystem );
  system->nullCharPos   = AREAMAN_POS_NOTFOUND;
  system->forceDispFlag = FALSE;
  system->dispFlag      = FALSE;
  system->prevZoneID    = ZONE_ID_MAX;
  system->nowZoneID     = ZONE_ID_MAX;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: InitPNSystem\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 地名表示システムを破棄する
 * 
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DeletePNSystem( FIELD_PLACE_NAME* system )
{
  GFL_HEAP_FreeMemory( system );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: DeletePNSystem\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief ワードセットを生成する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void CreateWordset( FIELD_PLACE_NAME* system )
{
  HEAPID heapID;

  GF_ASSERT( system->wordset == NULL );

  heapID = GetHeapID( system ); 
  system->wordset = WORDSET_Create( heapID );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CreateWordset\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief メッセージデータを生成する
 * 
 * @param system
 */
//-----------------------------------------------------------------------------------
static void CreateMessageData( FIELD_PLACE_NAME* system )
{
  HEAPID heapID;

  GF_ASSERT( system->message == NULL );

  heapID = GetHeapID( system ); 
	system->message = GFL_MSG_Create( 
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_place_name_dat, heapID );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CreateMessagData\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 地名バッファを生成する
 * 
 * @param system
 */
//-----------------------------------------------------------------------------------
static void CreatePlaceNameBuffer( FIELD_PLACE_NAME* system )
{
  HEAPID heapID;
  int length;

  GF_ASSERT( system->nameBuf == NULL );

  heapID = GetHeapID( system ); 
  length = MAX_NAME_LENGTH + 1; // EOMを考慮して+1
	system->nameBuf = GFL_STR_CreateBuffer( length, heapID ); 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CreatePlaceNameBuffer\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief ワードセットを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DeleteWordset( FIELD_PLACE_NAME* system )
{
  if( system->wordset ) {
    WORDSET_Delete( system->wordset );
    system->wordset = NULL;
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: DeleteWordset\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief メッセージデータを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DeleteMessageData( FIELD_PLACE_NAME* system )
{
  if( system->message ) {
    GFL_MSG_Delete( system->message );
    system->message = NULL;
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: DeleteMessageData\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 地名バッファを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DeletePlaceNameBuffer( FIELD_PLACE_NAME* system )
{
  if( system->nameBuf ) { 
    GFL_STR_DeleteBuffer( system->nameBuf );
    system->nameBuf = NULL;
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: DeletePlaceNameBuffer\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief ヒープIDを取得する
 *
 * @param system
 *
 * @return ヒープID
 */
//-----------------------------------------------------------------------------------
static HEAPID GetHeapID( const FIELD_PLACE_NAME* system )
{
  return system->heapID;
}

//-----------------------------------------------------------------------------------
/**
 * @brief アーカイブデータハンドルを取得する
 *
 * @param system
 *
 * @return アーカイブデータハンドル
 */
//-----------------------------------------------------------------------------------
static ARCHANDLE* GetArcHandle( const FIELD_PLACE_NAME* system )
{
  return system->arcHandle;
}

//-----------------------------------------------------------------------------------
/**
 * @brief フォントを取得する
 *
 * @param system
 *
 * @return フォント
 */
//-----------------------------------------------------------------------------------
static GFL_FONT* GetFont( const FIELD_PLACE_NAME* system )
{
  return system->font;
}

//-----------------------------------------------------------------------------------
/**
 * @brief メッセージデータを取得する
 *
 * @param system
 *
 * @return メッセージデータ
 */
//-----------------------------------------------------------------------------------
static GFL_MSGDATA* GetMessageData( const FIELD_PLACE_NAME* system )
{
  return system->message;
}

//-----------------------------------------------------------------------------------
/**
 * @brief ヒープIDを設定する
 *
 * @param system
 * @param heapID 
 */
//-----------------------------------------------------------------------------------
static void SetHeapID( FIELD_PLACE_NAME* system, HEAPID heapID )
{
  system->heapID = heapID;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: SetHeapID\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief フォントを設定する
 *
 * @param system
 * @param font 
 */
//-----------------------------------------------------------------------------------
static void SetFont( FIELD_PLACE_NAME* system, GFL_FONT* font )
{
  system->font = font;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: SetFont\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief システムの状態を設定する
 *
 * @param system
 * @param state  設定する状態
 */
//-----------------------------------------------------------------------------------
static void SetState( FIELD_PLACE_NAME* system, SYSTEM_STATE state )
{
  system->state = state;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: SetState ==> " );
  DEBUG_PrintSystemState( system );
  OS_TFPrintf( DEBUG_PRINT_TARGET, "\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief システムの状態を取得する
 *
 * @param system
 *
 * @return システムの状態
 */
//-----------------------------------------------------------------------------------
static SYSTEM_STATE GetState( const FIELD_PLACE_NAME* system )
{
  return system->state;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態カウンタを取得する
 *
 * @param system
 *
 * @return 状態カウンタの値
 */
//-----------------------------------------------------------------------------------
static u16 GetStateCount( const FIELD_PLACE_NAME* system )
{
  return system->stateCount;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態カウンタをリセットする
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void ResetStateCount( FIELD_PLACE_NAME* system )
{
  system->stateCount = 0;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: ResetStateCount\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態カウンタを更新する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void IncStateCount( FIELD_PLACE_NAME* system )
{
  system->stateCount++;

  // 最大値補正
  if( MAX_STATE_COUNT < system->stateCount ) {
    system->stateCount = MAX_STATE_COUNT;
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態内シーケンスを取得する
 *
 * @param system
 *
 * @return 状態内シーケンス番号
 */
//-----------------------------------------------------------------------------------
static u8 GetStateSeq( const FIELD_PLACE_NAME* system )
{
  return system->stateSeq;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態内シーケンスをリセットする
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void ResetStateSeq( FIELD_PLACE_NAME* system )
{
  system->stateSeq = 0;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 状態内シーケンスをインクリメントする
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void IncStateSeq( FIELD_PLACE_NAME* system )
{
  system->stateSeq++;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 現在のゾーンIDを読み込む
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void LoadNowZoneID( FIELD_PLACE_NAME* system )
{
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  SITUATION* situation;

  gameData  = GAMESYSTEM_GetGameData( system->gameSystem );
  save      = GAMEDATA_GetSaveControlWork( gameData );
  situation = SaveData_GetSituation( save );
  system->nowZoneID = Situation_GetPlaceNameLastDispID( situation );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, 
      "FIELD-PLACE-NAME: LoadNowZoneID (%d)\n", system->nowZoneID );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 現在のゾーンIDを記録する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void SaveNowZoneID( const FIELD_PLACE_NAME* system )
{
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  SITUATION* situation;

  gameData  = GAMESYSTEM_GetGameData( system->gameSystem );
  save      = GAMEDATA_GetSaveControlWork( gameData );
  situation = SaveData_GetSituation( save );
  Situation_SetPlaceNameLastDispID( situation, system->nowZoneID );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, 
      "FIELD-PLACE-NAME: SaveNowZoneID (%d)\n", system->nowZoneID );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 前回のゾーンIDを取得する
 *
 * @param system
 *
 * @return 最後に表示した場所のゾーンID
 */
//-----------------------------------------------------------------------------------
static u16 GetPrevZoneID( const FIELD_PLACE_NAME* system )
{
  return system->prevZoneID;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 現在のゾーンIDを取得する
 *
 * @param system
 *
 * @return 次に表示する場所のゾーンID
 */
//-----------------------------------------------------------------------------------
static u16 GetNowZoneID( const FIELD_PLACE_NAME* system )
{
  return system->nowZoneID;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 現在のゾーンIDを設定する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void SetNowZoneID( FIELD_PLACE_NAME* system, u16 zoneID )
{
  system->prevZoneID = system->nowZoneID;
  system->nowZoneID  = zoneID;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: SetNowZoneID (%d)\n", zoneID );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 地名バッファを取得する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static const STRBUF* GetPlaceName( const FIELD_PLACE_NAME* system )
{
  GF_ASSERT( system->nameBuf );

  return system->nameBuf;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 地名の文字数を取得する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static u8 GetPlaceNameLength( const FIELD_PLACE_NAME* system )
{
  return system->nameLen;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 地名表示フラグをチェックする
 *
 * @param zoneID チェックするゾーン
 * @param isSaveRecovery  セーブ復帰したかどうか
 *
 * @return 指定したゾーンの地名表示フラグが立っている場合 TRUE
 */
//-----------------------------------------------------------------------------------
static BOOL CheckPlaceNameFlag( u16 zoneID, BOOL isSaveRecovery )
{
  switch( ZONEDATA_GetPlaceNameFlag( zoneID ) )
  {
  case ZONEDATA_PLACENAME_FLAG_ALL_HIDE:
    return FALSE;
  case ZONEDATA_PLACENAME_FLAG_ALL_VIEW:
    return TRUE;
  case ZONEDATA_PLACENAME_FLAG_SAVE_VIEW:
    return isSaveRecovery;
  case ZONEDATA_PLACENAME_FLAG_INOUT_VIEW:
    return !isSaveRecovery;
  default:
    GF_ASSERT(0);
    return FALSE;
  }
}


//#ifdef DEBUG_PRINT_ON
//-----------------------------------------------------------------------------------
/**
 * @brief システム状態名を出力する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DEBUG_PrintSystemState( const FIELD_PLACE_NAME* system )
{
  switch( GetState(system) ) {
  case SYSTEM_STATE_HIDE:         OS_TFPrintf( DEBUG_PRINT_TARGET, "HIDE" );          break;
  case SYSTEM_STATE_SETUP:        OS_TFPrintf( DEBUG_PRINT_TARGET, "SETUP" );         break;
  case SYSTEM_STATE_FADEIN:       OS_TFPrintf( DEBUG_PRINT_TARGET, "FADEIN" );        break;
  case SYSTEM_STATE_WAIT_LAUNCH:  OS_TFPrintf( DEBUG_PRINT_TARGET, "WAIT_LAUNCH" );   break;
  case SYSTEM_STATE_LAUNCH:       OS_TFPrintf( DEBUG_PRINT_TARGET, "LAUNCH" );        break;
  case SYSTEM_STATE_WAIT_FADEOUT: OS_TFPrintf( DEBUG_PRINT_TARGET, "WAIT_FADEOUT" );  break;
  case SYSTEM_STATE_FADEOUT:      OS_TFPrintf( DEBUG_PRINT_TARGET, "FADEOUT" );       break;
  }
}
//#endif
