###############################################################################################
# @brief  icaファイルの <node_translate_data> にオフセットを加算するコンバータ
# @file   ica_trans_data_converter.rb
# @author obata
# @date   2009.11.20
#
# <コンバータ作成の理由>
#  H01のギミック動作アニメーションを再生したところ, 意図した位置に表示がされなかったため作成。
#  icaデータが, H01のマップ中央に置かれたモデルに対して作成されたために起きたと思われる。
#  (マップ中心までのオフセットが必要)
###############################################################################################


#----------------------------------------------------------------------------------------------
# @brief 指定した属性値を取得する
# @param filename ファイル名
# @param tag      タグ名
# @param element  属性名
# @return 指定したタグの属性値
#
# [前提条件]
#  タグ名と属性名は同一の行に存在すること。
#  属性値は element = "val" の形式であること。
#----------------------------------------------------------------------------------------------
def GetElement( filename, tag, element )

  # ファイルを読み込む
  file = File.open( filename, "r" )
  file_lines = file.readlines
  file.close

  # タグを検索
  file_lines.each do |line|
    if line.index(tag)!=nil && line.index(element)!=nil then
      i0 = line.index(element)
      i1 = line.index("\"", i0) + 1
      i2 = line.index("\"", i1) - 1
      val = line[i1..i2].to_i
      return val
    end
  end

  # 見つからなかった場合
  abort( "ERROR: タグ<#{tag}> 属性#{element} が見つかりません。" )
end

#----------------------------------------------------------------------------------------------
# @brief 指定したタグの値を全て取得する
# @param filename ファイル名
# @param tag      取得するデータのタグ名
# @return 取得した全データの配列
#
# [前提条件]
#  データ開始タグは, データ数を意味する属性 size を持つこと。
#  開始タグ・終了タグと同一行にはデータがないこと。
#  データはスペース区切りであること。
#  余計な空白や改行は存在しないこと。
#----------------------------------------------------------------------------------------------
def GetData( filename, tag )

  # ファイルを読み込む
  file = File.open( filename, "r" )
  file_lines = file.readlines
  file.close

  # データ配列
  value = Array.new

  # 開始タグを見つけたかどうか
  tag_open = false

  file_lines.each do |line|
    # 開始タグが開いている場合
    if tag_open==true then
      # 終了タグを発見
      if line.index(tag)!=nil then break end
      # データを取得
      line.strip!  # 前後の空白を削除
      data = line.split(/\s/)
      data.each do |item|
        value << item.to_f
      end
    # 開始タグが開いていない場合
    else
      # 開始タグを発見
      if line.index(tag)!=nil then
        tag_open = true 
      end
    end
  end

  # 取得したデータ数をチェック
  size = GetElement(filename, tag, "size")
  if value.size != size then
    abort("ERROR: 元データ取得に失敗")
  end
  return value 
end

#----------------------------------------------------------------------------------------------
# @brief 指定した全データにオフセットを加算
# @param data      元データ配列
# @param offset    オフセット配列(x, y, z)
# @param data_size データ数配列(x, y, z)
# @param data_head データ先頭位置配列(x, y, z)
# @return オフセット加算後のデータ配列
#----------------------------------------------------------------------------------------------
def AddOffset( data, offset, data_size, data_head )

  # 加算後のデータ配列
  conv_data = Array.new

  0.upto(data.size - 1) do |idx|
    # 処理対象がx, y, zの, どの値なのかを判断して加算する
    0.upto(2) do |type|
      if data_head[type]<=idx && idx<data_head[type]+data_size[type] then
        conv_data << data[idx] + offset[type]
      end
    end
  end

  # データ数チェック
  if data.size != conv_data.size then
    abort("ERROR: オフセット加算に失敗")
  end
  return conv_data
end

#----------------------------------------------------------------------------------------------
# @brief コンバート後のデータを持つ ica ファイルを出力する
# @param filename_org 元データのファイル名
# @param filename_cnv 出力先ファイル名
# @param conv_data    コンバート後のデータ
# @param data_tag     データのタグ名
#----------------------------------------------------------------------------------------------
def OutputConvertData( filename_org, filename_cnv, conv_data, data_tag )

  # 元ファイルを読み込む
  file = File.open( filename_org, "r" )
  file_lines = file.readlines
  file.close

  # コンバート後のデータを出力したかどうか
  data_out = false

  # タグが開いているかどうか
  tag_open = false

  # コンバートデータを出力
  file = File.open( filename_cnv, "w" )
  file_lines.each do |line_org|
    # タグが開いていない場合
    if tag_open == false then
      # 元データを出力
      file.puts(line_org)
    end
    # タグを発見
    if line_org.index(data_tag) != nil then
      # タグ状況反転
      tag_open = !tag_open
      # データ出力
      if data_out == false then
        data_out = true
        0.upto(conv_data.size - 1) do |idx|
          file.write("#{conv_data[idx]} ")
          if idx%10==0 then
            file.write("\n")
          end
        end
        file.write("\n")
        # タグを閉じる
        file.puts("</#{data_tag}>")
      end
    end
  end

  # コンバート印を押す
  file.puts("<converted!! original=""#{filename_org}""/>")
end

#----------------------------------------------------------------------------------------------
# @brief main
# @param ARGV[0] コンバート対象のicaファイルを指定
# @param ARGV[1] コンバート後のファイル名
# @param ARGV[2] icaデータに加算するxオフセット値
# @param ARGV[3] icaデータに加算するyオフセット値
# @param ARGV[4] icaデータに加算するzオフセット値
#----------------------------------------------------------------------------------------------

# データ数を取得
data_size_tx = GetElement(ARGV[0], "translate_x", "data_size")
data_size_ty = GetElement(ARGV[0], "translate_y", "data_size")
data_size_tz = GetElement(ARGV[0], "translate_z", "data_size")
data_size = Array.new
data_size << data_size_tx
data_size << data_size_ty
data_size << data_size_tz
# データ開始位置を取得
data_head_tx = GetElement(ARGV[0], "translate_x", "data_head")
data_head_ty = GetElement(ARGV[0], "translate_y", "data_head")
data_head_tz = GetElement(ARGV[0], "translate_z", "data_head")
data_head = Array.new
data_head << data_head_tx
data_head << data_head_ty
data_head << data_head_tz 
# オフセット値を取得
offset = Array.new
offset << ARGV[2].to_f
offset << ARGV[3].to_f
offset << ARGV[4].to_f

# 元データを取得
data = GetData(ARGV[0], "node_translate_data")

# 元データにオフセットを加算
conv_data = AddOffset( data, offset, data_size, data_head )

# コンバート後のデータを出力
OutputConvertData( ARGV[0], ARGV[1], conv_data, "node_translate_data" )
