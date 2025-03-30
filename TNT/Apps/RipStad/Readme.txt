-------------------------
 RipStad version 1.00.Alpha
-------------------------

| Comments |
  This is the revised stadium ripper for All-Star Baseball.  This ripper is heavily based on RipGeom.  Major differences are the
  MTF format, full lightmap support, stadium collision and stadium objects support.  

| Revision History |
  v 1.00.Alpha - snicholson - checked in

| Command Line |
  Command line options are as follows (also shown when run with no commands):

  Usage: RipGeom [options] <Inputs> <output.GDF>
  Options:
    -PS2 ............ Target PlayStation 2
    -PC ........... . Target PC
    -GC ............. Target GameCube
    -XBOX ........... Target XBOX
    -M .............. Use mip-mapping
	-N .............. Build as a night stadium (use night textures)
    -DL ............. Use dynamic lighting
    -A .............. Enable alpha
    -AA.............. Enable antialiasing
    -NT ............. Don't export textures
    -T <filename> ... Export texture names to <filename>
	-C <filename> ... Export collision data to <filename>, if you don't it will be left as geometry
	-G <filename> ... Export grid point data to <filename>
    -EP ............. Export vertex position ptrs (Always on XBOX)
    -ET ............. Export vertex texture coord. ptrs (Always on XBOX)
    -EC ............. Export vertex color data ptrs (Always on XBOX)
    -EN ............. Export vertex normal ptrs
    -V .............. Verbose output
  Special Notes:
    -DL cannot be combined with -EC  (except for XBOX)
    -EN requires -DL be specified
