#################################################################################
# 
# @brief  imdファイルの<tex_image>要素整列コンバータ
# @file   imd_tex_sort.rb
# @author obata
# @date   2009.08.31
#
# <tex_image>要素をname属性をキーにして並べ替えます。
# name属性が *.no であることを想定しています。
# "no"の部分に番号が入ります。
# "no"の昇順に並べ替えます。
#
# "no"は1オリジンを想定しています。
# indexは0オリジンです。
#
# [使用法]
# ruby imd_tex_sort.rb "コンバート対象ファイル名" "出力先へのパス/保存ファイル名"
#
#################################################################################

#----------------------------------------------------------------------
# @brief テクスチャの番号を取得する
#
# @param tex_image <tex_image>要素を表す文字列
#
# @return テクスチャ名に含まれる番号
#----------------------------------------------------------------------
def GetTexIndex( tex_image )

  tex_image.each do |line|

    # "name=" を発見
    if line.index( "name=" )!=nil then
      i0 = line.index( "name=" )
      i1 = line.index( "\"", i0+1 ) + 1
      i2 = line.index( "\"", i1+1 ) - 1
      tex_name = line[ i1..i2 ]
      i3 = tex_name.index( "." ) + 1
      tex_no = tex_name[ i3..tex_name.length ]
      no = tex_no.to_i
      return no
    end

  end

end

#----------------------------------------------------------------------
# @brief <tex_image> の index属性を変更する
#
# @param tex_image index属性を変更する文字列
# @param index     設定するindexの値
#
# @return index属性を変更した文字列
#----------------------------------------------------------------------
def SetIndexAttribute( tex_image, index )

  # index属性変更後の<tex_image>
  new_tex_image = ""

  tex_image.each do |line|

    # "index=" を発見
    if line.index( "index=" )!=nil then 
      i0 = line.index( "index=" )
      i1 = line.index( "\"", i0+1 )
      i2 = line.index( "\"", i1+1 )
      new_tex_image += line[0..i1]
      new_tex_image += index.to_s
      new_tex_image += line[i2..line.length]
    else
      new_tex_image += line
    end

  end

  return new_tex_image

end


#----------------------------------------------------------------------
# @brief <tex_image>要素を抽出する
#
# @param filename 抽出対象のimdファイル名
#
# @return <tex_image>要素の配列
#----------------------------------------------------------------------
def ExtractTexImage( filename )

  cnv_flag     = false      # <tex_image_array>内部ならtrue
  extract_flag = false      # <tex_image>内部ならtrue
  tex_image    = Array.new  # <tex_image>配列
  imd_file     = File.open( filename, "r" )

  # <tex_image>抽出バッファ
  str_tex_image = ""

  # 全ての<tex_image>要素を抽出
  imd_file.each do |line|

    # "tex_image_array" を発見
    if line.index( "tex_image_array" )!=nil then
      cnv_flag = !cnv_flag
      next
    end

    # "<tex_image" を発見
    if cnv_flag==true && line.index( "<tex_image" )!=nil then
      extract_flag = true
    end

    # <tex_image> 要素を抽出
    if extract_flag==true then
      str_tex_image += line
    end

    # "</tex_image" を発見
    if cnv_flag==true && line.index( "</tex_image" )!=nil then
      extract_flag = true
      tex_image << str_tex_image
      str_tex_image = ""
    end

  end


  # 抽出した<tex_image>要素を返す
  imd_file.close
  return tex_image

end



#----------------------------------------------------------------------
# @brief 指定したimdファイルの<tex_image>要素をテクスチャ名で整列する
#
# @param src_filename 処理対象のimdファイル名
# @param dst_filename 出力先ファイル名
#----------------------------------------------------------------------
def SortTexImageArrayByTexName( src_filename, dst_filename )

  # <tex_image> 要素を抽出
  tex_image = ExtractTexImage( src_filename )

  # <tex_image>要素数が1以下ならそのまま出力する
  if tex_image.length <= 1 then
    src_file = File.open( src_filename, "r" )
    dst_file = File.open( dst_filename, "w" )
    dst_file.write( src_file.read )
    dst_file.close
    src_file.close
    return
  end

  # <tex_image> 要素をテクスチャ番号で並べ替える
  tex_image_arranged = Array.new( tex_image.length )
  tex_image.each do |str|
    tex_index = GetTexIndex( str ) 
    tex_image_arranged[ tex_index-1 ] = SetIndexAttribute( str, tex_index-1 )
  end


  # データを書き換える
  data = ""
  file = File.open( src_filename, "r" ) 
  copy_flag = true
  file.each do |line|

    # "</tex_image_array" 発見 ==> コピー開始
    if line.index( "</tex_image_array" )!=nil then
      copy_flag = true
    end

    # コピー
    if copy_flag==true then
      data << line
    end

    # "<tex_image_array" 発見 ==> コピー停止 & 並べ替えた<tex_image>をコピー
    if line.index( "<tex_image_array" )!=nil then
      copy_flag = false
      tex_image_arranged.each do |str|
        data << str
      end
    end
  end
  file.close


  # 書き換えたデータをファイルに出力
  file = File.open( dst_filename, "w" )
  file.write( data )
  file.close

end

#----------------------------------------------------------------------
# @brief メイン
#
# ARGV[0] = コンバート対象imdファイル名
# ARGV[1] = 出力ファイル名
#----------------------------------------------------------------------
SortTexImageArrayByTexName( ARGV[0], ARGV[1] )
