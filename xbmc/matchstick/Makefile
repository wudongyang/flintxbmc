INCLUDES+=-I../../lib/libUPnP/Platinum/Source/Core \
          -I../../lib/libUPnP/Platinum/Source/Platinum \
          -I../../lib/libUPnP/Platinum/Source/Devices/MediaConnect \
          -I../../lib/libUPnP/Platinum/Source/Devices/MediaRenderer \
          -I../../lib/libUPnP/Platinum/Source/Devices/MediaServer \
          -I../../lib/libUPnP/Platinum/Source/Extras \
          -I../../lib/libUPnP/Neptune/Source/System/Posix \
          -I../../lib/libUPnP/Neptune/Source/Core
SRCS=GUIDialogMatchStick.cpp \
	 AutoUpdateMatchStick.cpp 

LIB=matchstick.a

include ../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
