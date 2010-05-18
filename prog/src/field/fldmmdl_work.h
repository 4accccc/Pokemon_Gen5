//======================================================================
/**
 * @file  fldmmdl_work.h
 * @brief  動作モデル ワーク定義
 * @author  kagaya
 * @date  05.07.13
 */
//======================================================================
#ifdef MMDL_SYSTEM_FILE
#ifndef MMDL_WORK_H_FILE
#define MMDL_WORK_H_FILE

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
//  MMDL 動作、描画関数ワークサイズ (byte size)
//--------------------------------------------------------------
#define MMDL_MOVE_WORK_SIZE    (16)  ///<動作関数用ワークサイズ
#define MMDL_MOVE_SUB_WORK_SIZE  (16)  ///<動作サブ関数用ワークサイズ
#define MMDL_MOVE_CMD_WORK_SIZE  (16)  ///<動作コマンド用ワークサイズ
#define MMDL_DRAW_WORK_SIZE    (32)  ///<描画関数用ワークサイズ

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///  MMDLSYS構造体
//--------------------------------------------------------------
struct _TAG_MMDLSYS
{
  u32 status_bit;          ///<ステータスビット
  
  u16 mmdl_max;          ///<MMDL最大数
  s16 mmdl_count;        ///<フィールド動作モデル現在数
  
  HEAPID sysHeapID;        ///<システム用 ヒープID
  HEAPID heapID;          ///<ヒープID
  
  s16 tcb_pri;          ///<TCBプライオリティ
  u8 padding0;
  u8 padding1;
  
  ARCHANDLE *arcH_res; ///<動作モデルリソースアーカイブハンドル
  ARCHANDLE *arcH_param; ///<動作モデルパラメタアーカイブハンドル
  
  MMDL *pMMdlBuf;      ///<MMDLワーク *
  void *pTCBSysWork;        ///<TCBワーク
  GFL_TCBSYS *pTCBSys;      ///<TCBSYS*
  
  MMDL_BLACTCONT *pBlActCont;  ///<MMDL_BLACTCONT
  MMDL_G3DOBJCONT *pObjCont; ///<MMDL_G3DOBJCONT
  
  MMDL_ROCKPOS *rockpos; ///<かいりき岩座標 セーブデータポインタ

#if 0  
  u8 *pOBJCodeParamBuf;      ///<OBJCODE_PARAMバッファ
  const OBJCODE_PARAM *pOBJCodeParamTbl; ///<OBJCODE_PARAM
#endif

  GAMEDATA *gdata; ///<GAMEDATA
  const FLDMAPPER *pG3DMapper;  ///<FLDMAPPER
  FLDNOGRID_MAPPER *pNOGRIDMapper;  ///<FLDNOGRID_MAPPER
  void *fieldMapWork; ///<FIELDMAP_WORK <-各ワーク単位での接続が良い。
  const u16 *targetCameraAngleYaw; //グローバルで参照するカメラ

  void *pMdlPrmWork;    //動作モデルパラメータポインタ
};

#define MMDLSYS_SIZE (sizeof(MMDLSYS)) ///<MMDLSYSサイズ

//--------------------------------------------------------------
///  MMDL構造体
//--------------------------------------------------------------
struct _TAG_MMDL
{
  u32 status_bit;        ///<ステータスビット
  u32 move_bit;        ///<動作ビット
  
  u16 obj_id;          ///<OBJ ID
  u16 zone_id;        ///<ゾーン ID
  u16 obj_code;        ///<OBJコード
  u16 move_code;        ///<動作コード
  u16 event_type;        ///<イベントタイプ
  u16 event_flag;        ///<イベントフラグ
  u16 event_id;        ///<イベントID
  u16 dir_head;        ///<MMDL_H指定方向
  u16 dir_disp;        ///<現在向いている方向
  u16 dir_move;        ///<現在動いている方向
  u16 dir_disp_old;      ///<過去の動いていた方向
  u16 dir_move_old;      ///<過去の動いていた方向
  
  u16 param0;          ///<ヘッダ指定パラメタ
  u16 param1;          ///<ヘッダ指定パラメタ
  u16 param2;          ///<ヘッダ指定パラメタ
  
  u16 acmd_code;        ///<アニメーションコマンドコード
  u16 acmd_seq;        ///<アニメーションコマンドシーケンス
  u16 draw_status;      ///<描画ステータス
  s16 move_limit_x;      ///<X方向移動制限
  s16 move_limit_z;      ///<Z方向移動制限
  s16 gx_init;        ///<初期グリッドX
  s16 gy_init;        ///<初期グリッドY
  s16 gz_init;        ///<初期グリッドZ
  s16 gx_old;          ///<過去グリッドX
  s16 gy_old;          ///<過去グリッドY
  s16 gz_old;          ///<過去グリッドZ
  s16 gx_now;          ///<現在グリッドX
  s16 gy_now;          ///<現在グリッドY
  s16 gz_now;          ///<現在グリッドZ
  VecFx32 vec_pos_now;    ///<現在実数座標
  VecFx32 vec_draw_offs;    ///<表示座標オフセット
  VecFx32 vec_draw_offs_outside;  ///<外部指定表示座標オフセット
  VecFx32 vec_attr_offs;    ///<アトリビュートによる座標オフセット
  u32 now_attr;        ///<現在のマップアトリビュート
  u32 old_attr;        ///<過去のマップアトリビュート
  
  u8 gx_size; ///<グリッドX方向サイズ
  u8 gz_size; ///<グリッドZ方向サイズ
  s8 offset_x; ///<オフセットX
  s8 offset_y; ///<オフセットY
  
  s8 offset_z; ///<オフセットZ
  u8 padding[3]; ///<4byte余り
  
  GFL_TCB *pTCB;        ///<動作関数TCB*
  MMDLSYS *pMMdlSys;    ///<MMDLSYS*
  
  const MMDL_MOVE_PROC_LIST *move_proc_list; ///<動作関数リスト
  const MMDL_DRAW_PROC_LIST *draw_proc_list; ///<描画関数リスト
  
  u8 move_proc_work[MMDL_MOVE_WORK_SIZE];///動作関数用ワーク
  u8 move_sub_proc_work[MMDL_MOVE_SUB_WORK_SIZE];///動作サブ関数用ワーク
  u8 move_cmd_proc_work[MMDL_MOVE_CMD_WORK_SIZE];///動作コマンド用ワーク
  u8 draw_proc_work[MMDL_DRAW_WORK_SIZE];///描画関数用ワーク
  
  ///obj_codeに設定されているOBJCODE_PARAM 基本は閲覧のみで操作禁止
  OBJCODE_PARAM objcode_param;
};

#define MMDL_SIZE (sizeof(MMDL)) ///<MMDLサイズ 224

#endif //MMDL_WORK_H_FILE
#endif //MMDL_SYSTEM_FILE
