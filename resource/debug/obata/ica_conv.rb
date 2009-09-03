################################################################################# 
#
# @brief .icaファイル コンバータ
# @file  ica_conv.rb
# @autor obata
# @date  2009.08.31
#
# [使用法]
#  ruby ica_conv.rb コンバート対象icaファイル名 出力バイナリファイル名
#
################################################################################# 

#================================================================================
# @brief <node_anm>要素を扱うクラス
#================================================================================
class NodeAnm
  # コンストラクタ
  def initialize
    @scale_x = Array.new
    @scale_y = Array.new
    @scale_z = Array.new
    @rotate_x = Array.new
    @rotate_y = Array.new
    @rotate_z = Array.new
    @translate_x = Array.new
    @translate_y = Array.new
    @translate_z = Array.new
  end

  # 指定したインデックスの<node_anm>要素を読み込む
  def read( filename, index )
    file = File.open( filename, "r" )
    extract_flag = false
    file.each do |line|
      if line.index( "<node_anm" )!=nil && line.index( "index" )!=nil then
        i0 = line.index("\"") + 1
        i1 = line.rindex("\"") - 1
        val = line[i0..i1]
        idx = val.to_i
        if index == idx then
          extract_flag = true
        end
      elsif line.index( "</node_anm" )!=nil then
        extract_flag = false
      elsif extract_flag==true then
        i0 = line.index( "frame_step" )
        i1 = line.index( "\"", i0 ) + 1
        i2 = line.index( "\"", i1 ) - 1
        frame_step = line[i1..i2].to_i
        i0 = line.index( "data_size" )
        i1 = line.index( "\"", i0 ) + 1
        i2 = line.index( "\"", i1 ) - 1
        data_size = line[i1..i2].to_i
        i0 = line.index( "data_head" )
        i1 = line.index( "\"", i0 ) + 1
        i2 = line.index( "\"", i1 ) - 1
        data_head = line[i1..i2].to_i 
        if line.index( "scale_x" )!=nil then @scale_x << frame_step << data_size << data_head 
        elsif line.index( "scale_y" )!=nil then @scale_y << frame_step << data_size << data_head
        elsif line.index( "scale_z" )!=nil then @scale_z << frame_step << data_size << data_head 
        elsif line.index( "rotate_x" )!=nil then @rotate_x << frame_step << data_size << data_head 
        elsif line.index( "rotate_y" )!=nil then @rotate_y << frame_step << data_size << data_head
        elsif line.index( "rotate_z" )!=nil then @rotate_z << frame_step << data_size << data_head 
        elsif line.index( "translate_x" )!=nil then @translate_x << frame_step << data_size << data_head 
        elsif line.index( "translate_y" )!=nil then @translate_y << frame_step << data_size << data_head
        elsif line.index( "translate_z" )!=nil then @translate_z << frame_step << data_size << data_head 
        end
      end
    end
    file.close
  end

  # 文字列に変換する
  def to_s
    str = ""
    str += "scale_x : frame_step=" + @scale_x[0].to_s + 
                     " data_size=" + @scale_x[1].to_s + 
                     " data_head=" + @scale_x[2].to_s + "\n"
    str += "scale_y : frame_step=" + @scale_y[0].to_s + 
                     " data_size=" + @scale_y[1].to_s + 
                     " data_head=" + @scale_y[2].to_s + "\n"
    str += "scale_z : frame_step=" + @scale_z[0].to_s + 
                     " data_size=" + @scale_z[1].to_s + 
                     " data_head=" + @scale_z[2].to_s + "\n"
    str += "rotate_x : frame_step=" + @rotate_x[0].to_s + 
                     " data_size=" + @rotate_x[1].to_s + 
                     " data_head=" + @rotate_x[2].to_s + "\n"
    str += "rotate_y : frame_step=" + @rotate_y[0].to_s + 
                     " data_size=" + @rotate_y[1].to_s + 
                     " data_head=" + @rotate_y[2].to_s + "\n"
    str += "rotate_z : frame_step=" + @rotate_z[0].to_s + 
                     " data_size=" + @rotate_z[1].to_s + 
                     " data_head=" + @rotate_z[2].to_s + "\n"
    str += "translate_x : frame_step=" + @translate_x[0].to_s + 
                     " data_size=" + @translate_x[1].to_s + 
                     " data_head=" + @translate_x[2].to_s + "\n"
    str += "translate_y : frame_step=" + @translate_y[0].to_s + 
                     " data_size=" + @translate_y[1].to_s + 
                     " data_head=" + @translate_y[2].to_s + "\n"
    str += "translate_z : frame_step=" + @translate_z[0].to_s + 
                     " data_size=" + @translate_z[1].to_s + 
                     " data_head=" + @translate_z[2].to_s + "\n"
    return str
  end

  # バイナリデータを出力する
  def OutputBinaryData( file )
    file.print( @scale_x.pack( "CSS" ) )
    file.print( @scale_y.pack( "CSS" ) )
    file.print( @scale_z.pack( "CSS" ) )
    file.print( @rotate_x.pack( "CSS" ) )
    file.print( @rotate_y.pack( "CSS" ) )
    file.print( @rotate_z.pack( "CSS" ) )
    file.print( @translate_x.pack( "CSS" ) )
    file.print( @translate_y.pack( "CSS" ) )
    file.print( @translate_z.pack( "CSS" ) )
  end
end


#================================================================================
# @brief icaファイルのデータを扱うクラス
#================================================================================
class Ica

  # コンストラクタ
  def initialize
    @frame_size = 0
    @scale      = Array.new
    @rotate     = Array.new
    @translate  = Array.new
    @node_anm   = Array.new
  end

  # icaファイルを読み込む
  def read( filename )
    self.read_anm_info( filename )
    self.read_scale_data( filename )
    self.read_rotate_data( filename )
    self.read_translate_data( filename )
    self.read_anm_array( filename )
  end

  # <node_anm_info>要素を読み込む
  def read_anm_info( filename )
    file = File.open( filename, "r" )
    extract_flag = false
    file.each do |line|
      if extract_flag==true && line.index( "/>" )!=nil then
        extract_flag = false
      elsif line.index( "<node_anm_info" )!=nil then
        extract_flag = true
      elsif extract_flag==true then
        if line.index( "frame_size" )!=nil then
          i0 = line.index( "\"" ) + 1
          i1 = line.rindex( "\"" ) - 1
          val = line[i0..i1]
          @frame_size = val.to_i
        end
      end
    end
    file.close
  end

  # <node_scale_data>要素を読み込む
  def read_scale_data( filename )
    file = File.open( filename, "r" )
    extract_flag = false
    file.each do |line|
      if line.index( "</node_scale_data" )!=nil then
        extract_flag = false
      elsif line.index( "<node_scale_data" )!=nil then
        extract_flag = true
      elsif extract_flag==true then
        line.strip!
        item = line.split( /\s/ )
        item.each do |val|
          @scale << val.to_f
        end
      end
    end
    file.close
  end

  # <node_rotate_data>を読み込む
  def read_rotate_data( filename )
    file = File.open( filename, "r" )
    extract_flag = false
    file.each do |line|
      if line.index( "</node_rotate_data" )!=nil then
        extract_flag = false
      end
      if extract_flag==true then
        line.strip!
        item = line.split( /\s/ )
        item.each do |val|
          @rotate << val.to_f
        end
      end
      if line.index( "<node_rotate_data" )!=nil then
        extract_flag = true
      end
    end
    file.close
  end

  # <node_translate_data>を読み込む
  def read_translate_data( filename )
    file = File.open( filename, "r" )
    extract_flag = false
    file.each do |line|
      if line.index( "</node_translate_data" )!=nil then
        extract_flag = false
      elsif line.index( "<node_translate_data" )!=nil then
        extract_flag = true
      elsif extract_flag==true then
        line.strip!
        item = line.split( /\s/ )
        item.each do |val|
          @translate << val.to_f
        end
      end
    end
    file.close
  end

  # <node_anm_array>要素を読み込む
  def read_anm_array( filename )
    file = File.open( filename, "r" )
    file.each do |line|
      if line.index( "<node_anm_array" )!=nil then
        i0 = line.index( "size" )
        i1 = line.index( "\"", i0 ) + 1
        i2 = line.index( "\"", i1 ) - 1
        size = line[i1..i2].to_i
        0.upto( size-1 ) do |i|
          node_anm = NodeAnm.new
          node_anm.read( filename, i )
          @node_anm << node_anm
        end
      end
    end
    file.close
  end

  # 文字列に変換する
  def to_s
    str = ""
    str += "frame_size = " + @frame_size.to_s + "\n"
    count = 0
    @scale.each do |val|
      str += "scale[" + count.to_s + "] = " + val.to_s + "\n" 
      count += 1
    end
    count = 0
    @rotate.each do |val|
      str += "rotate[" + count.to_s + "] = " + val.to_s + "\n" 
      count += 1
    end
    count = 0
    @translate.each do |val|
      str += "translate[" + count.to_s + "] = " + val.to_s + "\n" 
      count += 1
    end
    @node_anm.each do |anm|
      str += anm.to_s
    end
    return str
  end

  # バイナリデータを出力する
  def OutputBinaryData( filename )
    file = File.open( filename, "wb" )
    file.print( [ @frame_size ].pack( "S" ) )
    file.print( [ @scale.length ].pack( "S" ) )
    @scale.each do |val|
      file.print( [ val ].pack( "f" ) )
    end
    file.print( [ @rotate.length ].pack( "S" ) )
    @rotate.each do |val|
      file.print( [ val ].pack( "f" ) )
    end
    file.print( [ @translate.length ].pack( "S" ) )
    @translate.each do |val|
      file.print( [ val ].pack( "f" ) )
    end
    @node_anm[0].OutputBinaryData( file )
    file.close
  end
end



#================================================================================
#
# @brief メイン
#
# @param ARGV[0] コンバート対象のicaファイル名
# @param ARGV[1] 出力するバイナリデータのファイル名
#
#================================================================================
ica = Ica.new
ica.read( ARGV[0] )
ica.OutputBinaryData( ARGV[1] )
