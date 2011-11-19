#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
# 
# 	@file		area_data_cnv.rb
# 	@brief  エリアデータを生成する
# 	@author	Masayuki Onoue
# 	@date		2011.11.18
# 
# 
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"
require "room_type_hash"

# 構造体定義
$SAreaData = Struct.new("AREA_DATA",:data_end ,:fs_x ,:fs_y ,:area_line ,:fnum ,:room_num)

# =========================================
# エリアデータ解析クラス
class AREA_PURSER
  attr_accessor :area_no, :room_data, :member_num ,:area_size
  def initialize( data )
    # データの初期化
    @all_area_data = data
    @area_data = Array.new
    @area_size = 0
    
    # エリアサイズ
    get_area_data( 0 )

    @area_size.times{|area_id|
      puts "エリア%d:%s×%s"%[area_id,@area_data[area_id].fs_x,@area_data[area_id].fs_x]
      puts "%d階建て"%[@area_data[area_id].fnum]

      @area_data[area_id].fnum.times{|i|
        $room_type_hash.keys.size.times{|room_type_index|
          printf "%d,"%[@area_data[area_id].room_num[i][room_type_index]]
        }
        printf "\n"
      }
    }
    
  end

  def get_area_data( area_id )

    @all_area_data.each_with_index{|line,index|
      case index
      when 0,2 # ヘッダ行
        next
      when 1 # タイトル行
        # エリア数取得
        @area_size = line.size/3

        # エリアワーク作成、フロアサイズ取得
        @area_size.times{|area_id|
          area_line = area_id*3
          area_size = line[area_line+1].scan(/(\d)×(\d)/)

          @area_data[area_id] = $SAreaData.new
          @area_data[area_id].area_line = area_line
          @area_data[area_id].area_line = area_line
          @area_data[area_id].fs_x = area_size[0][0].to_i
          @area_data[area_id].fs_y = area_size[0][1].to_i
          @area_data[area_id].fnum = 0
          @area_data[area_id].room_num = Array.new

        }
      else
        # フロアデータ解析
        @area_size.times{|area_id|
          area_line = @area_data[area_id].area_line

          if !line[area_line].nil? then
            if line[area_line].include?("#END") then
              @area_data[area_id].data_end = true
            end
          end

          # データが終わっている場合はスキップ
          if @area_data[area_id].data_end == true then
            next
          end

          # 階数カウント
          if !line[area_line].nil? then
            room_num_index = @area_data[area_id].fnum
            @area_data[area_id].room_num[ room_num_index ] = Array.new
            # 部屋数の初期化
            $room_type_hash.keys.each_with_index{|key,i|
              @area_data[area_id].room_num[room_num_index][i] = 0
            }
            @area_data[area_id].fnum+=1 
          end

          # 部屋タイプチェック
          room_type = line[area_line+1]
          if !$room_type_hash.key?(room_type) then
            raise "存在しない部屋タイプ:%s"%room_type
          end
           
          # 部屋数保存
          fnum = @area_data[area_id].fnum
          room_index = $room_type_hash[room_type]
          @area_data[area_id].room_num[fnum-1][room_index] += line[area_line+2].to_i
        }
      end
    }
  end

  # フロア数データ出力
  def out_fnum out_data
    @area_size.times{|area_id|
      out_data.puts "#define INFINITYDUNGEON_FLOOR_AREA%d_MAX (%d)"%[area_id+1,@area_data[area_id].fnum]
    }
    out_data.printf "\n"
  end
  
  # エリアデータ出力
  def out_area_data out_data
    @area_size.times{|area_id|
      out_data.print "  { // エリア%d\n"%[area_id+1]
      @area_data[area_id].fnum.times{|i|
        out_data.print "    { "
        $room_type_hash.keys.size.times{|room_type_index|
          out_data.print "%d,"%[@area_data[area_id].room_num[i][room_type_index]]
        }
        out_data.print "},\n"
      }
      out_data.print "  },\n"
    }

    out_data
  end
  
  # フロアサイズデータ出力
  def out_floor_size out_data
    @area_size.times{|area_id|
      out_data.puts " { %d ,%d }, // エリア%d"%[@area_data[area_id].fs_x,@area_data[area_id].fs_x,area_id+1]
    }
  end
  
end

# =========================================
# メイン
W_AREA_CSV_FILENAME   = ARGV[0]
B_AREA_CSV_FILENAME   = ARGV[1]
OUT_FILENAME          = ARGV[2]
FLOOR_NUM_FILENAME = ARGV[3]
FLOOR_SIZE_CDAT_FILENAME = ARGV[4]

# データの読み込み
w_data_csv = CSV.open( W_AREA_CSV_FILENAME ,"r")
b_data_csv = CSV.open( B_AREA_CSV_FILENAME ,"r")

w_area_data = AREA_PURSER.new( w_data_csv )
b_area_data = AREA_PURSER.new( b_data_csv )

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョンフロア数データ
 *	@author	Masayuki Onoue
 *	@date		2011.11.18
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

EOS

out_data = open( FLOOR_NUM_FILENAME ,"w")
out_data.puts header%[FLOOR_NUM_FILENAME]
w_area_data.out_fnum( out_data )
out_data.close

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョンフロアデータ
 *	@author	Masayuki Onoue
 *	@date		2011.11.18
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// ホワイトデータ
static const u8 sc_floor_data_w[INFINITY_DUNGEON_AREA_ID_MAX][INFINITYDUNGEON_FLOOR_MAX][%d] =
{
EOS

out_data = open( OUT_FILENAME ,"w")
out_data.puts header%[OUT_FILENAME,$room_type_hash.keys.size]
w_area_data.out_area_data( out_data )
out_data.puts "};"


header =  <<EOS
// ブラックデータデータ
static const u8 sc_floor_data_b[INFINITY_DUNGEON_AREA_ID_MAX][INFINITYDUNGEON_FLOOR_MAX][%d] =
{
EOS
out_data.puts header%[$room_type_hash.keys.size]
b_area_data.out_area_data( out_data )
out_data.puts "};"
out_data.close


header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョンフロアサイズデータ
 *	@author	Masayuki Onoue
 *	@date		2011.11.18
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//-------------------------------------
// 各エリアのフロアサイズ
//=====================================
static const struct 
{
  int fs_x;
  int fs_y;
}sc_floor_size[INFINITY_DUNGEON_AREA_ID_MAX] =
{
EOS

out_data = open( FLOOR_SIZE_CDAT_FILENAME ,"w")
out_data.puts header%[FLOOR_SIZE_CDAT_FILENAME]
w_area_data.out_floor_size( out_data )
out_data.puts "};"
out_data.close

