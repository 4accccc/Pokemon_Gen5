■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■                                                                          ■
■  DSプロテクト                                                            ■
■                                                                          ■
■  Mar  5, 2010                                                            ■
■                                                                          ■
■  Copyright NetAgent Co., Ltd.                                            ■
■                                                                          ■
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

■はじめに

  DSプロテクトは、ニンテンドーDS 対応のマジコン、及び、エミュレータ上での DS 
  アプリケーションの利用を制限するためのミドルウェアプログラムです。

　【注意】Version 1.22よりフォルダ構成が変わっておりますのでご注意ください。

■インストール方法
  
  パッケージを任意の場所に展開し、DSProtect/include/AMProtect.h 及び 
  DSProtect/lib/ARM9-TS/Rom/AMProtectR2a.a をご使用の開発環境に導入してください。
  
  【注意】パッケージは TwlSDK に上書きコピーしないでください。
  
  サンプルプログラムをビルドする場合には、パッケージのディレクトリ構成を維持し
  たまま、DSProtect/build/demos/DSProtect にて以下のコマンドを実行してください。
  
  % make TWLSDK_PLATFORM=NITRO NITRO_FINALROM=TRUE

  NitroSDKを使用してビルドする場合は以下のコマンドを実行してください。

  % make -f Makefile.ntr NITRO_FINALROM=TRUE

　【注意】CodeWarrior環境をお使いの方は、CodeWarrior用プロジェクトファイルをご
          利用ください。CodeWarrior用プロジェクトファイルは、
          DSProtect/build/demos/DSProtect/CW/ 以下に配置されています。
          なお、TwlSDK用プロジェクトファイルは、TwlSDK 5.1 用の設定になってお
          ります。TwlSDK 5.2 をお使いの方は、Twlサポートページ内技術情報ペー
          ジ記載の移行方法を参照のうえ、プロジェクト設定を変更してください。

■プロテクションについて

  １種類のマジコン検出ライブラリを同梱しています。
  ライブラリの実装方法などは、「DSプロテクト ユーザーズマニュアル」をご確認く
  ださい。


■ファイルレイアウト

  DSProtect
  ├bin
  │└ARM9-TS
  │  └rom
  │      NITRO_Overlay.srl             …  DS用サンプルROM
  │      NITRO_Overlay_thumb.srl       …  DS用サンプルROM (thumb 版)
  │      TWL_Hybrid_Overlay.srl        …  DSi用 Hybrid サンプルROM
  │      TWL_Hybrid_Overlay_thumb.srl  …  DSi用 Hybrid サンプルROM (thumb 版)
  ├build
  │└demos
  │  └DSProtect                       …  サンプルプログラム
  │      ‥‥
  ├docs
  │└DSProtect
  ｜    index.html                      …  DSプロテクト ユーザーズマニュアル
  ｜    ‥‥
  ├include
  │  AMProtect.h                       …  ヘッダファイル
  ├lib
  │└ARM9-TS
  │  └rom
  │      AMProtectR2a.a                …  ライブラリファイル
  ├man
  │└ja_JP
  │  └tools                           … 各種ツールのリファレンスマニュアル。
  ｜      ‥‥
  └tools                                
    └bin                               … 各種ツール。詳しくは「DSプロテクト
        ‥‥                               ユーザーズマニュアル 5.同梱ツールに
                                           ついて」及びリファレンスマニュアル
                                           をご覧ください。


■改訂履歴
  改訂履歴は「DSプロテクトユーザーズマニュアル」の「改訂履歴」をご覧ください。

