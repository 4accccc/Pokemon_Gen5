//=============================================================================
/**
 * @file	  ircpokemontrade_local.h
 * @brief	  ポケモン交換 ローカル共有定義
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/08/28
 */
//=============================================================================

#pragma once

#if PM_DEBUG
#define _TRADE_DEBUG (1)
#else
#define _TRADE_DEBUG (0)
#endif

#include "../../field/event_ircbattle.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "system/touch_subwindow.h"

#include "print/wordset.h"

#include "savedata/box_savedata.h"  //デバッグアイテム生成用

#include "system/mcss.h"
#include "system/mcss_tool.h"
#include "app/app_taskmenu.h"
#include "ircpokemontrade_anim.h"
#include "system/ica_anime.h"
#include "ui/ui_easy_clwk.h"


typedef enum
{
  REEL_PANEL_OBJECT,
  TRADE01_OBJECT,
  TRADE_TRADE_OBJECT,
  TRADE_RETURN_OBJECT,
  TRADE_SPLASH_OBJECT,
  TRADE_END_OBJECT,
  OBJECT_MODEL_MAX,
} CELL_OBJECT_MODEL;


typedef enum
{
  SETUP_TRADE_BG_MODE_NORMAL,
  SETUP_TRADE_BG_MODE_DEMO,
} SETUP_TRADE_BG_MODE;




typedef enum
{
  _CHANGERAND = 43,    ///< ポケモン交換タイミング揺らぎ数
  _TIMING_ENDNO=12,
  _TIMING_TRADEDEMO_START,
  _TIMING_SAVEST,
  _TIMING_SAVELAST,
 } NET_TIMING_ENUM;

#define BOX_VERTICAL_NUM (5)
#define BOX_HORIZONTAL_NUM (6)

#define HAND_VERTICAL_NUM (3)
#define HAND_HORIZONTAL_NUM (2)

//#define BOX_MONS_NUM (30)


#define _BRIGHTNESS_SYNC (2)  // フェードのＳＹＮＣは要調整


//パレットの定義はあっても良いと
#define _FONT_PARAM_LETTER_BLUE (0x5)
#define _FONT_PARAM_SHADOW_BLUE (0x6)
#define _FONT_PARAM_LETTER_RED (0x3)
#define _FONT_PARAM_SHADOW_RED (0x4)


#define PLTID_OBJ_TYPEICON_M (10) // //3本
#define PLTID_OBJ_BALLICON_M (13)  // 1本使用

//#define CUR_NUM (3)
#define _BUTTON_WIN_PAL   (15)  // ウインドウ
#define _BUTTON_MSG_PAL   (14)  // メッセージフォント
#define	FBMP_COL_WHITE		(15)

#define _OBJPLT_BOX  (0)  //3本
#define _OBJPLT_BAR  (3)  //3本
#define _OBJPLT_POKEICON  (6)  //3本
#define _OBJPLT_COMMON  (9)  //?本

#define PLIST_RENDER_MAIN (1)


#define _POKE_PRJORTH_Y_COEFFICIENT (12)
#define _POKE_PRJORTH_X_COEFFICIENT (16)


#define _POKE_PRJORTH_TOP   (FX32_ONE*_POKE_PRJORTH_Y_COEFFICIENT)    ///< PRJORTH	→top	  :nearクリップ面上辺のY座標
#define _POKE_PRJORTH_RIGHT (FX32_ONE*_POKE_PRJORTH_X_COEFFICIENT)  ///< PRJORTH	→right	  :nearクリップ面右辺のX座標

#define _MCSS_POS_X(x) (x * FX32_ONE)
#define _MCSS_POS_Y(y) (y * FX32_ONE)
#define _MCSS_POS_Z(z) (z * FX32_ONE)

#define PSTATUS_MCSS_POS_X1 _MCSS_POS_X(-60)            //自分最初の位置X
#define PSTATUS_MCSS_POS_X2 _MCSS_POS_X(60)             //相手最初の位置X
#define PSTATUS_MCSS_POS_Y  _MCSS_POS_Y(-28)            //最初の位置Yは共通



#define PSTATUS_MCSS_POS_MYZ   (0)
#define PSTATUS_MCSS_POS_YOUZ  (0)



#define YESNO_CHARA_OFFSET	(21 * 20 )
#define YESNO_CHARA_W		( 8 )
#define YESNO_CHARA_H		( 4 )

#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (5)    // ウインドウ高さ


#define CONTROL_PANEL_Y (120+48)


#define CHANGEBUTTON_X (72)
#define CHANGEBUTTON_Y (CONTROL_PANEL_Y)
#define CHANGEBUTTON_WIDTH  (5*8)
#define CHANGEBUTTON_HEIGHT (3*8)
#define WINCLR_COL(col)	(((col)<<4)|(col))




#define YESBUTTON_X (200)
#define YESBUTTON_Y (160)
#define NOBUTTON_X (200)
#define NOBUTTON_Y (180)

#define _SUBMENU_LISTMAX (8)

#define _LING_LINENO_MAX (12)  //リングバッファ最大
//#define _G3D_MDL_MAX (1)  //モデルの数

/// ドット単位で位置を管理  8*20がBOX 8*12がてもち BOX_MAX_TRAY => 2880+96=2976

#define TOTAL_DOT_MAX  ((BOX_MAX_TRAY * 20 * 8) + (12 * 8))



typedef void (StateFunc)(IRC_POKEMON_TRADE* pState);


typedef enum
{
  PLT_POKEICON,   //ARCID_POKEICON
  PAL_SCROLLBAR, //ARCID_POKETRADE
  PLT_COMMON,
  PLT_POKECREATE,
  PLT_RESOURCE_MAX,

  CHAR_SCROLLBAR = PLT_RESOURCE_MAX,    //ARCID_POKETRADE
  CHAR_SELECT_POKEICON1, //選択中ポケモンアイコン１
  CHAR_SELECT_POKEICON2, //選択中ポケモンアイコン１
  CHAR_COMMON,
  CHAR_POKECREATE,
  CHAR_RESOURCE_MAX,

  ANM_POKEICON = CHAR_RESOURCE_MAX,  //ARCID_POKEICON
  ANM_SCROLLBAR, //ARCID_POKETRADE
  ANM_COMMON,
  ANM_POKECREATE,
  ANM_RESOURCE_MAX,

  CEL_RESOURCE_MAX=ANM_RESOURCE_MAX,
} _CELL_RESOURCE_TYPE;

typedef enum
{
  CELL_CUR_SELECT,
  CELL_CUR_MOJI,
  CELL_CUR_SCROLLBAR,
  CELL_CUR_POKE_SELECT,
  CELL_CUR_POKE_PLAYER,
  CELL_CUR_POKE_FRIEND,
  CHAR_LEFTPAGE_MARK,
  CHAR_RETURNPAGE_MARK,
  
  CELL_DISP_NUM
} _CELL_DISP_TYPE;



// セットアップ番号
enum
{
  SETUP_INDEX_BALL,
  SETUP_INDEX_ROUND,
  SETUP_INDEX_MAX
};

// ポケモンステータスを自分と相手を切り替える際のCELLの位置
#define _POKEMON_SELECT1_CELLX  (128-32)
#define _POKEMON_SELECT1_CELLY  (12)
#define _POKEMON_SELECT2_CELLX  (128+32)
#define _POKEMON_SELECT2_CELLY  (12)

// 日本語検索文字MAX
#define JAPANESE_SEARCH_INDEX_MAX (44)


/// @brief 3Dモデルカラーフェード
typedef struct
{
  GFL_G3D_RES* g3DRES;             //パレットフェード  対象3Dモデルリソースへのポインタ
  void*        pData_dst;          //パレットフェード  転送用ワーク
  u8            pal_fade_flag;      //パレットフェード  起動フラグ
  u8            pal_fade_count;     //パレットフェード  対象リソース数
  u8            pal_fade_start_evy; //パレットフェード　START_EVY値
  u8            pal_fade_end_evy;		//パレットフェード　END_EVY値
  u8            pal_fade_wait;			//パレットフェード　wait値
  u8            pal_fade_wait_tmp;	//パレットフェード　wait_tmp値
  u16           pal_fade_rgb;				//パレットフェード　end_evy時のrgb値
}_EFFTOOL_PAL_FADE_WORK;

/// @brief 2D面フェード
typedef struct
{
  s8   pal_fade_time;			//パレットフェード トータル時間
  s8   pal_fade_nowcount;	//パレットフェード 現在の進行時間
  s8   pal_start;
  s8   pal_end;
}_D2_PAL_FADE_WORK;


/// @brief ポケモン座標移動
typedef struct
{
  MCSS_WORK* pMcss;  ///< 動かす物体
  int   time;			   ///< トータル時間
  int   nowcount;	   ///<  現在の進行時間
  VecFx32  start;      ///< 開始位置
  VecFx32  end;        ///< 終わり位置
}_POKEMCSS_MOVE_WORK;


typedef struct
{
  // ボールアイコン
  UI_EASY_CLWK_RES      clres_ball;
  GFL_CLWK              *clwk_ball;
} _BALL_ICON_WORK;


typedef struct
{
  // タイプアイコン
  UI_EASY_CLWK_RES      clres_type_icon;
  GFL_CLWK                  *clwk_type_icon;
} _TYPE_ICON_WORK;




struct _IRC_POKEMON_TRADE{
  u8 FriendPokemonCol[732];         ///< 相手のポケモンBOXにあるポケモン色
  GFL_BMPWIN* StatusWin[2];
  POKEMON_PARAM* recvPoke[2];  ///< 受け取ったポケモンを格納する場所
  BOOL bPokemonSet[2];              ///<
  //	EVENT_IRCBATTLE_WORK* pParentWork;
  StateFunc* state;      ///< ハンドルのプログラム状態
  HEAPID heapID;

  //メッセージ系
  GFL_BMPWIN* MessageWin;

  GFL_ARCUTIL_TRANSINFO bgchar;


  TOUCH_SW_SYS			*TouchSubWindowSys;
  GFL_PTC_PTR ptc;
  GFL_PTC_PTR ptcOrthogonal;

  APP_TASKMENU_WORK* pAppTask;
  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
  APP_TASKMENU_RES* pAppTaskRes;
  GFL_BMPWIN* MyInfoWin;
  GFL_BMPWIN* mesWin;
  GFL_MSGDATA *pMsgData;  //
  WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
  GFL_FONT* pFontHandle;
  PRINT_STREAM* pStream;
  STRBUF* pStrBuf;
  STRBUF* pExStrBuf;
  STRBUF* pMessageStrBuf;
  STRBUF* pMessageStrBufEx;
  PRINT_QUE*            SysMsgQue;
  GFL_TCBLSYS *pMsgTcblSys;

  GFL_BMPWIN* SerchMojiWin[JAPANESE_SEARCH_INDEX_MAX];
  int selectMoji;
  
  GFL_G3D_RES* pG3dRes;
  GFL_G3D_OBJ* pG3dObj;
  GFL_G3D_OBJSTATUS*	pStatus;
  GFL_G3D_CAMERA		*pCamera;
  _EFFTOOL_PAL_FADE_WORK* pModelFade;
  _D2_PAL_FADE_WORK* pD2Fade;

  //  G3D_MDL_WORK			mdl[_G3D_MDL_MAX];
  //    BMPWINFRAME_AREAMANAGER_POS aPos;
  //3D
  int objCount;

  GFL_G3D_UTIL* g3dUtil;
  u16 unitIndex;
//  ICA_ANIME* icaAnime;
  ICA_ANIME* icaCamera;
  ICA_ANIME* icaTarget;
  //  GFL_G3D_CAMERA* camera;
  ICA_ANIME* icaBallin;
  GFL_EMIT_PTR pBallInPer;
  ICA_ANIME* icaBallout;
  GFL_EMIT_PTR pBallOutPer;
  GFL_CLWK* pPokeCreateCLWK;   //ポケモン登場エフェクトCLACT


  BOX_MANAGER* pBox;
  GAMEDATA* pGameData;
  MYSTATUS* pMy;
  MYSTATUS* pFriend;
  POKEPARTY* pMyParty;
  GFL_G3D_CAMERA    *camera;

  MCSS_SYS_WORK *mcssSys;
  MCSS_WORK     *pokeMcss[2];
  BOOL     mcssStop[2];
  _POKEMCSS_MOVE_WORK* pMoveMcss[2];

  GFL_ARCUTIL_TRANSINFO subchar;
  GFL_ARCUTIL_TRANSINFO subchar1;
  GFL_ARCUTIL_TRANSINFO subchar2;
  u32 cellRes[CEL_RESOURCE_MAX];

  //	GAMESYS_WORK* pGameSys;
  _BALL_ICON_WORK aBallIcon[3];
  _TYPE_ICON_WORK aTypeIcon[2];
  
  GFL_CLUNIT	*cellUnit;
  GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル

  u32 pokeIconNcgRes[_LING_LINENO_MAX][BOX_VERTICAL_NUM];
  GFL_CLWK* pokeIcon[_LING_LINENO_MAX][BOX_VERTICAL_NUM];
  u32 pokeIconNo[_LING_LINENO_MAX][BOX_VERTICAL_NUM];
  u32 pokeIconForm[_LING_LINENO_MAX][BOX_VERTICAL_NUM];
  u8 pokeIconLine[_LING_LINENO_MAX][BOX_VERTICAL_NUM];

  GFL_CLWK* curIcon[CELL_DISP_NUM];

  int windowNum;
  int anmCount;
  int saveStep;
  BOOL IsIrc;
  u32 connect_bit;
  BOOL connect_ok;
  BOOL receive_ok;
  u32 receive_result_param;
  u32 receive_first_param;

  int nowBoxno;  //いまのボックス
  u32 x;
  u32 y;
  BOOL bUpVec;
  GFL_CLWK* pCatchCLWK;   //つかんでるポケモンCLACT


  int workPokeIndex;  // マウス操作でにぎったポケモン
  int workBoxno;  //マウス操作でにぎったポケモン


  int selectIndex;  //候補のポケモンIndex
  int selectBoxno;  //候補のポケモンBox

  int underSelectIndex;  //まだ相手に見せてないポケモンIndex
  int underSelectBoxno;  //まだ相手に見せてないポケモンBox
  int pokemonsetCall;


  BOOL bChangeOK[2];

  BOOL bParent;
  BOOL bTouchReset;

  short BoxScrollNum;   ///< ドット単位で位置を管理  8*20がBOX 8*12がてもち BOX_MAX_TRAY => 2880+96=2976
  short FriendBoxScrollNum;   ///< 上記の相手側の値

  short oldLine; //現在の描画Line 更新トリガ

  BOOL bgscrollRenew;
  BOOL touckON;
  int bgscroll;
  int ringLineNo;

  int modelno;  ///< 表示しているモデルの番号
  int pokemonselectno;  ///< 自分と相手のポケモン選択中のどちらを指しているか

  u16* scrTray;
  u16* scrTemoti;
  u8* pCharMem;

} ;


#define UNIT_NULL		(0xffff)

extern void IRC_POKMEONTRADE_ChangeFinish(IRC_POKEMON_TRADE* pWork);


extern void IRC_POKETRADE_GraphicInitMainDisp(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_GraphicInitSubDisp(IRC_POKEMON_TRADE* pWork);

extern void IRC_POKETRADE_GraphicExit(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_SubStatusInit(IRC_POKEMON_TRADE* pWork,int pokeposx, int type);
extern void IRC_POKETRADE_SubStatusEnd(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_AppMenuOpen(IRC_POKEMON_TRADE* pWork, int *menustr,int num);
extern void IRC_POKETRADE_MessageOpen(IRC_POKEMON_TRADE* pWork, int msgno);
extern void IRC_POKETRADE_MessageWindowOpen(IRC_POKEMON_TRADE* pWork, int msgno);
extern void IRC_POKETRADE_MessageWindowClose(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_MessageWindowClear(IRC_POKEMON_TRADE* pWork);
extern BOOL IRC_POKETRADE_MessageEndCheck(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_TrayDisp(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_InitBoxIcon( BOX_MANAGER* boxData ,IRC_POKEMON_TRADE* pWork );
extern void IRC_POKETRADE_AllDeletePokeIconResource(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_PokeIcomPosSet(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_G3dDraw(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_SetSubdispGraphicDemo(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_GraphicFreeVram(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_SetMainDispGraphic(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_ResetSubDispGraphic(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_SetSubDispGraphic(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_3DGraphicSetUp( IRC_POKEMON_TRADE* pWork );
extern void IRC_POKETRADE_SetBgMode(SETUP_TRADE_BG_MODE type);
extern void IRC_POKETRADE_CreatePokeIconResource(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_InitBoxCursor(IRC_POKEMON_TRADE* pWork);
extern POKEMON_PARAM* IRC_POKEMONTRADE_GetRecvPP(IRC_POKEMON_TRADE *pWork, int index);

extern void IRC_POKETRADEDEMO_Init( IRC_POKEMON_TRADE* pWork );
extern void IRC_POKETRADEDEMO_Main( IRC_POKEMON_TRADE* pWork );
extern void IRC_POKETRADEDEMO_End( IRC_POKEMON_TRADE* pWork );
extern GFL_CLWK* IRC_POKETRADE_GetCLACT( IRC_POKEMON_TRADE* pWork , int x, int y, int* trayno, int* pokeindex);
extern void IRC_POKETRADEDEMO_SetModel( IRC_POKEMON_TRADE* pWork, int modelno);
extern void IRC_POKETRADEDEMO_RemoveModel( IRC_POKEMON_TRADE* pWork);

extern void IRC_POKMEONTRADE_STEP_ChangeDemo_PokeMove(IRC_POKEMON_TRADE* pWork);

extern void IRCPOKETRADE_PokeDeleteMcss( IRC_POKEMON_TRADE *pWork,int no  );
extern void IRCPOKETRADE_PokeCreateMcss( IRC_POKEMON_TRADE *pWork ,int no, int bFront, const POKEMON_PARAM *pp );
extern POKEMON_PASO_PARAM* IRCPOKEMONTRADE_GetPokeDataAddress(BOX_MANAGER* boxData , int trayNo, int index,IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_SetMainStatusBG(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_ResetMainStatusBG(IRC_POKEMON_TRADE* pWork);
extern void IRCPOKEMONTRADE_ResetPokemonStatusMessage(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_LeftPageMarkRemove(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_LeftPageMarkDisp(IRC_POKEMON_TRADE* pWork,int no);
extern void IRC_POKETRADE_InitSubMojiBG(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_EndSubMojiBG(IRC_POKEMON_TRADE* pWork);
extern void IRC_POKETRADE_ReturnPageMarkDisp(IRC_POKEMON_TRADE* pWork,int no);
extern void IRC_POKETRADE_ReturnPageMarkRemove(IRC_POKEMON_TRADE* pWork);


#if _TRADE_DEBUG

extern void IRC_POKMEONTRADE_changeStateDebug(IRC_POKEMON_TRADE* pWork,StateFunc* state, int line);

#define   _CHANGE_STATE(pWork, state)  IRC_POKMEONTRADE_changeStateDebug(pWork ,state, __LINE__)

#else  //_NET_DEBUG

extern void IRC_POKMEONTRADE_changeState(IRC_POKEMON_TRADE* pWork,StateFunc* state);

#define   _CHANGE_STATE(pWork, state)  IRC_POKMEONTRADE_changeState(pWork ,state)

#endif //_NET_DEBUG
