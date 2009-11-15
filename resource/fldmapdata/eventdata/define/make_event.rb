#============================================================================
#
#
#   イベントデータコンバーター
#
#   2009.05.24    tamada
#   2009.10.07    tomoya  pos_typeをあわせて出力するように変更
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

#ポジションタイプ定数
EVENTDATA_POS_TYPE_GRID = "EVENTDATA_POSTYPE_GRID"
EVENTDATA_POS_TYPE_RAIL = "EVENTDATA_POSTYPE_RAIL"
MMDL_POS_TYPE_GRID = "MMDL_HEADER_POSTYPE_GRID"
MMDL_POS_TYPE_RAIL = "MMDL_HEADER_POSTYPE_RAIL"

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

  def readSize lines
    line = read( lines, /#size/ )
    column = line.split
    return [Integer(column[0]), Integer(column[1])]
  end

  #ポジションタイプの情報があるのかチェックする
  def isPosType lines
    if lines[0] =~ /#Pos Type/
      return true;
    end
    return false;
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
  attr :rail_index
  attr :rail_front
  attr :rail_side
  attr :rail_front_size
  attr :rail_side_size


  def initialize lines, header
    super
    @number = readEventNumber( lines )
    @door_id = read( lines, /#Door Event Label/)
    @next_zone_id = read( lines, /#Next Zone ID Name/)
    @next_door_id = read(lines, /#Next Door ID Name/)
    @door_dir = read(lines, /#Door Direction/)
    @door_type = read(lines, /#Door Type/)

    if isPosType( lines ) == false
      #今は、POSTYPEが出力されないので、こうしておく
      @pos_type = EVENTDATA_POS_TYPE_GRID
      @x, @y, @z = readPosition(lines)
      @sizex = 1
      @sizez = 1
    else
      @pos_type = read(lines, /#Pos Type/)
      if @pos_type == EVENTDATA_POS_TYPE_GRID
        @x, @y, @z = readPosition(lines)
        @sizex, @sizez = readSize(lines)
      else
        @rail_index, @rail_front, @rail_side = readPosition(lines)
        @rail_front_size, @rail_side_size = readSize(lines)
      end
    end
  end

  def dump output
    gx,gz = calc_ofs @x, @z
    output.puts "\t{//#{@door_id} = #{@number}"
    #output.puts "\t\t//#{@x}, #{@z} -- GRID(#{@header.blockSizeX},#{@header.blockSizeY})"
    #output.puts "\t\t//{#{gx/GRID_SIZE}, #{@y/GRID_SIZE}, #{gz/GRID_SIZE}},"
    #output.puts "\t\t{#{@x}, #{@y}, #{@z}},"
    output.puts "\t\t#{@next_zone_id}, #{@next_door_id},"
    output.puts "\t\t#{@door_dir},//direction"
    output.puts "\t\t#{@door_type}, //type"
    output.puts "\t\tEVENTDATA_POSTYPE_GRID, //pos_type"    #ポジションタイプ
    output.puts "\t\t{#{gx}, #{@y}, #{gz}, 1, 1, },"    #座標、グリッドサイズ
    output.puts "\t},"
  end

  def dumpHeader output
    output.printf("\#define %-32s %2d\n", @door_id, @number)
  end

  def getIncludeHeader header

    #zone_id.h
    header.push( "../../zonetable/zone_id.h " )

    #door_id先のヘッダ
    if @next_door_id =~ /DOOR_ID_([^_]+)_/
      zone = $1
      zone = zone.downcase
      header.push( "../tmp/#{zone}.h " )
    else
      raise
    end

    #eventdata_type.h
    header.push( "../../../../prog/include/field/eventdata_type.h " )
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
  attr :pos_type
  attr :gx              #u16 grid x
  attr :gz              #u16 grid z
  attr :y               #fx32 y
  attr :rail_index
  attr :rail_front
  attr :rail_side

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

    if isPosType(lines) == false
      @pos_type = EVENTDATA_POS_TYPE_GRID
      @gx, @y, @gz = readPosition( lines )
      #@gx, @gy, @z = read(lines, /#position/).split
    else
      @pos_type = read( lines, /#Pos Type/ )
      if @pos_type == EVENTDATA_POS_TYPE_GRID
        @gx, @y, @gz = readPosition( lines )
      else
        @rail_index, @rail_front, @rail_side = readPosition( lines )
      end
    end
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
		output.puts "\t\tMMDL_HEADER_POSTYPE_GRID,"   #2,	///<ポジションタイプ
    output.puts "\t\t{"
		output.puts "\t\t\t#{gx},"   #757,	///<グリッドX
		output.puts "\t\t\t#{gz},"   #811,	///<グリッドZ
    @y<<=12 #FX32がたにして出力
		output.puts "\t\t\t#{@y},"   #0,	///<Y値 fx32型
    output.puts "\t\t},"
    output.puts "\t},"
  end

  def dumpHeader output
    output.printf( "\#define %-32s  %2d\n", @id, @number)
  end

  def getIncludeHeader header

    #fldmmdl_objcode.h
    header.push( "../../../../prog/arc/fieldmap/fldmmdl_objcode.h " )

    #fldmmdl_code.h
    header.push( "../../../../prog/src/field/fldmmdl_code.h " )

    #field_dir
    header.push( "../../../../prog/include/field/field_dir.h " )

    #fldmmdl
    header.push( "../../../../prog/src/field/fldmmdl.h " )

    #flag_define.h
    header.push( "../../flagwork/flag_define.h " )

    header.push( "../../../../prog/include/field/eventdata_type.h " )
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
  attr :pos_type
  attr :sizex
  attr :sizez
  attr :x 
  attr :y
  attr :z
  attr :rail_index
  attr :rail_front
  attr :rail_side
  attr :rail_front_size
  attr :rail_side_size


  def initialize lines, header
    super
    @number = readEventNumber( lines )
    @pos_id = read( lines, /#Pos Event Label/)
    @event_id = read(lines, /#Pos Script Name/)
    @workname = read(lines, /#Event Trigger Work Name/)
    @workvalue = read(lines, /#Event Trigger Work Value/)
    
    if isPosType( lines ) == false
      #今は、POSTYPEが出力されないので、こうしておく
      @pos_type = EVENTDATA_POS_TYPE_GRID
      @sizex = read(lines, /#Pos Event Size X/)
      @sizez = read(lines, /#Pos Event Size Z/)
      @x, @y, @z = readPosition(lines)
    else
      @pos_type = read(lines, /#Pos Type/)
      if @pos_type == EVENTDATA_POS_TYPE_GRID
        #@sizex = read(lines, /#Pos Event Size X/)
        #@sizez = read(lines, /#Pos Event Size Z/)
        @x, @y, @z = readPosition(lines)
        @sizex, @sizez = readSize(lines)
      else
        @rail_index, @rail_front, @rail_side = readPosition(lines)
        @rail_front_size, @rail_side_size = readSize(lines)
      end
    end
  end

  def dump output
    gx,gz = calc_grid_ofs @x, @z
    output.puts "\t{//#{@pos_id} = #{@number}"
    output.puts "\t\t#{@event_id},"
    output.puts "\t\t#{@workvalue},"
    output.puts "\t\t#{@workname},"
    output.puts "\t\tEVENTDATA_POSTYPE_GRID,"
    output.puts "\t\t{"
    output.puts "\t\t\t#{gx}, #{gz},"
    output.puts "\t\t\t#{@sizex}, #{@sizez},"
    output.puts "\t\t\t#{@y},  //height"
    output.puts "\t\t},"
    output.puts "\t},"
  end

  def dumpHeader output
    output.printf("\#define %-32s %2d\n", @pos_id, @number)
  end

  def getIncludeHeader header
    #work_define.h
    header.push( "../../flagwork/work_define.h " )

    #eventdata_type.h
    header.push( "../../../../prog/include/field/eventdata_type.h " )
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
  attr :pos_type
  attr :x 
  attr :y
  attr :z
  attr :rail_index
  attr :rail_front
  attr :rail_side


  def initialize lines, header
    super
    @number = readEventNumber( lines )
    @bg_id = read( lines, /#Bg Event Label/)
    @bg_type = read(lines, /#Bg Event Type/)
    @bg_dir = read(lines, /#Bg Event Reaction Direction ID/)
    @event_id = read(lines, /#Bg Script Name/)

    if isPosType( lines ) == false
      #今は、POSTYPEが出力されないので、こうしておく
      @pos_type = EVENTDATA_POS_TYPE_GRID
      @x, @y, @z = readPosition(lines)
    else

      @pos_type = read(lines, /#Pos Type/)

      if @pos_type == EVENTDATA_POS_TYPE_GRID
        @x, @y, @z = readPosition(lines)
      else
        @rail_index, @rail_front, @rail_side = readPosition(lines)
      end

    end
  end

  def dump output
    gx,gz = calc_grid_ofs @x, @z
    output.puts "\t{//#{@bg_id} = #{@number}"
    output.puts "\t\t#{@event_id},  //event id"
    output.puts "\t\t#{@bg_type}, //type"
    output.puts "\t\t#{@bg_dir},//direction"
    output.puts "\t\tEVENTDATA_POSTYPE_GRID,"
    output.puts "\t\t{"
    output.puts "\t\t\t#{gx}, #{gz},"
    output.puts "\t\t\t#{y},  //height"
    output.puts "\t\t},"
    output.puts "\t},"
  end

  def dumpHeader output
    output.printf("\#define %-32s %2d\n", @bg_id, @number)
  end

  def getIncludeHeader header

    #eventdata_type.h
    header.push( "../../../../prog/include/field/eventdata_type.h " )
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

  def getIncludeHeader header
    if @doors.length == 0 then
      return
    end

    @doors.each{|door| door.getIncludeHeader( header ) }
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

  def getIncludeHeader header
    if @objs.length == 0 then
      return
    end
    @objs.each{|obj| obj.getIncludeHeader( header ) }
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

  def getIncludeHeader header
    if @bgs.length == 0 then
      return
    end
    @bgs.each{|bg| bg.getIncludeHeader( header ) }
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

  def getIncludeHeader header
    if @poss.length == 0 then
      return
    end
    @poss.each{|pos| pos.getIncludeHeader( header ) }
  end
end

#============================================================================
#
# uniq　script list set
#
#============================================================================
def set_uniq_script_list_header( depend_header, filename )
  File.open(filename){|file|
    file.each{|line|
      if line =~/\w+\.ev/ then
        column = line.split
        depend_header.push( "../../script/#{column[0].sub(/\.ev/,"")}_def.h " )
      end
    }
  }
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
    ofilename = id.sub(/event_/,"../tmp/")

    File.open("#{ofilename}.h", "w"){|file|
      door_events.dumpHeader(file)
      obj_events.dumpHeader(file)
      bg_events.dumpHeader(file)
      pos_events.dumpHeader(file)
    }


    #依存関係
    depend_header = Array.new

    #set_uniq_script_list_header( depend_header, "../../script/uniq_script.list" )
    depend_header.push( "$(UNIQ_SCRIPTHEADER) " )

    zonename = door_events.zonename.downcase
    depend_header.push( "../tmp/#{zonename}.h " )
    depend_header.push( "../../script/#{zonename}_def.h " )

    door_events.getIncludeHeader( depend_header )
    obj_events.getIncludeHeader( depend_header )
    bg_events.getIncludeHeader( depend_header )
    pos_events.getIncludeHeader( depend_header )

    #重複をなくす
    depend_header = depend_header.uniq()

    File.open( "#{ofilename}.tmp", "w" ){|file|
      file.puts "##{ARGV[0]}"
      file.puts "#{ofilename}.bin:#{ARGV[0]} #{depend_header}"
      file.puts "\t@echo #{ARGV[0]}"
      file.puts "\t@ruby make_binary.rb $< #{depend_header}"

      file.puts ""
    }

#    File.open("#{ofilename}.cdat", "w"){|file|
#      file.puts "\#include \"../../script/#{door_events.zonename.downcase}_def.h\""
#      door_events.dump(file)
#      obj_events.dump(file)
#      bg_events.dump(file)
#      pos_events.dump(file)
#
#      file.puts ""
#      file.puts "static const EVENTDATA_TABLE #{id} = {"
#      file.puts "\t#{bg_events.items.length},"
#      file.puts "\t#{obj_events.items.length},"
#      file.puts "\t#{door_events.items.length},"
#      file.puts "\t#{pos_events.items.length},"
#
#      bg_events.putStructName( file )
#      obj_events.putStructName( file )
#      door_events.putStructName( file )
#      pos_events.putStructName( file )
#
#      file.puts "};"
#    }
  }
end




