

wind = Array.new
ship = Array.new
trailerL = Array.new
trailerR = Array.new


#-----------
file = File.open( ARGV[0], "r" ) 
line = file.readlines[1]
data = line.split( /\s/ )
wind << data[1].to_i
wind << data[3].to_f
wind << data[4].to_f
file.close

file = File.open( ARGV[0], "r" ) 
line = file.readlines[2]
data = line.split( /\s/ )
ship << data[1].to_i
ship << data[2].to_f
ship << data[5].to_i
file.close

file = File.open( ARGV[0], "r" ) 
line = file.readlines[3]
data = line.split( /\s/ )
trailerL << data[1].to_i
trailerL << data[2].to_f
trailerL << data[5].to_i
file.close

file = File.open( ARGV[0], "r" ) 
line = file.readlines[4]
data = line.split( /\s/ )
trailerR << data[1].to_i
trailerR << data[2].to_f
trailerR << data[5].to_i
file.close
#-----------

file = File.open( "3d_sound_h01.bin", "w" ) 
file.write( wind.pack( "Iff" ) )
file.write( ship.pack( "IFI" ) )
file.write( trailerL.pack( "IFI" ) )
file.write( trailerR.pack( "IFI" ) )
file.close
