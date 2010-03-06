/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_ACCOUNT_H_
#define DWC_ACCOUNT_H_

#ifdef __cplusplus
extern "C"
{
#endif


    /* -------------------------------------------------------------------------
            define
       ------------------------------------------------------------------------- */

// FriendData用の定義
#define DWC_ACC_USERDATA_BUFSIZE            64

#define DWC_ACC_USERNAME_STRING_LENGTH      20
#define DWC_ACC_USERNAME_GSBRCD_OFFSET      9
#define DWC_ACC_USERNAME_STRING_BUFSIZE     (DWC_ACC_USERNAME_STRING_LENGTH + 1)
#define DWC_ACC_FRIENDKEY_STRING_LENGTH     12
#define DWC_ACC_FRIENDKEY_STRING_BUFSIZE    (DWC_ACC_FRIENDKEY_STRING_LENGTH + 1)

#define DWC_ACC_MASKBITS(bits)          (((1u<<bits)-1))

#define DWC_ACC_USERID_BITS             43
#define DWC_ACC_PLAYERID_BITS           32

#define DWC_ACC_USERID_HI32_SHIFT       0
#define DWC_ACC_USERID_HI32_BITS        11
#define DWC_ACC_USERID_HI32_MASK        DWC_ACC_MASKBITS(DWC_ACC_USERID_HI32_BITS)
#define DWC_ACC_FLAGS_SHIFT             11
#define DWC_ACC_FLAGS_BITS              21
#define DWC_ACC_FLAGS_MASK              DWC_ACC_MASKBITS(DWC_ACC_FLAGS_BITS)

#define DWC_ACC_FRIENDDATA_NODATA           0
#define DWC_ACC_FRIENDDATA_LOGIN_ID         1
#define DWC_ACC_FRIENDDATA_FRIEND_KEY       2
#define DWC_ACC_FRIENDDATA_GS_PROFILE_ID    3
#define DWC_ACC_FRIENDDATA_MASK             3

#define DWC_ACC_FRIENDDATA_ISBUDDY          (1u << 2)
#define DWC_ACC_FRIENDDATA_ISBUDDY_MASK     (1u << 2)
#define DWC_ACC_FRIENDDATA_IS_REVERSEBUDDY         (1u << 3)
#define DWC_ACC_FRIENDDATA_IS_REVERSEBUDDY_MASK    (1u << 3)
#define DWC_ACC_FRIENDDATA_EXP_ISBUDDYFRIEND       (1u << 4)
#define DWC_ACC_FRIENDDATA_EXP_ISBUDDYFRIEND_MASK  (1u << 4)
#define DWC_ACC_FRIENDDATA_SENT_BUDDYREQ           (1u << 5)
#define DWC_ACC_FRIENDDATA_SENT_BUDDYREQ_MASK      (1u << 5)

#define DWC_ACC_USERDATA_DIRTY                    (1u)
#define DWC_ACC_USERDATA_DIRTY_MASK               (1u)



//  公開定義
//------------------------------------------------------
#define DWC_FRIENDDATA_NODATA        DWC_ACC_FRIENDDATA_NODATA
#define DWC_FRIENDDATA_LOGIN_ID      DWC_ACC_FRIENDDATA_LOGIN_ID
#define DWC_FRIENDDATA_FRIEND_KEY    DWC_ACC_FRIENDDATA_FRIEND_KEY
#define DWC_FRIENDDATA_GS_PROFILE_ID DWC_ACC_FRIENDDATA_GS_PROFILE_ID
#define DWC_FRIENDKEY_STRING_BUFSIZE DWC_ACC_FRIENDKEY_STRING_BUFSIZE



    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */

    // 友達コードの生成アルゴリズム列挙型
    typedef enum
    {
        DWCi_FRIENDKEY_GENERATOR_CRC,   // CRCによる生成方法(旧バージョン)
        DWCi_FRIENDKEY_GENERATOR_MD5,   // MD5による生成方法(新バージョン 3.0以降)

        DWCi_FRIENDKEY_GENERATOR_NUM    // カウンタ
    } DWCiFriendKeyGenMethod;



    /* -------------------------------------------------------------------------
            struct
       ------------------------------------------------------------------------- */


    // ユーザデータ格納用の構造体
    //------------------------------------------------------

    // フラグチェック用の構造体
    typedef struct DWCstAccFlag
    {
        u32         flags;
        u32         reserved;
        u32         reserved1;
    } DWCAccFlag;

    // ログインID格納用の構造体
    typedef struct DWCstAccLoginId
    {
        u32         id_data;
        u32         userid_lo32;
        u32         playerid;
    } DWCAccLoginId;

    // 友達登録鍵用の構造体
    typedef struct DWCstAccFriendKey
    {
        u32         id_data;
        u32         friendkey_lo32;
        u32         friendkey_hi32;
    } DWCAccFriendKey;

    // GS Profile ID用の構造体
    typedef struct DWCstAccGsProfileId
    {
        u32         id_data;
        int         id;
        u32         reserved;
    } DWCAccGsProfileId;

    // 友達情報格納用の共用体 12Bytes
    typedef union DWCstAccFriendData
    {
        DWCAccFlag          flags;
        DWCAccLoginId       login_id;
        DWCAccFriendKey     friend_key;
        DWCAccGsProfileId   gs_profile_id;
    } DWCAccFriendData;

    // 自分のデータ格納用の構造体 64Bytes
    typedef struct DWCstAccUserData
    {
        int                 size;               // 4
        DWCAccLoginId       pseudo;             // 12
        DWCAccLoginId       authentic;          // 12
        int                 gs_profile_id;      // 4
        int                 flag;               // 4
        u32                 gamecode;           // 4

        int                 reserved[5];        // 20
        u32                 crc32;              // 4
    } DWCAccUserData;


    /**
     * ログインIDなどが含まれる、プレイヤーごとの個人データです。
     *
     * この構造体のメンバには直接アクセスしないようにしてください。
     *
     * See also:  DWCAccFriendData
     */
    typedef DWCAccUserData DWCUserData;


    /**
     * 友達情報格納用の共用体です。
     *
     * この共用体のメンバには直接アクセスしないようにしてください。
     *
     * See also:  DWCAccUserData
     */
    typedef DWCAccFriendData DWCFriendData;



    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */


    // ユーザデータの作成／確認用。
    //------------------------------------------------------
    void    DWC_CreateUserData            ( DWCUserData* userdata );
    BOOL    DWC_CheckUserData             ( const DWCUserData* userdata );
    BOOL    DWC_CheckValidConsole         ( const DWCUserData* userdata );
    BOOL    DWC_CheckHasProfile           ( const DWCUserData* userdata );
    BOOL    DWC_CheckDirtyFlag            ( const DWCUserData* userdata );
    void    DWC_ClearDirtyFlag            ( DWCUserData* userdata );


    // 友達関係
    //------------------------------------------------------
    u64     DWC_CreateFriendKey           ( const DWCUserData*   userdata );
    u64     DWC_CreateFriendKeyLegacy     ( const DWCUserData*   userdata );

    void    DWC_CreateExchangeToken       ( const DWCUserData*   userdata, DWCFriendData* token );
    void    DWC_CreateFriendKeyToken      ( DWCFriendData* token,      u64 friend_key    );

    u64     DWC_GetFriendKey              ( const DWCFriendData* frienddata );

    BOOL    DWC_CheckFriendKey            ( const DWCUserData*   userdata, u64 friend_key );
    BOOL    DWC_CheckFriendKeyLegacy      ( const DWCUserData*   userdata, u64 friend_key );

    void    DWC_FriendKeyToString         ( char* string, u64 friendKey );
    u64     DWC_StringToFriendKey         ( const char* string );

    BOOL    DWC_IsEqualFriendData         ( const DWCFriendData* frienddata1, const DWCFriendData* frienddata2 );
    BOOL    DWC_IsValidFriendData         ( const DWCFriendData* frienddata );
    BOOL    DWC_IsBuddyFriendData         ( const DWCFriendData* frienddata );// ReverseBuddyとBuddyのフラグを両方見て判定する
    BOOL    DWC_WasBuddyFriendData    ( const DWCFriendData* frienddata );    // 友達が友達リストから私を削除していないか確認する。
    void    DWC_ClearBuddyFlagFriendData  ( DWCFriendData*       frienddata );

    int     DWC_GetFriendDataType         ( const DWCFriendData* frienddata );

    // 友達検索用
    int     DWC_GetGsProfileId            ( const DWCUserData* userdata, const DWCFriendData* frienddata );
    void    DWC_LoginIdToUserName         ( const DWCUserData* userdata, const DWCFriendData* frienddata, char* username );

    // デバッグ用
    void    DWC_ReportFriendData          ( const DWCUserData* userdata, const DWCFriendData* frienddata );
    void    DWC_ReportUserData            ( const DWCUserData* userdata );
    void    DWC_SetGsProfileId            ( DWCFriendData* frienddata, int gs_profile_id );


    // 内部関数。
    //------------------------------------------------------
    BOOL    DWCi_IsBuddyFriendData( const DWCAccFriendData* frienddata );         // DWC_IsBuddyFriendData と違い、Buddyフラグのみを見る
    BOOL    DWCi_IsReverseBuddyFriendData ( const DWCAccFriendData* frienddata ); // 片思いの友達を調べる為の関数
    BOOL    DWCi_IsBuddyMsgAlreadySent( const DWCAccFriendData* frienddata );     // 1回のログインに対し、1人/1メッセージの制限を加えるためのフラグ
    void    DWCi_Acc_CreateUserData       ( DWCAccUserData* userdata, u32 gamecode );
    void    DWCi_Acc_CreateTempLoginId    ( DWCAccLoginId* loginid );

    BOOL    DWC_Acc_CheckFriendKey        ( u64 friendkey, u32 gamecode, DWCiFriendKeyGenMethod method );
    u64     DWC_Acc_CreateFriendKey       ( int gs_profile_id, u32 gamecode, DWCiFriendKeyGenMethod method );
    int     DWC_Acc_FriendKeyToGsProfileId( u64 friendkey, u32 gamecode );

    u64     DWCi_Acc_GetUserId            ( const DWCAccLoginId*     data );
    u32     DWCi_Acc_GetPlayerId          ( const DWCAccLoginId*     data );
    u64     DWCi_Acc_GetFriendKey         ( const DWCAccFriendKey*   data );
    int     DWCi_Acc_GetGsProfileId       ( const DWCAccGsProfileId* data );
    void    DWCi_Acc_SetUserId            ( DWCAccLoginId*     data, u64 userid );
    void    DWCi_Acc_SetPlayerId          ( DWCAccLoginId*     data, u32 playerid );
    void    DWCi_Acc_SetFriendKey         ( DWCAccFriendKey*   data, u64 friend_key );
    void    DWCi_Acc_SetGsProfileId       ( DWCAccGsProfileId* data, int gs_profile_id );

    void    DWCi_Acc_U64ToString32        ( u64 num, int bitnum, char* result );
    void    DWCi_Acc_LoginIdToUserName    ( const DWCAccLoginId* loginid, u32 gamecode, char* username );
    void    DWCi_Acc_SetLoginIdToUserData ( DWCAccUserData* userdata, const DWCAccLoginId* loginid, int gs_profile_id );
    BOOL    DWCi_Acc_CheckConsoleUserId   ( const DWCAccLoginId* loginid );

    BOOL    DWCi_Acc_IsDirty              ( const DWCAccUserData* userdata );
    void    DWCi_Acc_ClearDirty           ( DWCAccUserData* userdata );
    BOOL    DWCi_Acc_IsValidLoginId       ( const DWCAccLoginId* loginid );
    BOOL    DWCi_Acc_IsAuthentic          ( const DWCAccUserData* userdata );
    BOOL    DWCi_Acc_IsValidFriendData    ( const DWCAccFriendData* frienddata );

    void    DWCi_SetBuddyFriendData       ( DWCAccFriendData* frienddata );
    void    DWCi_SetReverseBuddyFlag      ( DWCAccFriendData* frienddata, BOOL setflag );
    void    DWCi_SetExpIsBuddyFriendFlag  ( DWCAccFriendData* frienddata, BOOL setflag );
    void    DWCi_SetSentBuddyReqFlag      ( DWCAccFriendData* frienddata, BOOL setflag );

    void    DWCi_Acc_TestFlagFunc( void );



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DWC_ACCOUNT_H_
