#!/usr/bin/ruby
#
# プラチナのgmmをWBに強引変換するツールです
# このツールは  プログラマがサポートして使う事しか想定していません
#
#  1.テンプレート置き換え
#     第一引数のテンプレートをコンバートファイルのテンプレートに差し替え
#  2.タグ置き換え
#     タグテーブルを書けば勝手に置換してくれる
#  3.漢字行増加
#     今までの一言語だったのを,日本語と日本語漢字の二行に増やす
#  4.日本語 -> JPN変換
#     language="日本語" という定義が "JPN" に変わったので置換してくれる
#  
#  これらが個別実装していますので 使用用途にしたがって、末尾のプログラムをコメントにしつつ
#  変換を行ってください
#
#  使用方法例： ruby ./tools/plgmmconv.rb ./resource/message/template.gmm ./resource/message/src/pl_wifi_lobby.gmm
#
#

require "nkf"
require "rexml/document"
include REXML
require "tools/gmmconv.rb"

### メイン
begin
  
  GRead = GmmRead.new
  GRead.FileRead( ARGV[1] )  ##コンバート対象ファイルを読み込む
  GRead.TmplateRead( ARGV[0] ) ## テンプレを読み込む
  GRead.TmplateMake()  ##  テンプレを差し替える
  GRead.BodyAdd()
  GRead.JpnConv()    ##   日本語をJPNに変更
  GRead.JpnDupe()    ##   JPNとJPN_KANJIに増やす
  GRead.TagConv()   ## タグを置き換える
  GRead.FileWrite( ARGV[1])  ##コンバート結果を出力する
  
  
  exit(0)
end


