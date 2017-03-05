#====================================================================================================
# Description:                 FH 应用程序 MakeFile
#                              
# Author:                      YanDengxue, Fiberhome-Fuhua
#                              
# History:                     Created at 2011.03.22
#                              
# Version:                     1.00
#
# Rev History:  
#       <Author>        <Data>        <Version>        <Description>
#     YanDengxue       2011-03-22       1.00              Create
#====================================================================================================

#====================================================================================================
# 空格定义
#====================================================================================================
empty:=
space:= $(empty) $(empty)

#====================================================================================================
# 源码路径声明
#====================================================================================================
SRC_DIR =./src/
LIB_DIR =./lib/
INC_DIR =./inc/
LNK_DIR =./lnk/
TEMP_DIR=./tmp/
PUBLIC_DIR=./pub/
COMMON_DIR=../common/

#====================================================================================================
# 编译器路径声明
#====================================================================================================
PREX    = arm-none-linux-gnueabi-
AR      = $(PREX)ar
LD      = $(PREX)ld
CC      = $(PREX)gcc
STRIP   = $(PREX)strip
OBJCOPY = $(PREX)objcopy
OBJDUMP = $(PREX)objdump
MK      = make

#====================================================================================================
# 最终生成对象
#====================================================================================================
DEST_OBJ=MMS

ifeq "" "$(DYNAMIC_COMPILE_FLAG)"
DYNAMIC_COMPILE_FLAG=0
endif

#====================================================================================================
# 编译对象定义
#====================================================================================================
OBJS=main.o
ABS_OBJS :=$(addprefix $(TEMP_DIR),$(OBJS))

MMS_LIB_OBJS= ParseCidConfig.o MmsSocketHandle.o
ABS_MMS_LIB_OBJS :=$(addprefix $(TEMP_DIR),$(MMS_LIB_OBJS))

COMMON_LIB_OBJS= ParseString.o ParseConfig.o Asn1EncodeDecode.o
ABS_COMMON_LIB_OBJS :=$(addprefix $(TEMP_DIR),$(COMMON_LIB_OBJS))

ABS_DEPEND_FILES := $(addprefix $(DEPEND_DIR),$(OBJS:.o=.d))

#====================================================================================================
# 库声明
#====================================================================================================
LIB_MMS_DYNAMIC=libMMS.so
ABS_LIB_MMS_DYNAMIC	:=$(addprefix $(LIB_DIR)Dynamic/,$(LIB_MMS_DYNAMIC))

LIB_MMS_STATIC=libMMS.a
ABS_LIB_MMS_STATIC	:=$(addprefix $(LIB_DIR)Static/,$(LIB_MMS_STATIC))

LIB_COMMON=libCommon.a
ABS_LIB_COMMON	:=$(addprefix $(LIB_DIR),$(LIB_COMMON))

#====================================================================================================
# 命令缩写定义
#====================================================================================================
RM=rm -f
CP=cp

#====================================================================================================
# 参数定义
#====================================================================================================
# INCLUDE_DIR不为当前目录
ifneq "" "$(INC_DIR)"
INCLUDE_SWITCH=-I $(INC_DIR)
endif

ifneq "" "$(COMMON_DIR)"
INCLUDE_SWITCH+=-I $(COMMON_DIR)inc -I $(COMMON_DIR)Linux/inc
endif

# 编译参数
CFLAGS  = -mcpu=arm926ej-s -Wall -fPIC -shared -O3 -Werror -Wfatal-errors
CFLAGS  += $(INCLUDE_SWITCH)

# 连接参数定义
LINK_FLAGS_DYNAMIC = -mcpu=arm926ej-s -Wall -O3 -Werror -Wfatal-errors -fPIC -L./lib/Dynamic -lMMS -Wl,-rpath=./lib/Dynamic -lpthread
#LINK_FLAGS = -mcpu=arm926ej-s -Wall -fPIC -L./lib -Wl,-rpath=./lib -O3
LIB_LINK_FLAGS_DYNAMIC = -mcpu=arm926ej-s -Wall -O3 -Werror -Wfatal-errors -fPIC -shared

LINK_FLAGS_STATIC = -mcpu=arm926ej-s -Wall -O3 -Werror -Wfatal-errors -fPIC -static -lpthread
LIB_LINK_FLAGS_STATIC =

#====================================================================================================
# 影响所有的.c编译的PREREQUSITE定义
#====================================================================================================
COMPILE_PREREQUSITE_FOR_ALL =$(INC_DIR)*.h $(COMMON_DIR)inc/*.h $(COMMON_DIR)Linux/inc/*.h Makefile

#====================================================================================================
# 文件依赖定义
#====================================================================================================
ifeq "1" "$(DYNAMIC_COMPILE_FLAG)"
all : $(PUBLIC_DIR)Dynamic/$(DEST_OBJ)
else
all : $(PUBLIC_DIR)Static/$(DEST_OBJ)
endif

# TEMP_DIR不为空
ifneq "" "$(TEMP_DIR)"
$(OBJS)  : %.o:$(TEMP_DIR)%.o#保证可以不带路径可以直接编译
endif

$(PUBLIC_DIR)Dynamic/$(DEST_OBJ) : $(ABS_OBJS) $(ABS_LIB_MMS_DYNAMIC) $(ABS_LIB_COMMON)
	@echo "Dynamic linking..."
	@$(CC) -o $@ $(ABS_OBJS) $(LINK_FLAGS_DYNAMIC) $(ABS_LIB_COMMON)
	@echo "Compile success!"

$(LIB_DIR)Dynamic/libMMS.so: $(ABS_MMS_LIB_OBJS)
	@$(CC) -o $@ $^ $(LIB_LINK_FLAGS_DYNAMIC)

$(PUBLIC_DIR)Static/$(DEST_OBJ) : $(ABS_OBJS) $(ABS_LIB_MMS_STATIC) $(ABS_LIB_COMMON)
	@echo "Static linking..."
	@$(CC) -o $@ $^ $(LINK_FLAGS_STATIC)
	@$(STRIP) -s -g -X -x $@
	@echo "Compile success!"

$(LIB_DIR)Static/libMMS.a: $(ABS_MMS_LIB_OBJS)
	@$(AR) -r $@ $^ $(LIB_LINK_FLAGS_STATIC)

$(LIB_DIR)libCommon.a: $(ABS_COMMON_LIB_OBJS)
	@$(AR) -r $@ $^ $(LIB_LINK_FLAGS_STATIC)

# 通用对象
$(TEMP_DIR)%.o : $(SRC_DIR)%.c $(COMPILE_PREREQUSITE_FOR_ALL)
	@$(CC) -o $@ -c $< $(CFLAGS)

#====================================================================================================
# clean
#====================================================================================================
ifeq "1" "$(DYNAMIC_COMPILE_FLAG)"
clean : 
	$(RM) $(TEMP_DIR)* $(ABS_LIB_COMMON) $(PUBLIC_DIR)Dynamic/$(DEST_OBJ) $(ABS_LIB_MMS_DYNAMIC)
else
clean : 
	$(RM) $(TEMP_DIR)* $(ABS_LIB_COMMON) $(PUBLIC_DIR)Static/$(DEST_OBJ) $(ABS_LIB_MMS_STATIC)

endif
