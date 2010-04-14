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

#include "pm_define.h"
#include "field_oam_pal.h"  // for FLDOAM_PALNO_PLACENAME

#include "arc/arc_def.h"
#include "arc/place_name.naix"
#include "arc/message.naix"
#include "msg/msg_place_name.h"

#include "../../../resource/fldmapdata/zonetable/zone_id.h"


//===================================================================================
// ■ 定数・マクロ
//=================================================================================== 
//#define DEBUG_PRINT_ON // デバッグ出力スイッチ
#define DEBUG_PRINT_TARGET (3) // デバッグ出力先
#define MAX_STATE_COUNT ( 0xffff ) // 状態カウンタ最大値

// 最大文字数
#define MAX_NAME_LENGTH (BUFLEN_PLACE_NAME - BUFLEN_EOM_SIZE)

// 1キャラ = 8ドット
#define CHAR_SIZE (8) 

// 無効ゾーンID
#define INVALID_ZONE_ID (0xffffffff)

//-----------
// 表示設定
//-----------
#define BG_PALETTE_NO       (0)					// BGパレット番号
#define BG_FRAME            (GFL_BG_FRAME3_M)	// 使用するBGフレーム
#define BG_FRAME_PRIORITY   (1)					// BGフレームのプライオリティ

#define	COLOR_NO_LETTER     (1) // 文字本体のカラー番号
#define	COLOR_NO_SHADOW     (2) // 影部分のカラー番号
#define	COLOR_NO_BACKGROUND (0) // 背景部のカラー番号

#define ALPHA_PLANE_1 (GX_BLEND_PLANEMASK_BG3)	// αブレンドの第1対象面
#define ALPHA_PLANE_2 (GX_BLEND_PLANEMASK_BG0)	// αブレンドの第2対象面
#define ALPHA_VALUE_1 (16)						// 第1対象面のαブレンディング係数
#define ALPHA_VALUE_2 ( 4)						// 第1対象面のαブレンディング係数

#define Y_CENTER_POS ( CHAR_SIZE * 2 - 1 )		// 背景帯の中心y座標

#define LAUNCH_INTERVAL (3)	// 文字発射間隔[単位：フレーム]

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

// パレットリソース
typedef enum {
	PLTT_RES_CHAR_UNIT,	// 文字ユニットで使用するパレット
	PLTT_RES_NUM        // 総数
} PLTT_RES_INDEX;

// セルアクター・ユニット
typedef enum {
	CLUNIT_CHAR_UNIT, // 文字ユニット
	CLUNIT_NUM,       // 総数
} CLUNIT_INDEX;

// 各状態の動作設定
#define PROCESS_TIME_FADEIN  (10)
#define PROCESS_TIME_WAIT_LAUNCH   (10)
#define PROCESS_TIME_WAIT_FADEOUT   (30)
#define PROCESS_TIME_FADEOUT (20)

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
  SYSTEM_STATE_MAX = SYSTEM_STATE_NUM - 1
} SYSTEM_STATE;


static const PN_LETTER_SETUP_PARAM LetterSetupParam = 
{
  256,                               // 座標
  Y_CENTER_POS - ( 13 / 2 ), // 
  -20,                               // 速度
  0,                                 // 
  24,                                // 目標位置
  Y_CENTER_POS - ( 13 / 2 ), // 
  0,                                 // 目標位置での速度
  0,                                 // 

  0, // フォント
  0, // 文字
};

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



//===================================================================================
// ■地名表示システム
//===================================================================================
struct _FIELD_PLACE_NAME { 

  GAMESYS_WORK* gameSystem;
  GAMEDATA*     gameData;
  WORDSET*      wordset;
	HEAPID        heapID;
  ARCHANDLE*    arcHandle;
	GFL_MSGDATA*  msg;
  GFL_FONT*     font;

	// 地名
	STRBUF* nameBuf; // 表示中の地名文字列
	u8      nameLen; // 表示中の地名文字数

	// 帯
	GFL_BMPWIN*   bmpWin;	     // ビットマップ・ウィンドウ
	GFL_BMP_DATA* bmpOrg;	     // 文字が書き込まれていない状態
  u32           nullCharPos; // NULLキャラクタのキャラNo.

	// OBJ
	u32            resPltt[PLTT_RES_NUM];  // パレットリソース
	BMPOAM_SYS_PTR bmpOamSys;              // BMPOAMシステム
	GFL_CLUNIT*    clunit[CLUNIT_NUM];     // セルアクターユニット

	// 動作に使用するデータ
	SYSTEM_STATE state;	// システム状態
  u8  stateSeq;       // 状態内シーケンス
	u16	stateCount;		  // 状態カウンタ

  u16  lastZoneID; // 最後に表示したゾーンID
  u16  dispZoneID; // 次に表示するゾーンID
  BOOL dispZoneSetFlag; // 次に表示するゾーンIDがセットされたかどうか
  BOOL forceDispFlag; // 強制表示フラグ

  PN_LETTER* letters[ MAX_NAME_LENGTH ];
  u8 setupLetterNum; // セットアップが完了した文字オブジェクトの数
	u8 launchLetterNum;// 発射済み文字数
  u8 writeLetterNum; // BG に書き込んだ文字数
};


//===================================================================================
// ■システムに関する関数
//===================================================================================
// BG
static void SetupBG( FIELD_PLACE_NAME* system );
static void CleanBG( FIELD_PLACE_NAME* system );
static void ResetBG( FIELD_PLACE_NAME* system );	
static void LoadBGPalette( FIELD_PLACE_NAME* system ); // BG パレットを読み込む
static void AllocBGNullCharArea( FIELD_PLACE_NAME* system ); // 空白キャラエリアを確保する
static void FreeBGNullCharaArea( FIELD_PLACE_NAME* system ); // 空白キャラエリアを解放する
static void RecoverBlankBand( FIELD_PLACE_NAME* system ); // 帯を文字が書き込まれていない状態に復帰する
// OBJ
static void LoadOBJResource( FIELD_PLACE_NAME* system ); // OBJ リソースを読み込む
static void UnloadOBJResource( FIELD_PLACE_NAME* system ); // OBJ リソースを解放する
static void CreateBmpOamSystem( FIELD_PLACE_NAME* system ); // BMPOAM システムを生成する
static void DeleteBmpOamSystem( FIELD_PLACE_NAME* system ); // BMPOAM システムを破棄する
static void CreateClactUnit( FIELD_PLACE_NAME* system ); // セルアクターユニットを生成する
static void DeleteClactUnit( FIELD_PLACE_NAME* system ); // セルアクターユニットを破棄する
// 文字オブジェクト
static void SetupLetter( FIELD_PLACE_NAME* system ); // 文字オブジェクトをセットアップする
static BOOL CheckLetterSetupFinished( const FIELD_PLACE_NAME* system ); // 文字オブジェクトのセットアップが完了したかどうかをチェックする
static void HideLetters( FIELD_PLACE_NAME* system ); // 文字オブジェクトを非表示にする
static void LaunchLetter( FIELD_PLACE_NAME* system ); // 文字オブジェクトを発射する
static void MoveAllCharUnit( FIELD_PLACE_NAME* system ); 
// BG, 文字ユニットへの書き込み
static void WriteCharUnitIntoBitmapWindow( FIELD_PLACE_NAME* system );
static BOOL CheckLetterWriteToBitmapFinished( const FIELD_PLACE_NAME* system );
// 地名の更新
static void SetForceDispFlag( FIELD_PLACE_NAME* system );
static void ResetForceDispFlag( FIELD_PLACE_NAME* system );
static BOOL CheckForceDispFlag( const FIELD_PLACE_NAME* system );
static BOOL CheckDispStart( const FIELD_PLACE_NAME* system );
static BOOL CheckPlaceNameIDChange( const FIELD_PLACE_NAME* system );
static void UpdatePlaceName( FIELD_PLACE_NAME* system ); 
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
static u16 GetLastZoneID( const FIELD_PLACE_NAME* system ); // 最後に表示したゾーンIDを取得する
static void SetLastZoneID( FIELD_PLACE_NAME* system, u16 zoneID ); // 最後に表示したゾーンIDを設定する
static u16 GetDispZoneID( const FIELD_PLACE_NAME* system ); // 次に表示するゾーンIDを取得する
static void SetDispZoneID( FIELD_PLACE_NAME* system, u16 zoneID ); // 次に表示するゾーンIDを設定する
static const STRBUF* GetPlaceName( const FIELD_PLACE_NAME* system ); // 地名バッファを取得する
static u8 GetPlaceNameLength( const FIELD_PLACE_NAME* system ); // 地名の文字数を取得する
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
  OpenDataHandle( system );
  CreateWordset( system );
  CreateMessageData( system );
  CreatePlaceNameBuffer( system );
  SetHeapID( system, heapID );
  SetFont( system, FLDMSGBG_GetFontHandle( msgbg ) );

	LoadOBJResource( system );
	CreateClactUnit( system );
  CreateBmpOamSystem( system );

	CreateLetters( system ); // 文字オブジェクトを生成

  GFL_BG_SetBGControl( BG_FRAME, &BGCntHeader, GFL_BG_MODE_TEXT ); 

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
  DeleteLetters( system ); 
  DeletePlaceNameBuffer( system ); 
  DeleteBmpOamSystem( system ); 
  DeleteClactUnit( system );
  UnloadOBJResource( system ); 
  DeleteWordset( system );
  DeleteMessageData( system ); 
  DeleteBandBitmapWindow( system );
  DeleteBandBlankBitmap( system );
  CloseDataHandle( system );

	GFL_BG_FreeBGControl( BG_FRAME ); 

  DeletePNSystem( system );
} 


//====================================================================================
// ■動作
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 地名表示システムの動作処理
 *
 * @param system
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Process( FIELD_PLACE_NAME* system )
{
  IncStateCount( system ); // 状態カウンタを更新

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

	// 文字ユニットを動かす
  MoveAllCharUnit( system );
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
 * @brief ゾーンの切り替えを通知し, 新しい地名を表示する
 *
 * @param system
 * @param zoneID ゾーンID
 */
//------------------------------------------------------------------------------------
void FIELD_PLACE_NAME_Display( FIELD_PLACE_NAME* system, u32 zoneID )
{ 
  // ゾーンの地名表示フラグが立っていない場所では表示しない
  if( ZONEDATA_GetPlaceNameFlag( zoneID ) == FALSE ) { return; }

  // 初回は強制表示
  if( system->lastZoneID == ZONE_ID_MAX ) {
    SetForceDispFlag( system );
  }

	// 指定されたゾーンIDを次に表示すべきものとして記憶
  SetDispZoneID( system, zoneID );

	// 表示中のウィンドウを退出させる
	Cancel( system ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief 地名ウィンドウを強制的に表示する
 *
 * @param system
 * @param zoneID 表示する場所のゾーンID
 */
//------------------------------------------------------------------------------------
extern void FIELD_PLACE_NAME_DisplayForce( FIELD_PLACE_NAME* system, u32 zoneID )
{
  // 強制的に表示
  SetDispZoneID( system, zoneID );
  SetForceDispFlag( system );

	// 表示中のウィンドウを退出させる
	Cancel( system ); 
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
	LoadBGPalette( system ); 
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
 * @brief BGパレットを読み込む
 *
 * @param system
 */
//------------------------------------------------------------------------------------
static void LoadBGPalette( FIELD_PLACE_NAME* system )
{
  ARCDATID datID;
	ARCHANDLE* handle;
  HEAPID heapID;
	u32 size;
	void* src;
	NNSG2dPaletteData* pal;

  heapID = GetHeapID( system ); 
	handle = GetArcHandle( system );
  datID  = NARC_place_name_place_name_back_NCLR;

	size = GFL_ARC_GetDataSizeByHandle( handle, datID );// データサイズ取得
	src = GFL_HEAP_AllocMemory( system->heapID, size );	// データバッファ確保
	GFL_ARC_LoadDataByHandle( handle, datID, src );			// データ取得
	NNS_G2dGetUnpackedPaletteData( src, &pal );					// バイナリからデータを展開
	GFL_BG_LoadPalette( BG_FRAME, pal->pRawData, 0x20, BG_PALETTE_NO );	// パレットデータ転送
	GFL_HEAP_FreeMemory( src );											    // データバッファ解放
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

	// 文字が書き込まれていない状態をコピーして, VRAMに転送
	GFL_BMP_Copy( src, dest );	
	//GFL_BMPWIN_MakeTransWindow( system->bmpWin );
	//GFL_BMPWIN_TransVramCharacter( system->bmpWin );
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

	system->resPltt[ PLTT_RES_CHAR_UNIT ] = 
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
	system->clunit[ CLUNIT_CHAR_UNIT ] = 
		GFL_CLACT_UNIT_Create( MAX_NAME_LENGTH, BG_FRAME_PRIORITY, heapID );

	// 初期設定
	GFL_CLACT_UNIT_SetDrawEnable( system->clunit[ CLUNIT_CHAR_UNIT ], TRUE );
  
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
        system->heapID, system->bmpOamSys, system->resPltt[ PLTT_RES_CHAR_UNIT ] );
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
 * @brief 文字オブジェクトをセットアップする
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void SetupLetter( FIELD_PLACE_NAME* system )
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
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: SetupLetter[%d]\n", idx );
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
static BOOL CheckLetterSetupFinished( const FIELD_PLACE_NAME* system )
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
 * @brief 全文字ユニットを動かす
 *
 * @param sys 動作対象システム
 */
//-----------------------------------------------------------------------------------
static void MoveAllCharUnit( FIELD_PLACE_NAME* sys )
{
  int i;

	for( i=0; i<sys->nameLen; i++ )
	{
    PN_LETTER_Main( sys->letters[i] );
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
	system->bmpOamSys = BmpOam_Init( heapID, system->clunit[ CLUNIT_CHAR_UNIT ] );

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
 * @brief 表示中の文字ユニットをビットマップ・ウィンドウに書き込む
 *
 * @param sys 操作対象システム
 */
//-----------------------------------------------------------------------------------
static void WriteCharUnitIntoBitmapWindow( FIELD_PLACE_NAME* sys )
{
  int idx;
	u16 dx, dy;
  u16 width, height;
  PN_LETTER* letter;
	const GFL_BMP_DATA* p_src_bmp = NULL;
	GFL_BMP_DATA* p_dst_bmp = GFL_BMPWIN_GetBmp( sys->bmpWin );

  idx = sys->writeLetterNum;
  GF_ASSERT( idx < GetPlaceNameLength(sys) );

  letter = sys->letters[idx];
  p_src_bmp = PN_LETTER_GetBitmap( letter );
  dx = PN_LETTER_GetLeft( letter ) - ( BMPWIN_POS_X_CHAR * CHAR_SIZE );
  dy = PN_LETTER_GetTop( letter )  - ( BMPWIN_POS_Y_CHAR * CHAR_SIZE );
  width = PN_LETTER_GetWidth( letter );
  height = PN_LETTER_GetHeight( letter );
  GFL_BMP_Print( p_src_bmp, p_dst_bmp, 0, 0, dx, dy, width, height, 0 );

  sys->writeLetterNum++;

	// 更新されたキャラデータをVRAMに転送
	//GFL_BMPWIN_TransVramCharacter( sys->bmpWin );
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
static BOOL CheckLetterWriteToBitmapFinished( const FIELD_PLACE_NAME* system )
{
  if( GetPlaceNameLength(system) <= system->writeLetterNum ) {
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
 * @brief 地名の更新をチェックする
 *
 * @param system
 *
 * @return 地名の表示を開始する場合 TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------
static BOOL CheckDispStart( const FIELD_PLACE_NAME* system )
{
  // 強制表示
  if( CheckForceDispFlag(system) ) { return TRUE; }

  // 新しいゾーンが通知された
  if( system->dispZoneSetFlag ) {
    if( system->lastZoneID != system->dispZoneID ) {
      return TRUE;
    }
  }

  return FALSE;
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
  u32 lastID = ZONEDATA_GetPlaceNameID( system->lastZoneID );
  u32 dispID = ZONEDATA_GetPlaceNameID( system->dispZoneID );

  OS_Printf( "lastZoneID = %d, lastID = %d\n", system->lastZoneID, lastID );
  OS_Printf( "dispZoneID = %d, dispID = %d\n", system->dispZoneID, dispID );

  // 地名に変化がない
  if( lastID == dispID ) {
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
 * @param sys 更新するシステム
 */
//-----------------------------------------------------------------------------------
static void UpdatePlaceName( FIELD_PLACE_NAME* sys )
{
	u16 str_id;
  BOOL intrudeFlag = FALSE;
  INTRUDE_COMM_SYS_PTR intrudeComm;
  u8 intrudeNetID;
  
	// ゾーンIDから地名文字列を取得する
	str_id = ZONEDATA_GetPlaceNameID( GetDispZoneID(sys) );

  // エラー回避
	if( (str_id < 0) || (msg_place_name_max <= str_id) ){ str_id = 0; } 
	if( str_id == 0 ) 
  { //「なぞのばしょ」なら表示しない
    OBATA_Printf( "「なぞのばしょ」を検出( zone id = %d )\n", GetDispZoneID(sys) );
    FIELD_PLACE_NAME_Hide( sys );
  }

  // 侵入先かどうかを判定
  {
    GAME_COMM_SYS_PTR gameComm;
    FIELD_STATUS* fieldStatus;
    int myNetID;

    gameComm = GAMESYSTEM_GetGameCommSysPtr( sys->gameSystem );
    intrudeComm = Intrude_Check_CommConnect( gameComm );
    fieldStatus = GAMEDATA_GetFieldStatus( sys->gameData );
    myNetID = GAMEDATA_GetIntrudeMyID( sys->gameData );

    if( intrudeComm ) {
      intrudeNetID = Intrude_GetPalaceArea( intrudeComm );

      // 他人のフィールドにいる
      if( FIELD_STATUS_GetMapMode( fieldStatus ) == MAPMODE_INTRUDE ) { 
        intrudeFlag = TRUE;
      }
      // 他人のパレスにいる
      else if( ZONEDATA_IsPalace( GetDispZoneID(sys) ) && (myNetID != intrudeNetID) ) {
        intrudeFlag = TRUE;
      }
    }
    else {
      intrudeFlag = FALSE;
    }
  }

  // 侵入先にいる
  if( intrudeFlag ) {
    // 侵入先のプレイヤー名を展開
    STRBUF* strbuf = GFL_MSG_CreateString( sys->msg, MAPNAME_INTRUDE );
    MYSTATUS* status = Intrude_GetMyStatus( intrudeComm, intrudeNetID );
    GFL_MSG_GetString( sys->msg,	str_id, sys->nameBuf );
    WORDSET_RegisterPlayerName( sys->wordset, 0, status );
    WORDSET_RegisterPlaceName( sys->wordset, 1, str_id );
    WORDSET_ExpandStr( sys->wordset, sys->nameBuf, strbuf );
    GFL_STR_DeleteBuffer( strbuf );
  }
  // 自分のフィールドにいる
  else {
    GFL_MSG_GetString( sys->msg,	str_id, sys->nameBuf );
  }
  sys->nameLen = GFL_STR_GetBufferLength( sys->nameBuf );
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
  ResetStateCount( system ); // 状態カウンタをリセット
  ResetStateSeq( system ); // 状態内シーケンスをリセット

	// 遷移先の状態に応じた初期化
	switch( next_state ) {
		case SYSTEM_STATE_HIDE:	
			GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF );	// BGを非表示
			HideLetters( system ); // 文字オブジェクトを非表示にする
			break;
		case SYSTEM_STATE_FADEIN:
      //GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON );	// BGを表示
      //Draw_FADEIN( system );
			break;
		case SYSTEM_STATE_LAUNCH:
			//system->launchLetterNum = 0;	// 発射文字数をリセット
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
  switch( GetStateSeq(system) ) {
  case 0:
    if( CheckDispStart( system ) ) {
      system->dispZoneSetFlag = FALSE;
      IncStateSeq( system );
    }
    break;

  case 1:
    if( CheckForceDispFlag(system) ) {
      IncStateSeq( system );
    }
    else if( CheckPlaceNameIDChange( system ) ) {
      IncStateSeq( system );
    }
    else {
      ResetStateSeq( system );
    }
    break;

  case 2:
    // 表示開始
    ResetForceDispFlag( system );
    SetLastZoneID( system, GetDispZoneID(system) );
    ChangeState( system, SYSTEM_STATE_SETUP );
    break;
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
    system->setupLetterNum = 0;
    UpdatePlaceName( system );
    IncStateSeq( system );
    break;

  case 1:
    GFL_BG_ClearCharacter( BG_FRAME );
    IncStateSeq( system );
    break; 
  case 2:
    GFL_BG_ClearScreen( BG_FRAME );
    IncStateSeq( system );
    break;
  case 3:
    FreeBGNullCharaArea( system ); 
    DeleteBandBitmapWindow( system );
    DeleteBandBlankBitmap( system );
    IncStateSeq( system );
    break; 

  case 4:
    LoadBGPalette( system ); 
    IncStateSeq( system );
    break; 
  case 5:
    AllocBGNullCharArea( system );
    IncStateSeq( system );
    break; 
  case 6:
    CreateBandBlankBitmap( system );
    IncStateSeq( system );
    break; 
  case 7:
    CreateBandBitmapWindow( system );
    IncStateSeq( system );
    break; 
  case 8:
    RecoverBlankBand( system );
    IncStateSeq( system );
    break;
  case 9:
    GFL_BMPWIN_MakeTransWindow( system->bmpWin ); 
    IncStateSeq( system );
    break; 

  case 10:
    GFL_BG_SetPriority( BG_FRAME, BG_FRAME_PRIORITY );
    GFL_BG_SetVisible( BG_FRAME, VISIBLE_OFF ); 
    G2_SetBlendAlpha( ALPHA_PLANE_1, ALPHA_PLANE_2, ALPHA_VALUE_1, ALPHA_VALUE_2 );
    IncStateSeq( system );
    break; 

  case 11:
    SetupLetter( system ); // 文字オブジェクトをセットアップ

    // 全ての文字オブジェクトのセットアップが完了
    if( CheckLetterSetupFinished(system) ) {
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
    GFL_BG_SetVisible( BG_FRAME, VISIBLE_ON );	// BGを表示
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
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_WAIT_LAUNCH( FIELD_PLACE_NAME* sys )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_WAIT_LAUNCH < sys->stateCount )
	{
		ChangeState( sys, SYSTEM_STATE_LAUNCH );
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
      sys->writeLetterNum = 0;
      IncStateSeq( sys );
    }
    break;

  case 1:
    WriteCharUnitIntoBitmapWindow( sys );
    // 全ての文字の書き込み完了
    if( CheckLetterWriteToBitmapFinished(sys) ) {
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
 * @param sys 動かすシステム
 */
//-----------------------------------------------------------------------------------
static void Process_FADEOUT( FIELD_PLACE_NAME* sys )
{
	// 一定時間が経過したら, 次の状態へ
	if( PROCESS_TIME_FADEOUT < sys->stateCount )
	{
		ChangeState( sys, SYSTEM_STATE_HIDE );
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
  system->gameSystem = gameSystem;
  system->gameData = GAMESYSTEM_GetGameData( gameSystem );
  system->nullCharPos = AREAMAN_POS_NOTFOUND;
  system->forceDispFlag = FALSE;
  system->dispZoneSetFlag = FALSE;
  system->lastZoneID = ZONE_ID_MAX;
  system->dispZoneID = ZONE_ID_MAX;

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

  GF_ASSERT( system->msg == NULL );

  heapID = GetHeapID( system ); 
	system->msg = GFL_MSG_Create( 
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
  if( system->msg ) {
    GFL_MSG_Delete( system->msg );
    system->msg = NULL;
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
  return system->msg;
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
 * @brief 最後に表示したゾーンIDを取得する
 *
 * @param system
 *
 * @return 最後に表示した場所のゾーンID
 */
//-----------------------------------------------------------------------------------
static u16 GetLastZoneID( const FIELD_PLACE_NAME* system )
{
  return system->lastZoneID;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 最後に表示したゾーンIDを設定する
 *
 * @param system
 * @param zoneID
 */
//-----------------------------------------------------------------------------------
static void SetLastZoneID( FIELD_PLACE_NAME* system, u16 zoneID )
{
  system->lastZoneID = zoneID;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: SetLastZoneID (%d)\n", zoneID );
#endif
}

//-----------------------------------------------------------------------------------
/**
 * @brief 次に表示するゾーンIDを取得する
 *
 * @param system
 *
 * @return 次に表示する場所のゾーンID
 */
//-----------------------------------------------------------------------------------
static u16 GetDispZoneID( const FIELD_PLACE_NAME* system )
{
  return system->dispZoneID;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 次に表示するゾーンIDを設定する
 *
 * @param system
 */
//-----------------------------------------------------------------------------------
static void SetDispZoneID( FIELD_PLACE_NAME* system, u16 zoneID )
{
  system->dispZoneID = zoneID;
  system->dispZoneSetFlag = TRUE;

#ifdef DEBUG_PRINT_ON
  OS_TFPrintf( DEBUG_PRINT_TARGET, "FIELD-PLACE-NAME: SetDispZoneID (%d)\n", zoneID );
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






























