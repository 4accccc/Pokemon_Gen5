//======================================================================
/**
 * @file  fieldobj_move_0.c
 * @brief  フィールド動作モデル　基本動作系その0
 * @author  kagaya
 * @date  05.07.21
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procmove.h"

#include "field_player.h"   //event_trainer_eye.h
#include "field_encount.h"  //event_trainer_eye.h
#include "event_trainer_eye.h"

//======================================================================
//  define
//======================================================================
#define WAIT_END (-1)                    ///<ウェイト終端

#define BTN_DASH (PAD_BUTTON_B)

//--------------------------------------------------------------
//  MV_RND系　移動チェック方式識別
//--------------------------------------------------------------
enum
{
  MV_RND_MOVE_CHECK_NORMAL = 0,        ///<通常チェック
  MV_RND_MOVE_CHECK_RECT,            ///<矩形チェックアリ
  MV_RND_MOVE_CHECK_LIMIT_ONLY,        ///<移動制限チェックのみ
};

//--------------------------------------------------------------
///  MV_SPIN_DIR動作番号
//--------------------------------------------------------------
enum
{
  SEQNO_MV_SPIN_DIR_CMD_SET = 0,
  SEQNO_MV_SPIN_DIR_CMD_ACT,
  SEQNO_MV_SPIN_DIR_WAIT,
  SEQNO_MV_SPIN_DIR_NEXT_DIR_SET,
};

#define MV_SPIN_WAIT_FRAME (24)  //AGB 48 frame

//--------------------------------------------------------------
///  MV_REWAR動作番号
//--------------------------------------------------------------
enum
{
  SEQNO_MV_REWAR_DIR_CMD_SET = 0,
  SEQNO_MV_REWAR_DIR_CMD_ACT,
  SEQNO_MV_REWAR_DIR_WAIT,
  SEQNO_MV_REWAR_DIR_NEXT_DIR_SET,
};

//--------------------------------------------------------------
///  MV_RT2動作番号
//--------------------------------------------------------------
enum
{
  SEQNO_MV_RT2_DIR_CHECK = 0,
  SEQNO_MV_RT2_MOVE_SET,
  SEQNO_MV_RT2_MOVE,
};

//--------------------------------------------------------------
///  MV_RT3動作番号
//--------------------------------------------------------------
enum
{
  SEQNO_MV_RT3_MOVE_DIR_SET = 0,
  SEQNO_MV_RT3_MOVE,
};

#define MV_RT3_TURN_END_NO (3)  ///<3点巡回終了値

enum
{
  MV_RT3_CHECK_TYPE_X = 0,
  MV_RT3_CHECK_TYPE_Z,
};

//--------------------------------------------------------------
///  MV_RT4動作番号
//--------------------------------------------------------------
enum
{
  SEQNO_MV_RT4_JIKI_CHECK = 0,
  SEQNO_MV_RT4_DISCOVERY_JUMP,
  SEQNO_MV_RT4_MOVE_DIR_SET,
  SEQNO_MV_RT4_MOVE,
};

#define MV_RT4_TURN_END_NO_MAX (4)  ///<4点移動番号最大数

//--------------------------------------------------------------
///  サブ動作　クルクル移動　回転方向
//--------------------------------------------------------------
enum
{
  RT_KURU2_L = 0,
  RT_KURU2_R,
  RT_KURU2_MAX,
};

#define RT_KURU2_DIR_MAX (DIR_MAX4)    ///<クルクル移動最大回転数

//--------------------------------------------------------------
///  方向テーブルID
//--------------------------------------------------------------
enum
{
  DIRID_MvDirRndDirTbl = 0,
  DIRID_MvDirRndDirTbl_UL,
  DIRID_MvDirRndDirTbl_UR,
  DIRID_MvDirRndDirTbl_DL,
  DIRID_MvDirRndDirTbl_DR,
  DIRID_MvDirRndDirTbl_UDL,
  DIRID_MvDirRndDirTbl_UDR,
  DIRID_MvDirRndDirTbl_ULR,
  DIRID_MvDirRndDirTbl_DLR,
  DIRID_MvDirRndDirTbl_UD,
  DIRID_MvDirRndDirTbl_LR,
  DIRID_MV_RND_DirTbl,
  DIRID_MV_RND_V_DirTbl,
  DIRID_MV_RND_H_DirTbl,
  DIRID_MvRtURLD_DirTbl,
  DIRID_MvRtRLDU_DirTbl,
  DIRID_MvRtDURL_DirTbl,
  DIRID_MvRtLDUR_DirTbl,
  DIRID_MvRtULRD_DirTbl,
  DIRID_MvRtLRDU_DirTbl,
  DIRID_MvRtDULR_DirTbl,
  DIRID_MvRtRDUL_DirTbl,
  DIRID_MvRtLUDR_DirTbl,
  DIRID_MvRtUDRL_DirTbl,
  DIRID_MvRtRLUD_DirTbl,
  DIRID_MvRtDRLU_DirTbl,
  DIRID_MvRtRUDL_DirTbl,
  DIRID_MvRtUDLR_DirTbl,
  DIRID_MvRtLRUD_DirTbl,
  DIRID_MvRtDLRU_DirTbl,
  DIRID_MvRtUL_DirTbl,
  DIRID_MvRtDR_DirTbl,
  DIRID_MvRtLD_DirTbl,
  DIRID_MvRtRU_DirTbl,
  DIRID_MvRtUR_DirTbl,
  DIRID_MvRtDL_DirTbl,
  DIRID_MvRtLU_DirTbl,
  DIRID_MvRtRD_DirTbl,
  DIRID_MvDirSpin_DirTbl,
  DIRID_END,
  DIRID_MAX,
};

//--------------------------------------------------------------
//  debug
//--------------------------------------------------------------
#ifdef PM_DEBUG
//#define DEBUG_SEARCH  //有効で方向切り替え型動作全て自機サーチ
#endif

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
//  DIR_TBL構造体
//--------------------------------------------------------------
typedef struct
{
  int id;
  const int *tbl;
}DIR_TBL;

//--------------------------------------------------------------
///  RECT構造体
//--------------------------------------------------------------
typedef struct
{
  int left;
  int top;
  int right;
  int bottom;
}RECT;

//--------------------------------------------------------------
///  RT_KURUKURU_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  s8 origin_dir;
  s8 spin_type;
  u8 dir_pause;
  u8 dmy;
}RT_KURUKURU_WORK;

#define RT_KURUKURU_WORK_SIZE (sizeof(RT_KURUKURU_WORK))

//--------------------------------------------------------------
///  MV_DIR_RND_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  u16 seq_no;                        ///<シーケンスナンバー
  s16 wait;                        ///<表示ウェイト
  int tbl_id;
}MV_DIR_RND_WORK;

#define MV_DIR_RND_WORK_SIZE (sizeof(MV_DIR_RND_WORK))    ///<MV_DIR_RND_WORKサイズ

//--------------------------------------------------------------
///  MV_RND_WORK 構造体
//--------------------------------------------------------------
typedef struct
{
  s16 seq_no;
  s16 wait;
  int move_check_type;
  int acmd_code;
  int tbl_id;
}MV_RND_WORK;

#define MV_RND_WORK_SIZE (sizeof(MV_RND_WORK))

//--------------------------------------------------------------
///  MV_DIR_WORK 構造体
//--------------------------------------------------------------
typedef struct
{
  int dir;
  int seq_no;
}MV_DIR_WORK;

#define MV_DIR_WORK_SIZE (sizeof(MV_DIR_WORK))

//--------------------------------------------------------------
///  MV_SPIN_DIR_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  s8 spin_dir;
  s8 set_dir;
  s8 seq_no;
  s8 tbl_no;
  int wait;
}MV_SPIN_DIR_WORK;

#define MV_SPIN_DIR_WORK_SIZE (sizeof(MV_SPIN_DIR_WORK))

//--------------------------------------------------------------
///  MV_RT2_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  u8 seq_no;
  u8 turn_flag;
  u8 react_count;
  u8 padding;
  RT_KURUKURU_WORK kurukuru;
}MV_RT2_WORK;

#define MV_RT2_WORK_SIZE (sizeof(MV_RT2_WORK))

//--------------------------------------------------------------
///  MV_RT3_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  u8 seq_no;
  u8 turn_no;
  u8 turn_check_no;
  u8 turn_check_type;
  
  u8 tbl_id;
  u8 react_count;
  u8 padding0;
  u8 padding1;
  
  RT_KURUKURU_WORK kurukuru;
}MV_RT3_WORK;

#define MV_RT3_WORK_SIZE (sizeof(MV_RT3_WORK))

//--------------------------------------------------------------
///  MV_RT4_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  u8 seq_no;
  s8 turn_no;
  u8 turn_check_no;
  u8 turn_check_type;
  
  u8 tbl_id;
  u8 rev_flag;
  u8 react_count;
  u8 padding;
   
  RT_KURUKURU_WORK kurukuru;
}MV_RT4_WORK;

#define MV_RT4_WORK_SIZE (sizeof(MV_RT4_WORK))

//======================================================================
//  proto
//======================================================================
static void DirRndWorkInit( MMDL * mmdl, int id );
static u16 get_DashAlterNextDir( u16 mv_code, u16 dir );

static void MvRndWorkInit( MMDL * mmdl, int ac, int id, int check );

static BOOL checkDashButton( void );
static int TblNumGet( const int *tbl, int end );
static int TblRndGet( const int *tbl, int end );
static int TblIDRndGet( int id, int end );
static const int * MoveDirTblIDSearch( int id );
static int TrJikiDashSearch( MMDL * mmdl );
static int TrJikiDashSearchTbl( MMDL * mmdl, int id, int end );
static BOOL checkEvTypeDashReact( const MMDL *mmdl, u8 *count );
static BOOL checkMMdlEventTypeDashAccel( const MMDL *mmdl );

static void MvRndRectMake( MMDL * mmdl, RECT *rect );
static int MvRndRectMoveLimitCheck( MMDL * mmdl, int dir );

static void MvDirWorkInit( MMDL * mmdl, int dir );

static void MvSpinDirWorkInit( MMDL * mmdl, int dir );
int (* const DATA_MvSpinMoveTbl[])( MMDL * mmdl, MV_SPIN_DIR_WORK *work );

int (* const DATA_MvRewarMoveTbl[])( MMDL * mmdl, MV_SPIN_DIR_WORK *work );

int (* const DATA_MvRt2MoveTbl[])( MMDL * mmdl, MV_RT2_WORK *work );

static void MvRt3WorkInit( MMDL * mmdl, int check_no, int check_type, int id );
int (* const DATA_MvRt3MoveTbl[])( MMDL * mmdl, MV_RT3_WORK *work );

static void MvRt4WorkInit( MMDL * mmdl, int check_no, int check_type, int tbl_id );
int (* const DATA_MvRt4MoveTbl[])( MMDL * mmdl, MV_RT4_WORK *work );

static const int DATA_KuruKuruTbl[RT_KURU2_MAX][RT_KURU2_DIR_MAX];
static int MoveSub_KuruKuruCheck( MMDL * mmdl );
static void MoveSub_KuruKuruInit( MMDL * mmdl, RT_KURUKURU_WORK *work );
static void MoveSub_KuruKuruSet( MMDL * mmdl, RT_KURUKURU_WORK *work );
static void MoveSub_KuruKuruEnd( MMDL * mmdl, RT_KURUKURU_WORK *work );

//data
const int DATA_MvDirRndWaitTbl[];
const int DATA_MvDirRndDirTbl[];
const int DATA_MvDirRndDirTbl_UL[];
const int DATA_MvDirRndDirTbl_UR[];
const int DATA_MvDirRndDirTbl_DL[];
const int DATA_MvDirRndDirTbl_DR[];
const int DATA_MvDirRndDirTbl_UDL[];
const int DATA_MvDirRndDirTbl_UDR[];
const int DATA_MvDirRndDirTbl_ULR[];
const int DATA_MvDirRndDirTbl_DLR[];
const int DATA_MvDirRndDirTbl_UD[];
const int DATA_MvDirRndDirTbl_LR[];

const int DATA_MV_RND_DirTbl[];
const int DATA_MV_RND_V_DirTbl[];
const int DATA_MV_RND_H_DirTbl[];

const int DATA_MvRtURLD_DirTbl[];
const int DATA_MvRtRLDU_DirTbl[];
const int DATA_MvRtDURL_DirTbl[];
const int DATA_MvRtLDUR_DirTbl[];
const int DATA_MvRtULRD_DirTbl[];
const int DATA_MvRtLRDU_DirTbl[];
const int DATA_MvRtDULR_DirTbl[];
const int DATA_MvRtRDUL_DirTbl[];
const int DATA_MvRtLUDR_DirTbl[];
const int DATA_MvRtUDRL_DirTbl[];
const int DATA_MvRtRLUD_DirTbl[];
const int DATA_MvRtDRLU_DirTbl[];
const int DATA_MvRtRUDL_DirTbl[];
const int DATA_MvRtUDLR_DirTbl[];
const int DATA_MvRtLRUD_DirTbl[];
const int DATA_MvRtDLRU_DirTbl[];

const int DATA_MvRtUL_DirTbl[];
const int DATA_MvRtDR_DirTbl[];
const int DATA_MvRtLD_DirTbl[];
const int DATA_MvRtRU_DirTbl[];
const int DATA_MvRtUR_DirTbl[];
const int DATA_MvRtDL_DirTbl[];
const int DATA_MvRtLU_DirTbl[];
const int DATA_MvRtRD_DirTbl[];

static const DIR_TBL DATA_MoveDirTbl[DIRID_MAX];

const int JikiDashSensorMoveCodeTbl[];

//======================================================================
//  デバッグ
//======================================================================

//======================================================================
//  MV_DIR_RND  ランダムに方向切り替え
//======================================================================
//--------------------------------------------------------------
/**
 * MV_DIR_RND_WORK初期化
 * @param  mmdl  MMDL *
 * @param  id    方向テーブルID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DirRndWorkInit( MMDL * mmdl, int id )
{
  MV_DIR_RND_WORK *work;
  
  work = MMDL_InitMoveProcWork( mmdl, MV_DIR_RND_WORK_SIZE );  //動作ワーク初期化
  work->wait = TblRndGet( DATA_MvDirRndWaitTbl, WAIT_END );      //待ち時間セット
  work->tbl_id = id;
  
  MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );          //描画ステータス　停止
  MMDL_OffMoveBitMove( mmdl );                //常に停止中
}

//--------------------------------------------------------------
/**
 * MV_DIR_RND 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRnd_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RND_UL 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndUL_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_UL );
}

//--------------------------------------------------------------
/**
 * MV_RND_UR 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndUR_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_UR );
}

//--------------------------------------------------------------
/**
 * MV_RND_DL 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndDL_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_DL );
}

//--------------------------------------------------------------
/**
 * MV_RND_DR 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndDR_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_DR );
}

//--------------------------------------------------------------
/**
 * MV_RND_UDL 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndUDL_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_UDL );
}

//--------------------------------------------------------------
/**
 * MV_RND_UDR 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndUDR_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_UDR );
}

//--------------------------------------------------------------
/**
 * MV_RND_ULR 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndULR_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_ULR );
}

//--------------------------------------------------------------
/**
 * MV_RND_DLR 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndDLR_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_DLR );
}

//--------------------------------------------------------------
/**
 * MV_RND_UD 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndUD_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_UD );
}

//--------------------------------------------------------------
/**
 * MV_RND_LR 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRndLR_Init( MMDL * mmdl )
{
  DirRndWorkInit( mmdl, DIRID_MvDirRndDirTbl_LR );
}

//--------------------------------------------------------------
/**
 * MV_DIR_RND 動作
 * @param  mmdl    MMDL * 
 * @retval  int      TRUE=初期化成功
 */
//--------------------------------------------------------------
void MMDL_MoveDirRnd_Move( MMDL * mmdl )
{
  MV_DIR_RND_WORK *work = MMDL_GetMoveProcWork( mmdl );
  u16 type = MMDL_GetEventType( mmdl );
  
  if( type == EV_TYPE_TRAINER_DASH_ALTER )
  {
    if( MMDL_CheckPlayerDispSizeRect(mmdl) == TRUE ){
      if( checkDashButton() == TRUE ){
        u16 dir = MMDL_GetDirDisp( mmdl );
        dir = get_DashAlterNextDir( MMDL_GetMoveCode(mmdl), dir );
        
        if( dir != DIR_NOT ){
          MMDL_SetDirDisp( mmdl, dir );
        }
      }
    }
  }
  else
  {
    int dir = TrJikiDashSearchTbl( mmdl, work->tbl_id, DIR_NOT );

    if( dir != DIR_NOT ){
      MMDL_SetDirDisp( mmdl, dir );
    }else{
      switch( work->seq_no ){
      case 0:
        work->wait--;
      
        if( work->wait <= 0 ){                  //待ち時間 0
          work->wait = TblRndGet( DATA_MvDirRndWaitTbl, WAIT_END );
          MMDL_SetDirDisp( mmdl, TblIDRndGet(work->tbl_id,DIR_NOT) );
        }
      }
    }
  }
  
  MMDL_UpdateGridPosCurrent( mmdl );
}

//--------------------------------------------------------------
/// MV_DIR_RND系 EV_TYPE_TRAINER_DASH_ALTER用方向テーブル
/// [対応動作コード, 方向...DIR_NOT] 時計回り
//--------------------------------------------------------------
static const u16 data_DashAlterDirTbl[][1+(4+1)] =
{
  {MV_RND_V,   DIR_UP,DIR_DOWN,DIR_NOT},
  {MV_RND_H,   DIR_LEFT,DIR_RIGHT,DIR_NOT},
  {MV_RND_UL,  DIR_UP,DIR_LEFT,DIR_NOT},
  {MV_RND_UR,  DIR_UP,DIR_RIGHT,DIR_NOT},
  {MV_RND_DL,  DIR_DOWN,DIR_LEFT,DIR_NOT},
  {MV_RND_DR,  DIR_DOWN,DIR_RIGHT,DIR_NOT},
  {MV_RND_UDL, DIR_UP,DIR_DOWN,DIR_LEFT,DIR_NOT},
  {MV_RND_UDR, DIR_UP,DIR_RIGHT,DIR_DOWN,DIR_NOT},
  {MV_RND_ULR, DIR_UP,DIR_RIGHT,DIR_LEFT,DIR_NOT},
  {MV_RND_DLR, DIR_DOWN,DIR_LEFT,DIR_RIGHT,DIR_NOT},
  {MV_CODE_NOT},
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
static u16 get_DashAlterNextDir( u16 mv_code, u16 dir )
{
  const u16 *tbl;
  int i = 0,j;
  
  while( data_DashAlterDirTbl[i][0] != MV_CODE_NOT )
  {
    if( data_DashAlterDirTbl[i][0] == mv_code )
    {
      j = 1;
      tbl = data_DashAlterDirTbl[i];
      
      while( tbl[j] != DIR_NOT )
      {
        if( tbl[j] == dir )
        {
          j++;
          
          if( tbl[j] == DIR_NOT )
          {
            j = 1;
          }
          
          return( tbl[j] );
        }
        j++;
      }
      
      GF_ASSERT( 0 && "EV_TYPE_TRAINER_DASH_ALTER ERROR DIR\n" );
      return( DIR_NOT );
    }
    
    i++;
  }
  
  GF_ASSERT( 0 && "EV_TYPE_TRAINER_DASH_ALTER ERROR MV_CODE\n" );
  return( DIR_NOT );
}

//--------------------------------------------------------------
/**
 * MV_DIR_RND 削除関数
 * @param  mmdl  MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDirRnd_Delete( MMDL * mmdl )
{
}

//======================================================================
//  MV_RND系  ランダムに移動
//======================================================================
//--------------------------------------------------------------
/**
 * MV_RND 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRnd_Init( MMDL * mmdl )
{
  MvRndWorkInit( mmdl, AC_WALK_U_8F, DIRID_MV_RND_DirTbl, MV_RND_MOVE_CHECK_NORMAL );
}

//--------------------------------------------------------------
/**
 * MV_RND_V 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRndV_Init( MMDL * mmdl )
{
  MvRndWorkInit( mmdl, AC_WALK_U_8F, DIRID_MV_RND_V_DirTbl, MV_RND_MOVE_CHECK_NORMAL );
}

//--------------------------------------------------------------
/**
 * MV_RND_H 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRndH_Init( MMDL * mmdl )
{
  MvRndWorkInit( mmdl, AC_WALK_U_8F, DIRID_MV_RND_H_DirTbl, MV_RND_MOVE_CHECK_NORMAL );
}

//--------------------------------------------------------------
/**
 * MV_RND_H_LIM 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRndHLim_Init( MMDL * mmdl )
{
  MvRndWorkInit( mmdl, AC_WALK_U_8F, DIRID_MV_RND_H_DirTbl, MV_RND_MOVE_CHECK_LIMIT_ONLY );
}

//--------------------------------------------------------------
/**
 * MV_RND_UL系 初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRndRect_Init( MMDL * mmdl )
{
  MvRndWorkInit( mmdl, AC_WALK_U_8F, DIRID_MV_RND_DirTbl, MV_RND_MOVE_CHECK_RECT );
}

//======================================================================
//  MV_RND系　パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * MV_RND_WORK初期化
 * @param  mmdl    MMDL *
 * @param  ac      移動する際に使用するアニメーションコマンド。
 * MMDL_ChangeDirAcmdCode()で変換対象となる。AC_WALK_U_32F等
 * @param  tbl_id    方向テーブルID
 * @param  check    MV_RND_MOVE_CHECK_NORMAL等
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MvRndWorkInit( MMDL * mmdl, int ac, int tbl_id, int check )
{
  MV_RND_WORK *work;
  
  work = MMDL_InitMoveProcWork( mmdl, MV_RND_WORK_SIZE );
  work->move_check_type = check;
  work->acmd_code = ac;
  work->tbl_id = tbl_id;
  
  MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
  MMDL_OffMoveBitMove( mmdl );
}

//--------------------------------------------------------------
/**
 * MV_RND系　動作
 * @param  mmdl    MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MvRnd_Move( MMDL * mmdl )
{
  int ret;
  MV_RND_WORK *work;
  
  work = MMDL_GetMoveProcWork( mmdl );
  
  switch( work->seq_no ){
  case 0:
    MMDL_OffMoveBitMove( mmdl );
    MMDL_OffMoveBitMoveEnd( mmdl );
    
    ret = MMDL_GetDirDisp( mmdl );
    ret = MMDL_ChangeDirAcmdCode( ret, AC_DIR_U );
    MMDL_SetLocalAcmd( mmdl, ret );
    
    work->seq_no++;
    break;
  case 1:
    if( MMDL_ActionLocalAcmd(mmdl) == FALSE ){
      break;
    }
    
    work->wait = TblRndGet( DATA_MvDirRndWaitTbl, WAIT_END );
    work->seq_no++;
  case 2:
    work->wait--;
    
    if( work->wait ){
      break;
    }
    
    work->seq_no++; //即 次のシーケンスへ移る
  case 3:
    ret = TblIDRndGet( work->tbl_id, DIR_NOT );
    MMDL_SetDirAll( mmdl, ret );
    
    if( work->move_check_type == MV_RND_MOVE_CHECK_RECT ){
      if( MvRndRectMoveLimitCheck(mmdl,ret) == FALSE ){
        work->seq_no = 0;
        break;
      }
    }
    
    {
      u32 hit = MMDL_HitCheckMoveDir( mmdl, ret );
      
      if( hit != MMDL_MOVEHITBIT_NON ){
        if( work->move_check_type == MV_RND_MOVE_CHECK_LIMIT_ONLY ){
          if( (hit&MMDL_MOVEHITBIT_LIM) ){ //移動制限のみ見る
            work->seq_no = 0;
            break;
          }
        }else{
          work->seq_no = 0;
          break;
        }
      }
    }
    
    ret = MMDL_ChangeDirAcmdCode( ret, work->acmd_code );
    MMDL_SetLocalAcmd( mmdl, ret );
      
    MMDL_OnMoveBitMove( mmdl );
    work->seq_no++;
  case 4:
    if( MMDL_ActionLocalAcmd(mmdl) == FALSE ){
      break;
    }
    
    MMDL_OffMoveBitMove( mmdl );
    work->seq_no = 0;
  }
}

//======================================================================
//  MV_RND_UL等パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 移動可能範囲矩形作成
 * @param  mmdl  MMDL *
 * @param  rect  矩形格納先
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MvRndRectMake( MMDL * mmdl, RECT *rect )
{
  int code,ix,iz,lx,lz;
  
  ix = MMDL_GetInitGridPosX( mmdl );
  iz = MMDL_GetInitGridPosZ( mmdl );
  lx = MMDL_GetMoveLimitX( mmdl );
  lz = MMDL_GetMoveLimitZ( mmdl );
  
  code = MMDL_GetMoveCode( mmdl );
  
  switch( code ){
  case MV_RND_UL:
    rect->left = ix - lx; rect->right = ix;
    rect->top = iz - lz; rect->bottom = iz;
    break;
  case MV_RND_UR:
    rect->left = ix; rect->right = ix + lx;
    rect->top = iz - lz; rect->bottom = iz;
    break;
  case MV_RND_DL:
    rect->left = ix - lx; rect->right = ix;
    rect->top = iz; rect->bottom = iz + lz;
    break;
  case MV_RND_DR:
    rect->left = ix; rect->right = ix + lx;
    rect->top = iz; rect->bottom = iz + lz;
    break;
  case MV_RND_UDL:
    rect->left = ix - lx; rect->right = ix;
    rect->top = iz - lz; rect->bottom = iz + lz;
    break;
  case MV_RND_UDR:
    rect->left = ix; rect->right = ix + lx;
    rect->top = iz - lz; rect->bottom = iz + lz;
    break;
  case MV_RND_ULR:
    rect->left = ix - lx; rect->right = ix + lx;
    rect->top = iz - lz; rect->bottom = iz;
    break;
  case MV_RND_DLR:
    rect->left = ix - lx; rect->right = ix + lx;
    rect->top = iz; rect->bottom = iz + lz;
    break;
  default:
    GF_ASSERT( 0 && "MvRndRectMake()未対応動作コー" );
  }
}

//--------------------------------------------------------------
/**
 * 移動可能範囲チェック　方向指定
 * @param  mmdl  MMDL *
 * @param  dir    移動方向
 * @retval  int    TRUE=可能
 */
//--------------------------------------------------------------
static int MvRndRectMoveLimitCheck( MMDL * mmdl, int dir )
{
  int gx,gz;
  RECT rect;
  
  MvRndRectMake( mmdl, &rect );
  gx = MMDL_GetGridPosX( mmdl ) + MMDL_TOOL_GetDirAddValueGridX( dir );
  gz = MMDL_GetGridPosZ( mmdl ) + MMDL_TOOL_GetDirAddValueGridZ( dir );
  
  if( rect.left > gx || rect.right < gx ){
    return( FALSE );
  }
  
  if( rect.top > gz || rect.bottom < gz ){
    return( FALSE );
  }
  
  return( TRUE );
}

//======================================================================
//  MV_UP系　常に指定方向を向く
//======================================================================
//--------------------------------------------------------------
/**
 * MV_UP系　初期化
 * @param  mmdl  MMDL *
 * @param  dir    向く方向
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MvDirWorkInit( MMDL * mmdl, int dir )
{
  MV_DIR_WORK *work;
  
  work = MMDL_InitMoveProcWork( mmdl, MV_DIR_WORK_SIZE );
  work->dir = dir;
  
  MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
  MMDL_OffMoveBitMove( mmdl );
  MMDL_UpdateGridPosCurrent( mmdl );
}

//--------------------------------------------------------------
/**
 * MV_UP系　動作
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDir_Move( MMDL * mmdl )
{
  MV_DIR_WORK *work;
  
  work = MMDL_GetMoveProcWork( mmdl );
  
  switch( work->seq_no ){
  case 0:
    MMDL_SetDirDisp( mmdl, work->dir );
    work->seq_no++;
    break;
  case 1:
    break;
  }
}

//--------------------------------------------------------------
/**
 * MV_UP 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveUp_Init( MMDL * mmdl )
{
  MvDirWorkInit( mmdl, DIR_UP );
}

//--------------------------------------------------------------
/**
 * MV_DOWN 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveDown_Init( MMDL * mmdl )
{
  MvDirWorkInit( mmdl, DIR_DOWN );
}

//--------------------------------------------------------------
/**
 * MV_LEFT 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveLeft_Init( MMDL * mmdl )
{
  MvDirWorkInit( mmdl, DIR_LEFT );
}

//--------------------------------------------------------------
/**
 * MV_RIGHT 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRight_Init( MMDL * mmdl )
{
  MvDirWorkInit( mmdl, DIR_RIGHT );
}

//======================================================================
//  MV_SPIN系　方向回転
//======================================================================
//--------------------------------------------------------------
/**
 * MV_SPIN_DIR_WORK初期化
 * @param  mmdl  MMDL *
 * @param  dir    DIR_LEFT=左周り、DIR_RIGHT=右周り
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MvSpinDirWorkInit( MMDL * mmdl, int dir )
{
  MV_SPIN_DIR_WORK *work;
  
  work = MMDL_InitMoveProcWork( mmdl, MV_SPIN_DIR_WORK_SIZE );
  work->spin_dir = dir;
  
  MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
  MMDL_OffMoveBitMove( mmdl );
  MMDL_UpdateGridPosCurrent( mmdl );
}

//--------------------------------------------------------------
/**
 * MV_SPIN_L　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveSpinLeft_Init( MMDL * mmdl )
{
  MvSpinDirWorkInit( mmdl, DIR_LEFT );
}

//--------------------------------------------------------------
/**
 * MV_SPIN_R　初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveSpinRight_Init( MMDL * mmdl )
{
  MvSpinDirWorkInit( mmdl, DIR_RIGHT );
}

//--------------------------------------------------------------
/**
 * MV_SPIN系　動作
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveSpin_Move( MMDL * mmdl )
{
  MV_SPIN_DIR_WORK *work;
  
  work = MMDL_GetMoveProcWork( mmdl );
  while( DATA_MvSpinMoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * MV_SPIN系　動作　現在方向コマンドセット
 * @param  mmdl  MMDL *
 * @param  work  MV_SPIN_DIR_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvSpinMove_DirCmdSet( MMDL * mmdl, MV_SPIN_DIR_WORK *work )
{
 int ret = TrJikiDashSearchTbl( mmdl, DIRID_MvDirSpin_DirTbl, DIR_NOT );
  
  if( ret == DIR_NOT ){
    ret = MMDL_GetDirDisp( mmdl );
  }
  
  ret = MMDL_ChangeDirAcmdCode( ret, AC_DIR_U );
  MMDL_SetLocalAcmd( mmdl, ret );
  
  work->seq_no = SEQNO_MV_SPIN_DIR_CMD_ACT;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_SPIN系　動作　コマンドアクション
 * @param  mmdl  MMDL *
 * @param  work  MV_SPIN_DIR_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvSpinMove_CmdAction( MMDL * mmdl, MV_SPIN_DIR_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == FALSE ){
    return( FALSE );
  }
  
  if( MMDL_GetEventType(mmdl) != EV_TYPE_TRAINER_DASH_ALTER ){
    work->wait = 0; //ダッシュ反応は別の扱いになっている。
  }

  work->seq_no = SEQNO_MV_SPIN_DIR_WAIT;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_SPIN系　動作　ウェイト
 * @param  mmdl  MMDL *
 * @param  work  MV_SPIN_DIR_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvSpinMove_Wait( MMDL * mmdl, MV_SPIN_DIR_WORK *work )
{
  if( MMDL_GetEventType(mmdl) == EV_TYPE_TRAINER_DASH_ALTER ){
    if( work->wait != 0 ){
      work->wait = 0;
      return( FALSE );
    }
    
    if( MMDL_CheckPlayerDispSizeRect(mmdl) == FALSE ){
      return( FALSE );
    }
    
    if( checkDashButton() == FALSE ){
      return( FALSE );
    }
    
    work->wait++;
    work->seq_no = SEQNO_MV_SPIN_DIR_NEXT_DIR_SET;
  }else{
    if( work->wait ){
      if( TrJikiDashSearchTbl(
            mmdl,DIRID_MvDirSpin_DirTbl,DIR_NOT) != DIR_NOT ){
        work->seq_no = SEQNO_MV_SPIN_DIR_CMD_SET;
        return( TRUE );
      }
    }
  
    work->wait++;
  
    if( work->wait < MV_SPIN_WAIT_FRAME ){
      return( FALSE );
    }
  
    work->seq_no = SEQNO_MV_SPIN_DIR_NEXT_DIR_SET;
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_SPIN系　動作　次方向セット
 * @param  mmdl  MMDL *
 * @param  work  MV_SPIN_DIR_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvSpinMove_NextDirSet( MMDL * mmdl, MV_SPIN_DIR_WORK *work )
{
  int i,ret,*tbl;
  int left[5] = { DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT, DIR_NOT };
  int right[5] = { DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_NOT };
  
  if( work->spin_dir == DIR_LEFT ){
    tbl = left;
  }else{
    tbl = right;
  }
  
  ret = MMDL_GetDirDisp( mmdl );
  
  for( i = 0; tbl[i] != DIR_NOT; i++ ){
    if( ret == tbl[i] ){
      break;
    }
  }
  
  GF_ASSERT( tbl[i] != DIR_NOT && "FieldOBJ MV_SPIN初期方向エラー" );
  
  i++;
  
  if( tbl[i] == DIR_NOT ){
    i = 0;
  }
  
  ret = tbl[i];
  
  MMDL_SetDirDisp( mmdl, ret );
  
  work->seq_no = SEQNO_MV_SPIN_DIR_CMD_SET;
  return( TRUE );
}

//--------------------------------------------------------------
///  MV_SPIN動作テーブル
//--------------------------------------------------------------
static int (* const DATA_MvSpinMoveTbl[])( MMDL * mmdl, MV_SPIN_DIR_WORK *work ) =
{
  MvSpinMove_DirCmdSet,
  MvSpinMove_CmdAction,
  MvSpinMove_Wait,
  MvSpinMove_NextDirSet,
};

//======================================================================
//  MV_REWAR　右回転<->左回転
//======================================================================
//--------------------------------------------------------------
/**
 * MV_REWAR 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRewar_Init( MMDL * mmdl )
{
  MvSpinDirWorkInit( mmdl, DIR_RIGHT );
}

//--------------------------------------------------------------
/**
 * MV_REWAR系　動作
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRewar_Move( MMDL * mmdl )
{
  MV_SPIN_DIR_WORK *work;
  
  work = MMDL_GetMoveProcWork( mmdl );
  while( DATA_MvRewarMoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * MV_REWAR　動作　現在方向コマンドセット
 * @param  mmdl  MMDL *
 * @param  work  MV_SPIN_DIR_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRewarMove_DirCmdSet( MMDL * mmdl, MV_SPIN_DIR_WORK *work )
{
  int ret = MMDL_GetDirDisp( mmdl );
  ret = MMDL_ChangeDirAcmdCode( ret, AC_DIR_U );
  MMDL_SetLocalAcmd( mmdl, ret );
  
  work->seq_no = SEQNO_MV_REWAR_DIR_CMD_ACT;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_REWAR　動作　コマンドアクション
 * @param  mmdl  MMDL *
 * @param  work  MV_SPIN_DIR_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRewarMove_CmdAction( MMDL * mmdl, MV_SPIN_DIR_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == FALSE ){
    return( FALSE );
  }
  
  work->wait = 0;
  work->seq_no = SEQNO_MV_REWAR_DIR_WAIT;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_REWAR系　動作　ウェイト
 * @param  mmdl  MMDL *
 * @param  work  MV_SPIN_DIR_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRewarMove_Wait( MMDL * mmdl, MV_SPIN_DIR_WORK *work )
{
  work->wait++;
  
  if( work->wait < MV_SPIN_WAIT_FRAME ){
    return( FALSE );
  }
  
  work->seq_no = SEQNO_MV_REWAR_DIR_NEXT_DIR_SET;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_REWAR　動作　次方向セット
 * @param  mmdl  MMDL *
 * @param  work  MV_SPIN_DIR_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRewarMove_NextDirSet( MMDL * mmdl, MV_SPIN_DIR_WORK *work )
{
  int i,ret,*tbl;
  int left[5] = { DIR_UP, DIR_LEFT, DIR_DOWN, DIR_RIGHT, DIR_NOT };
  int right[5] = { DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_NOT };
  
  if( work->spin_dir == DIR_LEFT ){
    tbl = left;
  }else{
    tbl = right;
  }
  
  ret = MMDL_GetDirDisp( mmdl );
  
  for( i = 0; tbl[i] != DIR_NOT; i++ ){
    if( ret == tbl[i] ){
      break;
    }
  }
  
  GF_ASSERT( tbl[i] != DIR_NOT && "FieldOBJ MV_REWAR初期方向エラー" );
  
  i++;
  
  if( tbl[i] == DIR_NOT ){
    i = 0;
  }
  
  ret = tbl[i];
  
  MMDL_SetDirDisp( mmdl, ret );
  
  {
    int dir = MMDL_GetDirDisp( mmdl );
    int sdir = MMDL_GetDirHeader( mmdl );
    
    if( dir == sdir ){
      work->spin_dir = MMDL_TOOL_FlipDir( work->spin_dir );
    }
  }
  
  work->seq_no = SEQNO_MV_REWAR_DIR_CMD_SET;
  return( TRUE );
}

//--------------------------------------------------------------
///  MV_SPIN動作テーブル
//--------------------------------------------------------------
static int (* const DATA_MvRewarMoveTbl[])( MMDL * mmdl, MV_SPIN_DIR_WORK *work ) =
{
  MvRewarMove_DirCmdSet,
  MvRewarMove_CmdAction,
  MvRewarMove_Wait,
  MvRewarMove_NextDirSet,
};

//======================================================================
//  MV_RT2　一方向を往復  
//======================================================================
//--------------------------------------------------------------
/**
 * MV_RT2 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRoute2_Init( MMDL * mmdl )
{
  MV_RT2_WORK *work;
  
  work = MMDL_InitMoveProcWork( mmdl, MV_RT2_WORK_SIZE );
  
  if( MoveSub_KuruKuruCheck(mmdl) == TRUE ){
    MoveSub_KuruKuruInit( mmdl, &work->kurukuru );
  }
}

//--------------------------------------------------------------
/**
 * MV_RT2 動作
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRoute2_Move( MMDL * mmdl )
{
  MV_RT2_WORK *work = MMDL_GetMoveProcWork( mmdl );
  
  while( DATA_MvRt2MoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * MV_RT2 動作　方向チェック
 * @param  mmdl  MMDL *
 * @param  work  MV_RT2_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRt2_DirCheck( MMDL * mmdl, MV_RT2_WORK *work )
{
  int dir;
  
  dir = MMDL_GetDirHeader( mmdl );            //ヘッダ指定方向
  
  if( work->turn_flag == TRUE ){
    dir = MMDL_TOOL_FlipDir( dir );
  }
  
  MMDL_SetDirMove( mmdl, dir );
  
  if( MoveSub_KuruKuruCheck(mmdl) == FALSE ){
    MMDL_SetDirDisp( mmdl, dir );
  }
  
  work->seq_no = SEQNO_MV_RT2_MOVE_SET;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_RT2 動作　動作セット
 * @param  mmdl  MMDL *
 * @param  work  MV_RT2_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRt2_MoveSet( MMDL * mmdl, MV_RT2_WORK *work )
{
  if( checkEvTypeDashReact(mmdl,&work->react_count) == FALSE ){
    return( FALSE );
  }
  
  if( work->turn_flag ){
    int ix,iz,gx,gz;
    
    ix = MMDL_GetInitGridPosX( mmdl );
    iz = MMDL_GetInitGridPosZ( mmdl );
    gx = MMDL_GetGridPosX( mmdl );
    gz = MMDL_GetGridPosZ( mmdl );
    
    if( ix == gx && iz == gz ){
      int dir = MMDL_TOOL_FlipDir( MMDL_GetDirMove(mmdl) );
      
      MMDL_SetDirMove( mmdl, dir );
      
      if( MoveSub_KuruKuruCheck(mmdl) == FALSE ){
        MMDL_SetDirDisp( mmdl, dir );
      }
      
      work->turn_flag = FALSE;
    }
  }
  
  {
    int dir,ac;
    u32 ret;
    
    dir = MMDL_GetDirMove( mmdl );
    ret = MMDL_HitCheckMoveDir( mmdl, dir );
    
    if( (ret & MMDL_MOVEHITBIT_LIM) ){
      work->turn_flag = TRUE;
      dir = MMDL_TOOL_FlipDir( dir );
      ret = MMDL_HitCheckMoveDir( mmdl, dir );
    }
    
    ac = AC_WALK_U_8F;
    
    if( checkMMdlEventTypeDashAccel(mmdl) ){
      if( checkDashButton() == TRUE ){
        if( MMDL_CheckPlayerDispSizeRect(mmdl) == TRUE ){
          ac = AC_WALK_U_4F;
        }
      }
    }
    
    if( ret != MMDL_MOVEHITBIT_NON ){
      ac = AC_STAY_WALK_U_8F;
    }
    
    ac = MMDL_ChangeDirAcmdCode( dir, ac );
    MMDL_SetLocalAcmd( mmdl, ac );
    
    if( MoveSub_KuruKuruCheck(mmdl) == TRUE ){
      MoveSub_KuruKuruSet( mmdl, &work->kurukuru );
    }
  }
  
  MMDL_OnMoveBitMove( mmdl );
  work->seq_no = SEQNO_MV_RT2_MOVE;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_RT2 動作　動作中
 * @param  mmdl  MMDL *
 * @param  work  MV_RT2_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRt2_Move( MMDL * mmdl, MV_RT2_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == TRUE ){
    MMDL_OffMoveBitMove( mmdl );
    
    if( MoveSub_KuruKuruCheck(mmdl) == TRUE ){
      MoveSub_KuruKuruEnd( mmdl, &work->kurukuru );
    }
    
    work->seq_no = SEQNO_MV_RT2_DIR_CHECK;
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
///  MV_RT2動作テーブル
//--------------------------------------------------------------
static int (* const DATA_MvRt2MoveTbl[])( MMDL * mmdl, MV_RT2_WORK *work ) =
{
  MvRt2_DirCheck,
  MvRt2_MoveSet,
  MvRt2_Move,
};

//======================================================================
//  MV_RTURLD等　3箇所往復
//======================================================================
//--------------------------------------------------------------
/**
 * MV_RT3_WORK初期化
 * @param  mmdl    MMDL *
 * @param  check_no  3点移動で特定分岐とする点
 * @param  check_type  MV_RT3_CHECK_TYPE_X等
 * @param  tbl_id    テーブルID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MvRt3WorkInit( MMDL * mmdl, int check_no, int check_type, int tbl_id )
{
  MV_RT3_WORK *work;
  
  work = MMDL_InitMoveProcWork( mmdl, MV_RT3_WORK_SIZE );
  work->turn_check_no = check_no;
  work->turn_check_type = check_type;
  work->tbl_id = tbl_id;
  
  if( MoveSub_KuruKuruCheck(mmdl) == TRUE ){
    MoveSub_KuruKuruInit( mmdl, &work->kurukuru );
  }
}

//--------------------------------------------------------------
/**
 * MV_RTURLD 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteURLD_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtURLD_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTRLDU 初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteRLDU_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtRLDU_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTDURL初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteDURL_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtDURL_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTLDUR初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteLDUR_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtLDUR_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTULRD初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteULRD_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtULRD_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTLRDU初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteLRDU_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtLRDU_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTDULR初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteDULR_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtDULR_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTRDUL初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteRDUL_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtRDUL_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTLUDR初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteLUDR_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtLUDR_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTUDRL初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteUDRL_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtUDRL_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTRLUD初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteRLUD_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtRLUD_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTDRLU初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteDRLU_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtDRLU_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTRUDL初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteRUDL_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtRUDL_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTUDLR初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteUDLR_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtUDLR_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTLRUD初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteLRUD_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtLRUD_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTDLRU初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteDLRU_Init( MMDL * mmdl )
{
  MvRt3WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtDLRU_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RT3動作　
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRoute3_Move( MMDL * mmdl )
{
  MV_RT3_WORK *work;
  
  work = MMDL_GetMoveProcWork( mmdl );
  while( DATA_MvRt3MoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * MV_RT3動作　移動方向セット
 * @param  mmdl  MMDL *
 * @param  work  MV_RT3_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRt3Move_MoveDirSet( MMDL * mmdl, MV_RT3_WORK *work )
{
  if( checkEvTypeDashReact(mmdl,&work->react_count) == FALSE ){
    return( FALSE );
  }
  
  if( work->turn_no == work->turn_check_no  ){
    if( work->turn_check_type == MV_RT3_CHECK_TYPE_X ){
      int ix = MMDL_GetInitGridPosX( mmdl );
      int gx = MMDL_GetGridPosX( mmdl );
      
      if( ix == gx ){
        work->turn_no++;
      }
    }else{
      int iz = MMDL_GetInitGridPosZ( mmdl );
      int gz = MMDL_GetGridPosZ( mmdl );
      
      if( iz == gz ){
        work->turn_no++;
      }
    }
  }
  
  if( work->turn_no == MV_RT3_TURN_END_NO ){
    int ix = MMDL_GetInitGridPosX( mmdl );
    int iz = MMDL_GetInitGridPosZ( mmdl );
    int gx = MMDL_GetGridPosX( mmdl );
    int gz = MMDL_GetGridPosZ( mmdl );
    
    if( ix == gx && iz == gz ){
      work->turn_no = 0;
    }
  }
  
  {
    const int *tbl;
    int dir,ac;
    u32 ret;
    
    tbl = MoveDirTblIDSearch( work->tbl_id );
    dir = tbl[work->turn_no];
    
    MMDL_SetDirMove( mmdl, dir );
    
    if( MoveSub_KuruKuruCheck(mmdl) == FALSE ){
      MMDL_SetDirDisp( mmdl, dir );
    }
    
    ret = MMDL_HitCheckMoveDir( mmdl, dir );
    
    if( (ret & MMDL_MOVEHITBIT_LIM) ){
      work->turn_no++;
      dir = tbl[work->turn_no];
      
      MMDL_SetDirMove( mmdl, dir );
      
      if( MoveSub_KuruKuruCheck(mmdl) == FALSE ){
        MMDL_SetDirDisp( mmdl, dir );
      }
      
      ret = MMDL_HitCheckMoveDir( mmdl, dir );
    }
    
    ac = AC_WALK_U_8F;
    
    if( checkMMdlEventTypeDashAccel(mmdl) ){
      if( checkDashButton() == TRUE ){
        if( MMDL_CheckPlayerDispSizeRect(mmdl) == TRUE ){
          ac = AC_WALK_U_4F;
        }
      }
    }
 
    if( (ret != MMDL_MOVEHITBIT_NON) ){
      ac = AC_STAY_WALK_U_8F;
    }
    
    ac = MMDL_ChangeDirAcmdCode( dir, ac );
    MMDL_SetLocalAcmd( mmdl, ac );
    
    if( MoveSub_KuruKuruCheck(mmdl) == TRUE ){
      MoveSub_KuruKuruSet( mmdl, &work->kurukuru );
    }
  }
  
  MMDL_OnMoveBitMove( mmdl );
  work->seq_no = SEQNO_MV_RT3_MOVE;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_RT3動作　移動
 * @param  mmdl  MMDL *
 * @param  work  MV_RT3_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRt3Move_MoveDir( MMDL * mmdl, MV_RT3_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == TRUE ){
    MMDL_OffMoveBitMove( mmdl );
    
    if( MoveSub_KuruKuruCheck(mmdl) == TRUE ){
      MoveSub_KuruKuruEnd( mmdl, &work->kurukuru );
    }
    
    work->seq_no = SEQNO_MV_RT3_MOVE_DIR_SET;
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
///  MV_RT3動作テーブル
//--------------------------------------------------------------
static int (* const DATA_MvRt3MoveTbl[])( MMDL * mmdl, MV_RT3_WORK *work ) =
{
  MvRt3Move_MoveDirSet,
  MvRt3Move_MoveDir,
};

//======================================================================
//  MV_RTRL等　４点移動動作
//======================================================================
//--------------------------------------------------------------
/**
 * MV_RT4_WORK初期化
 * @param  mmdl    MMDL *
 * @param  check_no  4点移動で特定分岐とする点
 * @param  check_type  MV_RT3_CHECK_TYPE_X等
 * @param  tbl_id    テーブルID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MvRt4WorkInit( MMDL * mmdl, int check_no, int check_type, int tbl_id )
{
  MV_RT4_WORK *work;
  
  work = MMDL_InitMoveProcWork( mmdl, MV_RT4_WORK_SIZE );
  work->turn_check_no = check_no;
  work->turn_check_type = check_type;
  work->tbl_id = tbl_id;
  
  if( MoveSub_KuruKuruCheck(mmdl) == TRUE ){
    MoveSub_KuruKuruInit( mmdl, &work->kurukuru );
  }
}

//--------------------------------------------------------------
/**
 * MV_RTUL初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteUL_Init( MMDL * mmdl )
{
  MvRt4WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtUL_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTDR初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteDR_Init( MMDL * mmdl )
{
  MvRt4WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtDR_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTLD初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteLD_Init( MMDL * mmdl )
{
  MvRt4WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtLD_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTRU初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteRU_Init( MMDL * mmdl )
{
  MvRt4WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtRU_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTUR初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteUR_Init( MMDL * mmdl )
{
  MvRt4WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtUR_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTDL初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteDL_Init( MMDL * mmdl )
{
  MvRt4WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_Z, DIRID_MvRtDL_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTLU初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteLU_Init( MMDL * mmdl )
{
  MvRt4WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtLU_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RTRD初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRouteRD_Init( MMDL * mmdl )
{
  MvRt4WorkInit( mmdl, 2, MV_RT3_CHECK_TYPE_X, DIRID_MvRtRD_DirTbl );
}

//--------------------------------------------------------------
/**
 * MV_RT4系動作
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveRoute4_Move( MMDL * mmdl )
{
  MV_RT4_WORK *work = MMDL_GetMoveProcWork( mmdl );
  while( DATA_MvRt4MoveTbl[work->seq_no](mmdl,work) == TRUE ){};
}

//--------------------------------------------------------------
/**
 * MV_RT4動作　ターン回数増加
 * @param  work  MV_RT4_WORK *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MvRt4Move_TurnNoInc( MV_RT4_WORK *work )
{
  if( work->rev_flag == TRUE ){
    work->turn_no--;
    if( work->turn_no < 0 ){
      work->turn_no = 3;
    }
  }else{
    work->turn_no++;
  }
}

//--------------------------------------------------------------
/**
 * MV_RT4動作　自機チェック
 * @param  mmdl  MMDL *
 * @param  work  MV_RT4_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRt4Move_JikiCheck( MMDL * mmdl, MV_RT4_WORK *work )
{
  if( MMDL_GetEventType(mmdl) == EV_TYPE_TR_ESC ){
#ifdef DEBUG_ONLY_FOR_kagaya
    {
      int sx = 0xff, sz = 0xff;
      MMDL *jiki = MMDLSYS_SearchMMdlPlayer( MMDL_GetMMdlSys(mmdl) );
      int id = MMDL_GetOBJID( mmdl );
      int dir = MMDL_GetDirDisp( mmdl );
      int gx = MMDL_GetGridPosX( mmdl );
      int gz = MMDL_GetGridPosZ( mmdl );
      int jx = 0, jz = 0;
      if( jiki != NULL ){
        jx = MMDL_GetGridPosX(jiki); jz = MMDL_GetGridPosZ(jiki);
        sx = gx - jx;
        if( sx < 0 ){ sx = -sx; }
        sz = gz - jz;
        if( sz < 0 ){ sz = -sz; }
        sx += sz;
      }

      if( sx <= 1 ){
        KAGAYA_Printf( "MMDL TRAINER ESCAPE ID=%d, DIR=%d, GX=%d,GZ=%d (jiki GX=%d,GZ=%d)\n", id, dir, gx, gz, jx, jz );
        KAGAYA_Printf( "MMDL TRAINER ESCAPE HIT\n" );
      }
    }
#endif
    
    {
      u16 eye_dir = MMDL_GetDirDisp( mmdl );
      int eye_range = MMDL_GetParam( mmdl, MMDL_PARAM_0 );
      int ret = EVENT_CheckTrainerEyeRange( mmdl, eye_dir, eye_range, NULL );
    
      if( ret != EYE_CHECK_NOHIT ){
        u16 ac = MMDL_ChangeDirAcmdCode( eye_dir, AC_STAY_JUMP_U_8F );
        MMDL_SetLocalAcmd( mmdl, ac );
        MMDL_OnMoveBitMove( mmdl );
        work->seq_no = SEQNO_MV_RT4_DISCOVERY_JUMP;
        return( TRUE );
      }
    }
  }
  
  work->seq_no = SEQNO_MV_RT4_MOVE_DIR_SET;
  return( TRUE );
}

//--------------------------------------------------------------
///MV_RT4_REV_CODE ルート4反転コード
//--------------------------------------------------------------
typedef struct
{
  u16 m_code;
  u16 r_code;
}MV_RT4_REV_CODE;

///ルート４反転まとめ
static const MV_RT4_REV_CODE data_MoveRoute4_ReverseTbl[] =
{
  {MV_RTUL,MV_RTLU},
  {MV_RTDR,MV_RTRD},
  {MV_RTLD,MV_RTDL},
  {MV_RTRU,MV_RTUR},
  {MV_CODE_NOT,MV_CODE_NOT}, //end
};

//--------------------------------------------------------------
/**
 * MV_RT4動作　自機を発見　飛び上がり動作
 * @param  mmdl  MMDL *
 * @param  work  MV_RT4_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRt4Move_DiscoveryJump( MMDL * mmdl, MV_RT4_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == TRUE ){
    u16 r_code = MV_CODE_NOT;
    u16 mv_code = MMDL_GetMoveCode( mmdl );
    const MV_RT4_REV_CODE *data = data_MoveRoute4_ReverseTbl;
    
    while( data->m_code != MV_CODE_NOT ){
      if( data->m_code == mv_code ){
        r_code = data->r_code;
      }else if( data->r_code == mv_code ){
        r_code = data->m_code;
      }
      
      if( r_code != MV_CODE_NOT ){
        u16 turn_no = MV_RT4_TURN_END_NO_MAX - (work->turn_no+1);
        GF_ASSERT( turn_no < 4 );
        MMDL_ChangeMoveCode( mmdl, r_code );
        work->turn_no = turn_no; //反転した回転数
        break;
      }
       
      data++;
    }
    
    MMDL_OffMoveBitMove( mmdl );
    work->seq_no = SEQNO_MV_RT4_MOVE_DIR_SET;
    GF_ASSERT( r_code != MV_CODE_NOT );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * MV_RT4動作　移動方向セット
 * @param  mmdl  MMDL *
 * @param  work  MV_RT4_WORK
 * @retval  int    TRUE=再帰
*/
//--------------------------------------------------------------
static int MvRt4Move_MoveDirSet( MMDL * mmdl, MV_RT4_WORK *work )
{
  if( checkEvTypeDashReact(mmdl,&work->react_count) == FALSE ){
    return( FALSE );
  }
  
  if( work->turn_no == work->turn_check_no  ){
    if( work->turn_check_type == MV_RT3_CHECK_TYPE_X ){
      int ix = MMDL_GetInitGridPosX( mmdl );
      int gx = MMDL_GetGridPosX( mmdl );
      
      if( ix == gx ){
        MvRt4Move_TurnNoInc( work );
      }
    }else{
      int iz = MMDL_GetInitGridPosZ( mmdl );
      int gz = MMDL_GetGridPosZ( mmdl );
      
      if( iz == gz ){
        MvRt4Move_TurnNoInc( work );
      }
    }
  }
  
  if( work->turn_no == MV_RT3_TURN_END_NO ){
    int ix = MMDL_GetInitGridPosX( mmdl );
    int iz = MMDL_GetInitGridPosZ( mmdl );
    int gx = MMDL_GetGridPosX( mmdl );
    int gz = MMDL_GetGridPosZ( mmdl );
    
    if( ix == gx && iz == gz ){
      work->turn_no = 0;
    }
  }
  
  {
    const int *tbl;
    int dir,ac;
    u32 ret;
    
    tbl = MoveDirTblIDSearch( work->tbl_id );
    dir = tbl[work->turn_no];
    
    MMDL_SetDirMove( mmdl, dir );
    
    if( MoveSub_KuruKuruCheck(mmdl) == FALSE ){
      MMDL_SetDirDisp( mmdl, dir );
    }
    
    ret = MMDL_HitCheckMoveDir( mmdl, dir );
    
    if( (ret & MMDL_MOVEHITBIT_LIM) ){
      MvRt4Move_TurnNoInc( work );
      dir = tbl[work->turn_no];
      
      MMDL_SetDirMove( mmdl, dir );
      
      if( MoveSub_KuruKuruCheck(mmdl) == FALSE ){
        MMDL_SetDirDisp( mmdl, dir );
      }
      
      ret = MMDL_HitCheckMoveDir( mmdl, dir );
    }
    
    ac = AC_WALK_U_8F;

    if( checkMMdlEventTypeDashAccel(mmdl) ){
      if( checkDashButton() == TRUE ){
        if( MMDL_CheckPlayerDispSizeRect(mmdl) == TRUE ){
          ac = AC_WALK_U_4F;
        }
      }
    }
    
    if( (ret != MMDL_MOVEHITBIT_NON) ){
      ac = AC_STAY_WALK_U_8F;
    }
    
    ac = MMDL_ChangeDirAcmdCode( dir, ac );
    MMDL_SetLocalAcmd( mmdl, ac );
    
    if( MoveSub_KuruKuruCheck(mmdl) == TRUE ){
      MoveSub_KuruKuruSet( mmdl, &work->kurukuru );
    }
  }
  
  MMDL_OnMoveBitMove( mmdl );
  work->seq_no = SEQNO_MV_RT4_MOVE;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * MV_RT4動作　移動
 * @param  mmdl  MMDL *
 * @param  work  MV_RT4_WORK
 * @retval  int    TRUE=再帰
 */
//--------------------------------------------------------------
static int MvRt4Move_MoveDir( MMDL * mmdl, MV_RT4_WORK *work )
{
  if( MMDL_ActionLocalAcmd(mmdl) == TRUE ){
    MMDL_OffMoveBitMove( mmdl );
    
    if( MoveSub_KuruKuruCheck(mmdl) == TRUE ){
      MoveSub_KuruKuruEnd( mmdl, &work->kurukuru );
    }
    
    work->seq_no = SEQNO_MV_RT4_JIKI_CHECK;
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
///  MV_RT4動作テーブル
//--------------------------------------------------------------
static int (* const DATA_MvRt4MoveTbl[])( MMDL * mmdl, MV_RT4_WORK *work ) =
{
  MvRt4Move_JikiCheck,
  MvRt4Move_DiscoveryJump,
  MvRt4Move_MoveDirSet,
  MvRt4Move_MoveDir,
};

//======================================================================
//  パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * ダッシュボタンが押されているか
 * @param nothing
 * @retval BOOL TRUE=押されている
 */
//--------------------------------------------------------------
static BOOL checkDashButton( void )
{
  if( (GFL_UI_KEY_GetCont() & BTN_DASH) ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 方向テーブル要素数取得
 * @param  tbl    データテーブル。終端にend
 * @param  end    テーブル終端とする値
 * @retval  int    tbl要素数
 */
//--------------------------------------------------------------
static int TblNumGet( const int *tbl, int end )
{
  int i=0; while(tbl[i]!=end){i++;} GF_ASSERT(i&&"TblNumGet()要素数0"); return(i);
}

//--------------------------------------------------------------
/**
 * テーブルデータからランダムで値取得
 * @param  tbl    データテーブル。終端にend
 * @param  end    テーブル終端とする値
 * @retval  int    tbl内部の値
 */
//--------------------------------------------------------------
static int TblRndGet( const int *tbl, int end )
{
#if 0
  return( tbl[gf_rand()%TblNumGet(tbl,end)] );
#else
  return( tbl[GFUser_GetPublicRand(TblNumGet(tbl,end))] );
#endif
}

//--------------------------------------------------------------
/**
 * 方向IDからテーブルデータを取得しランダムで値取得
 * @param  tbl    データテーブル。終端にend
 * @param  end    テーブル終端とする値
 * @retval  int    tbl内部の値
 */
//--------------------------------------------------------------
static int TblIDRndGet( int id, int end )
{
  const int *tbl = MoveDirTblIDSearch( id );
  return( tbl[GFUser_GetPublicRand(TblNumGet(tbl,end))] );
}

//--------------------------------------------------------------
/**
 * DATA_MoveDirTbl検索
 * @param  id    検索ID
 * @retval  int    ヒットしたテーブル*
 */
//--------------------------------------------------------------
static const int * MoveDirTblIDSearch( int id )
{
  const DIR_TBL *tbl = DATA_MoveDirTbl;
  
  while( tbl->id != DIRID_END ){
    if( tbl->id == id ){ return( tbl->tbl ); }
    tbl++;
  }
  
  GF_ASSERT( 0 && "MoveDirTblIDSearch()ID異常" );
  return( NULL );
}

//--------------------------------------------------------------
/**
 * 自機が指定範囲内でダッシュしているか
 * @param  mmdl  MMDL *
 * @retval  int    DIR_NOT=範囲内、方向切り替え型では無い。その他DIR_UP等
 */
//--------------------------------------------------------------
static int TrJikiDashSearch( MMDL * mmdl )
{
  int ret = MMDL_GetEventType( mmdl );
  
#ifndef DEBUG_SEARCH
  if( ret != EV_TYPE_TRAINER && ret != EV_TYPE_TRAINER_EYEALL ){
    return( DIR_NOT );
  }
#endif

#if 0 //old
  if( Player_AcmdCodeDashCheck(jiki) == FALSE ){
    return( DIR_NOT );
  }
#else
  if( checkDashButton() == FALSE ){
    return( DIR_NOT );
  }
#endif
  
  {
    int code,i = 0;
    ret = MMDL_GetMoveCode( mmdl );
      
    do{
      code = JikiDashSensorMoveCodeTbl[i++];
      if( code == ret ){ break; }
    }while( code != MV_CODE_NOT );
      
    if( ret != code ){
      return( DIR_NOT );
    }
  }
    
  {
    MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
    const MMDL *jiki = MMDLSYS_SearchMMdlPlayer( mmdlsys );

    if( jiki == NULL ){
      return( DIR_NOT );
    }else{
      int jy = MMDL_GetGridPosY( jiki );
      int y = MMDL_GetGridPosY( mmdl );
      
      if( jy != y ){
        return( DIR_NOT );
      }
    }
    
    {
      int jx = MMDL_GetGridPosX( jiki );
      int jz = MMDL_GetGridPosZ( jiki );
#ifndef DEBUG_SEARCH
      int range = MMDL_GetParam( mmdl, MMDL_PARAM_0 );
#else
      int range = 4;
#endif
      int x = MMDL_GetGridPosX( mmdl );
      int z = MMDL_GetGridPosZ( mmdl );
      int sx = x - range;
      int ex = x + range;
      int sz = z - range;
      int ez = z + range;
      

      if( sz <= jz && ez >= jz ){
        if( sx <= jx && ex >= jx ){
          return( MMDL_TOOL_GetRangeDir(x,z,jx,jz) );
        }
      }
    }
  }

  return( DIR_NOT );
}

//--------------------------------------------------------------
/**
 * 自機が指定範囲内でダッシュしていればその方向を返す。方向テーブルアリ
 * @param  mmdl  MMDL *
 * @retval  int    DIR_NOT=範囲内、方向切り替え型では無い。その他DIR_UP等
 */
//--------------------------------------------------------------
static int TrJikiDashSearchTbl( MMDL * mmdl, int id, int end )
{
  const int *tbl = MoveDirTblIDSearch( id );
  int num = TblNumGet( tbl, end );
  
  if( num == 1 ){      //一方向のみ
    return( DIR_NOT );
  }
  
  {
    int dir;
    dir = TrJikiDashSearch( mmdl );
    
    if( dir == DIR_NOT ){
      return( dir );
    }
    
    {
      int i = 0;
      
      do{ if(tbl[i]==dir){return(dir);} i++; }while( i < num );
      
      {
        MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
        MMDL *jiki = MMDLSYS_SearchMMdlPlayer( mmdlsys );
        int dirx = DIR_NOT,dirz = DIR_NOT;
        int x = MMDL_GetGridPosX( mmdl );
        int z = MMDL_GetGridPosZ( mmdl );
        int jx = MMDL_GetGridPosX( jiki );
        int jz = MMDL_GetGridPosZ( jiki );
        
        if( x > jx ){ dirx = DIR_LEFT; }
        else if( x < jx ){ dirx = DIR_RIGHT; }
        
        if( z > jz ){ dirz = DIR_UP; }
        else if( z < jz ){ dirz = DIR_DOWN; }
        
        i = 0;
        
        if( dirx == DIR_NOT ){
          do{ if(tbl[i]==dirz){return(dirz);} i++; }while( i < num );
        }else if( dirz == DIR_NOT ){ 
          do{ if(tbl[i]==dirx){return(dirx);} i++; }while( i < num );
        }else{
          do{ if(tbl[i]==dirx){return(dirx);}
            if(tbl[i]==dirz){return(dirz);} i++; }while( i < num );
        }
      }  
    }
  }
  
  return( DIR_NOT );
}

//--------------------------------------------------------------
/**
 * EV_TYPE_TRAINER_DASH_REACT判定処理
 * @param mmdl MMDL*
 * @param count カウント格納先
 * @retval BOOL TRUE=移動可能 FALSE=不可
 */
//--------------------------------------------------------------
static BOOL checkEvTypeDashReact( const MMDL *mmdl, u8 *count )
{
  if( MMDL_GetEventType(mmdl) != EV_TYPE_TRAINER_DASH_REACT ){
    return( TRUE );
  }
  
  if( (*count) == 0 ){
    if( MMDL_CheckPlayerDispSizeRect(mmdl) == FALSE ){
      return( FALSE );
    }
      
    if( checkDashButton() == FALSE ){
      return( FALSE );
    }
      
    *count = MMDL_GetParam( mmdl, MMDL_PARAM_1 );
    GF_ASSERT( (*count) && "EV_TYPE_TRAINER_DASH_REACT NONE PARAM\n" );
    
  }
  
  if( (*count) ){
    (*count)--;
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * ダッシュに反応して移動速度を上げるイベントタイプかチェック
 * @param mmdl MMDL*
 * @retval BOOL TRUE=そう。
 */
//--------------------------------------------------------------
static BOOL checkMMdlEventTypeDashAccel( const MMDL *mmdl )
{
  u16 ev_type = MMDL_GetEventType( mmdl );
  
  if( ev_type == EV_TYPE_TR_D_ACC ||
      ev_type == EV_TYPE_TR_D_SPINS_L ||
      ev_type == EV_TYPE_TR_D_SPINS_R ||
      ev_type == EV_TYPE_TR_D_SPINM_L ||
      ev_type == EV_TYPE_TR_D_SPINM_R ){
    return( TRUE );
  }
  return( FALSE );
}

//======================================================================
//  ルート型　クルクル移動パーツ
//======================================================================
//--------------------------------------------------------------
///  クルクルテーブル
//--------------------------------------------------------------
static const int DATA_KuruKuruTbl[RT_KURU2_MAX][RT_KURU2_DIR_MAX] =
{
  {DIR_UP,DIR_LEFT,DIR_DOWN,DIR_RIGHT},
  {DIR_UP,DIR_RIGHT,DIR_DOWN,DIR_LEFT},
};

//--------------------------------------------------------------
/**
 * クルクルタイプチェック
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=クルクル回れ
 */
//--------------------------------------------------------------
static int MoveSub_KuruKuruCheck( MMDL * mmdl )
{
  int type = MMDL_GetEventType( mmdl );
  
  if( type == EV_TYPE_TRAINER_SPIN_MOVE_L  ||
      type == EV_TYPE_TRAINER_SPIN_MOVE_R  ||
      type == EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_L ||
      type == EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_R ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * くるくるタイプ初期化
 * @param  mmdl  MMDL *
 * @param  work  RT_KURUKURU_WORK *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MoveSub_KuruKuruInit( MMDL * mmdl, RT_KURUKURU_WORK *work )
{
  u16 ev_type = MMDL_GetEventType( mmdl );
  
  if( ev_type == EV_TYPE_TRAINER_SPIN_MOVE_L ||
      ev_type == EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_L ){
    work->spin_type = RT_KURU2_L;
  }else{
    work->spin_type = RT_KURU2_R;
  }
}

//--------------------------------------------------------------
/**
 * くるくるタイプ方向セット。
 * 注意：方向が固定される
 * @param  mmdl  MMDL *
 * @param  work  RT_KURUKURU_WORK *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MoveSub_KuruKuruSet( MMDL * mmdl, RT_KURUKURU_WORK *work )
{
  int i,dir = MMDL_GetDirDisp( mmdl );
  
#if 0  
  {
    switch( dir ){
    case DIR_UP: KAGAYA_Printf( "くるくる移動 0 方向は上　" ); break;
    case DIR_DOWN: KAGAYA_Printf( "くるくる移動 0 方向は下　" ); break;
    case DIR_LEFT: KAGAYA_Printf( "くるくる移動 0 方向は左　" ); break;
    case DIR_RIGHT: KAGAYA_Printf( "くるくる移動 0 方向は右　" ); break;
    }
  }
#endif
  
  for( i = 0; (i < RT_KURU2_DIR_MAX && dir != DATA_KuruKuruTbl[work->spin_type][i]); i++ ){}
  GF_ASSERT( i < RT_KURU2_DIR_MAX && "MoveSub_KuruKuruInit()方向異常" );
  
  work->origin_dir = dir;
  i = (i + 1) % RT_KURU2_DIR_MAX;                //+1=1つ先
  dir = DATA_KuruKuruTbl[work->spin_type][i];
  
  if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_PAUSE_DIR) ){
    work->dir_pause = TRUE;
  }else{
    work->dir_pause = FALSE;
  }
  
  MMDL_SetDirDisp( mmdl, dir );
  MMDL_OnStatusBit( mmdl, MMDL_STABIT_PAUSE_DIR );

#if 0
  {
    switch( dir ){
    case DIR_UP: KAGAYA_Printf( "1方向は上\n" ); break;
    case DIR_DOWN: KAGAYA_Printf( "1方向は下\n" ); break;
    case DIR_LEFT: KAGAYA_Printf( "1方向は左\n" ); break;
    case DIR_RIGHT: KAGAYA_Printf(  " 1方向は右\n" ); break;
    }
  }
#endif
}

//--------------------------------------------------------------
/**
 * くるくるタイプ方向終了
 * @param  mmdl  MMDL *
 * @param  work  RT_KURUKURU_WORK *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MoveSub_KuruKuruEnd( MMDL * mmdl, RT_KURUKURU_WORK *work )
{
  if( work->dir_pause == FALSE ){
    MMDL_OffStatusBit( mmdl, MMDL_STABIT_PAUSE_DIR );
  }
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
///  DIR_RND ウェイトテーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndWaitTbl[] =
{ 16, 32, 48, 64, WAIT_END };

//--------------------------------------------------------------
///  DIR_RND 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl[] =
{ DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UL 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UL[] =
{ DIR_UP, DIR_LEFT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UR[] =
{ DIR_UP, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_DL 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_DL[] =
{ DIR_DOWN, DIR_LEFT, DIR_NOT };

//--------------------------------------------------------------
/// RND_DR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_DR[] =
{ DIR_DOWN, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UDL 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UDL[] =
{ DIR_UP, DIR_DOWN, DIR_LEFT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UDR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UDR[] =
{ DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_ULR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_ULR[] =
{ DIR_UP, DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_DLR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_DLR[] =
{ DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UD 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UD[] =
{ DIR_UP, DIR_DOWN, DIR_NOT };

//--------------------------------------------------------------
/// RND_LR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_LR[] =
{ DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
///  MV_RND 移動可能方向
//--------------------------------------------------------------
static const int DATA_MV_RND_DirTbl[] =
{ DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
///  MV_RND_V 移動可能方向
//--------------------------------------------------------------
static const int DATA_MV_RND_V_DirTbl[] =
{ DIR_UP, DIR_DOWN, DIR_NOT };

//--------------------------------------------------------------
///  MV_RND_H 移動可能方向
//--------------------------------------------------------------
static const int DATA_MV_RND_H_DirTbl[] =
{ DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
///  MV_RTURLD方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtURLD_DirTbl[] = {DIR_UP,DIR_RIGHT,DIR_LEFT,DIR_DOWN};

//--------------------------------------------------------------
///  MV_RTRLDU方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtRLDU_DirTbl[] = {DIR_RIGHT,DIR_LEFT,DIR_DOWN,DIR_UP};

//--------------------------------------------------------------
///  MV_RTDURL方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtDURL_DirTbl[] = {DIR_DOWN,DIR_UP,DIR_RIGHT,DIR_LEFT};

//--------------------------------------------------------------
///  MV_RTLDUR方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtLDUR_DirTbl[] = {DIR_LEFT,DIR_DOWN,DIR_UP,DIR_RIGHT};

//--------------------------------------------------------------
///  MV_RTULRD方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtULRD_DirTbl[] = {DIR_LEFT,DIR_RIGHT,DIR_DOWN,DIR_UP};

//--------------------------------------------------------------
///  MV_RTLRDU方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtLRDU_DirTbl[] = {DIR_LEFT,DIR_RIGHT,DIR_DOWN,DIR_UP};

//--------------------------------------------------------------
///  MV_RTDULR方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtDULR_DirTbl[] = {DIR_DOWN,DIR_UP,DIR_LEFT,DIR_RIGHT};

//--------------------------------------------------------------
///  MV_RTRDUL方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtRDUL_DirTbl[] = {DIR_RIGHT,DIR_DOWN,DIR_UP,DIR_LEFT};

//--------------------------------------------------------------
///  MV_RTLUDR方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtLUDR_DirTbl[] = {DIR_LEFT,DIR_UP,DIR_DOWN,DIR_RIGHT};

//--------------------------------------------------------------
///  MV_RTUDRL方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtUDRL_DirTbl[] = {DIR_UP,DIR_DOWN,DIR_RIGHT,DIR_LEFT};

//--------------------------------------------------------------
///  MV_RTRLUD方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtRLUD_DirTbl[] = {DIR_RIGHT,DIR_LEFT,DIR_UP,DIR_DOWN};

//--------------------------------------------------------------
///  MV_RTDRLU方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtDRLU_DirTbl[] = {DIR_DOWN,DIR_RIGHT,DIR_LEFT,DIR_UP};

//--------------------------------------------------------------
///  MV_RTRUDL方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtRUDL_DirTbl[] = {DIR_RIGHT,DIR_UP,DIR_DOWN,DIR_LEFT};

//--------------------------------------------------------------
///  MV_RTUDLR方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtUDLR_DirTbl[] = {DIR_UP,DIR_DOWN,DIR_LEFT,DIR_RIGHT};

//--------------------------------------------------------------
///  MV_RTLRUD方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtLRUD_DirTbl[] = {DIR_LEFT,DIR_RIGHT,DIR_UP,DIR_DOWN};

//--------------------------------------------------------------
///  MV_RTDLRU方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtDLRU_DirTbl[] = {DIR_DOWN,DIR_LEFT,DIR_RIGHT,DIR_UP};

//--------------------------------------------------------------
///  MV_RTUL方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtUL_DirTbl[] = {DIR_UP,DIR_LEFT,DIR_DOWN,DIR_RIGHT};

//--------------------------------------------------------------
///  MV_RTDR方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtDR_DirTbl[] = {DIR_DOWN,DIR_RIGHT,DIR_UP,DIR_LEFT};

//--------------------------------------------------------------
///  MV_RTLD方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtLD_DirTbl[] = {DIR_LEFT,DIR_DOWN,DIR_RIGHT,DIR_UP};

//--------------------------------------------------------------
///  MV_RTRU方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtRU_DirTbl[] = {DIR_RIGHT,DIR_UP,DIR_LEFT,DIR_DOWN};

//--------------------------------------------------------------
///  MV_RTUR方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtUR_DirTbl[] = {DIR_UP,DIR_RIGHT,DIR_DOWN,DIR_LEFT};

//--------------------------------------------------------------
///  MV_RTDL方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtDL_DirTbl[] = {DIR_DOWN,DIR_LEFT,DIR_UP,DIR_RIGHT};

//--------------------------------------------------------------
///  MV_RTLU方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtLU_DirTbl[] = {DIR_LEFT,DIR_UP,DIR_RIGHT,DIR_DOWN};

//--------------------------------------------------------------
///  MV_RTRD方向テーブル
//--------------------------------------------------------------
static const int DATA_MvRtRD_DirTbl[] = {DIR_RIGHT,DIR_DOWN,DIR_LEFT,DIR_UP};

//--------------------------------------------------------------
///  MV_SPIN系方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirSpin_DirTbl[] = {DIR_UP,DIR_DOWN,DIR_LEFT,DIR_RIGHT,DIR_NOT};

//--------------------------------------------------------------
///  テーブルデータ
//--------------------------------------------------------------
static const DIR_TBL DATA_MoveDirTbl[DIRID_MAX] =
{
  {DIRID_MvDirRndDirTbl,DATA_MvDirRndDirTbl},
  {DIRID_MvDirRndDirTbl_UL,DATA_MvDirRndDirTbl_UL},
  {DIRID_MvDirRndDirTbl_UR,DATA_MvDirRndDirTbl_UR},
  {DIRID_MvDirRndDirTbl_DL,DATA_MvDirRndDirTbl_DL},
  {DIRID_MvDirRndDirTbl_DR,DATA_MvDirRndDirTbl_DR},
  {DIRID_MvDirRndDirTbl_UDL,DATA_MvDirRndDirTbl_UDL},
  {DIRID_MvDirRndDirTbl_UDR,DATA_MvDirRndDirTbl_UDR},
  {DIRID_MvDirRndDirTbl_ULR,DATA_MvDirRndDirTbl_ULR},
  {DIRID_MvDirRndDirTbl_DLR,DATA_MvDirRndDirTbl_DLR},
  {DIRID_MvDirRndDirTbl_UD,DATA_MvDirRndDirTbl_UD},
  {DIRID_MvDirRndDirTbl_LR,DATA_MvDirRndDirTbl_LR},
  {DIRID_MV_RND_DirTbl,DATA_MV_RND_DirTbl},
  {DIRID_MV_RND_V_DirTbl,DATA_MV_RND_V_DirTbl},
  {DIRID_MV_RND_H_DirTbl,DATA_MV_RND_H_DirTbl},
  {DIRID_MvRtURLD_DirTbl,DATA_MvRtURLD_DirTbl},
  {DIRID_MvRtRLDU_DirTbl,DATA_MvRtRLDU_DirTbl},
  {DIRID_MvRtDURL_DirTbl,DATA_MvRtDURL_DirTbl},
  {DIRID_MvRtLDUR_DirTbl,DATA_MvRtLDUR_DirTbl},
  {DIRID_MvRtULRD_DirTbl,DATA_MvRtULRD_DirTbl},
  {DIRID_MvRtLRDU_DirTbl,DATA_MvRtLRDU_DirTbl},
  {DIRID_MvRtDULR_DirTbl,DATA_MvRtDULR_DirTbl},
  {DIRID_MvRtRDUL_DirTbl,DATA_MvRtRDUL_DirTbl},
  {DIRID_MvRtLUDR_DirTbl,DATA_MvRtLUDR_DirTbl},
  {DIRID_MvRtUDRL_DirTbl,DATA_MvRtUDRL_DirTbl},
  {DIRID_MvRtRLUD_DirTbl,DATA_MvRtRLUD_DirTbl},
  {DIRID_MvRtDRLU_DirTbl,DATA_MvRtDRLU_DirTbl},
  {DIRID_MvRtRUDL_DirTbl,DATA_MvRtRUDL_DirTbl},
  {DIRID_MvRtUDLR_DirTbl,DATA_MvRtUDLR_DirTbl},
  {DIRID_MvRtLRUD_DirTbl,DATA_MvRtLRUD_DirTbl},
  {DIRID_MvRtDLRU_DirTbl,DATA_MvRtDLRU_DirTbl},
  {DIRID_MvRtUL_DirTbl,DATA_MvRtUL_DirTbl},
  {DIRID_MvRtDR_DirTbl,DATA_MvRtDR_DirTbl},
  {DIRID_MvRtLD_DirTbl,DATA_MvRtLD_DirTbl},
  {DIRID_MvRtRU_DirTbl,DATA_MvRtRU_DirTbl},
  {DIRID_MvRtUR_DirTbl,DATA_MvRtUR_DirTbl},
  {DIRID_MvRtDL_DirTbl,DATA_MvRtDL_DirTbl},
  {DIRID_MvRtLU_DirTbl,DATA_MvRtLU_DirTbl},
  {DIRID_MvRtRD_DirTbl,DATA_MvRtRD_DirTbl},
  {DIRID_MvDirSpin_DirTbl,DATA_MvDirSpin_DirTbl},
  {DIRID_END,NULL},
};

//--------------------------------------------------------------
///  自機ダッシュ感知動作コード
//--------------------------------------------------------------
static const int JikiDashSensorMoveCodeTbl[] =
{
  MV_DIR_RND,
  MV_RND_UL,MV_RND_UR,
  MV_RND_DL,MV_RND_DR,MV_RND_UDL,MV_RND_UDR,MV_RND_ULR,MV_RND_DLR,
  MV_RND_UD,MV_RND_LR,
  MV_SPIN_L,MV_SPIN_R,
  MV_CODE_NOT
};
