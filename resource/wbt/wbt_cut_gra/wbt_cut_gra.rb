#! ruby -Ks
#//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#/**
# *
# *	@file		wbt_face_gra.rb
# *	@brief  wbtビジョンの顔グラフィック用コンバータ
# *	@author	Katsuhiko=Ichiraku
# *	@date		2011.08.08
#
#   @note   trtype/trtype_list.xlsからトレーナータイプを取得し、
#           それに対応したトレーナーグラフィックncgから顔部分を
#           トリミングしてbmpに変換
#
#   @param  引数1:makefile [out]
#   @param  引数2:ncgファイル名一覧 [in]
#
#   トリミングのサイズをどう指定するか迷い中
# *
# */
#//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

# 文字コード指定
$KCODE = 'SJIS'

#=====================================
#	メイン
#=====================================

make_str = ""
arc_list = ""
check_array = Array.new  #重複チェック用
target_list = "TARGET_LIST = \\\n"

reader = File.open(ARGV[1], "r")

reader.each { |line|
  colum = line.split ","

  ncg_name = ARGV[2] + colum[0].chomp
  ncbr_name = colum[1].swapcase.chomp                # swapcase = 大文字変換

  ncbr_name.sub!(/type/, "wb")

  # typeをwbに置き換える

  # arc_list に書き込む
  arc_list += sprintf("\"cnv_tmp/%s.NCBR\"\n",ncbr_name)

  # targetに書き込む
  target_list += sprintf("\tcnv_tmp/%s.NCBR \\\n",ncbr_name)

  # make file に書き込む
  make_str += sprintf("cnv_tmp/%s.NCBR : %s.ncg \n", ncbr_name, ncg_name )
  make_str += sprintf("\t@g2dcvtr $< -bmp -bg -cr/28_28_4_4 -o/cnv_tmp\n")
  unless colum[0].chomp == ncbr_name then
    make_str += sprintf("\tcp cnv_tmp/%s.NCBR cnv_tmp/%s.NCBR \n\n", colum[0].chomp, ncbr_name)
  else
    make_str += sprintf("\n")
  end

  #check_array << ncbr_name
}
reader.close

=begin
# 重複要素を取り除く
check_array.uniq!

# 配列の中身を書き込む
check_array.each{ |name|

  ncg_name = ARGV[2] + name

  # targetに書き込む
  target_list += sprintf("\tcnv_tmp/%s.NCBR \\\n",name)

  # make file に書き込む
  make_str += sprintf("cnv_tmp/%s.NCBR : %s.ncg \n", name, ncg_name)
  make_str += sprintf("\t@g2dcvtr $< -bmp -bg -cr/16_16_4_4 -o/cnv_tmp\n\n")
}
=end

#最後の\を消し改行を入れる
target_list.chomp!("\\")
target_list += sprintf("\n")

arc_list += sprintf("\n")

#bmp変換後ファイル名一覧出力
makefile = File.open(ARGV[0],'w')
makefile .puts make_str
makefile .close

#TARGETLISTファイル
arcfiel = File.open(ARGV[3],'w')
arcfiel.puts arc_list
arcfiel.close

#TARGETLISTファイル
target = File.open(ARGV[4],'w')
target .puts target_list
target .close

exit(0)
