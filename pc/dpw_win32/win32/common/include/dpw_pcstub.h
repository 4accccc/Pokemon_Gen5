/*! @file
	@brief	DPWのWIN32版でDSのDWCを使えるようにするためのスタブコード
	
	@author	nakamud

	@version 1.00	initial release.
*/


#ifndef DPW_PCSTUB_H_
#define DPW_PCSTUB_H_


#ifdef __cplusplus
extern "C" {
#endif





//----------------------------------------------------------------------------
// enum
//----------------------------------------------------------------------------
// エラー列挙子
typedef enum {
    DWC_ERROR_NONE = 0,            // エラーなし
    DWC_ERROR_DS_MEMORY_ANY,       // 本体セーブデータに関するエラー
    DWC_ERROR_AUTH_ANY,            // 認証に関するエラー
    DWC_ERROR_AUTH_OUT_OF_SERVICE, // サービス終了
    DWC_ERROR_AUTH_STOP_SERVICE,   // サービス一時中断
    DWC_ERROR_AC_ANY,              // 自動接続に関するエラー
    DWC_ERROR_NETWORK,             // その他のネットワークエラー
    DWC_ERROR_DISCONNECTED,        // 切断された
    DWC_ERROR_FATAL,               // 致命的なエラー

    // [arakit] main 051011
    // シーケンス上の軽度のエラー（DWCErrorType = DWC_ETYPE_LIGHT）
    DWC_ERROR_FRIENDS_SHORTAGE,    // 指定の人数の有効な友達がいないのに友達指定ピアマッチメイクを始めようとした
    DWC_ERROR_NOT_FRIEND_SERVER,   // サーバクライアントマッチメイクで、サーバに指定した友達が相互に友達ではない、もしくはサーバを立てていない
    DWC_ERROR_MO_SC_CONNECT_BLOCK, // サーバクライアントマッチメイクのマッチメイクオプションで、サーバから接続を拒否された
    // [arakit] main 051011
    // [arakit] main 051024
    DWC_ERROR_SERVER_FULL,         // サーバクライアントマッチメイクのサーバが定員オーバー
        
    DWC_ERROR_NUM
} DWCError;

// [arakit] main 051010
// エラー検出時に必要な処理を示すエラータイプ
typedef enum {
    DWC_ETYPE_NO_ERROR = 0,  // エラーなし
    DWC_ETYPE_LIGHT,         // ゲーム固有の表示のみで、エラーコード表示は必要ない
    DWC_ETYPE_SHUTDOWN_FM,   // DWC_ShutdownFriendsMatch()を呼び出して、FriendsMatchライブラリを終了する必要がある。
                             // エラーコードの表示も必要。
    DWC_ETYPE_DISCONNECT,    // FriendsMatch処理中ならDWC_ShutdownFriendsMatch()を呼び出し、更にDWC_CleanupInet()で通信の切断も行う必要がある。
                             //エラーコードの表示も必要。
    DWC_ETYPE_FATAL,         // Fatal Error 相当なので、電源OFFを促す必要がある。
                             // エラーコードの表示も必要。
    DWC_ETYPE_NUM
} DWCErrorType;
// [arakit] main 051010

// デバッグ用dwc_baseエラーコード各要素
enum {
    // どのシーケンス中に起こったエラーか
    DWC_ECODE_SEQ_LOGIN          = (-60000),  // ログイン処理でのエラー
    DWC_ECODE_SEQ_FRIEND         = (-70000),  // 友達管理処理でのエラー
    DWC_ECODE_SEQ_MATCH          = (-80000),  // マッチメイク処理でのエラー
    DWC_ECODE_SEQ_ETC            = (-90000),  // 上記以外の処理でのエラー

    // GameSpyのエラーの場合、どのSDKが出したエラーか
    DWC_ECODE_GS_GP              =  (-1000),  // GameSpy GP のエラー
    DWC_ECODE_GS_PERS            =  (-2000),  // GameSpy Persistent のエラー
    DWC_ECODE_GS_STATS           =  (-3000),  // GameSpy Stats のエラー
    DWC_ECODE_GS_QR2             =  (-4000),  // GameSpy QR2 のエラー
    DWC_ECODE_GS_SB              =  (-5000),  // GameSpy Server Browsing のエラー
    DWC_ECODE_GS_NN              =  (-6000),  // GameSpy Nat Negotiation のエラー
    DWC_ECODE_GS_GT2             =  (-7000),  // GameSpy gt2 のエラー
    DWC_ECODE_GS_HTTP            =  (-8000),  // GameSpy HTTP のエラー
    DWC_ECODE_GS_ETC             =  (-9000),  // GameSpy その他のSDKのエラー

    // 通常エラー種別
    DWC_ECODE_TYPE_NETWORK       =   ( -10),  // ネットワーク障害
    DWC_ECODE_TYPE_SERVER        =   ( -20),  // GameSpyサーバ障害
    DWC_ECODE_TYPE_DNS           =   ( -30),  // DNS障害
    DWC_ECODE_TYPE_DATA          =   ( -40),  // 不正なデータを受信
    DWC_ECODE_TYPE_SOCKET        =   ( -50),  // ソケット通信エラー
    DWC_ECODE_TYPE_BIND          =   ( -60),  // ソケットのバインドエラー
    DWC_ECODE_TYPE_TIMEOUT       =   ( -70),  // タイムアウト発生
    DWC_ECODE_TYPE_PEER          =   ( -80),  // １対１の通信で不具合
    DWC_ECODE_TYPE_CONN_OVER     =   (-100),  // コネクション数オーバ
    DWC_ECODE_TYPE_STATS_AUTH    =   (-200),  // STATSサーバログインエラー
    DWC_ECODE_TYPE_STATS_LOAD    =   (-210),  // STATSサーバデータロードエラー
    DWC_ECODE_TYPE_STATS_SAVE    =   (-220),  // STATSサーバデータセーブエラー
    DWC_ECODE_TYPE_NOT_FRIEND    =   (-400),  // 指定された相手が相互に登録された友達ではない
    DWC_ECODE_TYPE_OTHER         =   (-410),  // 相手が通信切断した（主にサーバクライアントマッチメイク時）
    DWC_ECODE_TYPE_MUCH_FAILURE  =   (-420),  // NATネゴシエーションに規定回数以上失敗した
    DWC_ECODE_TYPE_SC_CL_FAIL    =   (-430),  // サーバクライアントマッチメイク・クライアントの接続失敗
    DWC_ECODE_TYPE_CLOSE         =   (-600),  // コネクションクローズ時のエラー
    DWC_ECODE_TYPE_TRANS_HEADER  =   (-610),  // ありえない受信ステートでReliableデータを受信した
    DWC_ECODE_TYPE_TRANS_BODY    =   (-620),  // 受信バッファオーバーフロー
    DWC_ECODE_TYPE_AC_FATAL      =   (-700),  // AC処理中のACでのエラーグループ。この場合下位桁は専用のstate値。
    DWC_ECODE_TYPE_OPEN_FILE     =   (-800),  // GHTTPファイルオープン失敗
    DWC_ECODE_TYPE_INVALID_POST  =   (-810),  // GHTTP無効な送信
    DWC_ECODE_TYPE_REQ_INVALID   =   (-820),  // GHTTPファイル名等無効
    DWC_ECODE_TYPE_UNSPECIFIED   =   (-830),  // GHTTP詳細不明のエラー
    DWC_ECODE_TYPE_BUFF_OVER     =   (-840),  // GHTTPバッファオーバーフロー
    DWC_ECODE_TYPE_PARSE_URL     =   (-850),  // GHTTP URLの解析エラー
    DWC_ECODE_TYPE_BAD_RESPONSE  =   (-860),  // GHTTPサーバからのレスポンス解析エラー
    DWC_ECODE_TYPE_REJECTED      =   (-870),  // GHTTPサーバからのリクエスト等拒否
    DWC_ECODE_TYPE_FILE_RW       =   (-880),  // GHTTPローカルファイルRead/Writeエラー
    DWC_ECODE_TYPE_INCOMPLETE    =   (-890),  // GHTTPダウンロード中断
    DWC_ECODE_TYPE_TO_BIG        =   (-900),  // GHTTPファイルサイズが大きすぎてダウンロード不可
    DWC_ECODE_TYPE_ENCRYPTION    =   (-910),  // GHTTPエンクリプションエラー

    // Fatal Error専用種別
    DWC_ECODE_TYPE_ALLOC         =     (-1),  // メモリ確保失敗
    DWC_ECODE_TYPE_PARAM         =     (-2),  // パラメータエラー
    DWC_ECODE_TYPE_SO_SOCKET     =     (-3),  // SOのエラーが原因のGameSpy gt2のソケットエラー
    DWC_ECODE_TYPE_NOT_INIT      =     (-4),  // ライブラリが初期化されていない。
    DWC_ECODE_TYPE_DUP_INIT      =     (-5),  // ライブラリが二度初期化された。
    DWC_ECODE_TYPE_WM_INIT       =     (-6),  // WMの初期化に失敗した。
    DWC_ECODE_TYPE_UNEXPECTED    =     (-9)   // 予期しない状態、もしくはUnkwonなGameSpyエラー発生
};







    

#define DWC_Printf( dummyarg, fmt, ... )	printf( fmt, __VA_ARGS__ )
#define OS_TVPrintf( fmt, vlist )			vprintf( fmt, vlist )
BOOL DWCi_IsError(void);
void DWCi_SetError(DWCError error, int errorCode);
void DWC_ClearError(void);

#define OS_Sleep		Sleep
#define OS_SPrintf		sprintf

typedef enum
{
    DWC_ALLOCTYPE_AUTH,
    DWC_ALLOCTYPE_AC,
    DWC_ALLOCTYPE_BM,
    DWC_ALLOCTYPE_UTIL,
    DWC_ALLOCTYPE_BASE,
    DWC_ALLOCTYPE_GS,
    DWC_ALLOCTYPE_LAST
} DWCAllocType;
    
void* DWC_Alloc     ( DWCAllocType name, u32 size );
void  DWC_Free      ( DWCAllocType name, void* ptr, u32 size );

void Debug_MemBrowse_begin();
void Debug_MemBrowse_end();

void MI_CpuClear8(void* ptr, int size);


typedef struct
{
    u32     x;                         // 乱数値
    u32     mul;                       // 乗数
    u32     add;                       // 加算する数
}
MATHRandContext16;

typedef struct
{
    u64     x;                         // 乱数値
    u64     mul;                       // 乗数
    u64     add;                       // 加算する数
}
MATHRandContext32;

void MATH_InitRand16(MATHRandContext16 *context, u32 seed);
u16 MATH_Rand16(MATHRandContext16 *context, u16 max);
void MATH_InitRand32(MATHRandContext32 *context, u64 seed);
u32 MATH_Rand32(MATHRandContext32 *context, u32 max);


#ifdef __cplusplus
}
#endif


#endif // DPW_PCSTUB_H_
