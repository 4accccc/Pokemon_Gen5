/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_UTILITY_H_
#define DWC_UTILITY_H_
/*****************************************************************************/
/*                                                                           */
/*  DWC ユーティリティ ヘッダファイル                                        */
/*      dwc_utility.h                                                        */
/*          ver. 1.2                                                         */
/*                                                                           */
/*****************************************************************************/

#ifdef  __cplusplus
extern  "C"
{
#endif         // __cplusplus

//=============================================================================
// 定義
//=============================================================================
// ワークメモリのサイズ
#define  DWC_UTILITY_WORK_SIZE    0x40000

    // 使用言語
    enum  DWC_LANGUAGE
    {
        DWC_LANGUAGE_JAPANESE,             // 日本語
        DWC_LANGUAGE_ENGLISH,              // 英語
        DWC_LANGUAGE_FRENCH,               // フランス語
        DWC_LANGUAGE_GERMAN,               // ドイツ語
        DWC_LANGUAGE_ITALIAN,              // イタリア語
        DWC_LANGUAGE_SPANISH,              // スペイン語
        DWC_LANGUAGE_HANGUL                // 韓国語
    };

    // 設定ユーティリティ開始関数返値
    enum  DWCUTILRESULT
    {
        DWC_UTIL_RESULT_SUCCESS   =  0,    // 成功
        DWC_UTIL_RESULT_FAILED    = -1,    // 引数エラー
        DWC_UTIL_RESULT_ERROR_TWL = -2     // TWLで起動した場合のエラー
    };

    // フラグ
    enum  DWCUTILFLAG0
    {
        DWC_UTIL_FLAG0_TOP      = 0x0000,
        DWC_UTIL_FLAG0_SETTING  = 0x0001
    };
    enum  DWCUTILFLAG1
    {
        DWC_UTIL_FLAG1_RAKU     = 0x0001,   // らくらく無線スタート対応
        DWC_UTIL_FLAG1_USA      = 0x0002    // 英語のサポート用電話番号対応
    };

//-----------------------------------------------------------------------------
// パラメータ
//-----------------------------------------------------------------------------
//-------- 仕向地：北米 --------
// トップメニューから開始
#define  DWC_UTILITY_TOP_MENU_FOR_USA                                         \
	(DWC_UTIL_FLAG0_TOP)

// Wi-Fi 接続先選択から開始
#define  DWC_UTILITY_SETTING_FOR_USA                                          \
	(DWC_UTIL_FLAG0_SETTING)

//-------- 仕向地：欧州 --------
// トップメニューから開始
#define  DWC_UTILITY_TOP_MENU_FOR_EUR                                         \
	(DWC_UTIL_FLAG0_TOP)

// Wi-Fi 接続先選択から開始
#define  DWC_UTILITY_SETTING_FOR_EUR                                          \
	(DWC_UTIL_FLAG0_SETTING)

//-------- 仕向地：日本 --------
// トップメニューから開始
#define  DWC_UTILITY_TOP_MENU_FOR_JPN                                         \
	(DWC_UTIL_FLAG0_TOP | (DWC_UTIL_FLAG1_RAKU << 4))

// Wi-Fi 接続先選択から開始
#define  DWC_UTILITY_SETTING_FOR_JPN                                          \
	(DWC_UTIL_FLAG0_SETTING | (DWC_UTIL_FLAG1_RAKU << 4))

//-------- 仕向地：共通 --------
// (※ 日本語は不可)
// トップメニューから開始
#define  DWC_UTILITY_TOP_MENU_COMMON                                          \
	(DWC_UTIL_FLAG0_TOP)

// Wi-Fi 接続先選択から開始
#define  DWC_UTILITY_SETTING_COMMON                                           \
	(DWC_UTIL_FLAG0_SETTING)

#define  DWC_UTILITY_TOP_MENU_EXCEPT_JPN                                      \
	DWC_UTILITY_TOP_MENU_COMMON
#define  DWC_UTILITY_SETTING_EXCEPT_JPN                                       \
	DWC_UTILITY_SETTING_COMMON

//=============================================================================
// 関数
//=============================================================================
    //-----------------------------------------------------------------------------
    // 機能 : 設定ユーティリティ開始
    // 引数 : work      - ワークメモリの先頭アドレス
    //                    ※ DWC_UTILITY_WORK_SIZE バイト以上でかつ、
    //                       32 バイトアラインされている必要があります
    //                    ※ 関数終了後は破棄してかまいません
    //        language  - アプリケーションで使用している言語
    //        param     - 以下から選択してください
    //                        DWC_UTILITY_TOP_MENU_FOR_USA
    //                        DWC_UTILITY_SETTING_FOR_USA
    //                        DWC_UTILITY_TOP_MENU_FOR_EUR
    //                        DWC_UTILITY_SETTING_FOR_EUR
    //                        DWC_UTILITY_TOP_MENU_FOR_JPN
    //                        DWC_UTILITY_SETTING_FOR_JPN
    //                        DWC_UTILITY_TOP_MENU_COMMON
    //                        DWC_UTILITY_SETTING_COMMON
    // 返値 : int       - 0        : 正常終了
    //                    それ以外 : 引数エラー
    //-----------------------------------------------------------------------------
    int  DWC_StartUtility(void* work, int language, int param);

    /*----------------------------------------------------------------------------/
    【設定ユーティリティの注意事項】

    ○ ROM イメージ
      設定ユーティリティで使用するデータを ROM イメージとして持つ必要があります。
      ファイル
          /file/dwc/utility.bin
      を NITRO 側のルートディレクトリ名で
          /dwc/
      の位置に配置する必要があります。
      具体的には、ROM スペックファイルの RomSpec セッションに

          RomSpec
          {
    	      .
    	      .
    	      .
              # ここから
    		  HostRoot    ./file/dwc    # PC 側ファイルディレクトリ名
              Root        /dwc
              File        utility.bin
              # ここまで
          }

      の 3行を付け加えてください。

    ○ アーカイブ名
          "dwc"
      というアーカイブ名を予約しています。
      アプリケーション側では使用しないでください。

    ○ 呼び出し時の注意
      ・すべての処理を終了させてから設定ユーティリティ開始関数をコールして
        ください。
    	タッチパネルのオートサンプリング等も停止させてください。
      ・設定ユーティリティは真っ白な画面から始まるように作成されているので、
        白い画面から呼び出すと自然な繋がりになります。

    ○ 関数終了時の注意
      ・LCD への出力が OFF の状態で終了します。
      ・すべての IRQ 割り込みを禁止した状態で終了します。
      ・VRAM の状態は不定です。
      ・ただし、関数の返値がエラーの場合は、何も処理をせず、すぐに終了となります。

    /*---------------------------------------------------------------------------*/

    //-----------------------------------------------------------------------------
    // 機能 : 設定ユーティリティ開始
    //          ワークメモリを内部で確保します
    // 引数 : language  - アプリケーションで使用している言語
    //        param     - 以下から選択してください
    //                        DWC_UTILITY_TOP_MENU_EXCEPT_JPN
    //                        DWC_UTILITY_SETTING_EXCEPT_JPN
    //                        DWC_UTILITY_TOP_MENU_FOR_JPN
    //                        DWC_UTILITY_SETTING_FOR_JPN
    // 返値 : int       - 0        : 正常終了
    //                    それ以外 : 引数エラー
    //-----------------------------------------------------------------------------
    int  DWC_StartUtilityEx(int language, int param);


#ifdef  __cplusplus
}
#endif         // __cplusplus


#endif // DWC_UTILITY_H_
