#-------------------------------------------------------------------------------
# デバッグサウンド用gmmファイルを作るツール
# 
# 実行すると「sound_data.sadl」の中から「SEQ_」がマッチする文字列を取り出して
# サウンドデバッグ画面で使うgmmファイルを作成します。
# 同じフォルダに「sound_data.sadl」「name_gmm.head」「name_gmm.fott」が必要です。
# 
# 2008.01.10 by AkitoMori
#-------------------------------------------------------------------------------



#文字コードをUTF-8に変換するのに必要
#require "iconv"
require "nkf"   ## iconvがcygwinのrubyだとバグってるらしいので nkfに変更 k.ohno 2009.1.9


#読み込みファイル名
input_soundfile  = "wb_sound_data.sadl"
input_headerfile = "name_gmm.head"
input_footerfile = "name_gmm.foot"

#出力ファイル名
output_gmm_file  = "snd_test_name.gmm"

#sprintf用定義
output_msg_line  = "\t<row id=\"%s\">\n\t\t<language name=\"日本語\">%s</language>\n\t</row>\n"



#出力に使うファイルの読み込み
headfile = File.open("name_gmm.head","r")
headstr  = headfile.read

footfile = File.open("name_gmm.foot","r")
footstr  = footfile.read

#出力ファイルのオープン
outputfile = File.open(output_gmm_file,"w")

#ヘッダ部分出力
outputfile.puts(headstr)

#サウンドシーケンスのみ取り出し整形してgmmに出力
File.open( input_soundfile ){|file|
	file.each{|line|
		divline = line.split(nil)
		seqname = divline[1]

		#「SEQ_」がついている定義名のみ取り出す
		if seqname=~/^SEQ_/ then

			#出力用に名前を加工
			msgid   = "msg_"+seqname.downcase
			msgstr  = seqname.sub(/SEQ_SE_/,"")
			msgstr  = msgstr.sub(/SEQ_/,"")
			msgstr  = msgstr.gsub(/_/,"-")
			
			#GMM用に文字列を整形
			tmpbuf = sprintf( output_msg_line, msgid, msgstr)

      #UTF-8に変換して出力
#            tbuf = Iconv.iconv("UTF-8","SJIS",tmpbuf)
      
            tbuf = NKF.nkf("-w",tmpbuf)
            outputfile.puts(tbuf)
			
		end
	}
}


#フッター出力
outputfile.puts(footstr)


