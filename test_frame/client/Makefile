CXX = g++
#FFMPEG = /usr/local
GOOGLE_LOG = /usr/local
#MEMORY = /mnt/hgfs/linux_study/work/tu/MemoryPool/C-98/
#MEMORY = /mnt/hgfs/linux_study/work/tu/MemoryPool/C-11/
INCLUDE_PATH = $(GOOGLE_LOG)/include/
#INCLUDE_PATH = $(FFMPEG)/include/
#INCLUDE_PATH = $(MEMORY)
#INCLUDE += 
#LIB_PATH = $(FFMPEG)/lib/
#FFMPEG_LIBS = -D__STDC_CONSTANT_MACROS -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lasound -lswscale -lswresample -lpostproc -lvpx
GOOGLE_LOG_LIBS = -lglog
#CFLAGS = -I$(FFMPEG)/include -L$(FFMPEG)/lib
#CFLAGS = -I$(INCLUDE) -L$(LIB_PATH)
CFLAGS = -I$(INCLUDE_PATH)
#CXXFLAGS = $(FFMPEG_LIBS) -ldl -lpthread -lSDL2 -lz -lm
CXXFLAGS = $(GOOGLE_LOG_LIBS) -levent_core -levent_extra -levent_pthreads -levent -ldl -lpthread -lz -lm
TARGET = client_frame
DIRS = .
OBJS_FILES = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(OBJS_FILES))
RM = rm -f

#LKFLAGS =
#INSTALLDIR = /mnt/
#install:
#CP = cp
#$(CP) $(TARGET) $(INSTALLDIR)
#VPATH = path1:path2...
#$(wildcard *.c) #搜索当前目前下匹配*.c的所有文件并返回文件列表
#$(patsubst %.c, %.o, $(wildcard *.c)) #将搜索到的与*.c匹配的所有以.c后缀的文件替换成以.o后缀的文件
#$(foreach dir, $(DIRS), $(wildcard $(dir)/*.c)) #循环$(DIRS)中的目录(以空格分隔)依次赋给变量dir，然后搜索该目录下匹配*.c的所有文件并返回文件列表

all:$(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(CXXFLAGS)

$(OBJS): %.o:%.cpp
	$(CXX) -g -c $< -o $@ $(CFLAGS)

#test.o: test.cpp
#	$(CXX) $(CFLAGS) $^

clean:
	@echo "cleaning project"
	-$(RM) $(OBJS)
	@echo "clean completed"
.PHONY: clean
