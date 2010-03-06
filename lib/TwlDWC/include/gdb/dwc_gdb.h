/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_GDB_H_
#define DWC_GDB_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <dwc.h>
#include <nonport/dwc_nonport.h>

    /* -------------------------------------------------------------------------
            define
       ------------------------------------------------------------------------- */
    /**
     *  ファイルアップロードの際に内部的に付加されるヘッダサイズ
     *
     */
#define DWC_GDB_UPLOADHEADER_SIZE 178


    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */
    
    /// フィールドの型を示す列挙型です。
    typedef enum
    {
        DWC_GDB_FIELD_TYPE_BYTE,            ///< 1バイトの符号なし整数です。
        DWC_GDB_FIELD_TYPE_SHORT,           ///< 2バイトの符号付き整数です。
        DWC_GDB_FIELD_TYPE_INT,             ///< 4バイトの符号付き整数です。
        DWC_GDB_FIELD_TYPE_FLOAT,           ///< 8バイトの浮動小数点数です。
        DWC_GDB_FIELD_TYPE_ASCII_STRING,    ///< 一文字1バイトの文字列です。
        DWC_GDB_FIELD_TYPE_UNICODE_STRING,  ///< 一文字2バイトの文字列です。
        DWC_GDB_FIELD_TYPE_BOOLEAN,         ///< 真偽値です。
        DWC_GDB_FIELD_TYPE_DATE_AND_TIME,   ///< UnixTimeで表される時間です。
        DWC_GDB_FIELD_TYPE_BINARY_DATA,     ///< バイナリデータです。
        DWC_GDB_FIELD_TYPE_INT64,           ///< 8バイトの符号付き整数です。
        DWC_GDB_FIELD_TYPE_NUM_FIELD_TYPES
    } DWCGdbFieldType;

    /**
     *  エラーを示す列挙型です。
     *  
     */
    typedef enum
    {
        DWC_GDB_ERROR_NONE = 0,                 ///< 正常に終了しました。
        DWC_GDB_ERROR_OUT_OF_MEMORY = 1,        ///< DWC_Initで与えたアロケータからメモリを確保できませんでした。
        DWC_GDB_ERROR_NOT_AVAILABLE = 2,        ///<
        DWC_GDB_ERROR_CORE_SHUTDOWN = 3,        ///<
        DWC_GDB_ERROR_NOT_LOGIN = 4,            ///< WiFiConnectionに正しくログインしていません。
        DWC_GDB_ERROR_NOT_AUTHENTICATED = 5,    ///< WiFiConnectionに正しくログインしていません。
        DWC_GDB_ERROR_BAD_INPUT = 6,            ///<
        DWC_GDB_ERROR_BAD_TABLENAME = 7,        ///<
        DWC_GDB_ERROR_BAD_FIELDS = 8,           ///<
        DWC_GDB_ERROR_BAD_NUM_FIELDS = 9,       ///<
        DWC_GDB_ERROR_BAD_FIELD_NAME = 10,      ///<
        DWC_GDB_ERROR_BAD_FIELD_TYPE = 11,      ///<
        DWC_GDB_ERROR_BAD_FIELD_VALUE = 12,     ///<
        DWC_GDB_ERROR_BAD_OFFSET = 13,          ///<
        DWC_GDB_ERROR_BAD_MAX = 14,             ///<
        DWC_GDB_ERROR_BAD_RECORDIDS = 15,       ///<
        DWC_GDB_ERROR_BAD_NUM_RECORDIDS = 16,   ///<
        DWC_GDB_ERROR_UNKNOWN = 17,             ///<
        DWC_GDB_ERROR_ALREADY_INITIALIZED = 18, ///< 既に初期化されています。
        DWC_GDB_ERROR_NOT_INITIALIZED = 19,     ///< 初期化が行われていません。
        DWC_GDB_ERROR_IN_ASYNC_PROCESS = 20,    ///< 非同期処理中です。
        DWC_GDB_ERROR_GHTTP = 21,               ///< GHTTPのエラーが発生しました。
        DWC_GDB_ERROR_IN_DWC_ERROR = 22,        ///< DWCのエラーが起こっています。エラーの対処を行ってください。
        DWC_GDB_ERROR_IN_CANCEL_PROCESS = 23    ///< 非同期処理のキャンセル中です。
    } DWCGdbError;

    /**
     *  非同期処理の結果を示す列挙型です。
     *  
     */
    typedef enum
    {
        DWC_GDB_ASYNC_RESULT_NONE = 0,                  ///< 非同期処理がまだ終了していません。
        DWC_GDB_ASYNC_RESULT_SUCCESS = 1,               ///< 成功しました。
        DWC_GDB_ASYNC_RESULT_SECRET_KEY_INVALID = 2,
        DWC_GDB_ASYNC_RESULT_SERVICE_DISABLED = 3,
        DWC_GDB_ASYNC_RESULT_CONNECTION_TIMEOUT = 4,
        DWC_GDB_ASYNC_RESULT_CONNECTION_ERROR = 5,
        DWC_GDB_ASYNC_RESULT_MALFORMED_RESPONSE = 6,
        DWC_GDB_ASYNC_RESULT_OUT_OF_MEMORY = 7,
        DWC_GDB_ASYNC_RESULT_DATABASE_UNAVAILABLE = 8,
        DWC_GDB_ASYNC_RESULT_LOGIN_TICKET_INVALID = 9,
        DWC_GDB_ASYNC_RESULT_LOGIN_TICKET_EXPIRED = 10,
        DWC_GDB_ASYNC_RESULT_TABLE_NOT_FOUND = 11,
        DWC_GDB_ASYNC_RESULT_RECORD_NOT_FOUND = 12,
        DWC_GDB_ASYNC_RESULT_FIELD_NOT_FOUND = 13,
        DWC_GDB_ASYNC_RESULT_FIELD_TYPE_INVALID = 14,
        DWC_GDB_ASYNC_RESULT_NO_PERMISSION = 15,
        DWC_GDB_ASYNC_RESULT_RECORD_LIMIT_REACHED = 16,
        DWC_GDB_ASYNC_RESULT_ALREADY_RATED = 17,
        DWC_GDB_ASYNC_RESULT_NOT_RATEABLE = 18,
        DWC_GDB_ASYNC_RESULT_NOT_OWNED = 19,
        DWC_GDB_ASYNC_RESULT_FILTER_INVALID = 20,
        DWC_GDB_ASYNC_RESULT_SORT_INVALID = 21,
        DWC_GDB_ASYNC_RESULT_UNKNOWN_ERROR = 22,
        DWC_GDB_ASYNC_RESULT_BAD_HTTP_METHOD = 23,
        DWC_GDB_ASYNC_RESULT_BAD_FILE_COUNT = 24,
        DWC_GDB_ASYNC_RESULT_MISSING_PARAMETER = 25,
        DWC_GDB_ASYNC_RESULT_FILE_NOT_FOUND = 26,
        DWC_GDB_ASYNC_RESULT_FILE_TOO_LARGE = 27,
        DWC_GDB_ASYNC_RESULT_FILE_UNKNOWN_ERROR = 28,
        DWC_GDB_ASYNC_RESULT_GHTTP_ERROR = 29,          ///< GHTTPのエラーが発生しました。
        DWC_GDB_ASYNC_RESULT_TARGET_FILTER_INVALID = 30,
        DWC_GDB_ASYNC_RESULT_SERVER_ERROR = 31,
        DWC_GDB_ASYNC_RESULT_REQUEST_CANCELLED = 32
    } DWCGdbAsyncResult;

    /**
     *  DWCGdbライブラリの状態を示す列挙型です。
     *  
     */
    typedef enum
    {
        DWC_GDB_STATE_UNINITIALIZED,    ///< 未初期化状態
        DWC_GDB_STATE_IDLE,             ///< 初期化が終了し、非同期処理が行われていない状態
        DWC_GDB_STATE_IN_ASYNC_PROCESS, ///< 非同期処理を行っている状態
        DWC_GDB_STATE_IN_CANCEL_PROCESS,///< 非同期処理のキャンセルを行っている状態
        DWC_GDB_STATE_ERROR_OCCURED     ///< エラーが発生した状態
    } DWCGdbState;


    /* -------------------------------------------------------------------------
            struct
       ------------------------------------------------------------------------- */

    /// バイナリデータ型の値です。
    typedef struct
    {
        u8*     data;       ///< データの内容です。この領域はコールバックを抜けると解放されます。
        s32     size;       ///< データのサイズです。
    }
    DWCGdbBinaryData;

    /// フィールドの値を示します。
    typedef union
    {
        u8      int_u8;         ///< 1バイトの符号なし整数です。
        s16     int_s16;        ///< 2バイトの符号付き整数です。
        s32     int_s32;        ///< 4バイトの符号付き整数です。
        f32     float_f64;      ///< 8バイトの浮動小数点数です。
        char*   ascii_string;   ///< 一文字1バイトの文字列です。
        u16*    wide_string;    ///< 一文字2バイトの文字列です。
        int     boolean;        ///< 真偽値です。
        DWCTick datetime;       ///< UnixTimeで表される時間です。
        DWCGdbBinaryData    binary_data;	///< バイナリデータです。
        s64     int_s64;        ///< 8バイトの符号付き整数です。
    } DWCGdbValue;

    /// フィールドを示します。
    typedef struct
    {
        char*           name;   ///< フィールドの名前です。
        DWCGdbFieldType type;   ///< フィールドの型です。
        DWCGdbValue     value;  ///< フィールドの値です。
    }
    DWCGdbField;

    /// 検索の条件を示します。
    ///
    /// target_filter、surrounding_numを設定した場合、offsetとmaxは無視されます。
    ///
    /// ownerids、ownerid_numを設定した場合、offset、target_filter、surrounding_num は無視されます。
    /// また、設定した値は考慮されませんが、sortには有効な文字列 (例えば recordid )を、maxには 0 以外の正の値(例えば 1 )を、設定して下さ い。
    ///
    /// unicodeフィールドの検索で、sort,target_filterを指定する場合、DWC_GdbUCS2ToUTF8Str関数を使うなどして、マルチバイトのUTF-8文字列で指定する必要があります。
    typedef struct
    {
        char*   filter;         ///< SQLのWHERE句と同じ働きをします。NULLを指定した場合は指定しないことになります。
        char*   sort;           ///< SQLのORDER BY句と同じ働きをします。NULLを指定した場合は指定しないことになります。
        int     offset;         ///< 検索結果の何番目から取得するか指定します。
        int     max;            ///< 検索結果を最大いくつ取ってくるか指定します。
        char*   target_filter;  ///< surrounding_numを使用して周辺のレコードを取得するには、このフィルターをレコードが一つだけ検索されるように設定する必要があります。
        ///< 使用しない場合はNULLを与えることができます。
        int     surrounding_num; ///< target_filterと組み合わせて使用します。1以上の数を指定すると、検索結果のレコードの上下指定個数ぶんのレコードを一緒に取得します。
        int*    ownerids;       ///< 検索するレコードの作成者のプロファイルIDを配列として指定します。ownerid_numに0を指定した場合はNULLを与えることができます。
        int     ownerid_num;    ///< owneridsで指定した配列の長さを指定します。使用しない場合は0を入れてください。
        BOOL    cache_flag;     ///< 検索結果をサーバでキャッシュするか否かを示します。キャッシュ期間は5分です。サーバの負荷を軽減するためにも、データの更新期間を短くしなければいけない場合を除いて、このフラグは有効にするようにしてください。
    }
    DWCGdbSearchCond;

    /* -------------------------------------------------------------------------
            typedef - callback
       ------------------------------------------------------------------------- */

    /**
     *  非同期処理の結果、レコードを取得する際のコールバックです。
     *
     *  引数で与えられるフィールドはコールバックを抜けると解放されてしまいますので、
     *  必要なデータはコールバック内でコピーするようにしてください。
     *
     *  Param:  record_num   レコードの数です。
     *  Param:  records      レコードの配列です。
     *  Param:  user_param   非同期処理の実行時に与えたパラメータです。
    */
    typedef void (*DWCGdbGetRecordsCallback)(int record_num, DWCGdbField** records, void* user_param);


    /**
     *  ファイル送受信の進行状況取得コールバックです。
     *  
     *  ファイル送受信の進行状況取得コールバックです。
     *  ファイルアップロードの際は、総送信サイズが、DWC_GDB_UPLOADHEADER_SIZE + (ファイル名の長さ)*2 だけ増えますので注意して下さい。
     *  
     *  Param:  bytesDone    送信/受信したデータの総バイト数
     *  Param:  totalBytes   総送受信サイズ。不明な場合は-1
     *  Param:  user_param   非同期処理の実行時に与えたパラメータです。
     *
     */
    typedef void (*DWCGdbProgressCallback)(int bytesDone, int totalBytes, void* user_param);

    /**
     *  非同期処理の結果、ファイルを取得する際のコールバックです。
     *
     *  DWC_GdbDownloadFileAsyncで引数clear_bufにTRUEを与えていた場合は、引数dataで与えられる領域はコールバックを抜けると解放されてしまいます。
     *  FALSEを与えていた場合は解放されませんので、解放してもよいタイミングになったら、DWC_Freeで解放してください。
     *
     *  Param:  data         取得したデータです。
     *  Param:  size         取得したデータサイズです。
     *  Param:  user_param   非同期処理の実行時に与えたパラメータです。
    */
    typedef void (*DWCGdbDownloadFileCallback)(const void* data, int size, void* user_param);


    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    //--- Auto Function Prototype --- Don't comment here.
    extern DWCGdbError DWC_GdbInitialize(int game_id, const DWCUserData* userdata, DWCSSLType ssl_type);
    extern DWCGdbError DWC_GdbCancelRequest();
    extern DWCGdbError DWC_GdbShutdown();
    extern void DWC_GdbProcess();
    extern DWCGdbState DWC_GdbGetState();
    extern DWCGdbAsyncResult DWC_GdbGetAsyncResult();
    extern DWCGdbField* DWC_GdbGetFieldByName(const char* field_name, DWCGdbField* fields, int field_num);
    extern DWCGdbError DWC_GdbCreateRecordAsync(const char* table_name, DWCGdbField* fields, int field_num, int* record_id);
    extern DWCGdbError DWC_GdbDeleteRecordAsync(const char* table_name, int record_id);
    extern DWCGdbError DWC_GdbUpdateRecordAsync(const char* table_name, int record_id, DWCGdbField* fields, int field_num);
    extern DWCGdbError DWC_GdbDownloadFileAsync(int file_id, int buf_size, BOOL clear_buf, DWCGdbProgressCallback progress_callback, DWCGdbDownloadFileCallback callback, void* user_param);
    extern DWCGdbError DWC_GdbUploadFileAsync(const void* data, int size, const char* file_name, int* file_id, DWCGdbProgressCallback progress_callback);
    extern DWCGdbError DWC_GdbGetMyRecordsAsync(const char* table_name, const char** field_names, int field_num, DWCGdbGetRecordsCallback callback, void* user_param);
    extern DWCGdbError DWC_GdbGetRecordLimitAsync(const char* table_name, int* limit_num, int* owned_num);
    extern DWCGdbError DWC_GdbGetRecordsAsync(const char* table_name, const int* record_ids, int record_num, const char** field_names, int field_num, DWCGdbGetRecordsCallback callback, void* user_param);
    extern DWCGdbError DWC_GdbRateRecordAsync(const char* table_name, int record_id, u8 rating);
    extern DWCGdbError DWC_GdbSearchRecordsAsync(const char* table_name, const char** field_names, int field_num, const DWCGdbSearchCond* search_cond, DWCGdbGetRecordsCallback callback, void* user_param);
    extern DWCGdbError DWC_GdbGetRandomRecordAsync(const char* table_name, const char** field_names, int field_num, const char* filter, DWCGdbGetRecordsCallback callback, void* user_param);
    extern DWCGdbError DWC_GdbGetRecordCountAsync(const char* table_name, const char* filter, BOOL cache_flag, int* count);
    extern int DWC_GdbUCS2ToUTF8Str(const u16* src, char* dst, const unsigned int dstlen);
    //--- End of Auto Function Prototype



#ifdef __cplusplus
}
#endif

#endif // DWC_GDB_H_
