//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_inner.h
 *	@brief	バトルレコーダー	プライベートヘッダ
 *	@author	Toru=Nagihashi
 *	@date		2009.11.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "net_app/gds/gds_rap.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	BG面設定
//=====================================
enum 
{
  BG_FRAME_M_TEXT = GFL_BG_FRAME0_M,
	BG_FRAME_M_FONT	= GFL_BG_FRAME1_M,
	BG_FRAME_M_WIN	= GFL_BG_FRAME2_M,
	BG_FRAME_M_BACK	= GFL_BG_FRAME3_M,
	BG_FRAME_S_FONT	= GFL_BG_FRAME0_S,
	BG_FRAME_S_WIN	= GFL_BG_FRAME1_S,
	BG_FRAME_S_BACK	= GFL_BG_FRAME2_S,
};

//-------------------------------------
///	PLT設定
//=====================================
enum 
{
	//BG
	PLT_BG_M_COMMON	= 0,
	PLT_BG_S_COMMON	= 0,
  PLT_BG_M_FONT	  = 14,
  PLT_BG_S_FONT	  = 14,

	//PLT
	PLT_OBJ_M_COMMON	  = 0,  //3本
  PLT_OBJ_M_POKEICON  = 4,  //3本

  PLT_OBJ_M_PMS       = 7,
	PLT_OBJ_S_COMMON	  = 0,  //3本
  PLT_OBJ_S_SPECIAL   = 4,

};

//-------------------------------------
///	文字色（BMPWINフォント）
//=====================================
#define BR_PRINT_COL_BOOT	  	( PRINTSYS_LSB_Make(  1, 14,  0 ) )	// 起動時の文字色
#define BR_PRINT_COL_NORMAL 	( PRINTSYS_LSB_Make( 15, 13,  0 ) )	// よく使う文字色
#define	BR_PRINT_COL_BLACK		( PRINTSYS_LSB_Make( 0xB, 0xD,  0 ) )	// フォントカラー
#define	BR_PRINT_COL_INFO		  ( PRINTSYS_LSB_Make( 15, 13, 12 ) )	// フォントカラー
#define	BR_PRINT_COL_PHOTO		( BR_PRINT_COL_NORMAL )
#define	BR_PRINT_COL_BOX_SHOT	( BR_PRINT_COL_NORMAL )
#define	BR_PRINT_COL_BOX_NAME	( PRINTSYS_LSB_Make( 15,  1,  0 ) )
#define	BR_PRINT_COL_FREC		  ( BR_PRINT_COL_NORMAL )
#define	BR_PRINT_COL_PROF		  ( BR_PRINT_COL_NORMAL )


//-------------------------------------
///外部セーブデータの状況
//=====================================
typedef enum
{ 
  BR_SAVEDATA_PLAYER,
  BR_SAVEDATA_OTHER_00,
  BR_SAVEDATA_OTHER_01,
  BR_SAVEDATA_OTHER_02,
  BR_SAVEDATA_NUM,
} BR_SAVEDATA_IDX;
typedef struct 
{
  BOOL    is_valid[BR_SAVEDATA_NUM]; //セーブデータが存在するか
  STRBUF  *p_name[BR_SAVEDATA_NUM];  //名前
  u32     sex[BR_SAVEDATA_NUM];      //性別
  u64     video_number[BR_SAVEDATA_NUM]; //ビデオナンバー
  BOOL    is_musical_valid;             //ミュージカルは存在するか
  BOOL    is_check;                     //１度チェックしたか
} BR_SAVE_INFO;

//-------------------------------------
///	ランキングで必要な情報
//=====================================
#define BR_OUTLINE_RECV_MAX   (30)
#define BR_OUTLINE_LOCAL_DATA (0xFFFF)
typedef struct 
{
	BATTLE_REC_OUTLINE_RECV	    data[ BR_OUTLINE_RECV_MAX ];			  ///< DL した概要一覧
	u8							            is_secure[ BR_OUTLINE_RECV_MAX ];	  ///< DL した概要のフラグ
	int							            data_num;			                      ///< DL した概要一覧の件数
  int                         data_idx;                           ///< 現在みているIDX（ローカルならばBR_OUTLINE_LOCAL_DATA）
} BR_OUTLINE_DATA;

//-------------------------------------
///	レコードで必要な情報
//=====================================
typedef struct 
{
  u64                       video_number;   //録画番号
} BR_RECORD_DATA;

//-------------------------------------
///	バトルレコーダー  ボタン管理復帰データ
//=====================================
#define BR_BTN_RECORVER_STACK_MAX 6
typedef struct 
{
  struct
  { 
    u16	  menuID;
    u16   btnID;
  } stack[BR_BTN_RECORVER_STACK_MAX];
  u32 stack_num;
} BR_BTN_SYS_RECOVERY_DATA;

//-------------------------------------
///	リスト位置
//=====================================
typedef struct 
{
  u16 list;
  u16 cursor;
} BR_LIST_POS;

//-------------------------------------
///	汎用
//=====================================
#define BR_VIDEO_NUMBER_FIG         12 //バトルビデオナンバーの桁
#define BR_VIDEO_NUMBER_BLOCK0_FIG  2  //バトルビデオナンバーブロック０の桁
#define BR_VIDEO_NUMBER_BLOCK1_FIG  5  //バトルビデオナンバーブロック１の桁
#define BR_VIDEO_NUMBER_BLOCK2_FIG  5  //バトルビデオナンバーブロック２の桁

#define RR_SEARCH_SE_FRAME         (96)   //SEを最低限ならすフレーム

//-------------------------------------
///	便利マクロ
//=====================================
static inline void BR_TOOL_GetVideoNumberToBlock( u64 number, u32 tbl[], u32 block_num )
{ 
  GF_ASSERT( block_num == 3 );

  tbl[ 0 ] = number % 100000;	///< 3block
  number /= 100000;
  tbl[ 1 ] = number % 100000;	///< 2block
  number /= 100000;
  tbl[ 2 ] = number;				    ///< 1block
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  デバッグ用
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_
#ifdef PM_DEBUG
static inline void DEBUG_BR_OUTLINE_SetData( BR_OUTLINE_DATA *p_data )
{ 
  static const u16 * sc_debug_name[] =
  { 
    L"ホワイ",
    L"ブラク",
    L"ダイヤ",
    L"パール",
    L"ハート",
    L"ソウル",
  };

  int i;
  int j;


  for( i = 0; i < BR_OUTLINE_RECV_MAX; i++ )
  { 
    //名前
    for( j = 0 ; j < 3; j++ )
    { 
      p_data->data[i].profile.name[j]  = sc_debug_name[ i % NELEMS(sc_debug_name) ][j];
    }
    p_data->data[i].profile.name[j] = GFL_STR_GetEOMCode();

    //モンスター番号
    for( j = 0 ; j < HEADER_MONSNO_MAX; j++ )
    {
      p_data->data[i].head.monsno[j]  = i+1;
      p_data->data[i].head.form_no_and_sex[j] = 0;
    }
  }
  p_data->data_num  = i;
}
#endif //PM_DEBUG
