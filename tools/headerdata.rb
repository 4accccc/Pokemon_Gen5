#============================================================================
#
#
#   ヘッダ読み込み→定義参照
#
#
#
#   2009.12.05  tamada  eventdataに友也くんが追加した機能を汎用化
#
#
#   C形式のヘッダファイルを解釈し、そこで定義されている定義名について
#   数値として返します。
#
#
#   使い方：
#   require "#{ENV["PROJECT_ROOT"]}tools/headerdata.rb"
#
#   headers = HeaderDataArray.new
#   heeaders.load( 適切なヘッダファイル )
#   heeaders.load( 適切なヘッダファイル )
#   heeaders.load( 適切なヘッダファイル )
#   …
#
#   begin
#     value = headers.search( "検索する定義名"、"エラー表示用の検索場所指定" )
#   rescue => errors
#     # 見つからない場合、HeaderSearchErrorが返る
#   end
#
#   注意点：
#   コメント// /* ～ */の解釈がきちんと出来ているわけではないため、
#   無効にしたい場合も読み込んでしまう。また、コメントの位置によっては
#   適切に解釈できない場合もある
#
#============================================================================
#------------------------------------------------------------------------------
# ヘッダーデータエラークラス
#------------------------------------------------------------------------------
class HeaderSearchError < Exception; end

#------------------------------------------------------------------------------
#ヘッダー情報を扱うクラス
#------------------------------------------------------------------------------
class HeaderData
  
  def initialize( filename )

    #定義情報
    @define         = Hash.new("none")
    @enum_in        = 0
    @enum_count     = 0

    #ヘッダーを読み込み　定数を取得する
    load( filename )
  end


  #読み込む
  def load( filename )
    File.open(filename){|file|
      file.each{|line|

        #追加インクルード
        if line =~ /#include[\s]+\"([^\"]+)\"/
          next_file = Regexp.last_match[1]
         debug_puts "hit include #{next_file}";
          #そのファイルをみつけられる？
          if File.exist?(next_file)
            debug_puts "include #{next_file}";
            load( next_file )
          end
        end
        
        #型の定義SEARCH
        # define
        if line =~ /#define[\s\t]+([^\s\t]+)[\s\t\(]+([xabcdefABCDEF\d]+)/
          
          #正規表現にマッチした部分を取得
          debug_puts "define #{Regexp.last_match[1]} = #{Regexp.last_match[2]}"
          index = Regexp.last_match[1]
          number = Regexp.last_match[2]
          
          #まだ入ってないときにだけ入れる
          if @define.member?(index) == false
            
            if number =~ /x/
              @define[ index ] = number.hex
            else
              @define[ index ] = number.to_i
            end

          end
        end

        # enum
        if @enum_in == 0
          
          if line =~ /enum/
            @enum_in = 1
            @enum_count = 0
          end

        else

          #終了チェック
          if line =~ /\}/
            @enum_in = 0
          #入力
          else
            if line =~ /[\s\t]+([^\s\t,]+)/
              @enum_name = Regexp.last_match[1]

              if line =~ /[\s\t]+[^\s\t,]+[\s\t]*=[\D]*([\d]+)/
                @enum_count = Regexp.last_match[1].to_i
              end

              #まだ入ってないときにだけ入れる
              if @define.member?(index) == false
            
                debug_puts "enum #{@enum_name} = #{@enum_count}"
                @define[ @enum_name ] = @enum_count
              end

              @enum_count += 1
            end
          end

        end
      }
    }
  end


  #定義を探す 
  #戻り値
  #0or1 , 数値
  def search name
    if @define.has_key?( name ) then
      @define[name]
    else
      nil
    end
  end
  
end


#------------------------------------------------------------------------------
#ヘッダーデータ配列クラス
#------------------------------------------------------------------------------
class HeaderDataArray

  def initialize
    @headerArray = Hash.new
  end

  def load( filename )
    #読み込み済みのものは読み込まない
    unless @headerArray.has_key?( filename )
      @headerArray[ filename ] = HeaderData.new( filename )
    end
  end

  def getCount
    return @headerArray.length
  end

  #全体SEARCH
  def search( name, error_comment )

    #数値クラスならそのまま返す
    if name.instance_of?( String ) == false
      return name
    end

    #数字文字列なら、そのまま返す
    if name =~ /^[+-]*\d/
      debug_puts "number out #{name}"
      return name.to_i
    end

    #余分なスペースなど破棄
    name = name.gsub( /\s/,"" )
    
    @headerArray.each do |key, headerData|
      value = headerData.search( name )
      if value != nil
        return value
      end
    end

    puts "#{error_comment}内の[#{name}]がみつかりません" 
    raise HeaderSearchError, "#{error_comment}内の[#{name}]がみつかりません" 
  end
end



