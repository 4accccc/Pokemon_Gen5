//======================================================================
/**
 *
 * @file	fieldobj_header.h
 * @brief	フィールド動作モデルヘッダー構造体定義ヘッダー
 * @author	kagaya
 * @data	05.07.13
 *
 */
//======================================================================
#ifndef FLDMMDL_HEADER_FILE
#define FLDMMDL_HEADER_FILE

//======================================================================
//	typedef strcut
//======================================================================
//--------------------------------------------------------------
//	デバッグ
//--------------------------------------------------------------

//--------------------------------------------------------------
//	FLDMMDL 動作、描画関数ワークサイズ (byte size)
//--------------------------------------------------------------
#define FLDMMDL_MOVE_WORK_SIZE		(16)	///<動作関数用ワークサイズ
#define FLDMMDL_MOVE_SUB_WORK_SIZE	(16)	///<動作サブ関数用ワークサイズ
#define FLDMMDL_MOVE_CMD_WORK_SIZE	(16)	///<動作コマンド用ワークサイズ
#define FLDMMDL_DRAW_WORK_SIZE		(32)	///<描画関数用ワークサイズ

//--------------------------------------------------------------
///	FLDMMDL_HEADER構造体
//--------------------------------------------------------------
typedef struct
{
	unsigned short id;			///<識別ID
	unsigned short obj_code;	///<表示するOBJコード
	unsigned short move_code;	///<動作コード
	unsigned short event_type;	///<イベントタイプ
	unsigned short event_flag;	///<イベントフラグ
	unsigned short event_id;	///<イベントID
	short dir;					///<指定方向
	unsigned short param0;		///<指定パラメタ 0
	unsigned short param1;		///<指定パラメタ 1
	unsigned short param2;		///<指定パラメタ 2
	short move_limit_x;			///<X方向移動制限
	short move_limit_z;			///<Z方向移動制限
	unsigned short gx;			///<グリッドX
	unsigned short gz;			///<グリッドZ
	int gy;						///<Y値 fx32型
}FLDMMDL_HEADER;

//--------------------------------------------------------------
///	FLDMMDL_SAVEDATA構造体
//--------------------------------------------------------------
typedef struct
{
	u32 status_bit;			///<ステータスビット
	u32 move_bit;			///<動作ビット
	u8 obj_id;				///<OBJ ID
	u8 move_code;			///<動作コード
	s8 move_limit_x;		///<X方向移動制限
	s8 move_limit_z;		///<Z方向移動制限
	s8 dir_head;			///<FLDMMDL_H指定方向
	s8 dir_disp;			///<現在向いている方向
	s8 dir_move;			///<現在動いている方向
	u8 dummy;				///<ダミー
	u16 zone_id;			///<ゾーン ID
	u16 obj_code;			///<OBJコード
	u16 event_type;			///<イベントタイプ
	u16 event_flag;			///<イベントフラグ
	u16 event_id;			///<イベントID
	s16 param0;				///<ヘッダ指定パラメタ
	s16 param1;				///<ヘッダ指定パラメタ
	s16 param2;				///<ヘッダ指定パラメタ
	s16 gx_init;			///<初期グリッドX
	s16 gy_init;			///<初期グリッドY
	s16 gz_init;			///<初期グリッドZ
	s16 gx_now;				///<現在グリッドX
	s16 gy_now;				///<現在グリッドY
	s16 gz_now;				///<現在グリッドZ
	fx32 fx32_y;			///<fx32型の高さ値
	u8 move_proc_work[FLDMMDL_MOVE_WORK_SIZE];///<動作関数用ワーク
	u8 move_sub_proc_work[FLDMMDL_MOVE_SUB_WORK_SIZE];///<動作サブ関数用ワーク
}FLDMMDL_SAVEDATA;

///<FLDMMDL_SAVE_DATA size
#define FLDMMDL_SAVE_DATA_SIZE (sizeof(FLDMMDL_SAVE_DATA))

#endif //FLDMMDL_HEADER_FILE
