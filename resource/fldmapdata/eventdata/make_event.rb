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

#============================================================================
#============================================================================
class EventHeader

  def initialize stream
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
  end

  def calcXofs x
    return ((x / 16) + 16 + @x_ofs * 32) * 16
  end

  def calcZofs z
    return ((-z / 16) + 16 + @z_ofs * 32) * 16
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
      debug_puts "event num = #{num}"
    else
      raise DataFormatError 
    end

    count = 0
    line = stream.gets

    while (count < @num - 1)
      #debug_puts "count:#{count} < @num:#{@num}"
      string = ""
      raise DataFormatError unless line =~/^#event number/
      string += line

      while(true)
        line = stream.gets
        if line =~/^\#event number/ then break end
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
      raise DataFormatError
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
  attr :posx 
  attr :posy
  attr :posz


  def initialize lines, header
    super
    @number = readEventNumber( lines )
    @door_id = read( lines, /#Door Event Label/)
    @next_zone_id = read( lines, /#Next Zone ID Name/)
    @next_door_id = read(lines, /#Next Door ID Name/)
    @x, @y, @z = readPosition(lines)
  end

  def dump output
    gx,gz = calc_ofs @x, @z
    output.puts "\t{//#{@door_id} = #{@number}"
    output.puts "\t\t{#{gx}, #{@y}, #{gz}},"
    #output.puts "\t\t{#{@x}, #{@y}, #{@z}},"
    output.puts "\t\t#{@next_zone_id}, #{@next_door_id},"
    output.puts "\t\tEXIT_DIR_DOWN, EXIT_TYPE_NONE"
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
    @type = read(lines, /#type/)
    @id = read(lines, /#ID name/)
    @obj_code = read(lines, /#OBJ CODE Number/)
    @move_code = read(lines, /#MOVE CODE Number/)
    @event_type = read(lines, /#EVENT TYPE Number/)
    @event_flag = read(lines, /#Flag Name/)
    @event_id = read(lines, "#Event Script Name")
    @dir = read(lines, /#Direction Type Number/)
    @param0 = read(lines, /#Parameter 0 Number/)
    @param1 = read(lines, /#Parameter 1 Number/)
    @param2 = read(lines, /#Parameter 2 Number/)
    @move_limit_x = read(lines, /#Move Limit X Number/)
    @move_limit_z = read(lines, /#Move Limit Z Number/)
    @gx, @gy, @z = read(lines, /#position/).split
  end
 
  def dump output
    gx,gz = calc_ofs @gx, @gz

    output.puts "\t{"
		output.puts "\t\t#{@id}"		    #///<識別ID
		output.puts "\t\t#{@obj_code}"   #MAN1,	///<表示するOBJコード
		output.puts "\t\t#{@move_code}"  #MV_RND,	///<動作コード
		output.puts "\t\t#{@event_type}" #0,	///<イベントタイプ
		output.puts "\t\t#{@event_flag}" #0,	///<イベントフラグ
		output.puts "\t\t#{@event_id}"   #1,	///<イベントID
		output.puts "\t\t#{@dir}"   #DIR_DOWN,	///<指定方向
		output.puts "\t\t#{@param0}"   #0,	///<指定パラメタ 0
		output.puts "\t\t#{@param1}"   #0,	///<指定パラメタ 1
		output.puts "\t\t#{@param2}"   #0,	///<指定パラメタ 2
		output.puts "\t\t#{@move_limit_x}"   #2,	///<X方向移動制限
		output.puts "\t\t#{@move_limit_z}"   #2,	///<Z方向移動制限
		output.puts "\t\t#{gx}"   #757,	///<グリッドX
		output.puts "\t\t#{gz}"   #811,	///<グリッドZ
		#output.puts "\t\t#{@gx}"   #757,	///<グリッドX
		#output.puts "\t\t#{@gz}"   #811,	///<グリッドZ
		output.puts "\t\t#{@y}"   #0,	///<Y値 fx32型
    output.puts "\t},"
  end

  def dumpHeader output
    output.printf( "\#define %-32s  %02d\n", @id, @number)
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
    output.puts "const CONNECT_DATA ConnectData_#{@zonename}[] = {"
    @doors.each{|door| door.dump output }
    output.puts "};"
    output.puts "const int ConnectCount_#{@zonename} = NELEMS(ConnectData_#{@zonename});"
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
    output.puts "\#include \"zone_#{@zonename}evc.h\""
    output.puts "\#include \"../script/#{@zonename}_def.h\""
    output.puts "const FLDMMDL_HEADER FldMMdlHeader_#{@zonename}[] = {"
    @objs.each{|obj| obj.dump(output) }
    output.puts "};"
    output.puts "const int ObjCount_#{@zonename} = NELEMS(FldMMdlHeader_#{zonename})"
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


#============================================================================
#
# コンバート本体
#
#============================================================================
begin
  File.open(ARGV[0]){|file|
    header = EventHeader.new(file)
    obj_events = ObjEventData.new(file, "OBJ_EVENT", header)
    bg_events = EventData.new(file, "BG_EVENT", header)
    pos_events = EventData.new(file, "POS_EVENT", header)
    door_events = DoorEventData.new(file, "DOOR_EVENT", header)

    id = "event_" + door_events.zonename.downcase

    File.open("#{id}.h", "w"){|file|
      door_events.dumpHeader(file)
    }

    File.open("#{id}.cdat", "w"){|file|
      door_events.dump(file)
      #obj_events.dump(file)

      file.puts ""
      file.puts "static const EVENTDATA_TABLE #{id} = {"
      file.puts "\t#{bg_events.items.length},"
      file.puts "\t#{obj_events.items.length},"
      file.puts "\t#{door_events.items.length},"
      file.puts "\t#{pos_events.items.length},"
      file.puts "\t(void *)NULL,"
      file.puts "\t(void *)NULL,"
      if door_events.items.length == 0 then
        file.puts "\t(void *)NULL,"
      else
        file.puts "\t(void *)&ConnectData_#{door_events.zonename.upcase},"
      end
      file.puts "\t(void *)NULL,"
      file.puts "};"
    }
  }
end




