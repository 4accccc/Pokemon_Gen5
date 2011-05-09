##############################################################
#
#
#	マップマトリックス用簡易コンバータ
#	2009.04.13	tamada
#
#	resource/fldmapdata/map_matrix/map_matrix.xlsを
#	タブ区切りテキストに変換したものを受け取り、
#	下記の形式の定義ファイルを生成する
#
#	#define MAPMATRIX_～	数値
#
#
##############################################################

#先頭行読み飛ばし
gets

count = 0
matrix_ids = Hash.new

while(line = gets)
	words = line.split
	if words[0] =~ /\#END/ then break end
  id = words[0].upcase
  if matrix_ids.has_key?(id) then
    STDERR.printf("resource/fldmapdata/map_matrix/map_matrix.xlsを確認してください\n")
    STDERR.printf("%d行目と%d行目でマトリックスID\"%s\"が重複しています！！\n", matrix_ids[id]+2, count+2, id )
    raise Exception
  else
    matrix_ids[id] = count
  end

	printf( "\#define MATRIX_ID_%-16s %3d\n", words[0].upcase, count )
	count += 1
end
