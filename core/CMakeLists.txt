# CMake for WWIV
include_directories(..)

set(COMMON_SOURCES
  clock.cpp
  crc32.cpp
  command_line.cpp
  connection.cpp
  datetime.cpp
  fake_clock.cpp
  file.cpp
  file_lock.cpp
  findfiles.cpp
  graphs.cpp
  http_server.cpp
  inifile.cpp
  log.cpp
  md5.cpp
  net.cpp
  os.cpp
  semaphore_file.cpp
  socket_connection.cpp
  socket_exceptions.cpp
  strings.cpp
  textfile.cpp
  version.cpp
  )

if(UNIX) 
  set(PLATFORM_SOURCES 
    file_unix.cpp
    os_unix.cpp
    wfndfile_unix.cpp
    )
  add_definitions ("-Wall")
endif()

if(WIN32)
  include(FindWWIVCurses)

  set(PLATFORM_SOURCES 
    file_win32.cpp
    os_win.cpp
    wfndfile_win32.cpp
    )
    SET_WARNING_LEVEL_4()

endif()


add_library(core ${COMMON_SOURCES} ${PLATFORM_SOURCES})
