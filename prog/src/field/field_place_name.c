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


//===================================================================================
// ■デバッグ
//=================================================================================== 
//#define DEBUG_PRINT_ON // デバッグ出力スイッチ
#define DEBUG_PRINT_TARGET (3) // デバッグ出力先

#ifdef PM_DEBUG
BOOL PlaceNameEnable = TRUE; // 地名表示の有効フラグ
#endif

//===================================================================================
// ■ 定数・マクロ
//=================================================================================== 
#define MAX_STATE_COUNT (0xffff) // 状態カウンタ最大値
//#define MAX_NAME_LENGTH (BUFLEN_PLACE_NAME - BUFLEN_EOM_SIZE) // 最大文字数
#define MAX_NAME_LENGTH (28)
// ↑パレス侵入中は侵入相手の名前を表示するため, 大きめに設定する.2010.05.11
// 現時点の最大長「ホワイトフォレスト」+ 名前5文字 = 14文字 
// ==> ローカライズ考慮で 28文字

//-----------
// 表示設定
//-----------
// BG
#define BG_PALETTE_NO          (0) // BGパレット番号
#define BG_FRAME (GFL_BG_FRAME3_M) // 使用するBGフレーム
#define BG_FRAME_PRIORITY      (1) // BGフレームのプライオリティ
// フォント
#define	COLOR_NO_LETTER     (1) // 文字本体のカラー番号
#define	COLOR_NO_SHADOW     (2) // 影部分のカラー番号
#define	COLOR_NO_BACKGROUND (0) // 背景部のカラー番号
// αブレンド
#define ALPHA_PLANE_1 (GX_BLEND_PLANEMASK_BG3) // αブレンドの第1対象面
#define ALPHA_PLANE_2 (GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_OBJ) // αブレンドの第2対象面
#define ALPHA_VALUE_1                     (16) // 第1対象面のαブレンディング係数
#define ALPHA_VALUE_2                      (4) // 第2対象面のαブレンディング係数

#define CHAR_SIZE                    (8) // 1キャラ = 8ドット
#define Y_CENTER_POS (CHAR_SIZE * 2 - 1) // 背景帯の中心y座標

//-----
// BG 
//-----
#define NULL_CHAR_NO       ( 0)								// NULLキャラ番号
#define	BMPWIN_WIDTH_CHAR  (32)								// ウィンドウ幅(キャラクタ単位)
#define BMPWIN_HEIGHT_CHAR ( 2)								// ウィンドウ高さ(キャラクタ単位)
#define BMPWIN_WIDTH_DOT   (BMPWIN_WIDTH_CHAR * CHAR_SIZE)	// ウィンドウ幅(ドット単位)
#define BMPWIN_HEIGHT_DOT  (BMPWIN_HEIGHT_CHAR * CHAR_SIZE)	// ウィンドウ高さ(ドット単位)
#define BMPWIN_POS_X_CHAR  ( 0)								// ウィンドウのx座標(キャラクタ単位)
#define BMPWIN_POS_Y_CHAR  ( 1)								// ウィンドウのy座標(キャラクタ単位)

//-----------
// 動作設定
//-----------
#define LAUNCH_INTERVAL (3)	// 文字オブジェクトの発射間隔[frame]

// 各状態の動作設定
#define PROCESS_TIME_FADEIN  (10)
#define PROCESS_TIME_WAIT_LAUNCH   (10)
#define PROCESS_TIME_WAIT_FADEOUT   (30)
#define PROCESS_TIME_FADEOUT (20)

//--------------
// インデックス
//--------------
// パレットリソース
typedef enum {
	PLTT_RES_LETTER,	// 文字オブジェクトで使用するパレット
	PLTT_RES_NUM      // 総数
} PLTT_RES_INDEX;

// セルアクターユニット
typedef enum {
	CLUNIT_LETTER, // 文字オブジェクト
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

// 文字オブジェクトのセットアップパラメータ
static const PN_LETTER_SETUP_PARAM LetterSetupParam = 
{
  256,                       // 座標
  Y_CENTER_POS - ( 13 / 2 ), // 
  -20,                       // 速度
  0,                         // 
  24,                        // 目標位置
  Y_CENTER_POS - ( 13 / 2 ), // 
  0,                         // 目標位置での速度
  0,                         // 
  NULL,                      // フォント
  0,                         // 文字コード
};

// BG コントロール
GFL_BG_BGCNT_HEADER BGCntHeader = 
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
  RENDERER_LETTER, // 文字オブジェクト用
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

	// 地名
	STRBUF* nameBuf; // 表示中の地名文字列
	u8      nameLen; // 表示中の地名文字数

	// 帯
	GFL_BMPWIN*   bmpWin;	     // ビットマップ・ウィンドウ
	GFL_BMP_DATA* bmpOrg;	     // 文字が書き込まれていない状態
  u32           nullCharPos; // NULLキャラクタのキャラNo.

	// OBJ
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

  // 文字オブジェクト
  PN_LETTER* letters[ MAX_NAME_LENGTH ];
  u8 setupLetterNum; // セットアップが完了した文字オブジェクトの数
	u8 launchLetterNum;// 発射済み文字数
  u8 writeLetterNum; // BG に書き込んだ文字数

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
static void LoadBGPaletteData( FIELD_PLACE_NAME* system ); // BG パレットデータを読み込む
static void UnloadBGPaletteData( FIELD_PLACE_NAME* system ); // BG パレットデータを解放する
static void TransBGPalette( FIELD_PLACE_NAME* system ); // BG パレットを転送する
static void AllocBGNullCharArea( FIELD_PLACE_NAME* system ); // 空白キャラエリアを確保する
static void FreeBGNullCharaArea( FIELD_PLACE_NAME* system ); // 空白キャラエリアを解放する
static void RecoverBlankBand( FIELD_PLACE_NAME* system ); // 帯を文字が書き込まれていない状態に復帰する
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
// 文字オブジェクト
static void SetupLetter_init( FIELD_PLACE_NAME* system ); // 文字オブジェクトのセットアップを開始する
static void SetupLetter_main( FIELD_PLACE_NAME* system ); // 文字オブジェクトを１文字ずつセットアップする
static BOOL SetupLetter_check( const FIELD_PLACE_NAME* system ); // 文字オブジェクトのセットアップが完了したかどうかをチェックする
static void HideLetters( FIELD_PLACE_NAME* system ); // 文字オブジェクトを非表示にする
static void LaunchLetter( FIELD_PLACE_NAME* system ); // 文字オブジェクトを発射する
static void MoveLetters( FIELD_PLACE_NAME* system ); // 文字オブジェクトを動かす
// BG, 文字ユニットへの書き込み
static void PrintLetter_init( FIELD_PLACE_NAME* system ); // 文字オブジェクトの書き込みを開始する
static void PrintLetter_main( FIELD_PLACE_NAME* system ); // 文字オブジェクトを１文字ずつビットマップ・ウィンドウに書き込む
static BOOL PrintLetter_check( const FIELD_PLACE_NAME* system ); // 文字オブジェクトの書き込み完了をチェックする
// 状態の変更
static void ChangeState( FIELD_PLACE_NAME* system, SYSTEM_STATE next_state );
static void Cancel( FIELD_PLACE_NAME* system ); 
// 各状態時の動作
static void Process_HIDE( FIELD_PLACE_NAME* system );
static void Process_SETUP( FIELD_PLACE_NAME* system );
static void Process_FADEIN( FIELD_PLACE_NAME* system );
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
static void CreateBandBlankBitmap( FIELD_PLACE_NAME* system ); // 白紙帯のビットマップを生成する
static void CreateBandBitmapWindow( FIELD_PLACE_NAME* system ); // 帯のビットマップウィンドウを生成する
static void CreateLetters( FIELD_PLACE_NAME* system ); // 文字オブジェクトを生成する
static void OpenDataHandle( FIELD_PLACE_NAME* system ); // アーカイブデータハンドルを開く
// 破棄
static void DeleteWordset( FIELD_PLACE_NAME* system ); // ワードセットを破棄する
static void DeleteMessageData( FIELD_PLACE_NAME* system ); // メッセージデータを破棄する
static void DeletePlaceNameBuffer( FIELD_PLACE_NAME* system ); // 地名バッファを破棄する
static void DeleteBandBlankBitmap( FIELD_PLACE_NAME* system ); // 白紙帯のビットマップを破棄する
static void DeleteBandBitmapWindow( FIELD_PLACE_NAME* system ); // 帯のビットマップウィンドウを破棄する
static void DeleteLetters( FIELD_PLACE_NAME* system ); // 文字オブジェクトを破棄する
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
static BOOL CheckPlaceNameFlag( u16 zoneID ); // 地名表示フラグをチェックする
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
  SetUserRenderer( system, CLUNIT_LETTER );
  CreateBmpOamSystem( system ); 
	CreateLetters( system );

  GFL_BG_SetBGControl( BG_FRAME, &BGCntHeader, GFL_BG_MODE_TEXT ); 
  LoadBGPaletteData( system );
  AllocBGNullCharArea( system );
  CreateBandBlankBitmap( system );
  CreateBandBitmapWindow( system );

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
  DeleteLetters( system ); 
  DeletePlaceNameBuffer( system ); 
  DeleteBmpOamSystem( system ); 
  DeleteClactUnit( system );
  UnloadOBJResource( system ); 
  DeleteClactRenderer( system );
  DeleteWordset( system );
  DeleteMessageData( system ); 
  DeleteBandBitmapWindow( system );
  DeleteBandBlankBitmap( system );
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

#ifdef TICK_TEST
  DEBUG_PrintSystemState( system );
  OS_TFPrintf( 3, "[%d]: ", GetStateSeq(system) );
  start = OS_GetTick();
#endif

	// 状態に応じた動作
	switch( GetState(system) ) {
  case SYSTEM_STATE_HIDE:         Process_HIDE( system );		      break;
  case SYSTEM_STATE_SETUP:        Process_SETUP( system );		    break;
  case SYSTEM_STATE_FADEIN:       Process_FADEIN( system );	      break;
  case SYSTEM_STATE_WAIT_LAUNCH:  Process_WAIT_LAUNCH( system );	break;
  case SYSTEM_STATE_LAUNCH:       Process_LAUNCH( system );	      break;
  case SYSTEM_STATE_WAIT_FADEOUT: Process_WAIT_FADEOUT( system );	break;
  case SYSTEM_STATE_FADEOUT:      Process_FADEOUT( system );	    break;
	}

	// 文字オブジェクトを動かす
  MoveLetters( system );

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
  if( CheckPlaceNameFlag( zoneID ) == FALSE ) { return; }

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
  if( CheckPlaceNameFlag( zoneID ) == FALSE ) { return; }

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
  RecoverBlankBand( system );
}


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
  CreateBandBlankBitmap( system );
  CreateBandBitmapWindow( system );

	// ビットマップ・ウィンドウのデータをVRAMに転送
	GFL_BMPWIN_MakeTransWindow( system->bmpWin ); 

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
  DeleteBandBitmapWindow( system );
  DeleteBandBlankBitmap( system );
}

//-----------------------------------------------------------------------------------
/**
 * @brief BGの再設定
 *
 * @param sys 地名表示システム
 */ 
//-----------------------------------------------------------------------------------
static void ResetBG( FIELD_PLACE_NAME* sys )
{ 
  CleanBG( sys );
  SetupBG( sys );
}

//------------------------------------------------------------------------------------
/**
 * @brief BG パレットデータを読み込む
 *
 * @param system
 */
//------------------------------------------------------------------------------------
static void LoadBGPaletteData( FIELD_PLACE_NAME* system )
{
  ARCDATID datID;
  ARCHANDLE* handle;
  HEAPID heapID;
  u32 size;

  GF_ASSERT( system->BGPalBinary == NULL );
  GF_ASSERT( system->BGPalData == NULL );

  heapID = GetHeapID( system ); 
  handle = GetArcHandle( system );
  datID  = NARC_place_name_place_name_back_NCLR;

  size = GFL_ARC_GetDataSizeByHandle( handle, datID ); // データサイズ取得
  system->BGPalBinary = GFL_HEAP_AllocMemory( heapID, size );	// データバッファ確保
  GFL_ARC_LoadDataByHandle( handle, datID, system->BGPalBinary ); // データ取得
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
 * @brief 帯を文字が書き込まれていない状態に復帰する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void RecoverBlankBand( FIELD_PLACE_NAME* system )
{
	GFL_BMP_DATA* src = system->bmpOrg;
	GFL_BMP_DATA* dest = GFL_BMPWIN_GetBmp( system->bmpWin );

	// 文字が書き込まれていない状態をコピー
	GFL_BMP_Copy( src, dest );	
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

	system->resPltt[ PLTT_RES_LETTER ] = 
		GFL_CLGRP_PLTT_RegisterEx( 
				handle, NARC_place_name_place_name_string_NCLR,
				CLSYS_DRAW_MAIN, FLDOAM_PALNO_PLACENAME * 32, 0, 1, heapID );

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
	system->clunit[ CLUNIT_LETTER ] = 
		GFL_CLACT_UNIT_Create( MAX_NAME_LENGTH, BG_FRAME_PRIORITY, heapID );

	// 初期設定
	GFL_CLACT_UNIT_SetDrawEnable( system->clunit[ CLUNIT_LETTER ], TRUE );
  
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
 * @brief 文字オブジェクトを生成する
 *
 * @param system 
 */
//-----------------------------------------------------------------------------------
static void CreateLetters( FIELD_PLACE_NAME* system )
{
	int i;

	for( i=0; i<MAX_NAME_LENGTH; i++ )
	{
    system->letters[i] = PN_LETTER_Create( 
        system->heapID, system->bmpOamSys, RENDERER_LETTER, 
        system->resPltt[ PLTT_RES_LETTER ] );
	} 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CreateLetters\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字オブジェクトを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DeleteLetters( FIELD_PLACE_NAME* system )
{
	int i;

	for( i=0; i<MAX_NAME_LENGTH; i++ )
	{
    PN_LETTER_Delete( system->letters[i] );
	} 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: DeleteLetters\n" );
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

//-----------------------------------------------------------------------------------
/**
 * @brief 文字オブジェクトのセットアップを開始する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void SetupLetter_init( FIELD_PLACE_NAME* system )
{
  system->setupLetterNum = 0;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字オブジェクトを１文字ずつセットアップする
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void SetupLetter_main( FIELD_PLACE_NAME* system )
{
  int i, idx;
  STRCODE code[ MAX_NAME_LENGTH ];
  PN_LETTER_SETUP_PARAM param;

  idx = system->setupLetterNum;
  GF_ASSERT( idx < GetPlaceNameLength(system) );

  GFL_STR_GetStringCode( GetPlaceName(system), code, MAX_NAME_LENGTH );
  param = LetterSetupParam;
  param.font = system->font;
  param.code = code[ idx ];

  // 目標位置を決定
  for( i=0; i<idx; i++ )
  {
    param.dx += PN_LETTER_GetWidth( system->letters[i] );
    param.dx += 1;
  }

  PN_LETTER_Setup( system->letters[ idx ], &param );
  system->setupLetterNum++;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: SetupLetter_main[%d]\n", idx );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字オブジェクトのセットアップが完了したかどうかをチェックする
 *
 * @param system
 *
 * @return すべての文字オブジェクトのセットアップが完了している場合 TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------
static BOOL SetupLetter_check( const FIELD_PLACE_NAME* system )
{
  if( GetPlaceNameLength(system) <= system->setupLetterNum ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字オブジェクトを非表示にする
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void HideLetters( FIELD_PLACE_NAME* system )
{
	int i;
	
	for( i=0; i<MAX_NAME_LENGTH; i++ )
	{
    PN_LETTER_SetDrawEnable( system->letters[i], FALSE );
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字オブジェクトを発射する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void LaunchLetter( FIELD_PLACE_NAME* system )
{
  int idx;

  // すでに全てのユニット発射済み
  GF_ASSERT( system->launchLetterNum < GetPlaceNameLength(system) );

  // 発射
  idx = system->launchLetterNum++;
  PN_LETTER_MoveStart( system->letters[ idx ] );
  PN_LETTER_SetDrawEnable( system->letters[ idx ], TRUE );

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: LaunchLetter[%d]\n", idx );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief すべての文字オブジェクトを動かす
 *
 * @param system 
 */
//-----------------------------------------------------------------------------------
static void MoveLetters( FIELD_PLACE_NAME* system )
{
  int i;
  int len = GetPlaceNameLength( system );

	for( i=0; i<len; i++ )
	{
    PN_LETTER_Main( system->letters[i] );
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
	system->bmpOamSys = BmpOam_Init( heapID, system->clunit[ CLUNIT_LETTER ] );

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
 * @brief 文字オブジェクトの書き込みを開始する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void PrintLetter_init( FIELD_PLACE_NAME* system )
{
  system->writeLetterNum = 0;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字オブジェクトを１文字ずつビットマップ・ウィンドウに書き込む
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void PrintLetter_main( FIELD_PLACE_NAME* system )
{
  int idx;
	u16 dx, dy;
  u16 width, height;
  PN_LETTER* letter;
	const GFL_BMP_DATA* sourceBMP = NULL;
	GFL_BMP_DATA* destBMP = NULL;

  idx = system->writeLetterNum;
  GF_ASSERT( idx < GetPlaceNameLength(system) );

  letter    = system->letters[idx];
  sourceBMP = PN_LETTER_GetBitmap( letter );
	destBMP   = GFL_BMPWIN_GetBmp( system->bmpWin );
  dx        = PN_LETTER_GetLeft( letter ) - ( BMPWIN_POS_X_CHAR * CHAR_SIZE );
  dy        = PN_LETTER_GetTop( letter )  - ( BMPWIN_POS_Y_CHAR * CHAR_SIZE );
  width     = PN_LETTER_GetWidth( letter );
  height    = PN_LETTER_GetHeight( letter );

  GFL_BMP_Print( sourceBMP, destBMP, 0, 0, dx, dy, width, height, 0 );
  system->writeLetterNum++;
}

//-----------------------------------------------------------------------------------
/**
 * @brief すべての文字オブジェクトの書き込みが完了したかどうかをチェックする
 *
 * @param system
 *
 * @return すべて完了した場合 TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------
static BOOL PrintLetter_check( const FIELD_PLACE_NAME* system )
{
  int len = GetPlaceNameLength( system ); 
  int num = system->writeLetterNum;

  if( len <= num ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
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
  if( CheckIntrudeField( system, zoneID ) ) {
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
  }
  // 自分のフィールドにいる
  else {
    GFL_MSG_GetString( system->message,	strID, system->nameBuf );
  }
  system->nameLen = GFL_STR_GetBufferLength( system->nameBuf );
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
			GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF );	// BGを非表示
			HideLetters( system ); // 文字オブジェクトを非表示にする
			break;
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 強制的にウィンドウを退出させる
 *
 * @param sys 状態を更新するシステム
 */
//-----------------------------------------------------------------------------------
static void Cancel( FIELD_PLACE_NAME* sys )
{
	SYSTEM_STATE next_state;
	int   start_count;
	float passed_rate;
	
	// 現在の状態に応じた処理
	switch( sys->state ) {
  case SYSTEM_STATE_HIDE:	
  case SYSTEM_STATE_FADEOUT:
  case SYSTEM_STATE_LAUNCH:
    return;
  case SYSTEM_STATE_WAIT_LAUNCH:
  case SYSTEM_STATE_WAIT_FADEOUT:
    next_state  = SYSTEM_STATE_WAIT_FADEOUT;
    start_count = PROCESS_TIME_WAIT_FADEOUT;
    break;
  case SYSTEM_STATE_FADEIN:
    next_state = SYSTEM_STATE_FADEOUT;
    // 経過済みフレーム数を算出
    // (stateCountから, 表示位置を算出しているため, 
    //  強制退出させた場合は stateCount を適切に計算する必要がある)
    passed_rate = sys->stateCount / (float)PROCESS_TIME_FADEIN;
    start_count = (int)( (1.0f - passed_rate) * PROCESS_TIME_FADEOUT );
    break;
  case SYSTEM_STATE_SETUP:
    next_state = SYSTEM_STATE_HIDE;
    start_count = 0;
    break;
	}

	// 状態を更新
	ChangeState( sys, next_state );
	sys->stateCount = start_count;
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
    RecoverBlankBand( system );
    GFL_BMPWIN_MakeTransWindow( system->bmpWin ); 
    GFL_BG_SetPriority( BG_FRAME, BG_FRAME_PRIORITY );
    GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF ); 
    G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, ALPHA_VALUE_1, ALPHA_VALUE_2 );
    SetupLetter_init( system );
    IncStateSeq( system );
    break; 

  case 3:
    SetupLetter_main( system ); // 文字オブジェクトをセットアップ

    // 全ての文字オブジェクトのセットアップが完了
    if( SetupLetter_check(system) ) {
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
static void Process_FADEIN( FIELD_PLACE_NAME* system )
{
  switch( GetStateSeq(system) ) {
  case 0:
    Draw_FADEIN( system ); // α値を初期化
    GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON ); // BGを表示
    IncStateSeq( system );
    break;

  case 1:
    // 一定時間が経過したら, 次の状態へ
    if( PROCESS_TIME_FADEIN < system->stateCount ) {
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
		system->launchLetterNum = 0;	// 発射文字数をリセット
    IncStateSeq( system );
    break;

  case 1:
    // 一定間隔で文字を発射
    if( system->stateCount % LAUNCH_INTERVAL == 0 ) {

      LaunchLetter( system ); // 発射

      // すべての文字を発射したら, 次の状態へ
      if( system->nameLen <= system->launchLetterNum ) {
        ChangeState( system, SYSTEM_STATE_WAIT_FADEOUT );
      }
    }
    break;
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェードアウト待ち状態時の動作
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT_FADEOUT( FIELD_PLACE_NAME* sys )
{
	int i;
  BOOL moveFinish = TRUE;

  switch( GetStateSeq(sys) ) {
  case 0:
    // 全ての文字の停止待ち
    for( i=0; i<sys->nameLen; i++ )
    {
      if( PN_LETTER_IsMoving( sys->letters[i] ) ) { 
        moveFinish = FALSE;
        break; 
      }
    }
    if( moveFinish ) {
      PrintLetter_init( sys );
      IncStateSeq( sys );
    }
    break;

  case 1:
    PrintLetter_main( sys );
    // 全ての文字の書き込み完了
    if( PrintLetter_check(sys) ) {
      IncStateSeq( sys );
    }
    break;
  case 2:
	  GFL_BMPWIN_TransVramCharacter( sys->bmpWin );
    IncStateSeq( sys );
    break; 
  case 3:
    HideLetters( sys ); // 文字オブジェクトを非表示にする
    IncStateSeq( sys );
    break; 

  case 4: 
    // 一定時間が経過
    if( PROCESS_TIME_WAIT_FADEOUT < sys->stateCount ) { 
      ChangeState( sys, SYSTEM_STATE_FADEOUT );
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
    GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF );	// BGを非表示にする
    HideLetters( system ); // 文字オブジェクトを非表示にする
	}
}

//-----------------------------------------------------------------------------------
/**
 * @brief 非表示状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_HIDE( const FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 準備状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_SETUP( const FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・イン状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_FADEIN( const FIELD_PLACE_NAME* sys )
{
	int val1, val2;
	float rate;

	// αブレンディング係数を更新
	rate  = (float)sys->stateCount / (float)PROCESS_TIME_FADEIN;
	val1 = (int)( ALPHA_VALUE_1 * rate );
	val2 = (int)( ALPHA_VALUE_2 + (16 - ALPHA_VALUE_2) * (1.0f - rate) );
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, val1, val2 );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_WAIT_LAUNCH( const FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 文字発射状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_LAUNCH( const FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief 待機状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_WAIT_FADEOUT( const FIELD_PLACE_NAME* sys )
{
}

//-----------------------------------------------------------------------------------
/**
 * @brief フェード・アウト状態時の描画処理
 *
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Draw_FADEOUT( const FIELD_PLACE_NAME* sys )
{
	int val1, val2;
	float rate;
	
	// αブレンディング係数を更新
	rate  = (float)sys->stateCount / (float)PROCESS_TIME_FADEOUT;
	val1 = (int)( ALPHA_VALUE_1 * (1.0f - rate) );
	val2 = (int)( ALPHA_VALUE_2 + (16 - ALPHA_VALUE_2) * rate );
	G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, val1, val2 );
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
 * @brief 白紙帯のビットマップを生成する
 * 
 * @param system
 */
//-----------------------------------------------------------------------------------
static void CreateBandBlankBitmap( FIELD_PLACE_NAME* system )
{
  HEAPID heapID;
  ARCHANDLE* handle;

  GF_ASSERT( system->bmpOrg == NULL );

  heapID = GetHeapID( system );
  handle = GetArcHandle( system );

  system->bmpOrg = 
    GFL_BMP_LoadCharacterByHandle( 
        handle, NARC_place_name_place_name_back_NCGR, FALSE, heapID ); 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CreateBandBlankBitmap\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 帯のビットマップウィンドウを生成する
 * 
 * @param system
 */
//-----------------------------------------------------------------------------------
static void CreateBandBitmapWindow( FIELD_PLACE_NAME* system )
{
  HEAPID heapID;

  GF_ASSERT( system->bmpWin == NULL );

  heapID = GetHeapID( system );
  system->bmpWin = GFL_BMPWIN_Create( BG_FRAME, 
                                      BMPWIN_POS_X_CHAR, BMPWIN_POS_Y_CHAR, 
                                      BMPWIN_WIDTH_CHAR, BMPWIN_HEIGHT_CHAR, 
                                      BG_PALETTE_NO, GFL_BMP_CHRAREA_GET_F ); 

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: CreateBandBitmapWindow\n" );
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
 * @brief 白紙帯のビットマップを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DeleteBandBlankBitmap( FIELD_PLACE_NAME* system )
{
  if( system->bmpOrg ) { 
    GFL_BMP_Delete( system->bmpOrg ); 
    system->bmpOrg = NULL;
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: DeleteBandBlankBitmap\n" );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 帯のビットマップウィンドウを破棄する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void DeleteBandBitmapWindow( FIELD_PLACE_NAME* system )
{
	if( system->bmpWin ) { 
    GFL_BMPWIN_Delete( system->bmpWin ); 
    system->bmpWin = NULL;
  }

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: DeleteBandBitmapWindow\n" );
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
 *
 * @return 指定したゾーンの地名表示フラグが立っている場合 TRUE
 */
//-----------------------------------------------------------------------------------
static BOOL CheckPlaceNameFlag( u16 zoneID )
{
  return ZONEDATA_GetPlaceNameFlag( zoneID );
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
