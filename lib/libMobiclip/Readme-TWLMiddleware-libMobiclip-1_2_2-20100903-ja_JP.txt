■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■                                                                          ■
■  Readme-TWLMiddleware-libMobiclip-1_2_2-20100903-ja_JP.txt               ■
■                                                                          ■
■  Mobiclip 1.2.2                                                          ■
■                                                                          ■
■  September 3rd, 2010                                                     ■
■                                                                          ■
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

【 はじめに 】

    Mobiclip Middleware for TWL は、 Mobiclip Inc. の独自ビデオコーデック 
    "Mobiclip (モビクリップ)" の TWL 用ライブラリです。高品質のビデオを再生する
    ことができるため、 TWL と Nitro 用のゲームソフトウェアの様々なシーンにご活用
    いただけます。ライブラリ上でビデオを再生するには、 Mobiclip Middleware for 
    TWL 専用のビデオフォーマットに変換したファイルを使用します。
    詳しい使用方法、パッケージ内容に関しては、
    libMobiclip/docs/ja_JP/Mobiclip/Quickstart.pdf を参照してください。



【 SDKのインストール 】

    本ライブラリをインストールするには、ダウンロードしたパッケージ（zipファイ
    ル）を解凍し、任意の場所に設置してください。



【 サンプルデモのビルド 】

    libMobiclip ディレクトリが属する場所に環境変数 TWL_LIBMOBICLIP_ROOT を定義し
    てください。このパスにはスぺースを含めないようにしてください。

    それぞれのサンプルコードのディレクトリには Makefile が存在します。デモをビル
    ドするには Cygwin shell から make コマンドを使用てください。：
     * Nitro プラットフォームでコンパイルしたい場合は
       'make TWLSDK_PLATFORM=NITRO' と入力してください。
     * TWL プラットフォームを LIMITED モードでコンパイルしたい場合は
       'make TWLSDK_PLATFORM=TWL TWL_ARCHGEN=LIMITED' と入力してください。
     * TWL プラットウォームを HYBRID モードでコンパイルしたい場合は
       'make TWLSDK_PLATFORM=TWL TWL_ARCHGEN=HYBRID' と入力してください。

    詳しい使用方法と解説は、 Mobiclip 関数リファレンスマニュアル
    libMobiclip/man/ja_JP/index.html  を参照してください。



【 注意：権利表記について 】

    本ライブラリを製品に使用する場合、取扱説明書に権利表記を行う必要があります。
    表示する文言などの表記方法については、弊社から提供している
    ソフト印刷物ガイドライン、電子取扱説明書ガイドラインを参考にしてください。



【 Mobiclip Toools for NINTENDO DS のライセンスについて】

    Mobiclip VFW コーデック及び関連するツールを動作させるには、ライセンスファイ
    ルを取得する必要があります。
    詳しい使用方法と解説は、libMobiclip/docs/ja_JP/Mobiclip/QuickStart.pdf を参
    照してください。



【 動作確認バージョン 】

    本ライブラリの動作確認が取れている環境は以下の通りです。

    - TWL-SDK 5.5
    - TWL-SDK 5.4 + patch 2
    - CodeWarrior for NINTENDO DSi v1.2 + patch 2

    また、 AviToMobiclipDs プログラムと Mobiclip Video For Windows コーデックは
    以下の環境で動作が確認されています。

    - Windows XP SP3.
    - Windows 7 64ビット版



【 Mobiclip VFW コーデックの CPU 条件 】

    Mobiclip VFW コーデックは、SSE2 x86 の命令セット (instruction set) を多用し
    ています。この命令セットに対応していない CPU の場合、VFW コーデックをクラッ
    シュさせてしまいます。

    以下は SSE2 命令セットに対応している代表的な CPU です。
    - Pentium 4 、またはそれ以降の Intel CPU と、全ての Intel Centrino プラット
      フォーム
    - Athlon 64 またはそれ以降の AMD CPU

    以下は SSE2 命令セットに対応していない代表的な CPU です。
    - Pentium 3 またはそれ以前の Intel CPU
    - Athlon XP またはそれ以前の AMD CPU



【 更新内容 】

Version 1.2.2(September 3rd, 2010)

+ Mobiclip_Tools_for_DS.exe インストーラ（自己解凍形式）を追加しました。
  Mobiclip VFW コーデックと AvitoMobiclipDS.exe プログラムをインストールします。
+ 以下を含むMobiclip VFWコーデックの変更を行いました。
    - StarForce のプロテクションシステムの代わりに Mobiclip ライセンスシステムが
      導入されました。
    - .inf ファイルを利用していたインストールの代わりに、Mobiclip_Tools_DS.exe 
      インストーラ(自己解凍形式)によってインストールされるようになりました。
    - GUI が変わりました。
    - 新しい機能を追加し、古い機能を削除しました。詳しくは「 Mobiclip Video for 
      Windows (VFW) コーデックの内容」の章を参照してください。
+ QuickStart と MobiclipFAQ の内容を更新しました。
+ 以下のドキュメントを削除しました。
    - AboutLogo.pdf
    - Starforce protection - Administration Guide.pdf
    - Starforce protection - Automatic Activation.pdf
    - Starforce protection - Manual Activation.pdf
    - VX To Mobiclip Shifting Guide.pdf
+ Html ドキュメントで MO_JumpNextIframe.html について更新し、MO_JumpNextIframe 
  関数の挙動の詳細を記載しました。


Version 1.2.1(October 22nd, 2009)

+ AviToMobiclipDS プログラムのバグを修正しました。Adobe Flash を使用して非圧縮の
  Avi ファイルを出力し、それをソースとして動画を圧縮すると、再生時にオーディオに
  障害が出ていましたが、その問題を解決しました。
+ QuickStart を更新し、VirtualDub バージョン 1.9.6 以降でのバッチジョブの使用方
  法を「マルチパスエンコーディングの操作手順」の章に追記しました。
+ libMobiclip/build/demos/Mobiclip ディレクトリに Makefile を追加しました。これ
  を使用することで全てのサンプルデモを一度にコンパイルできます。
+ 関数リファレンスマニュアルの singleScreen.html, singleScreenStereo.html, 
  dualScreen.html そして effectiveUsage.html において、ビルドされるファイル名に
  誤記がありましたが、これを修正しました。
+“Mobiclip VFW コーデックの CPU 条件”という項目を Readme（本書）に追記しました。


Version 1.2.0 (April 23rd, 2009)

+ サンプルデモ singleScreen, singleScreenStereo, dualScreen，multitrackAudio を
  修正し、TWL HYBRID と LIMITED のビルドで音声を再生できるようにしました。
+ サンプルコードのビルドシステムを変更しました。
    - Nitroターゲットのビルドにおいては、CodeWarrior .mcpプロジェクトファイル
      (NitroSDKを使用)、makefiles(TwlSDKを使用)のどちらでも使用できるようになり
      ました。 
    - TWLターゲットのビルド (LIMITED または HYBRID) においては、makefiles を使
      用できるよようになりました。
+ libMobiclipライブラリに TWL HYBRID とLIMITED のターゲットを追加しました。
    - libMobiclip.TWL.HYB.a
    - libMobiclip.TWL.LTD.a
+ 関数リファレンスマニュアルを更新し、TWLターゲット用にサンプルコードをコンパイ
  ルする方法を追記しました。
+ QuickStartを更新し、TWLターゲットと関連するライブラリのサポートに関する追記を
  しました。


Version 1.1.1 (March 9th, 2009)

+ サンプルデモeffectiveUsageを修正し、デコードが遅いと永久に再生をループさせるバ
  グを取り除きました。
+ サンプルデモsingleScreen, singleScreenStereo, dualScreen, multitrackAudioを修正
  しました。
    - DSが閉じられているときの処理をニンテンドーDS本体の仕様に合わせて再度修正し
      ました。
          * PM_SetLCDPower（'PM_LCDPOWER_OFF'パラメータ）関数でLCDをOFFにする場合、
            事前にGX_DispOff/GXS_DispOff関数を呼びだします。
          * PM_SetLCDPower（' PM_LCD_POWER_ON 'パラメータ）関数でLCDをONにした後、
            GX_DispOn/GXS_DispOn関数を呼び出します。
    - ‘MO_NB_BUFFERED_FRAME_MIN’より少ないフレーム数の動画の再生ができるように
      修正しました。
+ AviToMobiclipDS変換プログラムに機能を追加いたしました。
    - PCM 16bit以外のオーディオファイルを入力した場合、エラーメッセージが表示され
      るようになりました。
    - BWFのWAVファイルが読み込めるようになりました。サウンドの仕様はPCM 16bitのモ
      ノラルかステレオである必要があります。
    - VirtualDubで出力したWAVファイルを入力するとクラッシュしていた問題を修正しま
      した。
    - プログラムのバージョンは V1.2.0.2になりました。


Version 1.1.0 (November 21st, 2008)

+ サンプルデモを修正しました。
    - singleScreen, singleScreenStereo, 及び dualScreen の各サンプルデモで、ムー
      ビーの終端部分の再生処理において、DS が閉じられているときの処理を行ってい
      なかったため、これを修正しました。
+ AviToMobiclipDS変換プログラムに機能を追加いたしました。
    - IMA ADPCMおよびPCMに対応しました。
    - 複数の[audio] オプションを使って、異なるトラックを指定してください。
          現在、マルチトラックオーディオのビデオを再生する場合には、2つ以上のト
          ラックで可能となります。
          また、オーディオトラックの数の制限がなくなりました。
          それぞれのオーディオトラックは約5.2KB のメインメモリを使用します。
           制限事項:
            * すべてのオーディオトラックは周波数が同じである必要があります。
            * すべてのオーディオトラックはチャンネル数が同じである必要があります。
            * [audio]オプションを複数使用する場合、[debugsound]オプションは使用で
              きません。
+ QuickStartを更新しました。
    - AviToMobiclipDS変換プログラムの新しいオーディオフォーマットに関する説明を
      追加しました。
+ リファレンスマニュアル(Html)を更新しました。
    - MO_SkipFrameSoundOnePacketの使用制限を追加しました。
+ multitrackAudioのサンプルコードが追加されました。
    - 現在再生中のオーディオトラックを動的にかえる方法が明記されています。


Version 1.0.3 (September, 22nd 2008)

+ Starforce プロテクションを更新し、Windows Vista に対応したコーデックがインス
  トールできるようになりました。
+ Starforce プロテクションのドキュメントを更新しました。
    - シリアルキーコードのライセンス制限に関する情報を追加しました。
    - 新しいユーザーインターフェイスに合わせてスクリーンショットを更新しました。
    - コーデックの更新方法に関する情報を追加しました。
+ リファレンスマニュアル(Html)を更新しました。
    - A-Z の索引に含まれていなかった関数を追加しました。


Version 1.0.2 (May, 7th 2008)

+ AviToMobiclipDS変換プログラムに機能を追加しました。
    - [-audio]オプションを指定することで、外部のwavファイルをサウンドソースに用
      いることができます。プログラムのバージョンは V1.1.0.0になりました。
+ サンプルデモを修正しました。
    - singleScreen, singleScreenStereo, 及び dualScreen の各サンプルデモで以下
      の条件が同時に発生した場合、演算のバグが発生していたものを修正しました。
            * サウンドバッファのアンダーフローが発生
            * サウンドの再生が一時停止され、再びサウンドが再生された場合
+ パッケージにドキュメントを追加しました。
    - VX To Mobiclip Shifting Guide.pdf
            * libVXのアプリケーションをMobiclipに移行する方法を説明しています。
    - MobiclipFAQ.pdf
            * VX Middleware に対して寄せられたサポート案件のうち、 Mobiclipにも
              関連するものや活用できる情報をまとめたものです。
+ Mobiclip Video Codec For Windows に機能を追加しました。
    - Keyframe detection を無効にすることで、キーフレームを自動的に挿入させない
      ようにすることができます。


Version 1.0.1（March 14th, 2008）

+ サンプルコードを修正しました。
    - データTCM(DCTM)に空き領域を確保するために、スタックサイズを 4KB に設定しま
      した。
    - OS_GetDTCMArenaHi及びOS_GetDTCMArenaLoを使い、データTCM(DCTM)の使用可能領
      域を計算できるようになりました。
    - MO_OpenMovie 関数によってスタック領域が上書きされていないことを確認するた
      めに、MO_OpenMovie 関数の後に OS_CheckStack 関数を呼び出すようにしました。

+ リファレンスマニュアル(Html)を更新しました。
    - Mobiclipを使用するときに、適切なスタックのサイズを設定してデータTCM(DCTM)
      の領域を確保する必要がある旨の情報を追加しました。
    - 付属ドキュメントにある「SDRAMメモリ」の表記を「メインメモリ」に訂正しまし
      た。


Version 1.0 (March, 3rd 2008)

+ 初回リリース

