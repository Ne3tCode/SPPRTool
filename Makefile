GCC = gcc
GXX = g++
INCLUDE_PATH = "./3rdparty/OSW/OpenSteamworks"

CFLAGS = -Wall -Wextra -Wshadow -m32 -O2 -pipe -fno-ident -fno-exceptions -I$(INCLUDE_PATH)
CFLAGS += -DSTEAMWORKS_CLIENT_INTERFACES -DNO_CSTEAMID_STL -DSCRYPT_SALSA -DSCRYPT_SHA256 -DSCRYPT_TEST
CFLAGS += -Wformat-security -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wunreachable-code
CPPFLAGS = $(CFLAGS) -nostdinc++ -std=c++11
LDFLAGS = -s -static

SOURCES_SPPRT = spprt.cpp \
	protoreader.cpp \
	3rdparty\OSW\OpenSteamAPI\src\Interface_OSW.cpp

SOURCES_SCRYPT = 3rdparty\scrypt-jane\scrypt-jane.c

OBJECTS_SPPRT = $(SOURCES_SPPRT:.cpp=.o)
#OBJECTS_SCRYPT = $(SOURCES_SCRYPT:.c=.o)
TARGET_SPPRT = spprt.exe

.PHONY: all clean

.SUFFIXES: .cpp .c .o

all: $(TARGET_SPPRT)

$(TARGET_SPPRT): $(OBJECTS_SPPRT) $(OBJECTS_SCRYPT)
	$(GXX) $(OBJECTS_SPPRT) $(OBJECTS_SCRYPT) $(CPPFLAGS) $(LDFLAGS) -o $@

.cpp.o:
	$(GXX) $(CPPFLAGS) -c $< -o $@

.c.o:
	$(GCC) $(CFLAGS) -c $< -o $@

clean:
	del $(TARGET_SPPRT) $(OBJECTS_SPPRT) $(OBJECTS_SCRYPT)
