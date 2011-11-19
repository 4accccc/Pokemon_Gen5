//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		pokewood_rec.h
 *	@brief  ポケウッド用戦闘録画セーブデータ
 *	@author	Toru=Nagihashi
 *	@date		2011.06.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//全クライアントの操作内容保持バッファサイズ
//開始2-1=1 + アクション2+4*(1)+1+4*(1)+1=12-1=11*20ターン + 11*(6*2)入れ替え+ 制限時間2-1=1
#define POKEWOOD_REC_OPERATION_BUFFER_SIZE  (356)//(0xc00) 

//ポケウッドでセーブするクライアント数
#define POKEWOOD_REC_CLIENT_MAX             (1) //自分だけ

//ポケウッドでセーブする１クライアントあたりの手持ち数
#define POKEWOOD_REC_TEMOTI_MAX             (6)

//-------------------------------------
///	ロード番号
//=====================================
typedef enum
{
  LOADDATA_POKEWOOD0 = 0,
  LOADDATA_POKEWOOD1,
  LOADDATA_POKEWOOD2,
  LOADDATA_POKEWOOD3,
  LOADDATA_POKEWOOD4,
  LOADDATA_POKEWOOD5,
  LOADDATA_POKEWOOD6,
  LOADDATA_POKEWOOD7,

  LOADDATA_POKEWOOD_MAX,
}POKEWOOD_LOADDATA_NUM;

//-------------------------------------
///	取得引数
//=====================================
typedef enum 
{
  POKEWOOD_REC_PARAM_SENARIO,         ///< シナリオナンバー
  POKEWOOD_REC_PARAM_SUCCESS,         ///< 成功フラグ
  POKEWOOD_REC_PARAM_POKE0,           ///< 自分の手持ち0匹目のPOKEMON_PARAM
  POKEWOOD_REC_PARAM_POKE1,           ///< 自分の手持ち1匹目のPOKEMON_PARAM
  POKEWOOD_REC_PARAM_POKE2,           ///< 自分の手持ち2匹目のPOKEMON_PARAM
  POKEWOOD_REC_PARAM_POKE3,           ///< 自分の手持ち3匹目のPOKEMON_PARAM
  POKEWOOD_REC_PARAM_POKE4,           ///< 自分の手持ち4匹目のPOKEMON_PARAM
  POKEWOOD_REC_PARAM_POKE5,           ///< 自分の手持ち5匹目のPOKEMON_PARAM
  POKEWOOD_REC_PARAM_AUDIENCE_NUM,    ///< 初期観客数
  POKEWOOD_REC_PARAM_RENTAL_FLAG,     ///< レンタルポケモンを使用しているか？
  POKEWOOD_REC_PARAM_DATE,            ///< 日時
  POKEWOOD_REC_PARAM_PROFIT,          ///< 興行収入

} POKEWOOD_REC_PARAM;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ポケウッド戦闘録画本体
//=====================================
typedef struct _POKEWOOD_REC_WORK POKEWOOD_REC_WORK;

//-------------------------------------
///	ポケウッド操作内容保存バッファ
//=====================================
typedef struct _POKEWOOD_REC_OPERATION_BUFFER POKEWOOD_REC_OPERATION_BUFFER;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//save_tbl.cで仕様する関数
extern void POKEWOOD_REC_WorkInit( void *p_wk_adrs );
extern u32 POKEWOOD_REC_GetWorkSize( void );


//読み込みバッファの作成
extern void POKEWOOD_REC_Init( HEAPID heapID );
extern void POKEWOOD_REC_Exit( void );

//バッファ内容のバックアップ
void POKEWOOD_REC_Backup( void * backup_area );
//バッファ内容の復元
void POKEWOOD_REC_Restore( const void * backup_area );

//セーブ　ロード
extern LOAD_RESULT POKEWOOD_REC_Load( SAVE_CONTROL_WORK *p_sv, HEAPID heapID, POKEWOOD_LOADDATA_NUM num );
extern LOAD_RESULT POKEWOOD_REC_Load_ROM( SAVE_CONTROL_WORK *p_sv, HEAPID heapID, POKEWOOD_LOADDATA_NUM num );
extern SAVE_RESULT POKEWOOD_REC_Save( GAMEDATA *p_gamedata, HEAPID heapID, POKEWOOD_LOADDATA_NUM num, u16 *p_work );

//存在チェック
extern BOOL POKEWOOD_REC_IsExist( void );

//取得
extern POKEWOOD_REC_WORK* POKEWOOD_REC_GetWork( void );
extern u32 POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM param, void *dst );

//設定
extern void POKEWOOD_REC_SetData( POKEWOOD_REC_PARAM param, u32 data );

//撮影データを上映データへ変換
//->battle_rec_tool.cにあるのでBattleRec_LoadToolModuleしてください
extern void BattleRec_SetPokewoodRentalFlag( BOOL rental_flag );
