//==============================================================================
/**
 * @file	codein_pv.h
 * @brief	文字入力インターフェース privateヘッダ
 * @author	goto
 * @date	2007.07.11(水)
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================
#ifndef __CODEIN_PV_H__
#define __CODEIN_PV_H__

#include "system/palanm.h"
#include "system/bmp_oam.h"
#include "print/gf_font.h"

#include "msg/msg_battle_rec.h"
#include "code_input.naix"
#include "br_codein.h"

#define CODE_OAM_MAX		( 12 )					///< 文字コード数
#define BAR_OAM_MAX			( BR_CODE_BLOCK_MAX - 1 )	///< ブロック
#define CUR_OAM_MAX			( 3 )
#define BTN_OAM_MAX			( 2 )

#define POS_CODE_Y			( 24 )


///< メインシーケンス
enum {
	
	eSEQ_INIT		= 0,
	eSEQ_INPUT,
	eSEQ_FOCUS_MOVE,
//	eSEQ_CUR_MOVE,
	eSEQ_END,
};


///< OAM登録用ＩＤ
enum {
	
	eID_CODE_OAM	= 1000,
	eID_CUR_OAM,
	eID_BTN_OAM,
	eID_FNT_OAM,
		
};

///< 当たり判定ラベル
enum {
	
	eHRT_CODE_0 = 0,
	eHRT_CODE_1,
	eHRT_CODE_2,
	eHRT_CODE_3,
	eHRT_CODE_4,
	eHRT_CODE_5,
	eHRT_CODE_6,
	eHRT_CODE_7,
	eHRT_CODE_8,
	eHRT_CODE_9,
	eHRT_CODE_10,
	eHRT_CODE_11,
	
	eHRT_NUM_0,
	eHRT_NUM_1,
	eHRT_NUM_2,
	eHRT_NUM_3,
	eHRT_NUM_4,
	eHRT_NUM_5,
	eHRT_NUM_6,
	eHRT_NUM_7,
	eHRT_NUM_8,
	eHRT_NUM_9,
	
	eHRT_BACK,
	eHRT_END,
	
	eHRT_MAX,
};

///< アニメコード
enum {
	
	eANM_CODE_LN	= 0,	///< Large Nothing
	eANM_CODE_L0	= 1,	///< Large Num = 0
	eANM_CODE_SN	= 11,	///< Small Nothing
	eANM_CODE_S0	= 12,	///< Small Num = 0
	eANM_CODE_BAR	= 22,	///< bar	
	
	eANM_CUR_BAR	= 0,
	eANM_CUR_SQ,
	eANM_CUR_SQ_L,
	eANM_CUR_TOUCH,
};

///< モード
enum {
	
	eMPS_SET		= 0,
	eMPS_MOVE_SET,	
};

///< 各処理用
enum {
	
	eSTATE_INPUT	= 0,
	eSTATE_FOCUS_MOVE,
	eSTATE_CUR_MOVE,	
	
	eSTATE_BLANK	= 0xFF,
};

///< PAL
enum {
	
	ePAL_WIN		= 10,
	ePAL_FRAME,
	ePAL_FONT,
	ePAL_BACK,
};

//OBJリソース管理
enum
{
	CIO_RES1_PLT = 0,
	CIO_RES1_NCG = 1,
	CIO_RES1_ANM = 3,
	CIO_RES1_MAX = 4,
	CIO_RES2_CODE	= 0,
	CIO_RES2_BUTTON	= 1,
	CIO_RES2_CURSOL	= 2,
	CIO_RES2_MAX	= 3,
};

///< サイズとか
#define L_SIZE			( 32 )	///< でかいプレートの絵のサイズ
#define S_SIZE			( 8 )	///< ちいさいプレートの絵のサイズ
#define M_SIZE			( ( L_SIZE + S_SIZE ) / 2 )		///< 2つが隣り合う幅 XXXX-xxxとかxxxx-XXX
#define MOVE_WAIT		( 4 )
	
#define POS_4_4_4_p1	( 72 +  4 - 20 )
#define POS_4_4_4_p2	( 24 + 16 - 20 )
#define POS_4_4_4_p3	( 24 +  4 - 20 )

#define POS_2_5_5_p1	( 72 +  4 - 20 )
#define POS_2_5_5_p2	( 24 + 16 - 20 - 8 )
#define POS_2_5_5_p3	( 24 +  4 - 20 - 8 )

#define SCALE_CNT_F		( 6 )
#define SCALE_CNT_H		( SCALE_CNT_F / 2 )

#define CUR_BAR_OY		( 16 )

// -----------------------------------------
//
//	移動管理用
//
// -----------------------------------------
typedef struct {
	
	GFL_CLACTPOS	pos;
	u8	wait;	
	u8	scale;
	
} CODE_MOVE_WORK;


// -----------------------------------------
//
//	コード管理OAM
//
// -----------------------------------------
typedef struct {
	
	int				state;		///< codeなら数字・それ以外なら自分の位置
	int				group;		///< グループ
	BOOL			size;		///< TRUE = laege
	
	GFL_CLWK			*clwk;		///< OAM	
	GFL_UI_TP_HITTBL*	hit;		///< 当たり判定(アドレスだけもらって可変させる)
	CODE_MOVE_WORK	move_wk;	///< 移動制御
	
} CODE_OAM;


// -----------------------------------------
//
//	各処理用ワーク
//
// -----------------------------------------
typedef struct {
	
	int		param;				///< どんな処理をするか	
	int		target;				///< 対象は何か
	int		work;				///< 適当に使うワーク
	
} CODEIN_STATE;


// -----------------------------------------
//
//	システム系ワーク
//
// -----------------------------------------
typedef struct {
	
	ARCHANDLE*			p_handle;			///< アーカイブハンドル
	
	GFL_CLUNIT			*cellUnit;		///< OAMシステム
	u32					resIdx[CIO_RES1_MAX][CIO_RES2_MAX];

	PALETTE_FADE_PTR	pfd;				///< パレットフェード
	
	GFL_BUTTON_MAN*			btn;				///< ボタン	
	GFL_UI_TP_HITTBL		rht[ eHRT_MAX+1 ];	///< 当たり判定(managerに登録する)
	
	BOOL				touch;				///< モード

	BMPOAM_SYS_PTR			bmpoam_sys;
	BMPOAM_ACT_PTR			bmp_obj[ BTN_OAM_MAX ];
	
	GFL_BMPWIN		*win;
	
	GFL_FONT		*fontHandle;
	GFL_TCB			*vBlankTcb;
	
} CODEIN_SYS;


// -----------------------------------------
//
//	メインワーク
//
// -----------------------------------------
struct _BR_CODEIN_WORK{
	
	CODE_OAM		code[ CODE_OAM_MAX ];
	CODE_OAM		bar[ BAR_OAM_MAX ];
	CODE_OAM		cur[ CUR_OAM_MAX ];
	CODE_OAM		btn[ BTN_OAM_MAX ];
	
	s16				x_tbl[ BR_CODE_BLOCK_MAX + 1 ];
	u16				b_tbl[ BR_CODE_BLOCK_MAX + 1 ][ 2 ];
	
	int				seq;
	int				wait;
	int				gene_seq;
	int				gene_cnt;
	
	int				code_max;		///< コード入力数

	int				focus_now;		///< 入力対象
	int				focus_old;		///< 入力対象
	int				ls;				///< 拡大ブロック開始位置
	int				le;				///< 拡大ブロック終了位置
	int				ss;				///< 縮小ブロック開始位置
	int				se;				///< 縮小ブロック開始位置
	
	CODEIN_SYS		sys;			///< システム
	CODEIN_STATE	state;			///< 状態

	BR_CODEIN_PARAM	param;			///< 外側からもらうパラメータ
	
  HEAPID          heapID;     //ヒープ
  BR_CODEIN_SELECT   select;     //選択
};

///< codein_pv.c
extern void CI_pv_ParamInit( BR_CODEIN_WORK* wk );
extern BOOL CI_pv_MainUpdate( BR_CODEIN_WORK* wk );
extern void CI_pv_ButtonManagerInit( BR_CODEIN_WORK* wk );
extern void CI_pv_ButtonManagerCallBack( u32 button, u32 event, void* work );

extern void CI_pv_StateUpdate( BR_CODEIN_WORK* wk );
extern void CI_pv_StateInit( BR_CODEIN_WORK* wk );
extern void CI_pv_FocusSet( BR_CODEIN_WORK* wk, int next_focus );

extern void CI_pv_SeqChange( BR_CODEIN_WORK* wk, int seq );
extern BOOL CI_pv_MainInit( BR_CODEIN_WORK* wk );
extern BOOL CI_pv_MainInput( BR_CODEIN_WORK* wk );
extern BOOL CI_pv_MainFocusMove( BR_CODEIN_WORK* wk );
extern BOOL CI_pv_MainEnd( BR_CODEIN_WORK* wk );

extern int CI_pv_FocusTopSerach( BR_CODEIN_WORK* wk, int next );
extern int CI_pv_FocusBottomSerach( BR_CODEIN_WORK* wk, int next );
extern void CI_KEY_Main( BR_CODEIN_WORK* wk );

extern void CI_pv_Input_back( BR_CODEIN_WORK* wk );
extern void CI_pv_Input_End( BR_CODEIN_WORK* wk );

///< codein_disp.c
extern void CI_pv_disp_CodeRes_Load( BR_CODEIN_WORK* wk );
extern void CI_pv_disp_CodeOAM_Create( BR_CODEIN_WORK* wk );
extern int	CI_pv_disp_CodeAnimeGet( int state, BOOL size );
extern void CI_pv_disp_SizeFlagSet( BR_CODEIN_WORK* wk );
extern void CI_pv_disp_MovePosSet( BR_CODEIN_WORK* wk, int mode );

extern void CI_pv_disp_HitTableSet( BR_CODEIN_WORK* wk );

extern void CI_pv_disp_CurOAM_Create( BR_CODEIN_WORK* wk );
extern void CI_pv_disp_BtnOAM_Create( BR_CODEIN_WORK* wk );

extern void CI_pv_disp_CurBar_PosSet( BR_CODEIN_WORK* wk, int id );
extern void CI_pv_disp_CurSQ_PosSet( BR_CODEIN_WORK* wk, int id );
extern void CI_pv_disp_CurSQ_PosSetEx( BR_CODEIN_WORK* wk, int id, int cur_id );
extern void CI_pv_disp_CurUpdate( BR_CODEIN_WORK* wk );
extern void CI_pv_disp_CurOAM_AnimeChange( BR_CODEIN_WORK* wk, int no, int anime );

extern void CI_pv_disp_CurOAM_Visible( BR_CODEIN_WORK* wk, int no, BOOL flag );
extern BOOL CI_pv_disp_CurOAM_VisibleGet( BR_CODEIN_WORK* wk, int no );

extern void CI_pv_FontOam_SysInit( BR_CODEIN_WORK* wk );
extern void CI_pv_FontOam_SysDelete( BR_CODEIN_WORK* wk );
extern void CI_pv_FontOam_ResourceLoad( BR_CODEIN_WORK* wk );
extern void CI_pv_FontOam_Add( BR_CODEIN_WORK* wk );
extern void CI_pv_FontOam_Enable( BR_CODEIN_WORK* wk, BOOL flag );
extern void CI_pv_FontOam_Create( BR_CODEIN_WORK* wk, int no, int x, int y, int pal_offset );

extern void CI_pv_disp_CodeRes_Delete( BR_CODEIN_WORK* wk );

#endif	///< __CODEIN_PV_H__
