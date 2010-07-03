//======================================================================
/**
 * @file  fldmmdl_blact.c
 * @brief  動作モデル ビルボードアクター管理
 * @author  kagaya
 * @date  05.07.13
 */
//======================================================================
#include "fldmmdl.h"

#include "field/fldmmdl_list_symbol.h"

//======================================================================
//  debug
//======================================================================
//DEBUG リソースメモリ使用量の検査
//フィールドマップメモリ使用量調査のため作成
//テクスチャリソースの確保、破棄時に、メモリ確保サイズを計算する。
//091117 tomoya takahashi
//
//kagayaさんへ　ソースを編集しにくいなどあったら、いってください。
#ifdef DEBUG_MMDL_RESOURCE_MEMORY_SIZE

static u32 DEBUG_MMDL_RESOURCE_MemorySize = 0;  // リソース
// リソースメモリーサイズ追加
#define DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Plus( x )    DEBUG_MMDL_RESOURCE_MemorySize += GFL_HEAP_DEBUG_GetMemoryBlockSize((x))
// リソースメモリーサイズ削除
#define DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus( x )    DEBUG_MMDL_RESOURCE_MemorySize -= GFL_HEAP_DEBUG_GetMemoryBlockSize((x))
// リソースメモリーサイズ クリーンチェック
#define DEBUG_MMDL_RESOURCE_MEMORY_SIZE_IsAllDelete    GF_ASSERT( DEBUG_MMDL_RESOURCE_MemorySize == 0 )

#else   // DEBUG_MMDL_RESOURCE_MEMORY_SIZE

// リソースメモリーサイズ追加
#define DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Plus( x )   /**/ 
// リソースメモリーサイズ削除
#define DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus( x )    /**/
// リソースメモリーサイズ クリーンチェック
#define DEBUG_MMDL_RESOURCE_MEMORY_SIZE_IsAllDelete   /**/ 

#endif  // DEBUG_MMDL_RESOURCE_MEMORY_SIZE

//======================================================================
//  define
//======================================================================
#define REGIDCODE_MAX (0xffff) ///<BBDRESID最大
#define BLACT_RESID_NULL (0xffff) ///<ビルボードリソース 無効ID
//#define RES_DIGEST_FRAME_MAX (4) ///<1フレームに消化できるリソース数
#define RES_DIGEST_FRAME_MAX (1) ///<1フレームに消化できるリソース数
#define DMYACT_MAX (4) ///<ダミーアクター総数

typedef enum
{
  BBDRESBIT_GUEST = (1<<0), ///<リソース識別 ゲスト登録 OFF時=レギュラー
  BBDRESBIT_TRANS = (1<<1), ///<リソース識別 ON=VRAM転送用リソース
  
  ///リソース　VRAM常駐　レギュラー
  BBDRES_VRAM_REGULAR =(0),
  ///リソース　VRAM転送　レギュラー
  BBDRES_TRANS_REGULAR =(BBDRESBIT_TRANS),
  ///リソース　VRAM常駐　ゲスト
  BBDRES_VRAM_GUEST=(BBDRESBIT_GUEST),
  ///リソース　VRAM転送　ゲスト
  BBDRES_TRANS_GUEST=(BBDRESBIT_TRANS|BBDRESBIT_GUEST),
}BBDRESBIT;

///ビルボード描画スケール。0x1000=x1:0x2000=x2
#define MMDL_BBD_GLOBAL_SCALE ((FX32_ONE+0xc00)*4)

///ビルボード描画スケール等倍 0x2000=x2
#define MMDL_BBD_GLOBAL_SCALE_ONE (FX32_ONE*4)

///ビルボード描画サイズ 0x8000=1 0x4000=1/2
//#define MMDL_BBD_DRAW_SIZE (FX16_ONE*8-1)
#define MMDL_BBD_DRAW_SIZE (FX16_ONE*4)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///  IDCODE
//--------------------------------------------------------------
typedef struct
{
  u16 code; ///<登録コード
  u16 id; ///<登録コードから返すID
  u16 flag; ///<登録内容を表すフラグ
  u16 dmy;
}IDCODE;

//--------------------------------------------------------------
///  IDCODEIDX
//--------------------------------------------------------------
typedef struct
{
  int max;
  IDCODE *pIDCodeBuf;
}IDCODEIDX;

//--------------------------------------------------------------
/// ADDRES_RESERVE_PARAM
//--------------------------------------------------------------
typedef struct
{
  u16 code;
  u16 res_idx;
  u8 mdl_size;
  u8 tex_size;
  BBDRESBIT flag;
}ADDRES_RESERVE_PARAM;

//--------------------------------------------------------------
/// ADDRES_RESERVE
//--------------------------------------------------------------
typedef struct
{
  u8 compFlag; //データの設定が完了したフラグ
  BBDRESBIT flag; //登録用フラグ
  
  u16 code; //登録用コード
  u8 mdl_size; //モデルサイズ
  u8 tex_size; //テクスチャサイズ
  
  MMDL *mmdl; //関連の動作モデル 無い場合はNULL
  GFL_G3D_RES *pG3dRes; //登録リソース
}ADDRES_RESERVE;

//--------------------------------------------------------------
/// ADDACT_RESERVE
//--------------------------------------------------------------
typedef struct
{
  u16 *outID;
  MMDL *mmdl;
  
  u16 code; //OBJコード
  u8 mdl_size;
  u8 tex_size;
  
  u8 anm_id; //アニメID
  u8 compFlag;    //データの設定が完了したフラグ
  u8 padding[2];
  
  GFL_G3D_RES *pTransActRes; ///<転送用アクターリソース
  
  ///ユーザー側が指定するアクター追加完了時に呼び出す関数
  MMDL_BLACTCONT_ADDACT_USERPROC user_init_proc;
  ///ユーザー側が指定するuser_init_procで設定するワーク
  void *user_init_work;
  ///ユーザー側が指定するアクター追加時の座標
  VecFx32 user_add_pos;
}ADDACT_RESERVE;

//--------------------------------------------------------------
/// DELRES_RESERVE
//--------------------------------------------------------------
typedef struct
{
  u16 compFlag; //データ設置完了ふらぐ
  u16 res_idx;  //削除するリソースインデックス
}DELRES_RESERVE;

//--------------------------------------------------------------
/// DMYACT_RESCHG
//--------------------------------------------------------------
typedef struct
{
  MMDL *mmdl;
  u16 dmy_act_id;
  u16 dmy_obj_code;
  u16 next_code;
  u8 padding[2];
}DMYACT_RESCHG;

//--------------------------------------------------------------
/// BLACT_RESERVE
//--------------------------------------------------------------
typedef struct
{
  BOOL funcFlag; //TRUE=予約処理を実行する
  u16 resMax;
  u16 resDigestFrameMax;
  u16 actMax;
  u16 dmy;
  ADDRES_RESERVE_PARAM *pReserveResParam;
  ADDRES_RESERVE *pReserveRes;
  ADDACT_RESERVE *pReserveAct;
  DELRES_RESERVE *pReserveDelRes;
}BLACT_RESERVE;

//--------------------------------------------------------------
///  MMDL_BLACTCONT
//--------------------------------------------------------------
struct _TAG_MMDL_BLACTCONT
{
  u8 bbdUpdateFlag;
  u8 padding;
  u16 resourceMax;
  
  GFL_BBDACT_SYS *pBbdActSys; //ユーザーから
  GFL_BBDACT_RESUNIT_ID bbdActResUnitID;
  GFL_BBDACT_ACTUNIT_ID bbdActActUnitID;
  u16 bbdActResCount;
  u16 bbdActActCount;
  
  MMDLSYS *mmdlsys;
  
  IDCODEIDX BBDResUnitIdx;
  BLACT_RESERVE *pReserve;
  DMYACT_RESCHG *pDmyActTbl;
};

//======================================================================
//  proto
//======================================================================
static void BlActFunc( GFL_BBDACT_SYS *bbdActSys, int actIdx, void *work );

static void IDCodeIndex_Init( IDCODEIDX *idx, int max, HEAPID heapID );
static void IDCodeIndex_RegistCode(
    IDCODEIDX *idx, u16 code, u16 id, u16 flag );
static void IDCodeIndex_DeleteCode( IDCODEIDX *idx, u16 code );
static BOOL IDCodeIndex_SearchCode(
    IDCODEIDX *idx, u16 code, u16 *outID, u16 *outFlag );
static void BBDResUnitIndex_Init( MMDL_BLACTCONT *pBlActCont, int max );
static void BBDResUnitIndex_Delete( MMDL_BLACTCONT *pBlActCont );
static void BBDResUnitIndex_AddResource(
    MMDL_BLACTCONT *pBlActCont, GFL_G3D_RES *g3dres,
    u16 obj_code, u8 mdl_size, u8 tex_size, BBDRESBIT flag );
static void BBDResUnitIndex_AddResUnit(
    MMDL_BLACTCONT *pBlActCont, const OBJCODE_PARAM *pParam, BBDRESBIT flag );
static void BBDResUnitIndex_RemoveResUnit(
    MMDL_BLACTCONT *pBlActCont, u16 obj_code );
static void BBDResUnitIndex_RegistRemoveResUnit(
    MMDL_BLACTCONT *pBlActCont, u16 act_id, u16 obj_code );
static BOOL BBDResUnitIndex_SearchResID(
  MMDL_BLACTCONT *pBlActCont, u16 obj_code, u16 *outID, u16 *outFlag );

static void BlActAddReserve_Init( MMDL_BLACTCONT *pBlActCont );
static void BlActAddReserve_Delete( MMDL_BLACTCONT *pBlActCont );
static u32 BlActAddReserve_CheckReserve( const MMDL_BLACTCONT *pBlActCont );
static void BlActAddReserve_RegistResourceParam(MMDL_BLACTCONT *pBlActCont,
    u16 code, u8 mdl_size, u8 tex_size, u16 res_idx, BBDRESBIT flag );
static void BlActAddReserve_DigestResourceParam(
    MMDL_BLACTCONT *pBlActCont );
static BOOL BlActAddReserve_RegistResource( MMDL_BLACTCONT *pBlActCont,
    u16 code, u8 mdl_size, u8 tex_size, u16 res_idx, BBDRESBIT flag );
static void BlActAddReserve_DigestResource( MMDL_BLACTCONT *pBlActCont );
static BOOL BlActAddReserve_SearchResource(
    MMDL_BLACTCONT *pBlActCont, u16 code, u16 *outFlag );
static BOOL BlActAddReserve_CancelResource(
    MMDL_BLACTCONT *pBlActCont, u16 code );
static void BlActAddReserve_RegistActor(
    MMDL_BLACTCONT *pBlActCont,
    MMDL *mmdl,
    const OBJCODE_PARAM *pParam,
    u16 *outID,
    MMDL_BLACTCONT_ADDACT_USERPROC init_proc,
    void *init_work, const VecFx32 *user_pos );
static void BlActAddReserve_DigestActorCore(
    MMDL_BLACTCONT *pBlActCont, ADDACT_RESERVE *pRes );
static void BlActAddReserve_DigestActor( MMDL_BLACTCONT *pBlActCont );
static BOOL BlActAddReserve_SearchActorOBJCode(
    MMDL_BLACTCONT *pBlActCont, u16 code, MMDL *mmdl );
static BOOL BlActAddReserve_SearchActorOBJCodeOutID(
    MMDL_BLACTCONT *pBlActCont, u16 code, const u16 *outID );
static void BlActAddReserve_CancelActor(
    MMDL_BLACTCONT *pBlActCont, MMDL *mmdl );
static void BlActAddReserve_ClearWork( ADDACT_RESERVE *pRes );
static BOOL BlActDelReserve_RegistResource(
    MMDL_BLACTCONT *pBlActCont, u16 res_idx );

static void DummyAct_Init( MMDL_BLACTCONT *pBlActCont );
static void DummyAct_Delete( MMDL_BLACTCONT *pBlActCont );
static void DummyAct_Update( MMDL_BLACTCONT *pBlActCont );
static BOOL DummyAct_CheckMMdl(
    MMDL_BLACTCONT *pBlActCont, const MMDL *mmdl, u16 actID );

static const MMDL_BBDACT_ANMTBL * BlActAnm_GetAnmTbl( u32 no );
static GFL_BBDACT_RESUNIT_ID BlActRes_AddResCore(
    MMDL_BLACTCONT *pBlActCont,
    u16 code, u8 mdl_size, u8 tex_size,
    GFL_G3D_RES *g3dres, GFL_BBDACT_RESTYPE type );
#if 0
static GFL_BBDACT_RESUNIT_ID BlActRes_AddRes(
    MMDL_BLACTCONT *pBlActCont, u16 code,
    GFL_G3D_RES *g3dres, GFL_BBDACT_RESTYPE type );
static GFL_BBDACT_RESUNIT_ID BlActRes_AddResMMdl(
    MMDL_BLACTCONT *pBlActCont, const MMDL *mmdl, 
    GFL_G3D_RES *g3dres, GFL_BBDACT_RESTYPE type );
#endif

static const u16 data_BBDTexSizeTbl[TEXSIZE_MAX];

//test data
/*
const GFL_BBDACT_RESDATA testResTable[];
const u32 testResTableMax;
*/

//======================================================================
//  フィールド動作モデル　ビルボードアクター管理
//======================================================================
//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT ビルボードアクター管理　セットアップ
 * @param  mmdlsys  構築済みのMMDLSYS
 * @param  pBbdActSys  構築済みのGFL_BBDACT_SYS
 * @param res_max リソース数最大
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_Setup( MMDLSYS *mmdlsys,
  GFL_BBDACT_SYS *pBbdActSys, int res_max )
{
  HEAPID heapID;
  GFL_BBDACT_ACTUNIT_ID actUnitID;
  MMDL_BLACTCONT *pBlActCont;
  
  heapID = MMDLSYS_GetHeapID( mmdlsys );
  pBlActCont = GFL_HEAP_AllocClearMemory( heapID, sizeof(MMDL_BLACTCONT) );
  pBlActCont->mmdlsys = mmdlsys;
  pBlActCont->pBbdActSys = pBbdActSys;
  pBlActCont->resourceMax = res_max;
  
  MMDLSYS_SetBlActCont( mmdlsys, pBlActCont );
  
  BBDResUnitIndex_Init( pBlActCont, res_max );
  BlActAddReserve_Init( pBlActCont );
  DummyAct_Init( pBlActCont );

  { //Parameter
    GFL_BBD_SYS *bbdSys = GFL_BBDACT_GetBBDSystem( pBbdActSys );

    { //Scale
      VecFx32 scale =
      {MMDL_BBD_GLOBAL_SCALE,MMDL_BBD_GLOBAL_SCALE,MMDL_BBD_GLOBAL_SCALE};
      GFL_BBD_SetScale( bbdSys, &scale );
    }
    
    { //Poligon ID
      u8 id = 0;
      GFL_BBD_SetPolID( bbdSys, &id );
    }
  }
}

//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT ビルボードアクター管理　削除
 * @param  mmdlsys  MMDLSYS
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_Release( MMDLSYS *mmdlsys )
{
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  DummyAct_Delete( pBlActCont );
  BlActAddReserve_Delete( pBlActCont );
  BBDResUnitIndex_Delete( pBlActCont );
  
  GFL_HEAP_FreeMemory( pBlActCont );
  MMDLSYS_SetBlActCont( mmdlsys, NULL );
}

//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT 更新処理
 * @param mmdlsys MMDLSYS
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_Update( MMDLSYS *mmdlsys )
{
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  if( pBlActCont != NULL ){
    if( pBlActCont->pReserve != NULL ){ //予約消化
      //アクター予約消化
      BlActAddReserve_DigestActor( pBlActCont );
      //リソースパラメタ予約消化
      BlActAddReserve_DigestResourceParam( pBlActCont );
    }
    
    DummyAct_Update( pBlActCont ); //ダミーアクター更新処理
    
    pBlActCont->bbdUpdateFlag = 0;
  }
}

//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT Vブランク処理
 * @param  mmdlsys  MMDLSYS
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_ProcVBlank( MMDLSYS *mmdlsys )
{
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  BlActAddReserve_DigestResource( pBlActCont );
}

#if 0
//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT GFL_BBDACT_SYS取得
 * @param mmdlsys MMDLSYS*
 * @retval GFL_BBDACT_SYS*
 */
//--------------------------------------------------------------
GFL_BBDACT_SYS * MMDL_BLACTCONT_GetBlActSys( MMDLSYS *mmdlsys )
{
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  GF_ASSERT( pBlActCont != NULL );
  return( pBlActCont->pBbdActSys );
}
#endif

//--------------------------------------------------------------
/**
 * MMDLSYS ビルボードアクタースケールを等倍に
 * @param mmdlsys MMDLSYS
 * @retval nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_SetGlobalScaleOne( MMDLSYS *mmdlsys )
{
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  VecFx32 scale = {MMDL_BBD_GLOBAL_SCALE_ONE,
    MMDL_BBD_GLOBAL_SCALE_ONE,MMDL_BBD_GLOBAL_SCALE_ONE};
  GFL_BBD_SYS *bbdSys = GFL_BBDACT_GetBBDSystem( pBlActCont->pBbdActSys );
  GFL_BBD_SetScale( bbdSys, &scale );
}

//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT 予約リソース、アクターチェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL MMDL_BLACTCONT_IsThereReserve( const MMDLSYS *mmdlsys )
{
  const MMDL_BLACTCONT *pBlActCont =
    MMDLSYS_GetBlActCont( (MMDLSYS*)mmdlsys );

  if( BlActAddReserve_CheckReserve(pBlActCont) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT ビルボードアクターが動作しているかチェック
 * @param mmdl MMDL* 利便性からMMDL指定
 * @retval BOOL TRUE=動作している FALSE=していない
 */
//--------------------------------------------------------------
BOOL MMDL_BLACTCONT_CheckUpdateBBD( const MMDL *mmdl )
{
  const MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
  const MMDL_BLACTCONT *pBlActCont =
    MMDLSYS_GetBlActCont( (MMDLSYS*)mmdlsys );
  return( pBlActCont->bbdUpdateFlag );
}

//======================================================================
//  フィールド動作モデル　ビルボードリソース
//======================================================================
#if 0
//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT リソース追加 レギュラー、VRAM常駐型で登録
 * @param  mmdlsys  MMDLSYS
 * @param  code 表示コード配列
 * @param  max code要素数
 * @retval  nothing
 * @note 呼ばれたその場で読み込みが発生する。
 * @attention アーカイブデータを連続でロードする為、処理速度は重い。
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_AddResourceTex(
  MMDLSYS *mmdlsys, const u16 *code, int max )
{
  OBJCODE_PARAM param;
  const OBJCODE_PARAM_BUF_BBD *prm_bbd;
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  while( max ){
    MMDLSYS_LoadOBJCodeParam( mmdlsys, *code, &param );
    BBDResUnitIndex_AddResUnit( pBlActCont, &param, BBDRES_VRAM_REGULAR );
    code++;
    max--;
  }
}
#endif

//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT リソース追加 レギュラー、VRAM常駐型で登録
 * @param  mmdlsys  MMDLSYS
 * @param  code 表示コード配列
 * @param  max code要素数
 * @retval  nothing
 * @note 呼ばれたその場で読み込みが発生する。
 * @attention アーカイブデータを連続でロードする為、処理速度は重い。
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_AddResourceTexCode( MMDLSYS *mmdlsys, const u16 code )
{
  OBJCODE_PARAM param;
  const OBJCODE_PARAM_BUF_BBD *prm_bbd;
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  MMDLSYS_LoadOBJCodeParam( mmdlsys, code, &param );
  BBDResUnitIndex_AddResUnit( pBlActCont, &param, BBDRES_VRAM_REGULAR );
}

//--------------------------------------------------------------
/**
 * MMDL_BLACTCONT リソース追加 レギュラー、VRAM常駐型で登録
 * @param  mmdlsys  MMDLSYS
 * @param  code 表示コード配列
 * @param  max code要素数
 * @retval  nothing
 * @note 呼ばれたその場で読み込みが発生する。
 * @attention アーカイブデータを連続でロードする為、処理速度は重い。
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_AddResourceTex( MMDLSYS *mmdlsys )
{
  int i;
  OBJCODE_PARAM param;
  const OBJCODE_PARAM_BUF_BBD *prm_bbd;
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  int max = MMDLSYS_GetSysOBJCodeParamNum( mmdlsys );
  
  for(i=0;i<max;i++){
    MMDLSYS_GetSysOBJCodeParam( mmdlsys, i, &param );
    BBDResUnitIndex_AddResUnit( pBlActCont, &param, BBDRES_VRAM_REGULAR );
  }
}

//======================================================================
//  フィールド動作モデル ビルボードアクター追加
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター　アクター追加
 * @param mmdlsys MMDLSYS
 * @retval GFL_BBDACT_ACYUNIT_ID
 */
//--------------------------------------------------------------
static GFL_BBDACT_ACTUNIT_ID blact_AddActorCore(
  MMDLSYS *mmdlsys , const VecFx32 *pos,
  MMDL_BLACT_MDLSIZE mdl_size, MMDL_BLACT_ANMTBLNO anm_id, u16 resID )
{
  GFL_BBDACT_ACTDATA actData;
  GFL_BBDACT_ACTUNIT_ID actID;
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  actData.resID = resID;
  
  {
    const u16 *size = DATA_MMDL_BLACT_MdlSizeDrawSize[mdl_size];
    actData.sizX = size[0];
    actData.sizY = size[1];
  }
  
  actData.trans = *pos;
  actData.alpha = 31;
  actData.drawEnable = TRUE;
  actData.lightMask = GFL_BBD_LIGHTMASK_0;
  actData.work = pBlActCont;
  actData.func = BlActFunc;
  
  actID = GFL_BBDACT_AddAct(
    pBlActCont->pBbdActSys, pBlActCont->bbdActResUnitID, &actData, 1 );
   
  {
    const MMDL_BBDACT_ANMTBL *anmTbl = BlActAnm_GetAnmTbl( anm_id );
    
    if( anmTbl->pAnmTbl != NULL ){
      GFL_BBDACT_SetAnimeTable( pBlActCont->pBbdActSys,
        actID, (GFL_BBDACT_ANMTBL)anmTbl->pAnmTbl, anmTbl->anm_max );
      GFL_BBDACT_SetAnimeIdxOn( pBlActCont->pBbdActSys, actID, 0 );
    }
  }
  
  return( actID );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター アクター追加　動作モデル版
 * @param  mmdl  MMDL
 * @param  resID 追加するアクターで使用するリソースインデックス
 * @retval GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static GFL_BBDACT_ACTUNIT_ID blact_AddActor( MMDL *mmdl, u16 resID )
{
  VecFx32 pos;
  MMDLSYS *mmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  const OBJCODE_PARAM *pParam = MMDL_GetOBJCodeParam( mmdl );
  MMDL_GetDrawVectorPos( mmdl, &pos );
  
  return( blact_AddActorCore(
        mmdlsys,&pos,pParam->mdl_size,pParam->anm_id,resID) );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター　リソースからアクターセット
 * @param pos 表示座標
 * @param code 表示コード
 * @param resID アクターリソースID
 * @param transResID 転送用リソース
 * @retval GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static GFL_BBDACT_ACTUNIT_ID blact_SetResActorCore(
    MMDLSYS *mmdlsys, const VecFx32 *pos,
    MMDL_BLACT_MDLSIZE mdl_size, MMDL_BLACT_ANMTBLNO anm_id,
    u16 resID, u16 transResID )
{
  GFL_BBDACT_ACTUNIT_ID actID;
  
  actID = blact_AddActorCore( mmdlsys, pos, mdl_size, anm_id, resID );
  
  if( transResID != BLACT_RESID_NULL ){ //転送用リソースと結ぶ
    MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, transResID );
  }
  
  return( actID );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター　リソースからアクターセット　動作モデル版
 * @param mmdl MMDL
 * @param code 表示コード
 * @param resID アクターリソースID
 * @param transResID 転送用リソース
 * @retval GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static GFL_BBDACT_ACTUNIT_ID blact_SetResActor(
    MMDL *mmdl, u16 resID, u16 transResID )
{
  VecFx32 pos;
  MMDLSYS *mmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  const OBJCODE_PARAM *param = MMDL_GetOBJCodeParam( mmdl );
  MMDL_GetDrawVectorPos( mmdl, &pos );
  
  return( blact_SetResActorCore(
        mmdlsys,&pos,param->mdl_size,param->anm_id,resID,transResID) );
}

#if 0
//--------------------------------------------------------------
/**
 * ビルボードアクター　転送型アクターセット
 * @param mmdl MMDL
 * @param code 表示コード
 * @param transResID 転送用リソースID
 * @param pTransActRes 転送用ビルボードリソースID
 * @retval GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static GFL_BBDACT_ACTUNIT_ID blact_SetTransActor(
    MMDL *mmdl, u16 code, u16 transResID, GFL_G3D_RES pTransActRes )
{
  u16 resID;
  GFL_BBDACT_ACTUNIT_ID actID;
  MMDLSYS *mmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  {
    const OBJCODE_PARAM *prm;
    GFL_BBDACT_G3DRESDATA data;
    prm = MMDLSYS_GetOBJCodeParam( pBlActCont->mmdlsys, obj_code );
    
    data.g3dres = g3dres;
    data.texFmt = GFL_BBD_TEXFMT_PAL16;
    data.texSiz = prm->tex_size;
    data.celSizX = 32;        //いずれmdl_sizeから
    data.celSizY = 32;
  }

    actID = blact_AddActor( mmdl, code, resID );
  
  GFL_BBDACT_ACTUNIT_ID blact_SetResActor(
    MMDL *mmdl, u16 code, u16 resID, resID );
  u16 transResID )
  if( transResID != BLACT_RESID_NULL ){ //転送用リソースと結ぶ
    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, transResID );
  }
   
  return( actID );
}
#endif

//--------------------------------------------------------------
/**
 * ビルボードアクター　アクターセット
 * @param mmdl MMDL
 * @param code 表示コード
 * @retval nothing
 */
//--------------------------------------------------------------
#if 0
static GFL_BBDACT_ACTUNIT_ID blact_SetActor( MMDL *mmdl, u16 code )
{
  u16 resID,flag;
  GFL_BBDACT_ACTUNIT_ID actID;
  MMDLSYS *mmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
  
  if( resID == REGIDCODE_MAX ){ //非登録リソース
    MMDL_BLACTCONT_AddOBJCodeRes( mmdlsys, code,  FALSE, TRUE );
    BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
    GF_ASSERT( resID != REGIDCODE_MAX );
    
    #ifdef DEBUG_MMDL
    D_MMDL_DPrintf( "MMDL ADD GUEST RESOURCE %xH\n", code );
    #endif
  }
  
  if( (flag&BBDRESBIT_TRANS) ){ //転送型 転送先リソース作成
    //転送用リソースを取得し追加。
    //resID = アクター用のリソース
    GF_ASSERT( 0 ); //現状未対応
  }
  
  actID = blact_AddActor( mmdl, code, resID );
  
  if( (flag&BBDRESBIT_TRANS) ){ //転送型 リソースと結ぶ
    BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, resID );
  }
  
  return( actID );
}
#endif

static GFL_BBDACT_ACTUNIT_ID blact_SetActor( MMDL *mmdl, u16 code )
{
  u16 resID,flag;
  GFL_BBDACT_ACTUNIT_ID actID;
  MMDLSYS *mmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  BBDResUnitIndex_SearchResID( pBlActCont, code, &resID, &flag );
  GF_ASSERT( resID != REGIDCODE_MAX ); //リソース無し
  GF_ASSERT( !(flag&BBDRESBIT_TRANS) ); //転送タイプ
  
  actID = blact_AddActor( mmdl, resID );
  return( actID );
}

#if 0
//--------------------------------------------------------------
/**
 * ビルボードアクター 追加
 * @param  mmdl  MMDL
 * @param  code 使用するOBJコード
 * @param outID 追加アクターID格納先 MMDL_BBDACT_ACTID_NULL=追加中
 * @retval BOOL TRUE=追加。FALSE=追加中。
 */
//--------------------------------------------------------------
GFL_BBDACT_ACTUNIT_ID MMDL_BLACTCONT_AddActor( MMDL *mmdl, u32 code )
{
  u16 flag;
  VecFx32 pos;
  GFL_BBDACT_ACTDATA actData;
  GFL_BBDACT_ACTUNIT_ID actID;
  const MMDL_BBDACT_ANMTBL *anmTbl;
  const OBJCODE_PARAM *prm;
  MMDLSYS *mmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  prm = MMDLSYS_GetOBJCodeParam( mmdlsys, code );
  MMDL_GetDrawVectorPos( mmdl, &pos );
  
  BBDResUnitIndex_SearchResID( mmdlsys, code, &actData.resID, &flag );
  
  if( actData.resID == REGIDCODE_MAX ){ //非登録リソース
    MMDL_BLACTCONT_AddOBJCodeRes( mmdlsys, code,  FALSE, TRUE );
    BBDResUnitIndex_SearchResID( mmdlsys, code, &actData.resID, &flag );
    GF_ASSERT( actData.resID != REGIDCODE_MAX );
    
    #ifdef DEBUG_MMDL
    D_MMDL_DPrintf( "MMDL ADD GUEST RESOURCE %xH\n", code );
    #endif
  }
  
  actData.sizX = FX16_ONE*8-1;
  actData.sizY = FX16_ONE*8-1;
  actData.trans = pos;
  actData.alpha = 31;
  actData.drawEnable = TRUE;
  actData.lightMask = GFL_BBD_LIGHTMASK_0;
  actData.work = mmdl;
  actData.func = BlActFunc;
  
  actID = GFL_BBDACT_AddAct(
    pBlActCont->pBbdActSys, pBlActCont->bbdActResUnitID, &actData, 1 );
  
#if 0 //転送用リソースとバインド
  if( flag & == HERO ){
//    GFL_BBDACT_BindActTexRes( pBlActCont->pBbdActSys, actID, actData.resID );
  }
#endif

  anmTbl = BlActAnm_GetAnmTbl( prm->anm_id );
  
  if( anmTbl->pAnmTbl != NULL ){
    GFL_BBDACT_SetAnimeTable( pBlActCont->pBbdActSys,
      actID, (GFL_BBDACT_ANMTBL)anmTbl->pAnmTbl, anmTbl->anm_max );
  }
  
  GFL_BBDACT_SetAnimeIdxOn(
      pBlActCont->pBbdActSys, actID, 0 );
  return( actID );
}
#endif

//--------------------------------------------------------------
/**
 * ビルボードアクター 追加
 * @param  mmdl  MMDL
 * @param  code 使用するOBJコード
 * @param outID 追加アクターID格納先 MMDL_BLACTID_NULL=追加中
 * @retval BOOL TRUE=追加。FALSE=追加中。
 */
//--------------------------------------------------------------
BOOL MMDL_BLACTCONT_AddActor( MMDL *mmdl, GFL_BBDACT_ACTUNIT_ID *outID )
{
  u16 resID,flag;
  MMDLSYS *mmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  const OBJCODE_PARAM *pParam = MMDL_GetOBJCodeParam( mmdl );
  
  BBDResUnitIndex_SearchResID( pBlActCont, pParam->code, &resID, &flag );
  
  if( resID != REGIDCODE_MAX && (flag&BBDRESBIT_TRANS) == 0 ){
    (*outID) = blact_SetActor( mmdl, pParam->code );
    return( TRUE );
  }
  
  *outID = MMDL_BLACTID_NULL; //予約登録
  BlActAddReserve_RegistActor(
      pBlActCont, mmdl, pParam, outID, NULL, NULL, NULL );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター　リソース削除
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void blact_DeleteResource(
    MMDL_BLACTCONT *pBlActCont, MMDL *mmdl, u16 actID, u16 code )
{
  u16 id,flag,res_idx;
  
  if( BBDResUnitIndex_SearchResID(pBlActCont,code,&id,&flag) == TRUE )
  {
    if( (flag&BBDRESBIT_TRANS) ) //転送型 アクターの転送用リソース削除
    {
      u16 res_idx = GFL_BBDACT_GetResIdx( pBlActCont->pBbdActSys, actID );
      GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys, res_idx, 1 );
    }
    
    if( (flag&BBDRESBIT_GUEST) ) //ゲスト登録 他に利用無ければ削除
    {
      if( MMDL_SearchUseOBJCode(mmdl,code) == FALSE )
      {
        #if 1 //その場で削除する
        BBDResUnitIndex_RemoveResUnit( pBlActCont, code );
        #else //その場で削除せず。リソース削除を予約する
        BBDResUnitIndex_RegistRemoveResUnit( pBlActCont, actID, code );
        #endif
        D_MMDL_DPrintf(
          "MMDL DEL GUEST RESOURCE CODE=%d, RESID=%d\n", code, id );
        return;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * ビルボードアクター 削除
 * @param  mmdl  MMDL*
 * @param  actID GFL_BBDACT_ACTUNIT_ID
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_DeleteActor( MMDL *mmdl, u32 actID )
{
  u16 id,flag,res_idx;
  u16 code = MMDL_GetOBJCode( mmdl );
  MMDLSYS *pMMdlSys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( pMMdlSys );
  
  if( actID == MMDL_BLACTID_NULL ){ //まだ追加されていない。
    BlActAddReserve_CancelActor( pBlActCont, mmdl ); //予約あればキャンセル
    return;
  }
  
  if( DummyAct_CheckMMdl(pBlActCont,mmdl,actID) == TRUE ){
    return; //ダミー登録されている
  }
  
  blact_DeleteResource( pBlActCont, mmdl, actID, code );
  GFL_BBDACT_RemoveAct( pBlActCont->pBbdActSys, actID, 1 );
}

#if 0 //old
void MMDL_BLACTCONT_DeleteActor( MMDL *mmdl, u32 actID )
{
  u16 id,flag;
  u16 code = MMDL_GetOBJCode( mmdl );
  MMDLSYS *pMMdlSys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( pMMdlSys );
  
  if( actID == MMDL_BLACTID_NULL ){ //まだ追加されていない。
    BlActAddReserve_CancelActor( pBlActCont, mmdl ); //予約あればキャンセル
    return;
  }
  
  if( BBDResUnitIndex_SearchResID(pBlActCont,code,&id,&flag) == TRUE )
  {
    if( (flag&BBDRESBIT_TRANS) ) //転送型 アクターの転送用リソース削除
    {
      u16 idx = GFL_BBDACT_GetResIdx( pBlActCont->pBbdActSys, actID );
      GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys, idx, 1 );
    }
    
    if( (flag&BBDRESBIT_GUEST) ) //ゲスト登録 他に利用無ければ削除
    {
      if( MMDL_SearchUseOBJCode(mmdl,code) == FALSE )
      {
        BBDResUnitIndex_RemoveResUnit( pBlActCont, code );
        
        D_MMDL_DPrintf(
          "MMDL DEL GUEST RESOURCE CODE=%d, RESID=%d\n", code, id );
      }
    }
  }
  
  GFL_BBDACT_RemoveAct( pBlActCont->pBbdActSys, actID, 1 );
}
#endif

//======================================================================
//  フィールド動作モデル ビルボードアクター追加 ユーザー用
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター　アクター追加　ユーザー用
 * @param mmdlsys MMDLSYS*
 * @param code 使用するOBJコード
 * @param outID 追加アクターID格納先 MMDL_BLACTID_NULL=追加中
 * @param pos 表示座標
 * @param init_proc アクター追加完了時に呼び出す関数 NULL=呼び出し無し
 * @param init_work init_procに指定する引数ワーク
 * @retval BOOL TRUE=追加。FALSE=追加中。
 * @note 必要なリソースは予め追加されている事が前提。
 */
//--------------------------------------------------------------
BOOL MMDL_BLACTCONT_USER_AddActor( MMDLSYS *mmdlsys,
    const OBJCODE_PARAM *pParam,
    MMDL_BLACTWORK_USER *userAct, const VecFx32 *pos,
    MMDL_BLACTCONT_ADDACT_USERPROC init_proc, void *init_work )
{
  u16 resID,flag;
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  BBDResUnitIndex_SearchResID(
      pBlActCont, pParam->code, &resID, &userAct->flag );
  GF_ASSERT( resID != REGIDCODE_MAX ); //リソース無し
  
  if( (userAct->flag&BBDRESBIT_TRANS) == 0 ){
    userAct->actID = blact_AddActorCore(
        mmdlsys, pos, pParam->mdl_size, pParam->anm_id, resID );
    return( TRUE );
  }
  
  userAct->actID = MMDL_BLACTID_NULL; //リソース転送型
  
  BlActAddReserve_RegistActor( pBlActCont,
      NULL, pParam,
      &userAct->actID,
      init_proc, init_work, pos );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター　アクター削除　ユーザー用
 * @param mmdlsys MMDLSYS*
 * @param actID MMLD_BLACTCONT_USER_AddActor()指定したID格納ポインタ
 * @retval nothing
 * @note actIDは使用しているアクターIDが格納されている事。
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_USER_DeleteActor(
    MMDLSYS *mmdlsys, MMDL_BLACTWORK_USER *userAct )
{
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  if( userAct->actID == MMDL_BLACTID_NULL ){ //まだ追加されていない
//BlActAddReserve_CancelActor( pBlActCont, mmdl ); //予約あればキャンセル
  }else{
    if( (userAct->flag&BBDRESBIT_TRANS) ){ //転送型 転送用リソース削除
      u16 idx = GFL_BBDACT_GetResIdx(
         pBlActCont->pBbdActSys, userAct->actID );
      GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys, idx, 1 );
    }
    GFL_BBDACT_RemoveAct( pBlActCont->pBbdActSys, userAct->actID, 1 );
  }
  
  userAct->flag = 0;
  userAct->actID = MMDL_BLACTID_NULL;
}

//======================================================================
//  フィールド動作モデル　ビルボードアクター動作
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター動作関数
 * @param  bbdActSys  GFL_BBDACT_SYS
 * @param  actIdx    int
 * @param  work    void*
 * @retval  nothing
 */
//--------------------------------------------------------------
static void BlActFunc( GFL_BBDACT_SYS *bbdActSys, int actIdx, void *work )
{
  //ビルボードアクターが動作した事を示す
  ((MMDL_BLACTCONT*)work)->bbdUpdateFlag |= 1;
}

//======================================================================
//  フィールド動作モデル　ビルボードアクター管理　参照
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター管理　GFL_BBDACT_SYS取得
 * @param  pBlActCont MMDL_BLACTCONT
 * @retval  GFL_BBDACT_SYS
 */
//--------------------------------------------------------------
GFL_BBDACT_SYS * MMDL_BLACTCONT_GetBbdActSys(
    MMDL_BLACTCONT *pBlActCont )
{
  return( pBlActCont->pBbdActSys );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター管理 GFL_BBDACT_RESUNIT_ID取得
 * @param  pBlActCont MMDL_BLACTCONT
 * @retval  GFL_BBDACT_RESUNIT_ID
 */
//--------------------------------------------------------------
GFL_BBDACT_RESUNIT_ID MMDL_BLACTCONT_GetResUnitID(
    MMDL_BLACTCONT *pBlActCont )
{
  return( pBlActCont->bbdActResUnitID );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター　指定された動作モデルのアクター描画サイズを取得する
 * @param
 * @retval
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_GetMMdlDrawSize(
    const MMDL *mmdl, u16 *pSizeX, u16 *pSizeY )
{
  const OBJCODE_PARAM *prm = MMDL_GetOBJCodeParam( mmdl );
  const u16 *size = DATA_MMDL_BLACT_MdlSizeDrawSize[prm->mdl_size];
  *pSizeX = size[0];
  *pSizeY = size[1];
}

//======================================================================
//  IDCODEIDX
//======================================================================
//--------------------------------------------------------------
/**
 * IDCODEIDX 初期化
 * @param  idx IDCODEIDX*
 * @param  max 登録最大数
 * @param  heapID HEAPID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void IDCodeIndex_Init( IDCODEIDX *idx, int max, HEAPID heapID )
{
  IDCODE *tbl;
  GF_ASSERT( max );
  tbl = GFL_HEAP_AllocClearMemory( heapID, sizeof(IDCODE)*max );
  idx->max = max;
  idx->pIDCodeBuf = tbl;
  do{
    tbl->code = REGIDCODE_MAX;
    tbl->id = REGIDCODE_MAX;
    tbl++;
    max--;
  }while( max );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX 削除
 * @param  idx  IDCODEIDX*
 * @retval  nothing
 */
//--------------------------------------------------------------
static void IDCodeIndex_Delete( IDCODEIDX *idx )
{
  GF_ASSERT( idx->pIDCodeBuf );
  GFL_HEAP_FreeMemory( idx->pIDCodeBuf );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX 指定されたコードとID、フラグを登録
 * @param  idx IDCODEIDX
 * @param  code コード
 * @param  id ID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void IDCodeIndex_RegistCode(
    IDCODEIDX *idx, u16 code, u16 id, u16 flag )
{
  int i = 0;
  IDCODE *tbl = idx->pIDCodeBuf;
  do{
    if( tbl->code == REGIDCODE_MAX ){
      tbl->code = code;
      tbl->id = id;
      tbl->flag = flag;
      return;
    }
    tbl++;
    i++;
  }while( i < idx->max );
  
  GF_ASSERT_MSG( 0,
      "MMDL BLACT RESOURCE MAX(%d) OBJCODE %d RES %d", idx->max, code, id );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX 指定されたコードを削除
 * @param  idx IDCODEIDX
 * @param  code OBJコード
 * @retval  nothing
 */
//--------------------------------------------------------------
static void IDCodeIndex_DeleteCode( IDCODEIDX *idx, u16 code )
{
  int i = 0;
  IDCODE *tbl = idx->pIDCodeBuf;
  do{
    if( tbl->code == code ){
      tbl->code = REGIDCODE_MAX;
      tbl->id = REGIDCODE_MAX;
      tbl->flag = 0;
      return;
    }
    tbl++;
    i++;
  }while( i < idx->max );
  
  GF_ASSERT_MSG( 0,
      "MMDL BLACT NONE RESOURCE OBJCODE %d", code );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX 指定されたコードに一致するID、フラグを返す
 * @param  idx IDCODEIDX
 * @param  code OBJコード
 * @param outID ID格納先 NULL=格納しない
 * @param outFlag フラグ格納先 NULL=格納しない
 * @retval BOOL TRUE=一致有り、FALSE=コードが存在しない
 */
//--------------------------------------------------------------
static BOOL IDCodeIndex_SearchCode(
    IDCODEIDX *idx, u16 code, u16 *outID, u16 *outFlag )
{
  int i = 0;
  IDCODE *tbl = idx->pIDCodeBuf;
  
  if( outID != NULL ){
    *outID = REGIDCODE_MAX;
  }
  
  do{
    if( tbl->code == code ){
      if( outID != NULL ){
        *outID = tbl->id;
      }
      if( outFlag != NULL ){
        *outFlag = tbl->flag;
      }
      return( TRUE );
    }
    tbl++;
    i++;
  }while( i < idx->max );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 初期化
 * @param  pBlActCont MMDL_BLACTCONT
 * @param  max  登録最大数
 * @retval  nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_Init( MMDL_BLACTCONT *pBlActCont, int max )
{
  HEAPID heapID = MMDLSYS_GetHeapID( pBlActCont->mmdlsys );
  IDCodeIndex_Init( &pBlActCont->BBDResUnitIdx, max, heapID );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 削除
 * @param  pBlActCont MMDL_BLACTCONT
 * @param  max  登録最大数
 * @retval  nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_Delete( MMDL_BLACTCONT *pBlActCont )
{
  int i = 0;
  IDCODE *tbl = pBlActCont->BBDResUnitIdx.pIDCodeBuf;
  
  do{
    if( tbl->code != REGIDCODE_MAX ){
      BBDResUnitIndex_RemoveResUnit( pBlActCont, tbl->code );
    }
    tbl++;
    i++;
  }while( i < pBlActCont->BBDResUnitIdx.max );
  
  IDCodeIndex_Delete( &pBlActCont->BBDResUnitIdx );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT リソース追加
 * @param  pBlActCont MMDL_BLACTCONT
 * @param  code  OBJコード
 * @param flag 登録フラグ BBDRESBIT_VRAM等
 * @retval  nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_AddResource(
    MMDL_BLACTCONT *pBlActCont, GFL_G3D_RES *g3dres,
    u16 obj_code, u8 mdl_size, u8 tex_size, BBDRESBIT flag )
{
  GFL_BBDACT_RESUNIT_ID id;
  GFL_BBDACT_RESTYPE type;
  
  type = GFL_BBDACT_RESTYPE_DATACUT; //基本はリソース破棄型
  
  if( (flag&BBDRESBIT_TRANS) ){ //転送用リソース指定
    type = GFL_BBDACT_RESTYPE_TRANSSRC;
  }
  
  id = BlActRes_AddResCore(
      pBlActCont, obj_code, mdl_size, tex_size, g3dres, type );
  IDCodeIndex_RegistCode( &pBlActCont->BBDResUnitIdx, obj_code, id, flag );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT リソースユニット追加
 * @param  pBlActCont MMDL_BLACTCONT
 * @param  code  OBJコード
 * @param flag 登録フラグ BBDRESBIT_VRAM等
 * @retval  nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_AddResUnit(
    MMDL_BLACTCONT *pBlActCont, const OBJCODE_PARAM *pParam, BBDRESBIT flag )
{
  GFL_G3D_RES *g3dres;
  const OBJCODE_PARAM_BUF_BBD *prm_bbd;
  
  prm_bbd = MMDL_TOOL_GetOBJCodeParamBufBBD( pParam );
  
  g3dres = GFL_G3D_CreateResourceHandle(
      MMDLSYS_GetResArcHandle(pBlActCont->mmdlsys), prm_bbd->res_idx );
  BBDResUnitIndex_AddResource( pBlActCont, g3dres,
      pParam->code, pParam->mdl_size, pParam->tex_size, flag );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定OBJコードのリソースユニット削除
 * @param  pBlActCont MMDL_BLACTCONT
 * @param  code  OBJコード
 * @retval  nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_RemoveResUnit(
    MMDL_BLACTCONT *pBlActCont, u16 obj_code )
{
  GFL_BBDACT_RESUNIT_ID id;
  BOOL ret = IDCodeIndex_SearchCode(
      &pBlActCont->BBDResUnitIdx, obj_code, &id, NULL );
  GF_ASSERT( ret == TRUE );
  GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys, id, 1 );
  IDCodeIndex_DeleteCode( &pBlActCont->BBDResUnitIdx, obj_code );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定OBJコードのリソースユニット削除。
 * 登録データは削除し、リソース削除予約を行う。
 * @param  pBlActCont MMDL_BLACTCONT
 * @param  code  OBJコード
 * @retval  nothing
 */
//--------------------------------------------------------------
static void BBDResUnitIndex_RegistRemoveResUnit(
    MMDL_BLACTCONT *pBlActCont, u16 act_id, u16 obj_code )
{
  GFL_BBDACT_RESUNIT_ID id;
  BOOL ret = IDCodeIndex_SearchCode(
      &pBlActCont->BBDResUnitIdx, obj_code, &id, NULL );
  GF_ASSERT( ret == TRUE );
  
  if( BlActDelReserve_RegistResource(pBlActCont,id) == FALSE ){
    //予約一杯ならその場で削除
    GF_ASSERT( 0 && "MMDL RESERVE DEL RES MAX" );
    GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys, id, 1 );
  }
  
  IDCodeIndex_DeleteCode( &pBlActCont->BBDResUnitIdx, obj_code );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT　指定コードのリソースIDを取得
 * @param  pBlActCont MMDL_BLACTCONT
 * @param  code  OBJコード
 * @retval  BOOL TRUE=指定コード登録有り
 */
//--------------------------------------------------------------
static BOOL BBDResUnitIndex_SearchResID(
    MMDL_BLACTCONT *pBlActCont, u16 obj_code, u16 *outID, u16 *outFlag )
{
  BOOL ret = IDCodeIndex_SearchCode(
    &pBlActCont->BBDResUnitIdx, obj_code, outID, outFlag );
  return( ret );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースIDが追加済みかチェック
 * @param  mmdlsys  MMDLSYS
 * @param  code OBJコード
 * @retval  BOOL  TRUE=追加済み FALSE=追加まだ
 */
//--------------------------------------------------------------
BOOL MMDL_BLACTCONT_CheckOBJCodeRes( MMDLSYS *mmdlsys, u16 code )
{
  u16 res;
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  
  if( IDCodeIndex_SearchCode(&pBlActCont->BBDResUnitIdx,code,NULL,NULL) ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースを追加
 * @param  mmdlssy  MMDLSYS
 * @param  code  OBJコード
 * @param trans TRUE=テクスチャ転送型で登録
 * @param guest TRUE=使用されなくなると破棄されるゲストタイプで登録
 * @retval  BOOL  TRUE=追加した。FALSE=既に追加済み
 * @note 呼ばれたその場で読み込みが発生する。
 */
//--------------------------------------------------------------
BOOL MMDL_BLACTCONT_AddOBJCodeRes(
    MMDLSYS *mmdlsys, u16 code, BOOL trans, BOOL guest )
{
  if( MMDL_BLACTCONT_CheckOBJCodeRes(mmdlsys,code) == FALSE ){
    OBJCODE_PARAM param;
    MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
    BBDRESBIT flag = 0;
    if( trans ){ flag |= BBDRESBIT_TRANS; }
    if( guest ){ flag |= BBDRESBIT_GUEST; }
    MMDLSYS_LoadOBJCodeParam( mmdlsys, code, &param );
    BBDResUnitIndex_AddResUnit( pBlActCont, &param, flag );
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * IDCODEIDX BBDACT 指定コードのリソースを削除
 * @param  mmdlssy  MMDLSYS
 * @param  code  OBJコード
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_DeleteOBJCodeRes( MMDLSYS *mmdlsys, u16 code )
{
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  BBDResUnitIndex_RemoveResUnit( pBlActCont, code );
}

//======================================================================
//  アクター追加予約処理
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター追加予約処理　初期化
 * @param pBlActCont MMDL_BLACTCONT*
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_Init( MMDL_BLACTCONT *pBlActCont )
{
  HEAPID heapID = MMDLSYS_GetHeapID( pBlActCont->mmdlsys );
  BLACT_RESERVE *pReserve = 
    GFL_HEAP_AllocClearMemory( heapID, sizeof(BLACT_RESERVE) );
  
  pReserve->resMax = pBlActCont->resourceMax;
  pReserve->actMax = MMDLSYS_GetMMdlMax( pBlActCont->mmdlsys );
  pReserve->resDigestFrameMax = RES_DIGEST_FRAME_MAX;
  
  pReserve->pReserveResParam = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(ADDRES_RESERVE_PARAM)*pReserve->resMax );
  
  DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Plus( pReserve->pReserveResParam );
  
  {
    int i = 0;
    ADDRES_RESERVE_PARAM *pResParam = pReserve->pReserveResParam;
    for( ; i < pReserve->resMax; i++, pResParam++ ){
      pResParam->code = REGIDCODE_MAX;
    }
  }
  
  pReserve->pReserveRes = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(ADDRES_RESERVE)*pReserve->resDigestFrameMax );
  pReserve->pReserveAct = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(ADDACT_RESERVE)*pReserve->actMax );
  pReserve->pReserveDelRes = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(DELRES_RESERVE)*pReserve->resMax );
  
  pReserve->funcFlag = TRUE;
  pBlActCont->pReserve = pReserve;
}

//--------------------------------------------------------------
/**
 * ビルボードアクター追加予約処理　削除
 * @param pBlActCont MMDL_BLACTCONT*
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_Delete( MMDL_BLACTCONT *pBlActCont )
{
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  
  if( pReserve != NULL ){
    u32 i;
    pReserve->funcFlag = FALSE;
    
    { //リソース予約パラメタ
      DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus( pReserve->pReserveResParam );
      GFL_HEAP_FreeMemory( pReserve->pReserveResParam );
    }

    { //リソース
      ADDRES_RESERVE *pRes = pReserve->pReserveRes;
      for( i = 0; i < pReserve->resDigestFrameMax; i++, pRes++ ){
        if( pRes->pG3dRes != NULL ){
          DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus(
              GFL_G3D_GetResourceFileHeader( pRes->pG3dRes ) );
          DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus( pRes->pG3dRes );
          GFL_G3D_DeleteResource( pRes->pG3dRes );
        }
      }
      GFL_HEAP_FreeMemory( pReserve->pReserveRes );
    }
    
    { //削除リソース
      DELRES_RESERVE *pDelRes = pReserve->pReserveDelRes;
      for( i = 0; i < pReserve->resMax; i++, pDelRes++ ){
        if( pDelRes->compFlag == TRUE ){
          pDelRes->compFlag = FALSE;
          GFL_BBDACT_RemoveResourceUnit(
              pBlActCont->pBbdActSys, pDelRes->res_idx, 1 );
        }
      }
      GFL_HEAP_FreeMemory( pReserve->pReserveDelRes );
    }
    
    { //アクター
      ADDACT_RESERVE *pRes = pReserve->pReserveAct;
      for( i = 0; i < pReserve->actMax; i++, pRes++ ){
        if( pRes->outID != NULL && pRes->pTransActRes != NULL ){
          DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus(
              GFL_G3D_GetResourceFileHeader( pRes->pTransActRes ) );
          DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus( pRes->pTransActRes );
          GFL_G3D_DeleteResource( pRes->pTransActRes );
        }
      }
      GFL_HEAP_FreeMemory( pReserve->pReserveAct );
    }
    
    GFL_HEAP_FreeMemory( pReserve );
    pBlActCont->pReserve = NULL;
  }
}

//--------------------------------------------------------------
/**
 * ビルボードアクター追加予約処理　予約あるかチェック
 * @param MMDL_BLACTCONT *pBlActCont
 * @retval BOOL TRUE=予約あり
 */
//--------------------------------------------------------------
static u32 BlActAddReserve_CheckReserve( const MMDL_BLACTCONT *pBlActCont )
{
  u32 i;
  u32 count = 0;
  const BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  
  { //予約パラメタ
    const ADDRES_RESERVE_PARAM *pResParam = pReserve->pReserveResParam;
    
    for( i = 0; i < pReserve->resMax; i++, pResParam++ ){
      if( pResParam->code != REGIDCODE_MAX ){
        return( TRUE );
      }
    }
  }
  
  { //リソース
    const ADDRES_RESERVE *pRes = pReserve->pReserveRes;
    
    for( i = 0; i < pReserve->resDigestFrameMax; i++, pRes++ ){
      if( pRes->pG3dRes != NULL ){
        return( TRUE );
      }
    }
  }

  { //アクター
    const ADDACT_RESERVE *pRes = pReserve->pReserveAct;
    
    for( i = 0; i < pReserve->actMax; i++, pRes++ ){
      if( pRes->outID != NULL && pRes->pTransActRes != NULL ){
        return( TRUE );
      }
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約パラメタ登録
 * @param pBlActCont MMDL_BLACTCONT
 * @param code 表示コード
 * @param flag BBDRESBIT
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_RegistResourceParam(MMDL_BLACTCONT *pBlActCont,
    u16 code, u8 mdl_size, u8 tex_size, u16 res_idx, BBDRESBIT flag )
{
  u32 i = 0;
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  ADDRES_RESERVE_PARAM *pResParam = pReserve->pReserveResParam;
  
  for( ; i < pReserve->resMax; i++, pResParam++ ){
    if( pResParam->code == REGIDCODE_MAX ){
      pResParam->code = code;
      pResParam->mdl_size = mdl_size;
      pResParam->tex_size = tex_size;
      pResParam->res_idx = res_idx;
      pResParam->flag = flag;
      D_MMDL_DPrintf( "MMDL BLACT RESERVE REG RESPRM CODE=%d\n", code );
      return;
    }
  }
  
  GF_ASSERT_MSG( 0,
      "MMDL BLACT REG RESPRM MAX(%d) OBJCODE %d", pReserve->resMax, code );
}

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約パラメタ消化
 * @param pBlActCont MMDL_BLACTCONT
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_DigestResourceParam(
    MMDL_BLACTCONT *pBlActCont )
{
  u32 i = 0,j = 0;
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  ADDRES_RESERVE_PARAM *pResParam = pReserve->pReserveResParam;
  
  for( ; i < pReserve->resMax; i++, pResParam++ ){
    if( pResParam->code != REGIDCODE_MAX ){
      if( BlActAddReserve_RegistResource(pBlActCont,
          pResParam->code,pResParam->mdl_size,pResParam->tex_size,
          pResParam->res_idx,pResParam->flag) == FALSE ){
        D_MMDL_DPrintf( "MMDL DIG RESPRM RESERVE OVER\n" );
        break; //予約一杯
      }
      
      pResParam->code = REGIDCODE_MAX;

      j++;
      if( j >= pReserve->resDigestFrameMax ){
        D_MMDL_DPrintf( "MMDL DIG RESPRM FRAME OVER\n" );
        break;
      }
    }
  }
  
  if( j ){  //消化後の前詰め
    pResParam = pReserve->pReserveResParam;
    
    for( i = 0; i < (pReserve->resMax-1); i++ ){
      if( pResParam[i].code == REGIDCODE_MAX ){
        for( j = i+1; j < pReserve->resMax; j++ ){
          if( pResParam[j].code != REGIDCODE_MAX ){
            pResParam[i] = pResParam[j];
            pResParam[j].code = REGIDCODE_MAX;
            break;
          }
        }
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約登録
 * @param pBlActCont MMDL_BLACTCONT
 * @param code 表示コード
 * @param flag BBDRESBIT
 * @retval BOOL TRUE=登録
 */
//--------------------------------------------------------------
static BOOL BlActAddReserve_RegistResource( MMDL_BLACTCONT *pBlActCont,
    u16 code, u8 mdl_size, u8 tex_size, u16 res_idx, BBDRESBIT flag )
{
  u32 i = 0;
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  ADDRES_RESERVE *pRes = pReserve->pReserveRes;
  
  for( ; i < pReserve->resDigestFrameMax; i++, pRes++ ){
    if( pRes->pG3dRes == NULL ){
      pRes->compFlag = FALSE;
      pRes->code = code;
      pRes->mdl_size = mdl_size;
      pRes->tex_size = tex_size;
      pRes->flag = flag;
      pRes->pG3dRes = GFL_G3D_CreateResourceHandle(
          MMDLSYS_GetResArcHandle(pBlActCont->mmdlsys), res_idx );
      
      pRes->compFlag = TRUE;
      
      DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Plus(
          GFL_G3D_GetResourceFileHeader( pRes->pG3dRes ) );
      DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Plus( pRes->pG3dRes );
      
      D_MMDL_DPrintf(
          "MMDL BLACT RESERVE ADD RESOURCE CODE=%d,ARCIDX=%d\n",
          pRes->code, res_idx );
      return( TRUE );
    }
  }
  
#if 0
  //予約消化が出来ていない事を知らせるASSERT
  GF_ASSERT_MSG( 0,
      "MMDL BLACT RESERVE REG RES MAX:CODE %d", code );
#else
  D_MMDL_Printf( 0,
      "MMDL BLACT RESERVE REG RES MAX:CODE %d", code );
#endif
  
  return( FALSE );
}


#if 0 //new
static BOOL BlActAddReserve_RegistResource( MMDL_BLACTCONT *pBlActCont,
    u16 code, u8 mdl_size, u8 tex_size, u16 res_idx, BBDRESBIT flag )
{
  u32 i = 0;
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  ADDRES_RESERVE *pRes = pReserve->pReserveRes;
  
  for( ; i < pReserve->resDigestFrameMax; i++, pRes++ ){
    if( pRes->pG3dRes == NULL ){
      pRes->compFlag = FALSE;
      pRes->code = code;
      pRes->mdl_size = mdl_size;
      pRes->tex_size = tex_size;
      pRes->flag = flag;
      pRes->pG3dRes = GFL_G3D_CreateResourceHandle(
          MMDLSYS_GetResArcHandle(pBlActCont->mmdlsys), res_idx );
      
      pRes->compFlag = TRUE;
      
      DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Plus(
          GFL_G3D_GetResourceFileHeader( pRes->pG3dRes ) );
      DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Plus( pRes->pG3dRes );
      
      D_MMDL_DPrintf(
          "MMDL BLACT RESERVE ADD RESOURCE CODE=%d,ARCIDX=%d\n",
          pRes->code, res_idx );
      return( TRUE );
    }
  }
  
#if 0
  //予約消化が出来ていない事を知らせるASSERT
  GF_ASSERT_MSG( 0,
      "MMDL BLACT RESERVE REG RES MAX:CODE %d", code );
#else
  D_MMDL_Printf( 0,
      "MMDL BLACT RESERVE REG RES MAX:CODE %d", code );
#endif
  
  return( FALSE );
}
#endif

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約消化
 * @param MMDLSYS *mmdlsys
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_DigestResource( MMDL_BLACTCONT *pBlActCont )
{
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  
  if( pReserve != NULL )
  {
    if( pReserve->funcFlag == TRUE )
    {
      u32 i = 0;
      BLACT_RESERVE *pReserve = pBlActCont->pReserve;
      ADDRES_RESERVE *pRes = pReserve->pReserveRes;
      
      for( ; i < pReserve->resDigestFrameMax; i++, pRes++ ){
        if( pRes->pG3dRes != NULL && pRes->compFlag == TRUE ){
          pRes->compFlag = FALSE;
          
          BBDResUnitIndex_AddResource( pBlActCont, pRes->pG3dRes,
              pRes->code, pRes->mdl_size, pRes->tex_size, pRes->flag );
          
          pRes->pG3dRes = NULL;
          
          #ifdef DEBUG_MMDL_DEVELOP
          {
            u16 id,flag;
            BBDResUnitIndex_SearchResID(
                pBlActCont, pRes->code, &id, &flag );
            D_MMDL_DPrintf(
                "MMDL BLACT RESERVE DIG RESOURCE CODE=%d, RESID=%d\n",
                pRes->code, id );
          }
          #endif
        }
      }
      
      if( pBlActCont->bbdUpdateFlag ){ //リソース削除
        DELRES_RESERVE *pDelRes = pReserve->pReserveDelRes;
        
        for( i = 0; i < pReserve->resMax; i++, pDelRes++ ){
          if( pDelRes->compFlag == TRUE ){
            pDelRes->compFlag = FALSE;
            GFL_BBDACT_RemoveResourceUnit(
              pBlActCont->pBbdActSys, pDelRes->res_idx, 1 );
          }
        }
      }
    }
  }
}

/*
36704
  | 434
362d0
  | 274
3605c
  | 434
35c28
  | 274
359b4
  |
35580
3530c
34ed8
34c64
34830
345bc
34188
*/

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約消化
 * @param pBlActCont MMDL_BLACTCONT
 * @retval nothing
 */
//--------------------------------------------------------------
#if 0
static void BlActAddReserve_DigestResource( MMDL_BLACTCONT *pBlActCont )
{
  u32 i = 0;
  BLACT_RESERVE *pReserve = pBlActCont->pReserve;
  ADDRES_RESERVE *pRes = pReserve->pReserveRes;
  
  for( ; i < pReserve->resMax; i++, pRes++ ){
    if( pRes->pG3dRes != NULL && pRes->compFlag == TRUE ){
      pRes->compFlag = FALSE;
      BBDResUnitIndex_AddResource(
          pBlActCont, pRes->pG3dRes, pRes->code, pRes->flag );
      pRes->pG3dRes = NULL;
    
#ifdef DEBUG_MMDL_DEVELOP
      {
        u16 id,flag;
        BBDResUnitIndex_SearchResID( pBlActCont, pRes->code, &id, &flag );
        D_MMDL_DPrintf(
            "MMDL BLACT RESERVE DIG RESOURCE CODE=%d, RESID=%d\n",
            pRes->code, id );
      }
#endif
    }
  }
}
#endif

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約検索
 * @param pBlActCont MMDL_BLACTCONT
 * @param code 検索する表示コード
 * @param outFlag *予約の際に指定されたBBDRESBIT格納先
 * @retval BOOL TRUE=予約済み FALSE=予約なし
 */
//--------------------------------------------------------------
static BOOL BlActAddReserve_SearchResource(
    MMDL_BLACTCONT *pBlActCont, u16 code, u16 *outFlag )
{
  { //パラメタ予約
    u32 i = 0;
    BLACT_RESERVE *pReserve = pBlActCont->pReserve;
    ADDRES_RESERVE_PARAM *pResParam = pReserve->pReserveResParam;
    
    for( ; i < pReserve->resMax; i++, pResParam++ ){
      if( pResParam->code == code ){
        *outFlag = pResParam->flag;
        return( TRUE );
      }
    }
  }
  
  { //リソース予約
    u32 i = 0;
    BLACT_RESERVE *pReserve = pBlActCont->pReserve;
    ADDRES_RESERVE *pRes = pReserve->pReserveRes;
  
    for( ; i < pReserve->resDigestFrameMax; i++, pRes++ ){
      if( pRes->code == code && pRes->pG3dRes != NULL ){
        *outFlag = pRes->flag;
        return( TRUE );
      }
    }
  }

  return( FALSE );
}

//--------------------------------------------------------------
/**
 * リソース追加予約処理　予約キャンセル
 * @param pBlActCont MMDL_BLACTCONT
 * @param code キャンセルする表示コード
 * @retval BOOL TRUE=キャンセルした FALSE=予約なし
 */
//--------------------------------------------------------------
static BOOL BlActAddReserve_CancelResource(
    MMDL_BLACTCONT *pBlActCont, u16 code )
{
  { //パラメタ予約
    u32 i = 0;
    BLACT_RESERVE *pReserve = pBlActCont->pReserve;
    ADDRES_RESERVE_PARAM *pResParam = pReserve->pReserveResParam;
    
    for( ; i < pReserve->resMax; i++, pResParam++ ){
      if( pResParam->code == code ){
        pResParam->code = REGIDCODE_MAX;
        D_MMDL_DPrintf( "MMDL BLACT RESERVE CANCEL CODE=%d\n", code );
        return( TRUE );
      }
    }
  }
  
  { //リソース予約
    u32 i = 0;
    BLACT_RESERVE *pReserve = pBlActCont->pReserve;
    ADDRES_RESERVE *pRes = pReserve->pReserveRes;
    
    for( ; i < pReserve->resDigestFrameMax; i++, pRes++ ){
      if( pRes->pG3dRes != NULL && pRes->code == code ){
        pRes->compFlag = FALSE;
        
        DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus(
            GFL_G3D_GetResourceFileHeader( pRes->pG3dRes ) );
        DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus( pRes->pG3dRes );
        
        GFL_G3D_DeleteResource( pRes->pG3dRes );
        pRes->pG3dRes = NULL;
        D_MMDL_DPrintf( "MMDL BLACT RESERVE CANCEL CODE=%d\n", code );
        return( TRUE );
      }
    }
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約登録
 * @param pBlActCont MMDL_BLACTCONT
 * @param mmdl MMDL*
 * @param code 表示コード
 * @param outID アクターID格納先
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_RegistActor(
    MMDL_BLACTCONT *pBlActCont,
    MMDL *mmdl,
    const OBJCODE_PARAM *pParam,
    u16 *outID,
    MMDL_BLACTCONT_ADDACT_USERPROC init_proc,
    void *init_work, const VecFx32 *user_pos )
{
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->mmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ )
  {
    if( pRes->outID == NULL )
    {
      u16 resID,flag;
      
      pRes->compFlag = FALSE;
      pRes->mmdl = mmdl;
      pRes->outID = outID;
      pRes->code = pParam->code;
      pRes->mdl_size = pParam->mdl_size;
      pRes->tex_size = pParam->tex_size;
      pRes->anm_id = pParam->anm_id;
      
      BBDResUnitIndex_SearchResID( pBlActCont, pRes->code, &resID, &flag );
      
      if( resID == REGIDCODE_MAX ) //非登録リソース
      {
        if( BlActAddReserve_SearchResource( //リソース予約検索
              pBlActCont,pRes->code,&flag) == FALSE )
        {
          const OBJCODE_PARAM_BUF_BBD *prm_bbd;
          prm_bbd = MMDL_TOOL_GetOBJCodeParamBufBBD( pParam );
          
          flag = BBDRES_VRAM_GUEST; //基本VRAM常駐ゲスト型で登録
          BlActAddReserve_RegistResourceParam( pBlActCont,
              pRes->code, pRes->mdl_size, pRes->tex_size,
              prm_bbd->res_idx, flag );
        }
      }
      
      if( (flag&BBDRESBIT_TRANS) ){ //転送型 転送先リソース作成
        #if 0
        //転送用リソースデータを取得
        pRes->pTransActRes = GFL_G3D_CreateResourceHandle(
              pBlActCont->arcH_res, prm->res_idx );
        #else
        GF_ASSERT( 0 ); //現状未対応
        #endif
      }
      
      pRes->user_init_proc = init_proc;
      pRes->user_init_work = init_work;
      
      if( user_pos != NULL ){
        pRes->user_add_pos = *user_pos;
      }

      pRes->compFlag = TRUE;
      
      #ifdef DEBUG_MMDL_DEVELOP
      if( mmdl != NULL ){
        D_MMDL_DPrintf(
            "MMDL BLACT RESERVE ADD ACTOR OBJID=%d, CODE=%d\n", 
            MMDL_GetOBJID(mmdl), pRes->code );
      }else{
        D_MMDL_DPrintf(
            "MMDL BLACT RESERVE ADD ACTOR OBJID=NON, CODE=%d\n", 
            pRes->code );
      }
      #endif

      return;
    }
  }
  
  GF_ASSERT( 0 && "MMDL BLACT RESERVE ADD MAX" );
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約消化コア部分
 * @param MMDLSYS *mmdlsys
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_DigestActorCore(
    MMDL_BLACTCONT *pBlActCont, ADDACT_RESERVE *pRes )
{
  BOOL ret;
  u16 resID,flag;
  u16 transID = BLACT_RESID_NULL;
  
  pRes->compFlag = FALSE;
   
  //一応チェック
  if( pRes->pTransActRes != NULL ) //VRAM転送型
  {
    resID = BlActRes_AddResCore( pBlActCont,
        pRes->code, pRes->mdl_size, pRes->tex_size,
        pRes->pTransActRes, GFL_BBDACT_RESTYPE_DATACUT );
    pRes->pTransActRes = NULL;
    
    ret = BBDResUnitIndex_SearchResID(
          pBlActCont, pRes->code, &transID, &flag );
    
    GF_ASSERT( ret == TRUE &&
        "BLACT ADD RESERVE RESOURCE NONE" ); //転送用リソース無し
    GF_ASSERT( (flag&BBDRESBIT_TRANS) &&
        "BLACT ADD RESERVE RESOURCE ERROR" ); //転送用リソースでは無い
  }
  else //VRAM常駐型
  {
    ret = BBDResUnitIndex_SearchResID(
          pBlActCont, pRes->code, &resID, &flag );
     
    if( ret != TRUE ){
      D_MMDL_Printf( "BLACT ADD RESOURCE ERROR CODE=%xH\n", pRes->code );
    }
        
    //リソース無し
    GF_ASSERT( ret == TRUE && "BLACT ADD RESERVE RESOURCE NONE" );
  }
      
  if( pRes->mmdl != NULL ){ //アクター追加 動作モデル
    *(pRes->outID) = blact_SetResActor( pRes->mmdl, resID, transID );
  }else{ //アクター追加 ユーザー指定
    *(pRes->outID) = blact_SetResActorCore( pBlActCont->mmdlsys,
        &pRes->user_add_pos, pRes->mdl_size, pRes->anm_id, resID, transID );
  }
      
  if( pRes->user_init_proc != NULL ){ //ユーザー指定初期化処理呼び出し
    pRes->user_init_proc( *pRes->outID, pRes->user_init_work );
  }
      
  if( pRes->mmdl != NULL ){
    MMDL_CallDrawProc( pRes->mmdl ); //描画処理呼び出し
  }
      
  #ifdef DEBUG_MMDL_DEVELOP
  D_MMDL_DPrintf( "MMDL BLACT RESERVE DIG ACTOR " );
  if( pRes->mmdl != NULL ){
    D_MMDL_DPrintf( "OBJID=%d, CODE=%d, RESID=%d,TRANSID=%d\n",
      MMDL_GetOBJID(pRes->mmdl), pRes->code, resID, transID );
  }else{
    D_MMDL_DPrintf( "OBJID=NONE, CODE=%d, RESID=%d,TRANSID=%d\n",
      MMDL_GetOBJID(pRes->mmdl), pRes->code, resID, transID );
  }
  #endif
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約消化
 * @param MMDLSYS *mmdlsys
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_DigestActor( MMDL_BLACTCONT *pBlActCont )
{
  BOOL ret;
  u16 flag,transID;
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->mmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ )
  {
    if( pRes->outID != NULL && pRes->compFlag == TRUE )
    {
      ret = BBDResUnitIndex_SearchResID(
          pBlActCont, pRes->code, &transID, &flag );
      
      if( ret == TRUE ){ //リソースが登録された
        BlActAddReserve_DigestActorCore( pBlActCont, pRes );
        BlActAddReserve_ClearWork( pRes );
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約済みの表示コードを検索
 * @param pBlActCont MMDL_BLACTCONT
 * @param code 検索する表示コード
 * @param mmdl 検索対象としないMMDL
 * @retval BOOL TRUE=一致有り FALSE=一致無し
 */
//--------------------------------------------------------------
static BOOL BlActAddReserve_SearchActorOBJCode(
    MMDL_BLACTCONT *pBlActCont, u16 code, MMDL *mmdl )
{
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->mmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ ){
    if( pRes->outID != NULL && pRes->mmdl != NULL && pRes->mmdl != mmdl ){
      if( pRes->code == code ){
        return( TRUE );
      }
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約済みの表示コードを検索 ID格納アドレス版
 * @param pBlActCont MMDL_BLACTCONT
 * @param code 検索する表示コード
 * @param outID 検索対象としないoutID
 * @retval BOOL TRUE=一致有り FALSE=一致無し
 */
//--------------------------------------------------------------
static BOOL BlActAddReserve_SearchActorOBJCodeOutID(
    MMDL_BLACTCONT *pBlActCont, u16 code, const u16 *outID )
{
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->mmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ ){
    if( pRes->outID != NULL && pRes->outID != outID ){
      if( pRes->code == code ){
        return( TRUE );
      }
    }
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約キャンセル
 * @param pBlActCont MMDL_BLACTCONT
 * @param mmdl 予約したMMDL*
 * @retval nothing
 * @note 追加時に指定されたコードがゲストリソースとして
 * 追加予約に入っていた際はリソース予約キャンセルも合わせて行う。
 */
//--------------------------------------------------------------
static void BlActAddReserve_CancelActorOutID(
    MMDL_BLACTCONT *pBlActCont, const u16 *outID )
{
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->mmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ )
  {
    if( pRes->outID != NULL && pRes->outID == outID )
    {
      pRes->compFlag = FALSE;
      
      if( pRes->pTransActRes != NULL ) //転送用リソース有り
      {
        DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus( GFL_G3D_GetResourceFileHeader( pRes->pTransActRes ) );
        DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus( pRes->pTransActRes );
        GFL_G3D_DeleteResource( pRes->pTransActRes );
        pRes->pTransActRes = NULL;
      }
      
      D_MMDL_DPrintf(
          "MMDL BLACT RESERVE CANCEL OBJID=NON, CODE=%d\n",  pRes->code );
      
      BlActAddReserve_ClearWork( pRes );
      return;
    }
  }
}

//--------------------------------------------------------------
/**
 * アクター追加予約処理　予約キャンセル IDポインタ
 * @param pBlActCont MMDL_BLACTCONT
 * @param outID 予約時に指定したID格納先ポインタ
 * @retval nothing
 * @note リソース関連のキャンセルは行わない。
 */
//--------------------------------------------------------------
static void BlActAddReserve_CancelActor(
    MMDL_BLACTCONT *pBlActCont, MMDL *mmdl )
{
  u32 i = 0;
  u32 max = MMDLSYS_GetMMdlMax( pBlActCont->mmdlsys );
  ADDACT_RESERVE *pRes = pBlActCont->pReserve->pReserveAct;
  
  for( ; i < max; i++, pRes++ )
  {
    if( pRes->outID != NULL && pRes->mmdl == mmdl )
    {
      u16 flag;
      pRes->compFlag = FALSE;
      
      if( BlActAddReserve_SearchResource( //リソース予約有り
            pBlActCont,pRes->code,&flag) == TRUE )
      {
        if( (flag&BBDRESBIT_GUEST) ){ //ゲストリソース
          if( BlActAddReserve_SearchActorOBJCode( //他の予約で使用無し
                pBlActCont,pRes->code,pRes->mmdl) == FALSE )
          {
            BlActAddReserve_CancelResource( pBlActCont, pRes->code );
          }
        }
      }
      
      if( pRes->pTransActRes != NULL ) //転送用リソース有り
      {
        DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus(
            GFL_G3D_GetResourceFileHeader( pRes->pTransActRes ) );
        DEBUG_MMDL_RESOURCE_MEMORY_SIZE_Minus( pRes->pTransActRes );
        
        GFL_G3D_DeleteResource( pRes->pTransActRes );
        pRes->pTransActRes = NULL;
      }
      
      D_MMDL_DPrintf( "MMDL BLACT RESERVE CANCEL OBJID=%d, CODE=%d\n", 
          MMDL_GetOBJID(mmdl), pRes->code );
      
      BlActAddReserve_ClearWork( pRes );
      return;
    }
  }
}

//--------------------------------------------------------------
/**
 * ADDACT_RESERVEクリア
 * @param pRes ADDACT_RESERVE
 * @retval nothing
 */
//--------------------------------------------------------------
static void BlActAddReserve_ClearWork( ADDACT_RESERVE *pRes )
{
  pRes->compFlag = FALSE;
  MI_CpuClear8( pRes, sizeof(ADDACT_RESERVE) );
}

//--------------------------------------------------------------
/**
 * リソース削除予約　登録
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL BlActDelReserve_RegistResource(
    MMDL_BLACTCONT *pBlActCont, u16 res_idx )
{
  u32 i = 0;
  u32 max = pBlActCont->pReserve->resMax;
  DELRES_RESERVE *pDelRes = pBlActCont->pReserve->pReserveDelRes;

  for( i = 0; i < max; i++, pDelRes++ ){
    if( pDelRes->compFlag == FALSE ){
      pDelRes->res_idx = res_idx;
      pDelRes->compFlag = TRUE;
      return( TRUE );
    }
  }
  
  return( FALSE );
}

//======================================================================
//  ダミーアクター関連
//======================================================================
//--------------------------------------------------------------
/**
 * ダミーアクターを利用しOBJコードを変更する
 * @param mmdl MMDL*
 * @param next_code 変更するOBJコード
 * @retval nothing
 * @note 現状使用しているビルボードアクターをダミーアクターとして扱い
 * その裏でOBJコードの変更を行う。変更後にダミーアクターは削除される。
 */
//--------------------------------------------------------------
void MMDL_BLACTCONT_ChangeOBJCodeWithDummy( MMDL *mmdl, u16 next_code )
{
  int i;
  u16 actID,resID,flag;
  MMDLSYS *mmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
  MMDL_BLACTCONT *pBlActCont = MMDLSYS_GetBlActCont( mmdlsys );
  DMYACT_RESCHG *pDmyAct = pBlActCont->pDmyActTbl;
  
  BBDResUnitIndex_SearchResID( pBlActCont, next_code, &resID, &flag );
  
  //リソースが既に有る状態ならダミー登録の必要なし
  if( resID != REGIDCODE_MAX && (flag&BBDRESBIT_TRANS) == 0 ){
    MMDL_ChangeOBJCode( mmdl, next_code );
    return;
  }
  
  //アクターIDが存在しない場合はダミーは追加せず。
  actID = MMDL_CallDrawGetProc( mmdl, 0 );
  
  if( actID == MMDL_BLACTID_NULL ){
    MMDL_ChangeOBJCode( mmdl, next_code );
    return;
  }
  
  i = 0;
  while( i < DMYACT_MAX ){ 
    if( pDmyAct->mmdl == NULL ){
      break;
    }
    i++;
  }
    
  if( i >= DMYACT_MAX ){ //ダミー満杯。アサートを出し登録はせず終わる
    GF_ASSERT( 0 && "MMDL BLACT DUMMY MAX" );
    MMDL_ChangeOBJCode( mmdl, next_code );
    return;
  }
  
  //ダミー登録
  pDmyAct->mmdl = mmdl;
  pDmyAct->dmy_act_id = actID;
  pDmyAct->dmy_obj_code = MMDL_GetOBJCode( mmdl );
  pDmyAct->next_code = next_code;
  
  //next_codeを指定し動作モデルOBJコード変更呼び出し
  //描画削除処理を呼びアクターを削除するが
  //ダミーアクターに登録されているので
  //アクター削除は行わない。
  //続いて描画初期化処理呼び出して
  //内部でnext_codeのadd_actor()が行われリソース追加等が起きる筈
  MMDL_ChangeOBJCode( mmdl, next_code );
  
  //念のためここでアクター追加完了チェック。
  //完了済みなら即終了
  if( MMDL_CallDrawGetProc(mmdl,0) != MMDL_BLACTID_NULL ){
    GF_ASSERT( 0 ); //本来在り得ない。アサート
    blact_DeleteResource(
        pBlActCont, mmdl, pDmyAct->dmy_act_id, pDmyAct->dmy_obj_code );
    GFL_BBDACT_RemoveAct( pBlActCont->pBbdActSys, pDmyAct->dmy_act_id, 1 );
    pDmyAct->mmdl = NULL;
  }
}

//--------------------------------------------------------------
/**
 * ダミーアクター　アクターテーブル初期化
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void DummyAct_Init( MMDL_BLACTCONT *pBlActCont )
{
  HEAPID heapID = MMDLSYS_GetHeapID( pBlActCont->mmdlsys );
  pBlActCont->pDmyActTbl = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(DMYACT_RESCHG) * DMYACT_MAX );
}

//--------------------------------------------------------------
/**
 * ダミーアクター　削除
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void DummyAct_Delete( MMDL_BLACTCONT *pBlActCont )
{
  DMYACT_RESCHG *pDmyAct = pBlActCont->pDmyActTbl;
  
  if( pDmyAct != NULL ){
    int i = 0;
    for( ; i < DMYACT_MAX; i++, pDmyAct++ ){
      if( pDmyAct->mmdl != NULL ){
        GFL_BBDACT_RemoveAct(
            pBlActCont->pBbdActSys, pDmyAct->dmy_act_id, 1 );
        pDmyAct->mmdl = NULL;
      }
    }
    
    GFL_HEAP_FreeMemory( pBlActCont->pDmyActTbl );
  }
}

//--------------------------------------------------------------
/**
 * ダミーアクター　更新
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void DummyAct_UpdateAct(
    MMDL_BLACTCONT *pBlActCont, DMYACT_RESCHG *pDmyAct )
{
  MMDL *mmdl = pDmyAct->mmdl;
  
  if( MMDL_CheckStatusBitUse(mmdl) == FALSE ){ //死亡
    blact_DeleteResource(
        pBlActCont, mmdl, pDmyAct->dmy_act_id, pDmyAct->dmy_obj_code );
    GFL_BBDACT_RemoveAct( pBlActCont->pBbdActSys, pDmyAct->dmy_act_id, 1 );
    pDmyAct->mmdl = NULL;
  }else{
    u16 actID = MMDL_CallDrawGetProc( mmdl, 0 );

    if( actID != MMDL_BLACTID_NULL ){ //登録がされた
      if( MMDL_GetOBJCode(mmdl) != pDmyAct->dmy_obj_code ){ 
        //現状と違うコードで登録されたならダミーアクター用リソースは削除
        blact_DeleteResource(
            pBlActCont, mmdl, pDmyAct->dmy_act_id, pDmyAct->dmy_obj_code );
      }
    
      GFL_BBDACT_RemoveAct(
          pBlActCont->pBbdActSys, pDmyAct->dmy_act_id, 1 );
      pDmyAct->mmdl = NULL;
    }
  }
}

//--------------------------------------------------------------
/**
 * ダミーアクター　更新処理
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void DummyAct_Update( MMDL_BLACTCONT *pBlActCont )
{
  DMYACT_RESCHG *pDmyAct = pBlActCont->pDmyActTbl;
  
  if( pDmyAct != NULL ){
    int i = 0;
    for( ; i < DMYACT_MAX; i++, pDmyAct++ ){
      if( pDmyAct->mmdl != NULL ){
        DummyAct_UpdateAct( pBlActCont, pDmyAct );
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * ダミーアクター　動作モデル登録チェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL DummyAct_CheckMMdl(
    MMDL_BLACTCONT *pBlActCont, const MMDL *mmdl, u16 actID )
{
  DMYACT_RESCHG *pDmyAct = pBlActCont->pDmyActTbl;
  
  if( pDmyAct != NULL ){
    int i = 0;
    for( ; i < DMYACT_MAX; i++, pDmyAct++ ){
      if( pDmyAct->mmdl != NULL ){
        if( pDmyAct->mmdl == mmdl && pDmyAct->dmy_act_id == actID ){
          return( TRUE );
        }
      }
    }
  }
  
  return( FALSE );
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * テーブル番号からアクターアニメテーブル取得
 * @param  no  MMDL_BLACT_ANMTBLNO_MAX
 * @retval  MMDL_BBDACT_ANMTBL*
 */
//--------------------------------------------------------------
static const MMDL_BBDACT_ANMTBL * BlActAnm_GetAnmTbl( u32 no )
{
  GF_ASSERT( no < MMDL_BLACT_ANMTBLNO_MAX );
  return( &DATA_MMDL_BBDACT_ANM_ListTable[no] );
}

//--------------------------------------------------------------
/**
 * OBJコードで使用するアクターアニメテーブル取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
#if 0
const MMDL_BBDACT_ANMTBL * MMDL_BLACTCONT_GetObjAnimeTable(
  const MMDLSYS *mmdlsys, u16 code )
{
  u16 id;
  const MMDL_BBDACT_ANMTBL *anmTbl;
  const OBJCODE_PARAM *prm = MMDLSYS_GetOBJCodeParam( mmdlsys, code );
  
  id = prm->anm_id;
  
  if( prm->draw_type != MMDL_DRAWTYPE_BLACT ){
    GF_ASSERT( 0 );
    id = MMDL_BLACT_ANMTBLNO_BLACT;
  }
  
  anmTbl = BlActAnm_GetAnmTbl( id );
  return( anmTbl );
}
#endif

#if 0
//--------------------------------------------------------------
/**
 * ビルボードアニメ 描画ステータスからテーブル要素数を取得。基本タイプ
 * @param sta DRAW_STA_STOP等
 * @retval u32 対応するアニメーションテーブル要素数
 */
//--------------------------------------------------------------
u32 FLDMMDL_BLACTCONT_GetAnmTblNo_Status( u32 sta )
{
  GF_ASSERT( sta < DRAW_STA_MAX );
  //上下左右の並びで各４方向
  return( sta * DIR_MAX4 );
}

//--------------------------------------------------------------
/**
 * ビルボードアニメ 描画ステータスからテーブル要素数を取得。自機専用
 * @param sta DRAW_STA_STOP等
 * @retval u32 対応するアニメーションテーブル要素数
 */
//--------------------------------------------------------------
u32 FLDMMDL_BLACTCONT_GetAnmTblNo_StatusHero( u32 sta )
{
  GF_ASSERT( sta < DRAW_STA_HERO_MAX );
  //上下左右の並びで各４方向
  return( sta * DIR_MAX4 );
}
#endif

//--------------------------------------------------------------
/**
 * ビルボードアクターリソース追加　コア部分
 * @param pBlActCont MMDL_BLACTCONT
 * @param code OBJコード
 * g3dres 追加するリソース*
 * @param type リソースタイプ
 * @retval GFL_BBDACT_RESUNIT_ID
 */
//--------------------------------------------------------------
static GFL_BBDACT_RESUNIT_ID BlActRes_AddResCore(
    MMDL_BLACTCONT *pBlActCont,
    u16 code, u8 mdl_size, u8 tex_size,
    GFL_G3D_RES *g3dres, GFL_BBDACT_RESTYPE type )
{
  GFL_BBDACT_RESUNIT_ID id;
  GFL_BBDACT_G3DRESDATA data;
  
  data.g3dres = g3dres;
  data.texFmt = GFL_BBD_TEXFMT_PAL16;
  data.texSiz = data_BBDTexSizeTbl[tex_size];
  
  {
    const u16 *size = DATA_MMDL_BLACT_MdlSize[mdl_size];
    data.celSizX = size[0];
    data.celSizY = size[1];
    #if 0   
    D_MMDL_DPrintf( "テクスチャリソース追加 セルサイズ X=%d,Y=%d\n",
        data.celSizX, data.celSizY );
    #endif
  }
  
  data.dataCut = type;
  
  id = GFL_BBDACT_AddResourceG3DResUnit( pBlActCont->pBbdActSys, &data, 1 );
  
  D_MMDL_DPrintf( "MMDL ADD RESOURCE CODE=%d,RESID=%d\n", code, id );
  return( id );
}

//--------------------------------------------------------------
/**
 * ビルボードアクターリソース追加 OBJコード指定
 * @param pBlActCont MMDL_BLACTCONT
 * @param code OBJコード
 * g3dres 追加するリソース*
 * @param type リソースタイプ
 * @retval GFL_BBDACT_RESUNIT_ID
 */
//--------------------------------------------------------------
#if 0 //不要となった
static GFL_BBDACT_RESUNIT_ID BlActRes_AddRes(
    MMDL_BLACTCONT *pBlActCont, u16 code, u16 res_idx,
    GFL_G3D_RES *g3dres, GFL_BBDACT_RESTYPE type )
{
  GFL_BBDACT_RESUNIT_ID id;
  OBJCODE_PARAM param;
  GFL_BBDACT_G3DRESDATA data;
  
  MMDLSYS_LoadOBJCodeParam( pBlActCont->mmdlsys, code, &param );
  return( BlActRes_AddResCore(pBlActCont,&param,g3dres,type) );
}
#endif

//--------------------------------------------------------------
/**
 * ビルボードアクターリソース追加 動作モデル指定
 * @param pBlActCont MMDL_BLACTCONT
 * @param code OBJコード
 * g3dres 追加するリソース*
 * @param type リソースタイプ
 * @retval GFL_BBDACT_RESUNIT_ID
 */
//--------------------------------------------------------------
#if 0 //現状不要
static GFL_BBDACT_RESUNIT_ID BlActRes_AddResMMdl(
    MMDL_BLACTCONT *pBlActCont, const MMDL *mmdl, 
    GFL_G3D_RES *g3dres, GFL_BBDACT_RESTYPE type )
{
  GFL_BBDACT_RESUNIT_ID id;
  const OBJCODE_PARAM *param;
  GFL_BBDACT_G3DRESDATA data;
  
  param = MMDL_GetOBJCodeParam( mmdl );
  return( BlActRes_AddResCore(pBlActCont,param,g3dres,type) );
}
#endif

//======================================================================
//  data
//======================================================================
static const u16 data_BBDTexSizeTbl[TEXSIZE_MAX] =
{
  GFL_BBD_TEXSIZ_8x8,
  GFL_BBD_TEXSIZ_8x16,
  GFL_BBD_TEXSIZ_8x32,
  GFL_BBD_TEXSIZ_8x64,
  GFL_BBD_TEXSIZ_8x128,
  GFL_BBD_TEXSIZ_8x256,
  GFL_BBD_TEXSIZ_8x512,
  GFL_BBD_TEXSIZ_8x1024,
  GFL_BBD_TEXSIZ_16x8,
  GFL_BBD_TEXSIZ_16x16,
  GFL_BBD_TEXSIZ_16x32,
  GFL_BBD_TEXSIZ_16x64,
  GFL_BBD_TEXSIZ_16x128,
  GFL_BBD_TEXSIZ_16x256,
  GFL_BBD_TEXSIZ_16x512,
  GFL_BBD_TEXSIZ_16x1024,
  GFL_BBD_TEXSIZ_32x8,
  GFL_BBD_TEXSIZ_32x16,
  GFL_BBD_TEXSIZ_32x32,
  GFL_BBD_TEXSIZ_32x64,
  GFL_BBD_TEXSIZ_32x128,
  GFL_BBD_TEXSIZ_32x256,
  GFL_BBD_TEXSIZ_32x512,
  GFL_BBD_TEXSIZ_32x1024,
  GFL_BBD_TEXSIZ_64x8,
  GFL_BBD_TEXSIZ_64x16,
  GFL_BBD_TEXSIZ_64x32,
  GFL_BBD_TEXSIZ_64x64,
  GFL_BBD_TEXSIZ_64x128,
  GFL_BBD_TEXSIZ_64x256,
  GFL_BBD_TEXSIZ_64x512,
  GFL_BBD_TEXSIZ_64x1024,
  GFL_BBD_TEXSIZ_128x8,
  GFL_BBD_TEXSIZ_128x16,
  GFL_BBD_TEXSIZ_128x32,
  GFL_BBD_TEXSIZ_128x64,
  GFL_BBD_TEXSIZ_128x128,
  GFL_BBD_TEXSIZ_128x256,
  GFL_BBD_TEXSIZ_128x512,
  GFL_BBD_TEXSIZ_128x1024,
  GFL_BBD_TEXSIZ_256x8,
  GFL_BBD_TEXSIZ_256x16,
  GFL_BBD_TEXSIZ_256x32,
  GFL_BBD_TEXSIZ_256x64,
  GFL_BBD_TEXSIZ_256x128,
  GFL_BBD_TEXSIZ_256x256,
  GFL_BBD_TEXSIZ_256x512,
  GFL_BBD_TEXSIZ_256x1024,
  GFL_BBD_TEXSIZ_512x8,
  GFL_BBD_TEXSIZ_512x16,
  GFL_BBD_TEXSIZ_512x32,
  GFL_BBD_TEXSIZ_512x64,
  GFL_BBD_TEXSIZ_512x128,
  GFL_BBD_TEXSIZ_512x256,
  GFL_BBD_TEXSIZ_512x512,
  GFL_BBD_TEXSIZ_512x1024,
  GFL_BBD_TEXSIZ_1024x8,
  GFL_BBD_TEXSIZ_1024x16,
  GFL_BBD_TEXSIZ_1024x32,
  GFL_BBD_TEXSIZ_1024x64,
  GFL_BBD_TEXSIZ_1024x128,
  GFL_BBD_TEXSIZ_1024x256,
  GFL_BBD_TEXSIZ_1024x512,
  GFL_BBD_TEXSIZ_1024x1024,
};

//======================================================================
//  debug
//======================================================================
#ifdef DEBUG_MMDL_RESOURCE_MEMORY_SIZE
//--------------------------------------------------------------
//  @brief  リソースメモリサイズを返す
//--------------------------------------------------------------
u32 DEBUG_MMDL_GetUseResourceMemorySize( void )
{
  return DEBUG_MMDL_RESOURCE_MemorySize;
}
#endif

//======================================================================
//  test data
//======================================================================
/*
static const GFL_BBDACT_RESDATA testResTable[] = {
#ifdef MMDL_BLACT_HEAD3_TEST
  { ARCID_MMDL_BTX, NARC_fldmmdl_btx_obj_kabi32_nsbtx, 
    GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
#else
  { ARCID_FLDMAP_ACTOR, NARC_fld_act_hero_nsbtx, 
    GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x1024, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
#endif
  { ARCID_FLDMAP_ACTOR, NARC_fld_act_achamo_nsbtx, 
    GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
  { ARCID_FLDMAP_ACTOR, NARC_fld_act_badman_nsbtx, 
    GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
  { ARCID_FLDMAP_ACTOR, NARC_fld_act_beachgirl_nsbtx,
    GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
  { ARCID_FLDMAP_ACTOR, NARC_fld_act_idol_nsbtx,
    GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
  { ARCID_FLDMAP_ACTOR, NARC_fld_act_lady_nsbtx,
    GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
  { ARCID_FLDMAP_ACTOR, NARC_fld_act_oldman1_nsbtx,
    GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
  { ARCID_FLDMAP_ACTOR, NARC_fld_act_policeman_nsbtx,
    GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
};
const u32 testResTableMax = NELEMS( testResTable );
*/

//======================================================================
//  動作モデル管理環境で定義されるテクスチャサイズ指定が
//  ビルボードアクターで定義されているものと同一かどうか
//  プリプロセッサ命令でチェック
//======================================================================
//#define MMDL_CHECK_GFL_BBD_TEXSIZDEF ///<定義でチェック有効

#ifdef MMDL_CHECK_GFL_BBD_TEXSIZDEF

//-------------------------------------
///	ビルボードテクスチャサイズ define定義版
//-------------------------------------
#define GFL_BBD_TEXSIZDEF_8x8 0
#define GFL_BBD_TEXSIZDEF_8x16 1
#define GFL_BBD_TEXSIZDEF_8x32 2
#define GFL_BBD_TEXSIZDEF_8x64 3
#define GFL_BBD_TEXSIZDEF_8x128 4
#define GFL_BBD_TEXSIZDEF_8x256 5
#define GFL_BBD_TEXSIZDEF_8x512 6
#define GFL_BBD_TEXSIZDEF_8x1024 7
#define GFL_BBD_TEXSIZDEF_16x8 8
#define GFL_BBD_TEXSIZDEF_16x16 9
#define GFL_BBD_TEXSIZDEF_16x32 10
#define GFL_BBD_TEXSIZDEF_16x64 11
#define GFL_BBD_TEXSIZDEF_16x128 12
#define GFL_BBD_TEXSIZDEF_16x256 13
#define GFL_BBD_TEXSIZDEF_16x512 14
#define GFL_BBD_TEXSIZDEF_16x1024 15
#define GFL_BBD_TEXSIZDEF_32x8 16
#define GFL_BBD_TEXSIZDEF_32x16 17
#define GFL_BBD_TEXSIZDEF_32x32 18
#define GFL_BBD_TEXSIZDEF_32x64 19
#define GFL_BBD_TEXSIZDEF_32x128 20
#define GFL_BBD_TEXSIZDEF_32x256 21
#define GFL_BBD_TEXSIZDEF_32x512 22
#define GFL_BBD_TEXSIZDEF_32x1024 23
#define GFL_BBD_TEXSIZDEF_64x8 24 
#define GFL_BBD_TEXSIZDEF_64x16 25
#define GFL_BBD_TEXSIZDEF_64x32 26
#define GFL_BBD_TEXSIZDEF_64x64 27
#define GFL_BBD_TEXSIZDEF_64x128 28
#define GFL_BBD_TEXSIZDEF_64x256 29
#define GFL_BBD_TEXSIZDEF_64x512 30
#define GFL_BBD_TEXSIZDEF_64x1024 31
#define GFL_BBD_TEXSIZDEF_128x8 32
#define GFL_BBD_TEXSIZDEF_128x16 33
#define GFL_BBD_TEXSIZDEF_128x32 34
#define GFL_BBD_TEXSIZDEF_128x64 35
#define GFL_BBD_TEXSIZDEF_128x128 36
#define GFL_BBD_TEXSIZDEF_128x256 37
#define GFL_BBD_TEXSIZDEF_128x512 38
#define GFL_BBD_TEXSIZDEF_128x1024 39
#define GFL_BBD_TEXSIZDEF_256x8 40
#define GFL_BBD_TEXSIZDEF_256x16 41
#define GFL_BBD_TEXSIZDEF_256x32 42
#define GFL_BBD_TEXSIZDEF_256x64 43
#define GFL_BBD_TEXSIZDEF_256x128 44
#define GFL_BBD_TEXSIZDEF_256x256 45
#define GFL_BBD_TEXSIZDEF_256x512 46
#define GFL_BBD_TEXSIZDEF_256x1024 47
#define GFL_BBD_TEXSIZDEF_512x8 48
#define GFL_BBD_TEXSIZDEF_512x16 49
#define GFL_BBD_TEXSIZDEF_512x32 50
#define GFL_BBD_TEXSIZDEF_512x64 51
#define GFL_BBD_TEXSIZDEF_512x128 52
#define GFL_BBD_TEXSIZDEF_512x256 53
#define GFL_BBD_TEXSIZDEF_512x512 54
#define GFL_BBD_TEXSIZDEF_512x1024 55
#define GFL_BBD_TEXSIZDEF_1024x8 56
#define GFL_BBD_TEXSIZDEF_1024x16 57
#define GFL_BBD_TEXSIZDEF_1024x32 58
#define GFL_BBD_TEXSIZDEF_1024x64 59
#define GFL_BBD_TEXSIZDEF_1024x128 60
#define GFL_BBD_TEXSIZDEF_1024x256 61
#define GFL_BBD_TEXSIZDEF_1024x512 62
#define GFL_BBD_TEXSIZDEF_1024x1024 63

#include "field/fldmmdl_list_symbol.h"

#if (GFL_BBD_TEXSIZDEF_8x8-TEXSIZE_8x8)
#error (GFL_BBD_TEXSIZDEF_8x8-TEXSIZE_8x8)
#endif
#if (GFL_BBD_TEXSIZDEF_8x16-TEXSIZE_8x16)
#error (GFL_BBD_TEXSIZDEF_8x16-TEXSIZE_8x16)
#endif
#if (GFL_BBD_TEXSIZDEF_8x32-TEXSIZE_8x32)
#error (GFL_BBD_TEXSIZDEF_8x32-TEXSIZE_8x32)
#endif
#if (GFL_BBD_TEXSIZDEF_8x64-TEXSIZE_8x64)
#error (GFL_BBD_TEXSIZDEF_8x64-TEXSIZE_8x64)
#endif
#if (GFL_BBD_TEXSIZDEF_8x128-TEXSIZE_8x128)
#error (GFL_BBD_TEXSIZDEF_8x128-TEXSIZE_8x128)
#endif
#if (GFL_BBD_TEXSIZDEF_8x256-TEXSIZE_8x256)
#error (GFL_BBD_TEXSIZDEF_8x256-TEXSIZE_8x256)
#endif
#if (GFL_BBD_TEXSIZDEF_8x512-TEXSIZE_8x512)
#error (GFL_BBD_TEXSIZDEF_8x512-TEXSIZE_8x512)
#endif
#if (GFL_BBD_TEXSIZDEF_8x1024-TEXSIZE_8x1024)
#error (GFL_BBD_TEXSIZDEF_8x1024-TEXSIZE_8x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_16x8-TEXSIZE_16x8)
#error (GFL_BBD_TEXSIZDEF_16x8-TEXSIZE_16x8)
#endif
#if (GFL_BBD_TEXSIZDEF_16x16-TEXSIZE_16x16)
#error (GFL_BBD_TEXSIZDEF_16x16-TEXSIZE_16x16)
#endif
#if (GFL_BBD_TEXSIZDEF_16x32-TEXSIZE_16x32)
#error (GFL_BBD_TEXSIZDEF_16x32-TEXSIZE_16x32)
#endif
#if (GFL_BBD_TEXSIZDEF_16x64-TEXSIZE_16x64)
#error (GFL_BBD_TEXSIZDEF_16x64-TEXSIZE_16x64)
#endif
#if (GFL_BBD_TEXSIZDEF_16x128-TEXSIZE_16x128)
#error (GFL_BBD_TEXSIZDEF_16x128-TEXSIZE_16x128)
#endif
#if (GFL_BBD_TEXSIZDEF_16x256-TEXSIZE_16x256)
#error (GFL_BBD_TEXSIZDEF_16x256-TEXSIZE_16x256)
#endif
#if (GFL_BBD_TEXSIZDEF_16x512-TEXSIZE_16x512)
#error (GFL_BBD_TEXSIZDEF_16x512-TEXSIZE_16x512)
#endif
#if (GFL_BBD_TEXSIZDEF_16x1024-TEXSIZE_16x1024)
#error (GFL_BBD_TEXSIZDEF_16x1024-TEXSIZE_16x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_32x8-TEXSIZE_32x8)
#error (GFL_BBD_TEXSIZDEF_32x8-TEXSIZE_32x8)
#endif
#if (GFL_BBD_TEXSIZDEF_32x16-TEXSIZE_32x16)
#error (GFL_BBD_TEXSIZDEF_32x16-TEXSIZE_32x16)
#endif
#if (GFL_BBD_TEXSIZDEF_32x32-TEXSIZE_32x32)
#error (GFL_BBD_TEXSIZDEF_32x32-TEXSIZE_32x32)
#endif
#if (GFL_BBD_TEXSIZDEF_32x64-TEXSIZE_32x64)
#error (GFL_BBD_TEXSIZDEF_32x64-TEXSIZE_32x64)
#endif
#if (GFL_BBD_TEXSIZDEF_32x128-TEXSIZE_32x128)
#error (GFL_BBD_TEXSIZDEF_32x128-TEXSIZE_32x128)
#endif
#if (GFL_BBD_TEXSIZDEF_32x256-TEXSIZE_32x256)
#error (GFL_BBD_TEXSIZDEF_32x256-TEXSIZE_32x256)
#endif
#if (GFL_BBD_TEXSIZDEF_32x512-TEXSIZE_32x512)
#error (GFL_BBD_TEXSIZDEF_32x512-TEXSIZE_32x512)
#endif
#if (GFL_BBD_TEXSIZDEF_32x1024-TEXSIZE_32x1024)
#error (GFL_BBD_TEXSIZDEF_32x1024-TEXSIZE_32x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_64x8-TEXSIZE_64x8)
#error (GFL_BBD_TEXSIZDEF_64x8-TEXSIZE_64x8)
#endif
#if (GFL_BBD_TEXSIZDEF_64x16-TEXSIZE_64x16)
#error (GFL_BBD_TEXSIZDEF_64x16-TEXSIZE_64x16)
#endif
#if (GFL_BBD_TEXSIZDEF_64x32-TEXSIZE_64x32)
#error (GFL_BBD_TEXSIZDEF_64x32-TEXSIZE_64x32)
#endif
#if (GFL_BBD_TEXSIZDEF_64x64-TEXSIZE_64x64)
#error (GFL_BBD_TEXSIZDEF_64x64-TEXSIZE_64x64)
#endif
#if (GFL_BBD_TEXSIZDEF_64x128-TEXSIZE_64x128)
#error (GFL_BBD_TEXSIZDEF_64x128-TEXSIZE_64x128)
#endif
#if (GFL_BBD_TEXSIZDEF_64x256-TEXSIZE_64x256)
#error (GFL_BBD_TEXSIZDEF_64x256-TEXSIZE_64x256)
#endif
#if (GFL_BBD_TEXSIZDEF_64x512-TEXSIZE_64x512)
#error (GFL_BBD_TEXSIZDEF_64x512-TEXSIZE_64x512)
#endif
#if (GFL_BBD_TEXSIZDEF_64x1024-TEXSIZE_64x1024)
#error (GFL_BBD_TEXSIZDEF_64x1024-TEXSIZE_64x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_128x8-TEXSIZE_128x8)
#error (GFL_BBD_TEXSIZDEF_128x8-TEXSIZE_128x8)
#endif
#if (GFL_BBD_TEXSIZDEF_128x16-TEXSIZE_128x16)
#error (GFL_BBD_TEXSIZDEF_128x16-TEXSIZE_128x16)
#endif
#if (GFL_BBD_TEXSIZDEF_128x32-TEXSIZE_128x32)
#error (GFL_BBD_TEXSIZDEF_128x32-TEXSIZE_128x32)
#endif
#if (GFL_BBD_TEXSIZDEF_128x64-TEXSIZE_128x64)
#error (GFL_BBD_TEXSIZDEF_128x64-TEXSIZE_128x64)
#endif
#if (GFL_BBD_TEXSIZDEF_128x128-TEXSIZE_128x128)
#error (GFL_BBD_TEXSIZDEF_128x128-TEXSIZE_128x128)
#endif
#if (GFL_BBD_TEXSIZDEF_128x256-TEXSIZE_128x256)
#error (GFL_BBD_TEXSIZDEF_128x256-TEXSIZE_128x256)
#endif
#if (GFL_BBD_TEXSIZDEF_128x512-TEXSIZE_128x512)
#error (GFL_BBD_TEXSIZDEF_128x512-TEXSIZE_128x512)
#endif
#if (GFL_BBD_TEXSIZDEF_128x1024-TEXSIZE_128x1024)
#error (GFL_BBD_TEXSIZDEF_128x1024-TEXSIZE_128x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_256x8-TEXSIZE_256x8)
#error (GFL_BBD_TEXSIZDEF_256x8-TEXSIZE_256x8)
#endif
#if (GFL_BBD_TEXSIZDEF_256x16-TEXSIZE_256x16)
#error (GFL_BBD_TEXSIZDEF_256x16-TEXSIZE_256x16)
#endif
#if (GFL_BBD_TEXSIZDEF_256x32-TEXSIZE_256x32)
#error (GFL_BBD_TEXSIZDEF_256x32-TEXSIZE_256x32)
#endif
#if (GFL_BBD_TEXSIZDEF_256x64-TEXSIZE_256x64)
#error (GFL_BBD_TEXSIZDEF_256x64-TEXSIZE_256x64)
#endif
#if (GFL_BBD_TEXSIZDEF_256x128-TEXSIZE_256x128)
#error (GFL_BBD_TEXSIZDEF_256x128-TEXSIZE_256x128)
#endif
#if (GFL_BBD_TEXSIZDEF_256x256-TEXSIZE_256x256)
#error (GFL_BBD_TEXSIZDEF_256x256-TEXSIZE_256x256)
#endif
#if (GFL_BBD_TEXSIZDEF_256x512-TEXSIZE_256x512)
#error (GFL_BBD_TEXSIZDEF_256x512-TEXSIZE_256x512)
#endif
#if (GFL_BBD_TEXSIZDEF_256x1024-TEXSIZE_256x1024)
#error (GFL_BBD_TEXSIZDEF_256x1024-TEXSIZE_256x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_512x8-TEXSIZE_512x8)
#error (GFL_BBD_TEXSIZDEF_512x8-TEXSIZE_512x8)
#endif
#if (GFL_BBD_TEXSIZDEF_512x16-TEXSIZE_512x16)
#error (GFL_BBD_TEXSIZDEF_512x16-TEXSIZE_512x16)
#endif
#if (GFL_BBD_TEXSIZDEF_512x32-TEXSIZE_512x32)
#error (GFL_BBD_TEXSIZDEF_512x32-TEXSIZE_512x32)
#endif
#if (GFL_BBD_TEXSIZDEF_512x64-TEXSIZE_512x64)
#error (GFL_BBD_TEXSIZDEF_512x64-TEXSIZE_512x64)
#endif
#if (GFL_BBD_TEXSIZDEF_512x128-TEXSIZE_512x128)
#error (GFL_BBD_TEXSIZDEF_512x128-TEXSIZE_512x128)
#endif
#if (GFL_BBD_TEXSIZDEF_512x256-TEXSIZE_512x256)
#error (GFL_BBD_TEXSIZDEF_512x256-TEXSIZE_512x256)
#endif
#if (GFL_BBD_TEXSIZDEF_512x512-TEXSIZE_512x512)
#error (GFL_BBD_TEXSIZDEF_512x512-TEXSIZE_512x512)
#endif
#if (GFL_BBD_TEXSIZDEF_512x1024-TEXSIZE_512x1024)
#error (GFL_BBD_TEXSIZDEF_512x1024-TEXSIZE_512x1024)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x8-TEXSIZE_1024x8)
#error (GFL_BBD_TEXSIZDEF_1024x8-TEXSIZE_1024x8)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x16-TEXSIZE_1024x16)
#error (GFL_BBD_TEXSIZDEF_1024x16-TEXSIZE_1024x16)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x32-TEXSIZE_1024x32)
#error (GFL_BBD_TEXSIZDEF_1024x32-TEXSIZE_1024x32)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x64-TEXSIZE_1024x64)
#error (GFL_BBD_TEXSIZDEF_1024x64-TEXSIZE_1024x64)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x128-TEXSIZE_1024x128)
#error (GFL_BBD_TEXSIZDEF_1024x128-TEXSIZE_1024x128)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x256-TEXSIZE_1024x256)
#error (GFL_BBD_TEXSIZDEF_1024x256-TEXSIZE_1024x256)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x512-TEXSIZE_1024x512)
#error (GFL_BBD_TEXSIZDEF_1024x512-TEXSIZE_1024x512)
#endif
#if (GFL_BBD_TEXSIZDEF_1024x1024-TEXSIZE_1024x1024)
#error (GFL_BBD_TEXSIZDEF_1024x1024-TEXSIZE_1024x1024)
#endif
#endif //MMDL_CHECK_GFL_BBD_TEXSIZDEF
