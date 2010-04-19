
#!/usr/bin/ruby

################################################
#
# AttrCheck　アトリビュート整合性チェック
#
# ruby attr_check.rb pokewb.3at railフォルダー(../rail/)
#
################################################

ATTR_CHECK_RET_ERR = 0
ATTR_CHECK_RET_ERR_BIT = 1
ATTR_CHECK_RET_ERR_VALUE = 2
ATTR_CHECK_RET_ERR_COLOR = 3
ATTR_CHECK_RET_ERR_COMMENT = 4


ATTR_DATA_INIT_TYPE_3AT = 0
ATTR_DATA_INIT_TYPE_RAIL = 1

def debug_puts str

  #puts str
  
end


### アトリビュートクラス
class AttrData
  attr :bit
  attr :value
  attr :color
  attr :comment


  def initialize str, init_type

    str = str.sub(/\r\n/, "")
    str = str.sub(/\n/, "")

    if init_type == ATTR_DATA_INIT_TYPE_3AT then
      datas = str.split( /\s/ )

      debug_puts "add 3at data"+str

      @bit = datas[ 2 ]
      @value = datas[ 3 ]
      @color = datas[ 1 ]
      @comment = datas[ 4 ]
    else
      str = str.sub( /::/, "\s" )
      datas = str.split( /\s/ )

      debug_puts "add rail data"+str

      @bit = datas[ 2 ]
      @value = datas[ 3 ]
      @color = datas[ 1 ]
      @comment = datas[ 4 ]
    end
    
  end

  def checkComment check_com
  
    if @comment == check_com then
      return TRUE
    else
      return FALSE
    end
  end

  def checkAttr bit, value, color
    ret = [0,0,0,0,0]

    #ビットが違う
    if bit != @bit then
      ret[ATTR_CHECK_RET_ERR_BIT] = 1
      ret[ATTR_CHECK_RET_ERR] = 1
    end

    #バリューが違う
    if value != @value then
      ret[ATTR_CHECK_RET_ERR_VALUE] = 1
      ret[ATTR_CHECK_RET_ERR] = 1
    end

    #色設定が違う
    if color != @color then
      ret[ATTR_CHECK_RET_ERR_COLOR] = 1
      ret[ATTR_CHECK_RET_ERR] = 1
    end
    
    return ret
  end
  
end

class AttrArray
  attr :attr_buf

  def initialize filedata

    @attr_buf = Array.new
    filedata.each{|line|
      if line.index( "galv" ) != nil then
        @attr_buf << AttrData.new( line, ATTR_DATA_INIT_TYPE_3AT ) 
      end
    }
  end


  def checkAttr value, bit, comment, color

    @attr_buf.each{|obj|
      if obj.checkComment( comment ) then
        debug_puts("#{comment} check")
        return obj.checkAttr( bit, value, color )
      end
    }

    debug_puts("comment hit")
    return [1,0,0,0,1]#commentがみつからない
  end
  
end


### RailLineクラス
class RailLine

  attr :gridx
  attr :gridz
  attr :attr_buf
  attr :linename

  def initialize filedata

    datain = 0
    count = 0

    @attr_buf = Array.new

    while (line = filedata.gets)

      #ライン名取得
      if datain == 0 then
        if line.index("--NAME") == nil then
          #エラー
          puts("Data Format Error --Name")
          exit(1)
        end

        @linename = line.sub( /--NAME::/, "" )
        datain = 1
        
      #Xサイズ取得
      elsif datain == 1 then

        @gridx  = line.sub( /--X::/,"" )
        datain = 2
        
      #Zサイズ取得
      elsif datain == 2 then

        @gridz  = line.sub( /--X::/,"" )
        datain = 3
        
      else

        if line.index( "--LINEATTR::" ) != nil then
          @attr_buf << AttrData.new( line, ATTR_DATA_INIT_TYPE_RAIL ) 
          count += 1

          #完了チェック
          if count >= (@gridx.to_i * @gridz.to_i) then
            break
         end
       else
          #エラー
          puts("Data Format Error --LINEATTR")
          exit(1)

       end
        
      end
      
    end
    
  end


  def outputAttr attrArray, output

    count = 0
    ret = [0,0,0,0,0]
    
    debug_puts( "#{@linename} check..." )
    output.printf( " %s check...\n", @linename )
    
    @attr_buf.each{|obj|
      
      ret = attrArray.checkAttr( obj.value, obj.bit, obj.comment, obj.color )
      
      #エラー？
      if ret[ATTR_CHECK_RET_ERR] == 1 then

        index_x = (count % @gridx.to_i)
        index_z = (count / @gridx.to_i)
        output.printf( "  attrerr gridx[#{index_x}] gridz[#{index_z}] comment[#{obj.comment}] " )

        debug_puts( "#{ret}" )

        if ret[ATTR_CHECK_RET_ERR_COMMENT] == 1 then
          debug_puts( "  comment #{obj.comment} がみつかりません。" )
          output.printf( "  comment #{obj.comment} がみつかりません。" )

        else

          if ret[ATTR_CHECK_RET_ERR_BIT] == 1 then
            debug_puts( "  bit #{obj.bit} に違いがあります。" )
            output.printf( "  bit #{obj.bit} に違いがあります。" )
          end

          if ret[ATTR_CHECK_RET_ERR_VALUE] == 1 then
            debug_puts( "  value #{obj.value} に違いがあります。" )
            output.printf( "  value #{obj.value} に違いがあります。" )
          end

          if ret[ATTR_CHECK_RET_ERR_COLOR] == 1 then
            debug_puts( "  color #{obj.color} に違いがあります。" )
            output.printf( "  color #{obj.color} に違いがあります。" )
          end

        end
        
        output.printf( "\n" )

      end

      count += 1
    }


  end
  

end

### Railファイルクラス
class FileRailData

  attr :line
  attr :filename

  def initialize filedata, filename

    @filename = filename

    @line = Array.new
    while line = filedata.gets
      if line.index( "+LINE" ) != nil then
        @line << RailLine.new( filedata ) 
      end
    end

  end


  def outputAttr attrArray, output

    
    output.printf( "---------------------\n" )
    output.printf( "■ %s check...\n", @filename )

    @line.each{|obj|
      obj.outputAttr( attrArray, output )
    }
    
    output.printf( "---------------------\n" )
  end
  
end


def outputRail dir, output, attrArray
  dir = dir.sub(/\/$/, '') # 最後に / があれば取る
  
  Dir::foreach(dir){ |filename|
    if filename == '..'
      next
    end
    if filename == '.'
      next
    end
    if filename == '.svn'
      next
    end
    if filename =~ /\.attr$/
      debug_puts("#{dir}/#{filename}..")
      File.open( "#{dir}/#{filename}" ){|file|
        fileRail = FileRailData.new( file, "#{dir}/#{filename}" )
        
        fileRail.outputAttr( attrArray, output )
      }
    end
    if true == FileTest::directory?(dir + '/' + filename)
      debug_puts "dir  #{filename}..."
      outputRail( dir + '/' + filename , output, attrArray) #再帰的にチェック
    end
  }
end


### メイン
begin

  attrArray = nil

  #3atを読み込む。
  File.open( ARGV[0] ){|file|
    attrArray = AttrArray.new( file )
  }

  File.open( "attr_check_result.txt", "w" ){|ofile|
    
    outputRail( ARGV[1], ofile, attrArray )
  }
  
    
  
rescue => errs
  p errs
  exit(1)
else
end
  

