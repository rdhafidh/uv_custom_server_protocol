
win32-g++{
DEFINES += ERROR_ELEVATION_REQUIRED=740L
}
UVDIR=
win32*{
DEFINES += _WIN32_WINNT=0x0600 WIN32  _CRT_SECURE_NO_DEPRECATE  _CRT_NONSTDC_NO_DEPRECATE WIN32_LEAN_AND_MEAN

    UVDIR =D:\masteraplikasi\transferh11nov\libuvgit\libuv
SOURCES +=  $$UVDIR/src/fs-poll.c \
  $$UVDIR/src/inet.c \
  $$UVDIR/src/threadpool.c \
  $$UVDIR/src/uv-common.c \
  $$UVDIR/src/version.c \
  $$UVDIR/src/win/async.c \
  $$UVDIR/src/win/core.c \
  $$UVDIR/src/win/dl.c \
  $$UVDIR/src/win/error.c \
  $$UVDIR/src/win/fs.c \
  $$UVDIR/src/win/fs-event.c \
  $$UVDIR/src/win/getaddrinfo.c \
  $$UVDIR/src/win/getnameinfo.c \
  $$UVDIR/src/win/handle.c \
  $$UVDIR/src/win/loop-watcher.c \
  $$UVDIR/src/win/pipe.c \
  $$UVDIR/src/win/thread.c \
  $$UVDIR/src/win/poll.c \
  $$UVDIR/src/win/process.c \
  $$UVDIR/src/win/process-stdio.c \
  $$UVDIR/src/win/req.c \
  $$UVDIR/src/win/signal.c \
  $$UVDIR/src/win/snprintf.c \
  $$UVDIR/src/win/stream.c \
  $$UVDIR/src/win/tcp.c \
  $$UVDIR/src/win/tty.c \
  $$UVDIR/src/win/timer.c \
  $$UVDIR/src/win/udp.c \
  $$UVDIR/src/win/util.c \
  $$UVDIR/src/win/winapi.c \
  $$UVDIR/src/win/winsock.c \
 $$UVDIR/src/win/detect-wakeup.c
INCLUDEPATH += $$UVDIR/include $$UVDIR/src/win $$UVDIR/src
DEPENDPATH += $$UVDIR/include $$UVDIR/src/win $$UVDIR/src

}
win32-g++{
LIBS +=  -lIphlpapi -lPsapi -lws2_32 -ladvapi32 -luser32 -lgdi32 -luserenv
}
win32-msvc*{
LIBS +=   WS2_32.lib Iphlpapi.lib Psapi.lib advapi32.lib user32.lib gdi32.lib UserEnv.lib
}
linux*{
LIBS += -ldl
UVDIR = /home/hv/cpp/libuv
INCLUDEPATH += $$UVDIR $$UVDIR/src $$UVDIR/src/unix $$UVDIR/include
DEFINES += _LARGEFILE_SOURCE _FILE_OFFSET_BITS=64 _GNU_SOURCE
SOURCES += $$UVDIR/src/fs-poll.c \
	$$UVDIR/src/inet.c \
	$$UVDIR/src/threadpool.c \
	$$UVDIR/src/uv-common.c \
	$$UVDIR/src/version.c \
	$$UVDIR/src/unix/async.c \
	$$UVDIR/src/unix/core.c \
	$$UVDIR/src/unix/dl.c \
	$$UVDIR/src/unix/fs.c \
	$$UVDIR/src/unix/getaddrinfo.c \
	$$UVDIR/src/unix/getnameinfo.c \
	$$UVDIR/src/unix/loop.c \
	$$UVDIR/src/unix/loop-watcher.c \
	$$UVDIR/src/unix/pipe.c \
	$$UVDIR/src/unix/poll.c \
	$$UVDIR/src/unix/process.c \
	$$UVDIR/src/unix/signal.c \
	$$UVDIR/src/unix/stream.c \
	$$UVDIR/src/unix/tcp.c \
	$$UVDIR/src/unix/thread.c \
	$$UVDIR/src/unix/timer.c \
	$$UVDIR/src/unix/tty.c \
	$$UVDIR/src/unix/udp.c \
	$$UVDIR/src/unix/proctitle.c \
	$$UVDIR/src/unix/linux-core.c \
	$$UVDIR/src/unix/linux-inotify.c \
	$$UVDIR/src/unix/linux-syscalls.c

}
android*{

}
