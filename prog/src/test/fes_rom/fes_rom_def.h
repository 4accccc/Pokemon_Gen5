/*
 *  @file   fes_rom_def.h
 *  @brief  フェスミッション配信ロムアプリ リテラル定義
 *  @author Miyuki Iwasawa
 *  @date   12.04.03
 */

#pragma once

#define FES_ROM_HEAP_SIZE  (0x80000)  ///< ヒープサイズ

///WordSet
#define WORDSET_FES_ROM_BUFLEN  (256*2+EOM_SIZE)
#define WORDSET_FES_ROM_SETNUM  (32)

enum{
 SEQ_INI_DRAW,
 SEQ_FADEIN,
 SEQ_BASE_MAIN,
 SEQ_TASK_WAIT,
 SEQ_FADEOUT,
 SEQ_EXIT,
};

enum{
 KEY_LEFT,
 KEY_RIGHT,
 KEY_DOWN,
 KEY_UP,
};

//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_MSG_M	= GFL_BG_FRAME0_M,
	BG_FRAME_WIN_M	= GFL_BG_FRAME1_M,
	BG_FRAME_BASE_M	= GFL_BG_FRAME2_M,
	BG_FRAME_BACK_M	= GFL_BG_FRAME3_M,
	BG_FRAME_MSG_S	= GFL_BG_FRAME0_S,
	BG_FRAME_WIN_S	= GFL_BG_FRAME1_S,
	BG_FRAME_BASE_S	= GFL_BG_FRAME2_S,
	BG_FRAME_BACK_S	= GFL_BG_FRAME3_S,
};
//-------------------------------------
///	パレット
//=====================================
enum
{	
	//メインBG
	PLTID_BG_FONT_M				= 0,
	PLTID_BG_BACK_M			  =	1,
	PLTID_BG_SYSFONT_M	  = 3, //ポップアップウィンドウパレット
	PLTID_BG_SYSWIN_M	    = 4, //ポップアップウィンドウパレット
	PLTID_BG_TOUCHBAR_M		= 13, //タッチバー2本占有
	PLTID_BG_LOCALIZE_M		= 15, //ローカライズ用

	//サブBG
  PLTID_BG_FONT_S       = 0, //フォント
	PLTID_BG_BACK_S			  =	1,
	PLTID_BG_SYSFONT_S  	=	2,
	PLTID_BG_SYSWIN_S	   	=	3,
  PLTID_BG_PMS_S        = 10, //簡易会話デコメ用5本占有
	PLTID_BG_LOCALIZE_S		= 15, //ローカライズ用

	//メインOBJ
	PLTID_OBJ_NORMAL_M	= 0, //通常
  PLTID_OBJ_UNION_M = 10, //Unionオブジェ用
	PLTID_OBJ_TOUCHBAR_M	= 11, // 3本使用
	PLTID_OBJ_LOCALIZE_M	= 14,		//ローカライズ用
  PLTID_OBJ_WMI_M = 15,	  //通信アイコン

  //サブOBJ
  PLTID_OBJ_NORMAL_S = 0, //
  PLTID_OBJ_TRAINER_S = 8, // 2本使用
  PLTID_OBJ_PMS_DRAW = 10, // 5本使用
	PLTID_OBJ_LOCALIZE_S	= 15,		//ローカライズ用
};

#define BUFLEN_TMP_MSG  (256*2+EOM_SIZE)

#define	FCOL_FNTOAM   ( PRINTSYS_MACRO_LSB(1,2,0) )	 ///<OAMフォント黒抜
#define FCOL_FNTOAM_W ( PRINTSYS_MACRO_LSB(15,3,0))  ///<Oam白抜き
#define FCOL_WHITE_N  ( PRINTSYS_MACRO_LSB(15,2,0) )  ///<BG白抜き
#define FCOL_RED_N    ( PRINTSYS_MACRO_LSB(4,3,0) )  ///<BG赤抜き
#define FCOL_BLUE_N   ( PRINTSYS_MACRO_LSB(6,5,0) )  ///<BG青抜き
#define FCOL_YELLOW_N ( PRINTSYS_MACRO_LSB(8,7,0) )  ///<BG黄抜き
#define FCOL_GREEN_N  ( PRINTSYS_MACRO_LSB(10,9,0) )  ///<BG緑抜き
#define FCOL_PINK_N   ( PRINTSYS_MACRO_LSB(12,11,0) )  ///<BGピンク抜き
#define FCOL_GRAY_N   ( PRINTSYS_MACRO_LSB(1,2,0) )  ///<BGグレー抜き

#define FCOL_BLACK_W  ( PRINTSYS_MACRO_LSB(1,2,15) )  ///<BG黒白地

#define FCOL_POPUP_BASE (15)
#define FCOL_POPUP_MAIN (7)
#define FCOL_POPUP_SDW  (8)
#define FCOL_POPUP      ( PRINTSYS_MACRO_LSB(FCOL_POPUP_MAIN,FCOL_POPUP_SDW,FCOL_POPUP_BASE))  //BGポップアップ

#define FCOL_WIN_WHITE (15)
#define FCOL_WIN_CLEAR  (0)
#define FCOL_WIN_BASE1  (3)
#define FCOL_WIN_BASE2  (4)
#define FCOL_WIN_MAIN   (15)
#define FCOL_WIN_SDW    (14)

#define FCOL_WIN01 ( PRINTSYS_MACRO_LSB(FCOL_WIN_MAIN,FCOL_WIN_SDW,FCOL_WIN_BASE1))  //ビーコンウィンドウ1
#define FCOL_WIN02 ( PRINTSYS_MACRO_LSB(FCOL_WIN_MAIN,FCOL_WIN_SDW,FCOL_WIN_BASE2))  //ビーコンウィンドウ2

//画面ターゲットセット数
typedef enum{
 TARGET_VIEW_U,
 TARGET_VIEW_D,
 TARGET_VIEW_MAX,
}TARGET_VIEW;

//ターゲット内ウィンドウ数
typedef enum{
 WIN_BASE,
 WIN_INPUT,
 WIN_MSG,
 WIN_MAX,
}WIN_TYPE;

//ビーコンウィンドウの数
#define BEACON_WIN_MAX    (2)

#define UNION_CHAR_MAX  (16)

///////////////////////////////////////////////////
///ライン
enum{
 //up
 LINE_0,
 LINE_1,
 LINE_2,
 LINE_3,
 LINE_4,
 LINE_5,
 LINE_6,
 LINE_7,
 LINE_8,
 LINE_9,
 LINE_10,
 LINE_11,
 //down
 LINE_12,
 LINE_13,
 LINE_14,
 LINE_15,
 LINE_16,
 LINE_17,
 LINE_18,
 LINE_19,
 LINE_20,
 LINE_21,
 LINE_22,
 LINE_VIEW_NUM = 12,
 LINE_W = 8,
 LINE_H = 16,
};

typedef enum{
 SEND_MODE_TURN,
 SEND_MODE_RANDOM,
}SEND_MODE;

#define POS_TARGET_NONE  (-1)
///設定データID
typedef enum{
  PRM_BASE_ID_START = 0,
  PRM_BASE_BEACON_INTERVAL = PRM_BASE_ID_START,
  PRM_BASE_START_TIME,
  PRM_BASE_END_TIME,

  PRM_BASE_MODE,
  PRM_BASE_TR_VIEW,
  
  PRM_BASE_TR_NAME,
  PRM_BASE_SELFMSG,
  
  PRM_BASE_SAVE,
  PRM_BASE_SEND_START,
  
  PRM_BASE_MSEND_ID01,
  PRM_BASE_MSEND_TIME01,
  PRM_BASE_MSEND_BLANK01,
  PRM_BASE_MSEND_PER01,
  
  PRM_BASE_MSEND_ID02,
  PRM_BASE_MSEND_TIME02,
  PRM_BASE_MSEND_BLANK02,
  PRM_BASE_MSEND_PER02,

  PRM_BASE_MSEND_ID03,
  PRM_BASE_MSEND_TIME03,
  PRM_BASE_MSEND_BLANK03,
  PRM_BASE_MSEND_PER03,

  PRM_BASE_MSEND_ID04,
  PRM_BASE_MSEND_TIME04,
  PRM_BASE_MSEND_BLANK04,
  PRM_BASE_MSEND_PER04,

  PRM_BASE_MSEND_ID05,
  PRM_BASE_MSEND_TIME05,
  PRM_BASE_MSEND_BLANK05,
  PRM_BASE_MSEND_PER05,

  PRM_BASE_MSEND_ID06,
  PRM_BASE_MSEND_TIME06,
  PRM_BASE_MSEND_BLANK06,
  PRM_BASE_MSEND_PER06,

  PRM_BASE_MSEND_ID07,
  PRM_BASE_MSEND_TIME07,
  PRM_BASE_MSEND_BLANK07,
  PRM_BASE_MSEND_PER07,

  PRM_BASE_MSEND_ID08,
  PRM_BASE_MSEND_TIME08,
  PRM_BASE_MSEND_BLANK08,
  PRM_BASE_MSEND_PER08,

  PRM_BASE_MSEND_ID09,
  PRM_BASE_MSEND_TIME09,
  PRM_BASE_MSEND_BLANK09,
  PRM_BASE_MSEND_PER09,

  PRM_BASE_MSEND_ID10,
  PRM_BASE_MSEND_TIME10,
  PRM_BASE_MSEND_BLANK10,
  PRM_BASE_MSEND_PER10,

  PARAM_ID_MAX,
  PARAM_ID_NONE=PARAM_ID_MAX,
}PARAM_ID;

///設定データワードセット関連
typedef enum{
 WSET_NONE,
 WSET_BEACON_INTERVAL,
 WSET_TIME,
 WSET_MODE,
 WSET_TR_VIEW,
 WSET_TR_NAME,
 WSET_SELFMSG,

 WSET_MSEND_ID,
 WSET_MSEND_TIME,
 WSET_MSEND_BLANK,
 WSET_MSEND_PER,
 //
 WSET_MONSNAME,
 WSET_ITEMNAME,
 WSET_ITEMNAME_CHECK,
 WSET_ITEMNAME_CHECK_SP,
 WSET_FREEWORD,
 WSET_MBASE_EVENT,
 WSET_MBASE_NPC,
 WSET_MBASE_ITEM,
 WSET_MEX_EVENT_TRAIL,
 WSET_MEX_NPC_ITEM_TRADE,
 WSET_MEX_NPC_SHOPPING,
 WSET_MEX_NPC_TR_BATTLE,
 WSET_MEX_NPC_GET_A_PRESENT,
 WSET_MEX_NPC_POKE_QUIZ,
 WSET_MISSION_ID_NAME,
 WSET_MISSION_MSGID_NAME,
 WSET_TYPE_MAX,
}WSET_TYPE;

//実行時処理コード
typedef enum{
  EXE_CODE_INPUT_SWITCH,  //スイッチ式入力実行
  EXE_CODE_INPUT_NUM,     //数値入力実行
  EXE_CODE_INPUT_SELFMSG, //フリーワード入力実行
  EXE_CODE_INPUT_NAME,    //名前入力実行
  EXE_CODE_SAVE,          //セーブ実行
  EXE_CODE_SEND_START,    //配信開始

  EXE_CODE_MSEND_ID,   //ミッション選択
  EXE_CODE_MSEND_PER,  //ミッション送信パーセンテージ
}EXE_CODE;

//入力パターンコード
typedef enum{
 INPUT_CODE_NONE,
 INPUT_CODE_BEACON_INTERVAL,
 INPUT_CODE_TIME,
 INPUT_CODE_MODE,
 INPUT_CODE_TR_VIEW,
 INPUT_CODE_MSEND_TIME,
 INPUT_CODE_MSEND_BLANK,
 INPUT_CODE_MSEND_PER,
 INPUT_CODE_MAX,
}INPUT_CODE;

///////////////////////////////////////////////////
//SE関連

#define FESROM_SE_CANCEL (SEQ_SE_CANCEL1)
#define FESROM_SE_DECIDE (SEQ_SE_DECIDE1)
#define FESROM_SE_POPUP  (SEQ_SE_DECIDE2)

///////////////////////////////////////////////////
//BMP関連

/////////////////////////////////
#define BMP_BASE_PX (0)
#define BMP_BASE_PY (0)
#define BMP_BASE_SX (32)
#define BMP_BASE_SY (24)
#define BMP_BASE_PAL ( PLTID_BG_FONT_M )

#define BMP_INPUT_PX (0)
#define BMP_INPUT_PY (0)
#define BMP_INPUT_SX (32)
#define BMP_INPUT_SY (2)
#define BMP_INPUT_PAL ( PLTID_BG_FONT_M )

#define BMP_MSG_PX (1)
#define BMP_MSG_PY (20)
#define BMP_MSG_SX (30)
#define BMP_MSG_SY (4)
#define BMP_MSG_PAL ( PLTID_BG_FONT_M )

#if 0
#define BMP_PMS_PX (2)
#define BMP_PMS_PY (15)
#define BMP_PMS_SX (28)
#define BMP_PMS_SY (4)
#define BMP_PMS_PAL ( PLTID_BG_FONT_M )

//ポップアップウィンドウ
#define BMP_POPUP_PX (2)
#define BMP_POPUP_PY (22)
#define BMP_POPUP_SX  (28)
#define BMP_POPUP_SY  (6)
#define BMP_POPUP_FRM (BG_FRAME_MSG_M)
#define BMP_POPUP_PAL (PLTID_BG_FONT_M)
#endif

///////////////////////////////////////////
//ウィンドウレイアウト

#define BMP_PROF01_DAT_PX  (6)
#define BMP_PROF01_DAT_SX  (BMP_PROF_SX-BMP_PROF01_DAT_PX)
#define BMP_PROF01_DAT_SY  (BMP_PROF_SY)

#define BMP_PROF02_DAT_PX  (9)
#define BMP_PROF02_DAT_SX  (BMP_PROF_SX-BMP_PROF02_DAT_PX)
#define BMP_PROF02_DAT_SY  (BMP_PROF_SY)

#define BMP_RECORD_DAT_PX  (18)
#define BMP_RECORD_DAT_SX  (BMP_RECORD_SX-BMP_RECORD_DAT_PX)
#define BMP_RECORD_DAT_SY  (BMP_RECORD_SY)

//////////////////////////////////////////////
//アクター
enum{
 ACT_ICON_TR,
 ACT_ICON_EV,
 ACT_MAX
};

enum{
 ACT_SF_MAIN,
 ACT_SF_SUB,
 ACT_SF_MAX,
};

enum{
 ACTANM_ICON_TR,
 ACTANM_ICON_POKE,
 ACTANM_RANK01,
 ACTANM_RANK02,
 ACTANM_RANK03,
 ACTANM_RANK04,
 ACTANM_RANK05,
 ACTANM_OUTSIDE,
 ACTANM_INSIDE,
};

#define ACT_TRAINER_PX  (27*8+4)
#define ACT_TRAINER_PY  (5*8+4)
#define ACT_RANK_PX     (25*8)
#define ACT_RANK_PY     (10*8)
#define ACT_TRAINER_BGPRI (0)

#define ACT_ICON_BGPRI  (2)
#define ACT_ICON_SX   (24)
#define ACT_ICON_SY   (32)
#define ACT_ICON_SXH  (ACT_ICON_SX/2)
#define ACT_ICON_SYH  (ACT_ICON_SY/2)

#define ACT_INOUT_BGPRI   (1)
#define ACT_INOUT_PX      (8*28)
#define ACT_INOUT_PY_D    (8*22)
#define ACT_INOUT_PY_U    (ACT_INOUT_PY_D-(POPUP_HEIGHT))

