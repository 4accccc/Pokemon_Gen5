################################################################################# 
#
# @brief  .icaファイルコンバータ
# @file   ica_converter.rb
# @author obata
# @date   2009.09.15
#
# [概要]
# icaファイル内のデータをバイナリファイルに出力します。
# オプション指定により, 必要なデータのみを取り出すことが出来ます。
#
# [使用法]
# ruby ica_converter.rb [オプション指定] [icaファイル名] [binファイル名]
#
# [オプション指定]
# 以下のオプションを指定することで、
# binファイルに出力するデータの組み合わせを選択することが出来ます。
# -s   scale のみ
# -r   rotate のみ
# -t   translate のみ
# -sr  scale・rotate
# -st  scale・translate
# -rt  rotate・translate
# -srt scale・rotate・translate
#
# [バイナリデータフォーマット (オプションに -srt を指定した場合)]
# [ 0]: アニメーションフレーム数 (int)
# [ 4]: scale x (float)         |
# [ 8]: scale y (float)         |
# [12]: scale z (float)         | 
# [16]: rotate x (float)        |
# [20]: rotate y (float)        |------1フレーム目のデータ
# [24]: rotate z (float)        |
# [28]: translate x (float)     |
# [32]: translate y (float)     |
# [36]: translate z (float)     |
# 以下、全フレーム分のデータが存在
# また、オプション指定に該当しないデータは削減されます。
# 
#
#################################################################################

# 解析プログラム取り込み
lib_path = File.dirname(__FILE__).gsub(/\\/,"/") + "/ica_parser"
require lib_path

#================================================================================
# @brief メイン
#
# @param ARGV[0] オプション指定
# @param ARGV[1] コンバート対象icaファイル名
# @param ARGV[2] 出力binファイル名
#================================================================================
# 出力データの選択
output_scale     = false
output_rotate    = false
output_translate = false

#--------------------
# コマンドチェック
#--------------------
# 引数の数チェック
if ARGV.size!=3 then
	abort( "error: 引数に誤りがあります。" )
end

# オプションチェック
if ARGV[0].index( "s" )!=nil then 
	output_scale = true
end
if ARGV[0].index( "r" )!=nil then 
	output_rotate = true
end
if ARGV[0].index( "t" )!=nil then 
	output_translate = true
end

# コンバート対象ファイルの存在チェック
if File.exist?(ARGV[1])==false then
	abort( "error: 指定されたicaファイルは存在しません。(#{ARGV[1]})" )
end

# コンバート対象ファイルの拡張子チェック
if File::extname(ARGV[1])!=".ica" then
	abort( "error: コンバート対象にicaファイルを指定してください。(#{ARGV[1]})" )
end

#----------------------
# icaファイルの読み込み
#----------------------
ica = Ica.new
ica.read( ARGV[1] )

#--------------------
# バイナリデータ出力
#--------------------
# 出力ファイルオープン
file = File.open( ARGV[2], "wb" )

# アニメーションフレーム数を出力
file.write( [ica.frameSize].pack( "I" ) )

# 全フレーム分のアニメーションデータを出力
0.upto( ica.frameSize-1 ) do |i|
	# scale
	if output_scale==true then
		file.write( ica.GetScale(i).pack( "fff" ) )
	end
	# rotate
	if output_rotate==true then
		file.write( ica.GetRotate(i).pack( "fff" ) )
	end
	# translate
	if output_translate==true then
		file.write( ica.GetTranslate(i).pack( "fff" ) )
	end
end

# 出力ファイルクローズ
file.close
