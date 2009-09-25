#============================================================================
#
#
#   イベントデータコンバーター
#
#   2009.05.24    tamada
#
#
#============================================================================

#============================================================================
#============================================================================
class DataFormatError < Exception; end

class ReadWMSError < Exception; end

def debug_puts str
  #puts str
end

GRID_SIZE   =   16    #1グリッドのユニット数
BLOCK_SIZE  =   32    #地形ブロック当たりのグリッド数

#============================================================================
#============================================================================
class EventHeader

  attr_reader :blockSizeX, :blockSizeY

  def initialize stream
    @blockSizeX   =   BLOCK_SIZE
    @blockSizeY   =   BLOCK_SIZE

    line = stream.gets
    raise DataFormatError unless line =~ /^#event data/
    line = stream.gets
    raise DataFormatError unless line =~ /^#save date/
    line = stream.gets
    raise DataFormatError unless line =~/# linked worldmap file name/
    line = stream.gets.chomp!  #*.wmsファイル名
    if FileTest.exist? line then
    else
      raise ReadWMSError, "#{line}がみつかりません"
    end
    File.open(line){|file|
      readWMS file
    }
  end

  def read lines, exp
    line = lines.gets
    if line =~ exp then
      return lines.gets
    else
      raise DataFormatError
    end
  end

  def readWMS lines
    line = lines.gets
    raise ReadWMSError unless line =~/WorldMap SaveData/
    line = lines.gets
    raise ReadWMSError unless line =~/save date:/
    @width = read(lines, /^#Width/)
    @height = read(lines, /^#Height/)
    @x_ofs = Integer(read(lines, /^#block x offset/))
    @z_ofs = Integer(read(lines, /^#block z offset/))
    line = lines.gets
    raise ReadWMSError unless line =~/#3dp file names/
    (Integer(@width) * Integer(@height)).times {lines.gets}

    begin 
      line = lines.gets
      if line =~/#block size/ then
        col = lines.gets.split
        @blockSizeX = Integer(col[0])
        @blockSizeY = Integer(col[1])
        debug_puts "BLOCK SIZE  #{blockSizeX},#{blockSizeY}"
      end
    end
    
  end

  def calcXofs x
    return  x + (@blockSizeX / 2 + @x_ofs * @blockSizeX) * GRID_SIZE
  end

  def calcZofs z
    return -z + (@blockSizeY / 2 + @z_ofs * @blockSizeY) * GRID_SIZE
  end

  def calcXgridOfs x
    return x / GRID_SIZE + (@blockSizeX / 2 + @x_ofs * @blockSizeX)
    #return x / GRID_SIZE + (@blockSizeX / 2 + @x_ofs * @blockSizeX) - 1
  end

  def calcZgridOfs z
    return (- z) / GRID_SIZE + (@blockSizeY / 2 + @z_ofs * @blockSizeY)
  end

end

#============================================================================
#
#   イベントデータクラス
#
#============================================================================
class EventData

  attr :zonename
  attr :path
  attr :section_name
  attr :version
  attr :num
  attr :type
  attr :header

  attr :items

  def initialize stream, section_name, header
    @header = header
    @zonename = File.basename(stream.path,".*").upcase
    @path = stream.path
    @section_name = section_name
    @items = Array.new
    flag = false
    text_start = Regexp.new "\#SECTION START:#{@section_name}"
    text_end = Regexp.new "\#SECTION End:#{@section_name}"

    while(true) 
      line = stream.gets
      if line == nil then
        debug_puts "stream read end"
        break
      end
      if line =~text_start then
        debug_puts "SECTION START: #{@section_name}"
        flag = true
        next
      end
      if line =~text_end then
        debug_puts "SECTION END: #{@section_name}"
        break
      end
      if flag == false then
        next
      end

      if line =~/Map Event List data/ then
        debug_puts "START METHOD read_items"
        read_items stream
      end

    end
    if @version == nil || @num == nil || @type == nil then
      raise DataFormatError
    end
  end


  def read_items stream

    if @num == 0 then return end

    dummy = stream.gets #save data
    @version = Integer(stream.gets) #version
    debug_puts "Version = #{version}"

    line = stream.gets
    if line =~ /^#hold event type:/ then
      @type = Integer(stream.gets) #event type
      debug_puts "event type = #{type}"
    else
      raise DataFormatError, line
    end

    if stream.gets =~ /^#hold event number:/ then
      @num = Integer(stream.gets) #event number
      debug_puts "event num = #{@num}"
    else
      raise DataFormatError 
    end

    count = 0
    line = stream.gets

    while (count < @num )
      debug_puts "count:#{count} < @num:#{@num}"
      string = ""
      raise DataFormatError unless line =~/^#event number/
      string += line

      while(true)
        line = stream.gets
        if line =~/^\#event number/ then break end
        if line =~/^\#Map Event List Data End/ then break end
        string += line
      end

      @items << string
      count += 1
    end
    debug_puts "end of read_items"
  end


  def dump output
    debug_puts "number of events:#{@num}"
    @items.each{|item|
      output.puts"#{item}"
    }
  end

  def getStructName
    if @items.length > 0 then
      "#{@section_name}_#{@zonename}"
    else
      "NULL"
    end
  end

  def putStructName output
    if @items.length > 0 then
      output.puts "\t(void *)&#{getStructName},"
    else
      output.puts "\t(void *)NULL,"
    end
  end

end   # end of class EventData

#============================================================================
#============================================================================
class AllEvent
  attr :header

  def initialize lines, header
    @header = header
    #readEventNumber lines
  end

  def read lines, exp
    line = lines.shift
    if line =~ exp then
      return lines.shift
    else
      raise DataFormatError, "\"#{line}\", but need #{exp.to_s}"
    end
  end

  def readEventNumber lines
    line = lines.shift
    raise DataFormatError unless line =~/#event number:/
    column = line.split
    return Integer(column[2])
  end

  def readPosition lines
    line = read( lines, /#position/)
    column = line.split
    return [Integer(column[0]), Integer(column[1]), Integer(column[2])]
  end

  def calc_ofs x, z
    [@header.calcXofs(x), @header.calcZofs(z)]
  end

  def calc_grid_ofs x, z
    [@header.calcXgridOfs(x), @header.calcZgridOfs(z)]
  end

end

#============================================================================
#
# ドアイベントクラス
#
#============================================================================
class DoorEvent < AllEvent

  attr :number
  attr :door_id
  attr :next_zone_id
  attr :next_door_id
  attr :door_dir
  attr :door_type
  attr :posx 
  attr :posy
  attr :posz


  def initialize lines, header
    super
    @number = readEventNumber( lines )
    @door_id = read( lines, /#Door Event Label/)
    @next_zone_id = read( lines, /#Next Zone ID Name/)
    @next_door_id = read(lines, /#Next Door ID Name/)
    @door_dir = read(lines, /#Door Direction/)
    @door_type = read(lines, /#Door Type/)
    @x, @y, @z = readPosition(lines)
  end

  def dump output
    gx,gz = calc_ofs @x, @z
    output.puts "\t{//#{@door_id} = #{@number}"
    #output.puts "\t\t//#{@x}, #{@z} -- GRID(#{@header.blockSizeX},#{@header.blockSizeY})"
    output.puts "\t\t{#{gx}, #{@y}, #{gz}},"
    #output.puts "\t\t//{#{gx/GRID_SIZE}, #{@y/GRID_SIZE}, #{gz/GRID_SIZE}},"
    #output.puts "\t\t{#{@x}, #{@y}, #{@z}},"
    output.puts "\t\t#{@next_zone_id}, #{@next_door_id},"
    output.puts "\t\t#{@door_dir},//direction"
    output.puts "\t\t#{@door_type} //type"
    output.puts "\t},"
  end

  def dumpHeader output
    output.printf("\#define %-32s %2d\n", @door_id, @number)
  end
end

#============================================================================
#
# OBJイベントクラス
#
#============================================================================
class ObjEvent < AllEvent
  attr :number
  attr :type
  attr :id              #u16
  attr :obj_code        #u16
  attr :move_code       #u16
  attr :event_type      #u16
  attr :event_flag      #u16
  attr :event_id        #u16
  attr :dir             #s16
  attr :param0          #u16
  attr :param1          #u16
  attr :param2          #u16
  attr :move_limit_x    #s16
  attr :move_limit_z    #s16
  attr :gx              #u16 grid x
  attr :gz              #u16 grid z
  attr :y               #fx32 y

  def initialize lines, header
    super
    @number = readEventNumber( lines )
    @type = read(lines, /#type/)
    @id = read(lines, /#ID name/)
    @obj_code = read(lines, /#OBJ CODE Number/)
    @move_code = read(lines, /#MOVE CODE Number/)
    @event_type = read(lines, /#EVENT TYPE Number/)
    @event_flag = read(lines, /#Flag Name/)
    @event_id = read(lines, /#Event Script Name/)
    @dir = read(lines, /#Direction Type Number/)
    @param0 = read(lines, /#Parameter 0 Number/)
    @param1 = read(lines, /#Parameter 1 Number/)
    @param2 = read(lines, /#Parameter 2 Number/)
    @move_limit_x = read(lines, /#Move Limit X Number/)
    @move_limit_z = read(lines, /#Move Limit Z Number/)
    @gx, @y, @gz = readPosition( lines )
    #@gx, @gy, @z = read(lines, /#position/).split
  end
 
  def dump output
    gx,gz = calc_grid_ofs @gx, @gz

    output.puts "\t{"
		output.puts "\t\t#{@id},"		    #///<識別ID
		output.puts "\t\t#{@obj_code},"   #MAN1,	///<表示するOBJコード
		output.puts "\t\t#{@move_code},"  #MV_RND,	///<動作コード
		output.puts "\t\t#{@event_type}," #0,	///<イベントタイプ
		output.puts "\t\t#{@event_flag}," #0,	///<イベントフラグ
		output.puts "\t\t#{@event_id},"   #1,	///<イベントID
		output.puts "\t\t#{@dir},"   #DIR_DOWN,	///<指定方向
		output.puts "\t\t#{@param0},"   #0,	///<指定パラメタ 0
		output.puts "\t\t#{@param1},"   #0,	///<指定パラメタ 1
		output.puts "\t\t#{@param2},"   #0,	///<指定パラメタ 2
		output.puts "\t\t#{@move_limit_x},"   #2,	///<X方向移動制限
		output.puts "\t\t#{@move_limit_z},"   #2,	///<Z方向移動制限
		output.puts "\t\t#{gx},"   #757,	///<グリッドX
		output.puts "\t\t#{gz},"   #811,	///<グリッドZ
		output.puts "\t\t#{@y},"   #0,	///<Y値 fx32型
    output.puts "\t},"
  end

  def dumpHeader output
    output.printf( "\#define %-32s  %2d\n", @id, @number)
  end
end

#============================================================================
#
# POSイベントクラス
#
#============================================================================
class PosEvent < AllEvent

  attr :number
  attr :pos_id
  attr :event_id
  attr :workname
  attr :workvalue
  attr :sizex
  attr :sizez
  attr :x 
  attr :y
  attr :z


  def initialize lines, header
    super
    @number = readEventNumber( lines )
    @pos_id = read( lines, /#Pos Event Label/)
    @event_id = read(lines, /#Pos Script Name/)
    @workname = read(lines, /#Event Trigger Work Name/)
    @workvalue = read(lines, /#Event Trigger Work Value/)
    @sizex = read(lines, /#Pos Event Size X/)
    @sizez = read(lines, /#Pos Event Size Z/)
    @x, @y, @z = readPosition(lines)
  end

  def dump output
    gx,gz = calc_grid_ofs @x, @z
    output.puts "\t{//#{@pos_id} = #{@number}"
    output.puts "\t\t#{@event_id},"
    output.puts "\t\t#{gx}, #{gz},"
    output.puts "\t\t#{@sizex}, #{@sizez},"
    output.puts "\t\t#{@y},  //height"
    output.puts "\t\t#{@workvalue},"
    output.puts "\t\t#{@workname},"
    output.puts "\t},"
  end

  def dumpHeader output
    output.printf("\#define %-32s %2d\n", @pos_id, @number)
  end
end

#============================================================================
#
# BGイベントクラス
#
#============================================================================
class BgEvent < AllEvent

  attr :number
  attr :bg_id
  attr :bg_type
  attr :bg_dir
  attr :event_id
  attr :x 
  attr :y
  attr :z


  def initialize lines, header
    super
    @number = readEventNumber( lines )
    @bg_id = read( lines, /#Bg Event Label/)
    @bg_type = read(lines, /#Bg Event Type/)
    @bg_dir = read(lines, /#Bg Event Reaction Direction ID/)
    @event_id = read(lines, /#Bg Script Name/)
    @x, @y, @z = readPosition(lines)
  end

  def dump output
    gx,gz = calc_grid_ofs @x, @z
    output.puts "\t{//#{@bg_id} = #{@number}"
    output.puts "\t\t#{@event_id},  //event id"
    output.puts "\t\t#{@bg_type}, //type"
    output.puts "\t\t#{gx}, #{gz},"
    output.puts "\t\t#{y},  //height"
    output.puts "\t\t#{@bg_dir},//direction"
    output.puts "\t},"
  end

  def dumpHeader output
    output.printf("\#define %-32s %2d\n", @bg_id, @number)
  end
end

#============================================================================
#
# ドアイベントデータクラス
#
#============================================================================
class DoorEventData < EventData

  def initialize stream, section_name, header
    super
    @doors = Array.new
    @items.each{|item|
      lines = item.split(/\r\n/)
      @doors << DoorEvent.new(lines, header)
    }
  end

  def dump output
    if @doors.length == 0 then
      output.puts "//ConnectData_#{@zonename}[]はデータがないため存在しない"
      return
    end
    output.puts "const CONNECT_DATA #{getStructName}[] = {"
    @doors.each{|door| door.dump output }
    output.puts "};"
    output.puts "const int ConnectCount_#{@zonename} = NELEMS(#{getStructName});"
  end

  def dumpHeader output
    incguard = "ZONE_#{@zonename.upcase}_EVD_H"
    output.puts "//このファイルは#{@path}から自動生成されました"
    output.puts "#ifndef #{incguard}"
    output.puts "#define #{incguard}"
    output.puts ""
    @doors.each{|item| item.dumpHeader( output ) }
    output.puts "\n#endif //#{incguard}"
  end

end


#============================================================================
#============================================================================
class ObjEventData < EventData

  def initialize stream, section_name, header
    super
    @objs = Array.new
    @items.each{|item|
      lines = item.split(/\r\n/)
      @objs << ObjEvent.new(lines, header)
    }
  end

  def dump output
    if @objs.length == 0 then return end
    #output.puts "\#include \"zone_#{@zonename.downcase}evc.h\""
    output.puts "const FLDMMDL_HEADER #{getStructName}[] = {"
    @objs.each{|obj| obj.dump(output) }
    output.puts "};"
    output.puts "const int ObjCount_#{@zonename} = NELEMS(#{getStructName});"
  end

  def dumpHeader output
    incguard = "ZONE_#{@zonename.upcase}_EVC_H"
    output.puts "//このファイルは#{@path}から自動生成されました"
    output.puts "#ifndef #{incguard}"
    output.puts "#define #{incguard}"
    output.puts ""
    @objs.each{|item| item.dumpHeader( output ) }
    output.puts "\n#endif //#{incguard}"
  end

end

class BgEventData < EventData

  def initialize stream, section_name, header
    super
    @bgs = Array.new
    @items.each{|item|
      lines = item.split(/\r\n/)
      @bgs << BgEvent.new(lines, header)
    }
  end

  def dump output
    if @bgs.length == 0 then return end
    output.puts "const BG_TALK_DATA #{getStructName}[] = {"
    @bgs.each{|bg| bg.dump(output) }
    output.puts "};"
    output.puts "const int BgCount_#{@zonename} = NELEMS(#{getStructName});"
  end

  def dumpHeader output
  end
end

class PosEventData < EventData

  def initialize stream, section_name, header
    super
    @poss = Array.new
    @items.each{|item|
      lines = item.split(/\r\n/)
      @poss << PosEvent.new(lines, header)
    }
  end

  def dump output
    if @poss.length == 0 then return end
    output.puts "const POS_EVENT_DATA #{getStructName}[] = {"
    @poss.each{|pos| pos.dump(output) }
    output.puts "};"
    output.puts "const int PosCount_#{@zonename} = NELEMS(#{getStructName});"
  end

  def dumpHeader output
  end
end

#============================================================================
#
# コンバート本体
#
#============================================================================
begin
  File.open(ARGV[0]){|file|
    header = EventHeader.new(file)
    obj_events = ObjEventData.new(file, "OBJ_EVENT", header)
    bg_events = BgEventData.new(file, "BG_EVENT", header)
    pos_events = PosEventData.new(file, "POS_EVENT", header)
    door_events = DoorEventData.new(file, "DOOR_EVENT", header)

    id = "event_" + door_events.zonename.downcase
    ofilename = id.sub(/event_/,"tmp/")

    File.open("#{ofilename}.h", "w"){|file|
      door_events.dumpHeader(file)
      obj_events.dumpHeader(file)
      bg_events.dumpHeader(file)
      pos_events.dumpHeader(file)
    }

    File.open("#{ofilename}.cdat", "w"){|file|
      file.puts "\#include \"../script/#{door_events.zonename.downcase}_def.h\""
      door_events.dump(file)
      obj_events.dump(file)
      bg_events.dump(file)
      pos_events.dump(file)

      file.puts ""
      file.puts "static const EVENTDATA_TABLE #{id} = {"
      file.puts "\t#{bg_events.items.length},"
      file.puts "\t#{obj_events.items.length},"
      file.puts "\t#{door_events.items.length},"
      file.puts "\t#{pos_events.items.length},"

      bg_events.putStructName( file )
      obj_events.putStructName( file )
      door_events.putStructName( file )
      pos_events.putStructName( file )

      file.puts "};"
    }
  }
end




