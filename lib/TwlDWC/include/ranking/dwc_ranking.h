/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_RNK_H_
#define DWC_RNK_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /* -------------------------------------------------------------------------
            define
       ------------------------------------------------------------------------- */

    /**
     *  ユーザー定義データに使用できる最大サイズ
     *
     */
#define DWC_RNK_DATA_MAX		764

    /**
     *  カテゴリーとして指定できる最大値
     *
     */
#define DWC_RNK_CATEGORY_MAX	1000

/// 取得するランキングリストの昇順と降順を示す定数
    enum
    {
        DWC_RNK_ORDER_ASC	=	0,	///< 昇順
        DWC_RNK_ORDER_DES	=	1	///< 降順
    };

    /**
     *  取得するランキングリストの最大数の上限値
     *
     */
#define DWC_RNK_GET_MAX         30

    /**
     *  友達の最大数
     *
     */
#define DWC_RNK_FRIENDS_MAX		64


    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */

    /**
     *  エラーコード
     *
     */
    typedef enum
    {

        DWC_RNK_SUCCESS = 0,						///< 成功
        DWC_RNK_IN_ERROR,							///< エラー発生中

        DWC_RNK_ERROR_INVALID_PARAMETER,			///< 不正なパラメーター

        //
        // DWC_RnkInitialize のエラー値
        //
        DWC_RNK_ERROR_INIT_ALREADYINITIALIZED,		///< 初期化済
        DWC_RNK_ERROR_INIT_INVALID_INITDATASIZE,	///< 不正な初期化データサイズ
        DWC_RNK_ERROR_INIT_INVALID_INITDATA,		///< 不正な初期化データ
        DWC_RNK_ERROR_INIT_INVALID_USERDATA,		///< ユーザー情報が不正

        //
        // DWC_RnkPutScoreAsync のエラー値
        //
        DWC_RNK_ERROR_PUT_NOTREADY,					///< 初期化されていない
        DWC_RNK_ERROR_PUT_INVALID_KEY,				///< 暗号化キーが不正
        DWC_RNK_ERROR_PUT_NOMEMORY,					///< メモリー不足

        //
        // DWC_RnkGetScoreAsync のエラー値
        //
        DWC_RNK_ERROR_GET_NOTREADY,					///< 初期化されていない
        DWC_RNK_ERROR_GET_INVALID_KEY,				///< 暗号化キーが不正
        DWC_RNK_ERROR_GET_NOMEMORY,					///< メモリー不足

        //
        // DWC_RnkCancelProcess のエラー値
        //
        DWC_RNK_ERROR_CANCEL_NOTASK,				///< 非同期処理中の処理なし

        //
        // DWC_RnkProcess のエラー値
        //
        DWC_RNK_PROCESS_NOTASK,				        ///< 非同期処理中の処理なし
        DWC_RNK_PROCESS_TIMEOUT,                    ///< タイムアウト

        //
        // DWC_RnkGetResponse のエラー値
        //
        DWC_RNK_ERROR_INVALID_MODE,					///< 不正なモード
        DWC_RNK_ERROR_NOTCOMPLETED,					///< 通信未完了
        DWC_RNK_ERROR_EMPTY_RESPONSE				///< 空のレスポンス


    } DWCRnkError;


    /**
     * モジュールステータス
     *
     */
    typedef enum
    {

        DWC_RNK_STATE_NOTREADY	= 0,	///< 未初期化状態

        DWC_RNK_STATE_INITIALIZED,		///< 初期化完了
        DWC_RNK_STATE_PUT_ASYNC,		///< PUT非同期処理中
        DWC_RNK_STATE_GET_ASYNC,		///< GET非同期処理中
        DWC_RNK_STATE_COMPLETED,		///< 完了

        DWC_RNK_STATE_TIMEOUT,			///< タイムアウト
        DWC_RNK_STATE_ERROR				///< エラー発生

    } DWCRnkState;


    /**
     * リージョンコード
     *
     */
    typedef enum
    {

        DWC_RNK_REGION_JP	= 0x01,	///< Japan
        DWC_RNK_REGION_US	= 0x02,	///< United States
        DWC_RNK_REGION_EU	= 0x04,	///< Europe
        DWC_RNK_REGION_KR	= 0x08,	///< Korea
        DWC_RNK_REGION_CH	= 0x10,	///< China(旧定義)
        DWC_RNK_REGION_CN	= 0x10,	///< China

        DWC_RNK_REGION_ALL	= 0xff	///< World

    } DWCRnkRegion;


    /**
     * DWC_RnkGetScoreAsync 取得モード
     *
     */
    typedef enum
    {

        DWC_RNK_GET_MODE_ORDER,		///< 順位
        DWC_RNK_GET_MODE_TOPLIST,	///< ハイスコア
        DWC_RNK_GET_MODE_NEAR,		///< 周辺スコア
        DWC_RNK_GET_MODE_FRIENDS,	///< 友達指定ランキング
        DWC_RNK_GET_MODE_NEAR_HI,	///< 周辺スコア(自分よりも順位が高い)
        DWC_RNK_GET_MODE_NEAR_LOW	///< 周辺スコア(自分よりも順位が低い)

    } DWCRnkGetMode;


    /* -------------------------------------------------------------------------
            typedef - function
       ------------------------------------------------------------------------- */


    /* -------------------------------------------------------------------------
            struct
       ------------------------------------------------------------------------- */

    /**
     * スコア情報格納用構造体
     */
    typedef struct
    {

        u32					order;		///< 順位
        s32					pid;		///< GameSpy Profile ID
        s32					score;		///< スコア
        DWCRnkRegion		region;		///< リージョンコード
        u32					lastupdate;	///< 前回の更新からの経過時間(分)

        u32					size;		///< userdataのサイズ(bytes)
        void*				userdata;	///< ユーザー定義データのバッファ

    }
    DWCRnkData;

    /**
     * 順位取得モードで指定するパラメータ
     *
     * 順位取得モード(DWC_RNK_GET_MODE_ORDER)で取得するときのパラメータを指定します。
     * DWCRnkGetParamのメンバです。
     *
     * See also:  DWCRnkGetParam
     *
     */
    typedef struct
    {

        u32	sort;							///< DWC_RNK_ORDER_ASC:昇順
        ///< DWC_RNK_ORDER_DES:降順
        u32	since;							///< 何分前からか

    }
    DWCRnkGetParam_order;

    /**
     * トップランキングリスト取得モードで指定するパラメータ
     *
     * トップランキングリスト取得モード(DWC_RNK_GET_MODE_TOPLIST)で取得するときのパ
     * ラメータを指定します。
     * DWCRnkGetParamのメンバです。
     *
     * See also:  DWCRnkGetParam
     *
     */
    typedef struct
    {

        u32	sort;							///< DWC_RNK_ORDER_ASC:昇順
        ///< DWC_RNK_ORDER_DES:降順
        u32	limit;							///< 取得する最大数(2～30(DWC_RNK_GET_MAX))
        u32	since;							///< 何分前からか

    }
    DWCRnkGetParam_toplist;

    /**
     * 周辺ランキングリスト取得モードで指定するパラメータ
     *
     * 周辺ランキングリスト取得モード(DWC_RNK_GET_MODE_NEAR)で取得するときのパラメ
     * ータを指定します。
     * DWCRnkGetParamのメンバです。
     *
     * See also:  DWCRnkGetParam
     *
     */
    typedef struct
    {

        u32	sort;							///< DWC_RNK_ORDER_ASC:昇順
        ///< DWC_RNK_ORDER_DES:降順
        u32	limit;							///< 取得する最大数(2～30(DWC_RNK_GET_MAX))
        u32	since;							///< 何分前からか

    }
    DWCRnkGetParam_nearby;

    /**
     * 友達ランキングリスト取得モードで指定するパラメータ
     *
     * 友達ランキングリスト取得モード(DWC_RNK_GET_MODE_FRIENDS)で取得するときのパラメ
     * ータを指定します。
     * DWCRnkGetParamのメンバです。
     *
     * See also:  DWCRnkGetParam
     *
     */
    typedef struct
    {

        u32	sort;							///< DWC_RNK_ORDER_ASC:昇順
        ///< DWC_RNK_ORDER_DES:降順
        u32	limit;							///< 取得する最大数(2～65(DWC_RNK_FRIENDS_MAX+1))
        u32	since;							///< 何分前からか
        s32	friends[DWC_RNK_FRIENDS_MAX];	///< 友達のPIDリスト

    }
    DWCRnkGetParam_friends;

    /**
     * DWC_RnkGetScoreAsyncで指定するパラメータ
     *
     * DWC_RnkGetScoreAsyncでスコアを取得するときに指定するパラメータです。
     * DWC_RnkGetScoreAsyncで指定するスコア取得モードに応じて適切なパラメータを設定
     * してください。
     *
     * See also:  DWC_RnkGetScoreAsync
     *
     */
    typedef struct
    {

        u32				size;	///< byte単位のサイズ。
        ///< 取得モードに応じたパラメータのサイズを指定します。
        ///< 例：順位取得モードのときは、sizeof(DWCRnkGetParam_order)
        ///< を指定します。

        /// 各モードごとのパラメータ
        union
        {

            DWCRnkGetParam_order    order;   ///< モード = DWC_RNK_GET_MODE_ORDER のときに指定
            DWCRnkGetParam_toplist  toplist; ///< モード = DWC_RNK_GET_MODE_TOPLIST のときに指定
            DWCRnkGetParam_nearby   nearby;  ///< モード = DWC_RNK_GET_MODE_NEAR_* のときに指定
            DWCRnkGetParam_friends  friends; ///< モード = DWC_RNK_GET_MODE_FRIENDS のときに指定

            u32 data;   ///< ライブラリ内で使用

        };

    }
    DWCRnkGetParam;


    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    DWCRnkError	DWC_RnkInitialize(		const char* initdata,
                                    const DWCUserData* userdata);

    DWCRnkError	DWC_RnkShutdown();

    DWCRnkError	DWC_RnkPutScoreAsync(	u32 category,
                                      DWCRnkRegion region,
                                      s32 score,
                                      void* data,
                                      u32 size );

    DWCRnkError	DWC_RnkGetScoreAsync(	DWCRnkGetMode mode,
                                      u32 category,
                                      DWCRnkRegion region,
                                      DWCRnkGetParam* param );

    DWCRnkError	DWC_RnkCancelProcess();

    DWCRnkError	DWC_RnkProcess();

    DWCRnkState	DWC_RnkGetState();

//
// レスポンス取得用
//
    DWCRnkError	DWC_RnkResGetRow(		DWCRnkData* out,
                                   u32 index );

    DWCRnkError	DWC_RnkResGetRowCount(	u32* count );

    DWCRnkError	DWC_RnkResGetOrder(		u32* order );

    DWCRnkError	DWC_RnkResGetTotal(		u32* total );


#ifdef __cplusplus
}
#endif

#endif // DWC_RNK_H_
