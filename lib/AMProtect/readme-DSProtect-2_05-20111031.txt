■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■                                                                          ■
■  DSプロテクト                                                            ■
■                                                                          ■
■  Oct  31, 2011                                                           ■
■                                                                          ■
■  Copyright NetAgent Co., Ltd.                                            ■
■                                                                          ■
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

■はじめに

  DSプロテクトは、ニンテンドーDS 対応のマジコン、及び、エミュレータ上での DS 
  アプリケーションの利用を制限するためのミドルウェアプログラムです。

  【注意】DSプロテクトVer2.00以降はNitroSDKをサポートしておりません。
          NitroSDKでご使用の際は旧バージョン(Ver1.**シリーズ)をご使用ください。
          また、各検知関数のインターフェース・構成が変更されております。
          旧バージョン(ver1.**シリーズ)からのアップデートには、プログラムの修正が
          必要となるのでご注意ください。


■インストール方法
  
  パッケージを任意の場所に展開し、以下のファイルをご使用の開発環境に導入してください。
  DSProtect/include/AMProtect.h
  DSProtect/include/AMProtect_Instant.h
  DSProtect/lib/ARM9-TS/Rom/AMProtectR2a.a
  DSProtect/lib/ARM9-TS/Rom/AMProtectR2a_Instant.a

  【注意】パッケージは TwlSDK に上書きコピーしないでください。
  
  サンプルプログラムをビルドする場合には、パッケージのディレクトリ構成を維持し
  たまま、DSProtect/build/demos/DSProtect にて make コマンドを実行してください。
  

■プロテクションについて
  耐タンパー性は劣るが組み込みが容易な「簡易版」と、従来通りの検出・反映実装が
  行える「通常版」の２種類のマジコン検出ライブラリを同梱しています。ライブラリ
  の実装方法などは『DSプロテクト　ユーザーズマニュアル』をご確認ください。


■ファイルレイアウト

  DSProtect
  ├bin
  │└ARM9-TS
  │  └rom
  │      dsprotect_sample_ARM9-TS.srl             …  DS用サンプルROM（通常版）
  │      dsprotect_sample_ARM9-TS.HYB.srl         …  DSi対応(HybridROM) サンプルROM（通常版）
  │      dsprotect_sample_instant_ARM9-TS.srl     …  DS用サンプルROM (簡易版)
  │      dsprotect_sample_instant_ARM9-TS.HYB.srl …  DSi対応(HybridROM) サンプルROM (簡易版)
  ├build
  │└demos
  │  ├DSProtect                                  …  サンプルプログラム（通常版）
  │  ｜  ‥‥
  ｜  └DSProtect_Instant                          …  サンプルプログラム（簡易版）
  │      ‥‥
  ├docs
  │└DSProtect
  ｜    index.html                                 …  DSプロテクト ユーザーズマニュアル
  ｜    ‥‥
  ├include
  │  AMProtect.h                                  …  ヘッダファイル（通常版）
  ｜  AMProtect_Instant.h                          …  ヘッダファイル（簡易版）
  ├lib
  │└ARM9-TS
  │  └rom
  │      AMProtectR2a.a                           …  ライブラリファイル（通常版）
  ｜      AMProtectR2a_Instant.a                   …  ライブラリファイル（簡易版）
  ├man
  │└ja_JP
  │  └tools                                      … 各種ツールのリファレンスマニュアル。
  ｜      ‥‥
  └tools
    └bin                                          … 各種ツール。詳しくは「DSプロテクト
        ‥‥                                          ユーザーズマニュアル 6.同梱ツールに
                                                      ついて」及びリファレンスマニュアル
                                                      をご覧ください。


■改訂履歴
  改訂履歴は「DSプロテクトユーザーズマニュアル」の「改訂履歴」をご覧ください。

