#ifndef _TRCARD_SYS_H_
#define _TRCARD_SYS_H_

#include "system/bmp_menu.h"
#include "system/touch_subwindow.h"
#include "app/trainer_card.h"
#include "trcard_obj.h"
#include "trcard_snd.h"

#define TRC_USE_SND (0)
#define TRC_FRAME_RATE (60)

#define TRC_SCREEN_WIDTH	(32)
#define TR_CARD_WIN_MAX		(14)
#define TR_CARD_PARAM_MAX	(11)
#define TR_CPRM_STR_MAX		(14)

#define TR_CARD_BADGE_LV_MAX	(4)		//バッジの最大レベル
#define TR_BADGE_NUM_MAX	(16)

//パレット使用リスト
enum{
	CASE_BD_PAL = 0,
	YNWIN_PAL = 11,	//2本使う
	TALKWIN_PAL = 13,
	SYS_FONT_PAL = 14,
	TR_FONT_PAL = 15,
	BACK_FONT_PAL = 15,
};

typedef struct TR_SCRUCH_SND_tag
{
	s8 OldDirX;
	s8 OldDirY;
	s8 DirX;
	s8 DirY;
	u8 Snd;
}TR_SCRUCH_SND;

typedef struct TR_CARD_WORK_tag
{
	HEAPID heapId;
	GFL_BMPWIN	*win[TR_CARD_WIN_MAX];			// BMPウィンドウデータ（通常）

	TRCARD_CALL_PARAM* tcp;	///<カード呼び出しパラメータ
	TR_CARD_DATA *TrCardData;
	u8 TrSignData[SIGN_SIZE_X*SIGN_SIZE_Y*64];	//転送するので４バイト境界の位置に置くこと
	void *TrArcData;							//トレーナーグラフィックアーカイブデータ
	NNSG2dCharacterData* TrCharData;			//トレーナーキャラデータポインタ
	BOOL is_back;
	BOOL brushOK;
	
	fx32 CardScaleX;
	fx32 CardScaleY;
	fx32 CoverScaleY;
	int	 RotateCount;

//	int touch;
	int key_mode;
	BOOL rc_cover;
	int BeforeX;
	int BeforeY;
	TR_CARD_OBJ_WORK ObjWork;
//	int RevSpeed;
	int Scroll;
	BOOL ButtonPushFlg;
	SND_BADGE_WORK SndBadgeWork;
	TR_SCRUCH_SND ScruchSnd;
	
	void *TrScrnArcData;
	NNSG2dScreenData* ScrnData;
	void *pScrnBCase;
	NNSG2dScreenData* pSBCase;
	
	TOUCH_SW_SYS* ynbtn_wk;		// YesNo選択ボタンワーク
	GFL_TCB	*vblankFunc;
	GFL_TCBLSYS *vblankTcblSys;

	GFL_FONT	*fontHandle;
	GFL_MSGDATA* msgMan;
	
	STRBUF	*PlayTimeBuf;		//プレイ時間用文字列バッファ
	STRBUF	*SecBuf;			//プレイ時間コロン用文字列バッファ
	STRBUF	*TmpBuf;			//テンポラリ文字列
	STRBUF	*DigitBuf;			//カード数値パラメータ展開用文字列バッファ
	STRBUF	*ExpBuf[3];			//<説明メッセージ文字列バッファ
	STRBUF	*SignBuf[2];		//サイン説明メッセージ文字列バッファ
	STRBUF	*CPrmBuf[TR_CPRM_STR_MAX];	//カードパラメータ展開用文字列バッファ

//	u8 ScruchCounter[TR_BADGE_NUM_MAX];
	u8 badge[TR_BADGE_NUM_MAX];

	u8 IsOpen;
	u8 Counter;
	u8 sub_seq;
//	u8 ButtonAnmCount;
	
	u8 AnmNum;
	u8 AnimeType;
	u8 SecCount;				//秒表示のためのカウンタ
	u8 isClear:1;				//殿堂入りしているか否か？
	u8 isComm:1;				//通信中か否か？
	u8 aff_req:1;				//アフィン変換実行リクエスト
	u8 drug_f:1;				//ドラッグフラグ
	u8 Dummy:5;					//パディング
	
	PRINT_STREAM	*printHandle;
//	u8 msgIdx;
	u8 win_type;	///<ウィンドウタイプ
	u8 msg_spd;		///<メッセージスピード
	u8 scrl_ct;		///<スクロールカウンタ
	u32 tp_x;
	u32 tp_y;
	u8 draw_seq;
	u8 dmy;
}TR_CARD_WORK;

extern GFL_PROC_RESULT TrCardProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
extern GFL_PROC_RESULT TrCardProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
extern GFL_PROC_RESULT TrCardProc_End( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

#endif //_TRCARD_SYS_H_


