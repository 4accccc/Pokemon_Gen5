#=========================================================================================
=begin

	08.11.25 Satoshi Nohara

	16音→3音のシーケンスナンバー変換テーブル作成

=end
#=========================================================================================

#-----------------------------------------------------------------------------------------
=begin

	gb_table.datに書き込み

=end
#-----------------------------------------------------------------------------------------
def gb_table_write( file, io )

	io.print "//GBシーケンスナンバーへの変換テーブル\n"
	io.print "//コンバートしてファイル生成しています\n"
	io.print "//08.11.25 Satoshi Nohara\n\n"
	io.print "static const u16 gb_table[][2] = {\n"

	while text = file.gets do						#ファイルを１行ずつ読み込む
		text.chomp!									#改行がある場合のみ削る(破壊的)
		ary = text.split(/\s+/)						#自分自身を空白文字で分解する

		if( ary[1] == nil )
			next
		end

		#for( i=1; i < 8 ;i++ )
		i = 1

		#print ary[i]
		#io.print ary[i]

		if( ary[2] == nil )
			next
		end

		if( ary[4] =~ /●/ )
			io.print "\t{", ary[2], ","
			#io.print ary[4]
			io.print "\t", ary[3], "},\n"
		end

	end

	io.print "};\n"
	io.print "#define GB_TABLE_MAX\t\t( NELEMS(gb_table) )\n\n"
end


#-----------------------------------------------------------------------------------------
=begin

	メインの流れ

=end
#-----------------------------------------------------------------------------------------
io = open( "gb_table.dat", "w" )					#出力ファイル
file = open( ARGV[0] )								#gb_table.txt
gb_table_write( file, io )							#gb_table.datに書き込み
file.close
io.close											#出力ファイル閉じる


