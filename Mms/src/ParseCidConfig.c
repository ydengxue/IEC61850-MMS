/*****************************************************************************************************
* FileName:                    ParseCidConfig.c
*
* Description:                 CID配置文件解析
*                              
* Author:                      YanDengxue, Fiberhome-Fuhua
*                              
* Rev History:  
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
//====================================================================================================
// 本文件使用的头文件
//====================================================================================================
// 库头文件
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <bits/signum.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>

// 自定义头文件
#include "UserTypesDef.h"
#include "MmsBaseData.h"
#include "ParseString.h"
#include "ParseConfig.h"
#include "Debug.h"

//====================================================================================================
// 数据类型定义及宏定义
//====================================================================================================
typedef struct
{
    char const *const name;
    MMS_BTYPE  btype;
} MMS_BTYPE_DEFINE;

typedef struct
{
    char const *const name;
    MMS_USER_BTYPE  btype;
    Uint8      align;
} MMS_USER_BTYPE_DEFINE;

//====================================================================================================
// 本地函数声明,此处声明的函数不与外部接口
//====================================================================================================
//static int32 MmsCidParseTest(MMS_IED *p_mms_ied);

//static int32 MmsBtypeToString(void const *addr, MMS_BTYPE b_type, char *buffer, int32 buffer_length);
static int32 MmsDaiGetValueAddrType(MMS_SDI *p_mms_sdi, char const *p_saddr);
static void *MmsDaiDefaultValueMalloc(MMS_BTYPE type, char const *src);
static MMS_LN  *MallocLNodeInstance(MMS_LNODETYPE *p_lnodetype);
static MMS_DOI *MallocDoInstance(MMS_LNODETYPE_ELEMENT *p_lnodetype_element);
static MMS_SDI *MallocSdInstance(MMS_DOTYPE_ELEMENT *p_dotype_element);
static MMS_SDI *MallocDaInstance(MMS_BDA *p_bda);

static int32 ParseMmsIed(MMS_IED **pp_mms_ied, MMS_LNODETYPE *p_mms_lntype, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count);
static int32 ParseDataSet(MMS_LDEVICE *p_mms_ldevice, char const *src_buffer_start, char const *src_buffer_end, int32 line_count);
static int32 ParseDoInstance(MMS_DOI *p_doi, char const **pp_file_buffer, char const *src_buffer_end, int32 *p_line_count);
static int32 ParseSdInstance(MMS_SDI *p_sdi, char const **pp_file_buffer, char const *src_buffer_end, int32 *p_line_count);
static int32 ParseMmsLNodeType(MMS_LNODETYPE **pp_mms_lntype, MMS_DOTYPE *p_mms_dotype, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count);
static int32 ParseMmsDoType(MMS_DOTYPE **pp_mms_dotype, MMS_DATYPE *p_mms_datype, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count);
static int32 ParseMmsDaType(MMS_DATYPE **pp_mms_datype, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count);

//====================================================================================================
// 本文件定义的与外部的接口变量
//====================================================================================================
MMS_FC_DEFINE const mms_fc[] =
{
    {NULL,  0xFFFFu},
    {"ST",  MMS_FC_ST},
    {"MX",  MMS_FC_MX},
    {"CO",  MMS_FC_CO},
    {"SP",  MMS_FC_SP},
    {"SV",  MMS_FC_SV},
    {"CF",  MMS_FC_CF},
    {"DC",  MMS_FC_DC},
    {"SG",  MMS_FC_SG},
    {"SE",  MMS_FC_SE},
    {"EX",  MMS_FC_EX},
};

int32 const mms_fc_num = sizeof(mms_fc) / sizeof(mms_fc[0]);

//====================================================================================================
// 本地全局变量
//====================================================================================================
static int32 file_line_count_global;
static int32 file_size_global;
static char *file_buffer_global;
static char const *file_buffer_end_global;
static char *public_buffer_global;
static int32 const public_buffer_size_global = 1024;

static MMS_BTYPE_DEFINE const mms_btype[] =
{
    {"BOOLEAN",      MMS_BTYPE_BOOLEAN},
    {"INT8",         MMS_BTYPE_INT8},
    {"INT16",        MMS_BTYPE_INT16},
    {"INT32",        MMS_BTYPE_INT32},
    {"INT8U",        MMS_BTYPE_INT8U},
    {"INT16U",       MMS_BTYPE_INT16U},
    {"INT32U",       MMS_BTYPE_INT32U},
    {"FLOAT32",      MMS_BTYPE_FLOAT32},
    {"Enum",         MMS_BTYPE_ENUM},
    {"Dbpos",        MMS_BTYPE_DBPOS},
    {"Tcmd",         MMS_BTYPE_TCMD},
    {"Quality",      MMS_BTYPE_QUALITY},
    {"Timestamp",    MMS_BTYPE_TIMESTAMP},
    {"VisString64",  MMS_BTYPE_VISSTRING64},
    {"VisString255", MMS_BTYPE_VISSTRING255},
    {"Octet64",      MMS_BTYPE_OCTET64},
    {"Struct",       MMS_BTYPE_STRUCT},
    {"EntryTime",    MMS_BTYPE_ENTRYTIME},
    {"Unicode255",   MMS_BTYPE_UNICODE255},
    {"Check",        MMS_BTYPE_CHECK},
};

static MMS_USER_BTYPE_DEFINE const mms_user_btype[] =
{
    {"BOOLEAN",      MMS_USER_BTYPE_BOOLEAN,    sizeof(Uint8)},
    {"INT8",         MMS_USER_BTYPE_INT8,       sizeof(int8)},
    {"INT16",        MMS_USER_BTYPE_INT16,      sizeof(int16)},
    {"INT32",        MMS_USER_BTYPE_INT32,      sizeof(int32)},
    {"INT8U",        MMS_USER_BTYPE_INT8U,      sizeof(Uint8)},
    {"INT16U",       MMS_USER_BTYPE_INT16U,     sizeof(Uint16)},
    {"INT32U",       MMS_USER_BTYPE_INT32U,     sizeof(Uint32)},
    {"FLOAT32",      MMS_USER_BTYPE_FLOAT32,    sizeof(float)},
    {"STRING",       MMS_USER_BTYPE_STRING,     0},
};

static MMS_DATYPE *p_mms_datype;
static MMS_DOTYPE *p_mms_dotype;
static MMS_LNODETYPE *p_mms_lnodetype;

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
// 接口函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: ParseCidConfig
//      Input: char const *file_name 
//     Output: void
//     Return: int32: 函数执行情况
//Description: 系统程序入口
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
MMS_IED *ParseCidConfig(char const *file_name)
{
    int32 lv_line_count;
    int  lv_file_read_fd;
    char const *lv_parse_start_addr;
    char const *lv_parse_end_addr;
    MMS_IED *lv_p_mms_ied;
    struct stat lv_file_stat;

    if (NULL == file_name) 
    {
        TRACE("function entries can't be NULL!");
        return NULL;
    }

    if (0 != stat(file_name, &lv_file_stat))
    {
        perror("stat");
        TRACE("\"%s\" stat failed!", file_name);
        return NULL;
    }

    if ((lv_file_stat.st_size <= 0) || (lv_file_stat.st_size >= 0x200000u))
    {
        TRACE("\"%s\" size exceed the rage (0,0x200000)!", file_name);
        return NULL;
    }

    file_buffer_global = malloc(lv_file_stat.st_size);
    if (NULL == file_buffer_global)
    {
        TRACE("\"%s\" file_buffer malloc failed!", file_name);
        return NULL;
    }

    lv_file_read_fd = open(file_name, O_RDONLY);
    if (lv_file_read_fd < 0)
    {
        TRACE("\"%s\" file open failed!", file_name);
        free(file_buffer_global);
        return NULL;
    }

    file_size_global = read(lv_file_read_fd, file_buffer_global, lv_file_stat.st_size);
    if ((file_size_global < 0) || (file_size_global != lv_file_stat.st_size)) 
    {
        TRACE("\"%s\" file read failed!", file_name);
        close(lv_file_read_fd);
        free(file_buffer_global);
        return NULL;
    }

    close(lv_file_read_fd);

    file_line_count_global = 0;
    file_buffer_end_global = file_buffer_global + lv_file_stat.st_size;

    public_buffer_global = malloc(public_buffer_size_global);
    if (NULL == public_buffer_global)
    {
        TRACE("\"%s\" public_buffer malloc failed!", file_name);
        free(file_buffer_global);
        return NULL;
    }

    lv_parse_start_addr = GetKeyWordAddr("<DataTypeTemplates", file_buffer_global, file_buffer_end_global, &file_line_count_global);
    if (NULL == lv_parse_start_addr)
    {
        TRACE("\"%s\" DataTypeTemplates start addr failed!", file_name);
        free(file_buffer_global);
        free(public_buffer_global);
        return NULL;
    }

    lv_line_count = file_line_count_global;

    lv_parse_end_addr = GetKeyWordAddr("</DataTypeTemplates", lv_parse_start_addr, file_buffer_end_global, &lv_line_count);
    if (NULL == lv_parse_end_addr)
    {
        TRACE("\"%s\" DataTypeTemplates end addr failed!", file_name);
        free(file_buffer_global);
        free(public_buffer_global);
        return NULL;    
    }

    p_mms_datype = NULL;
    if (NORMAL_SUCCESS != ParseMmsDaType(&p_mms_datype, lv_parse_start_addr, lv_parse_end_addr, &lv_line_count))
    {
        TRACE("DAType parse failed");        
        free(file_buffer_global);
        free(public_buffer_global);
        return NULL;
    }

    p_mms_dotype = NULL;
    if (NORMAL_SUCCESS != ParseMmsDoType(&p_mms_dotype, p_mms_datype, lv_parse_start_addr, lv_parse_end_addr, &lv_line_count))
    {
        TRACE("DOType parse failed");        
        free(file_buffer_global);
        free(public_buffer_global);
        return NULL;
    }

    p_mms_lnodetype = NULL;
    if (NORMAL_SUCCESS != ParseMmsLNodeType(&p_mms_lnodetype, p_mms_dotype, lv_parse_start_addr, lv_parse_end_addr, &lv_line_count))
    {
        TRACE("LNodeType parse failed");        
        free(file_buffer_global);
        free(public_buffer_global);
        return NULL;
    }

    file_line_count_global = 0;

    lv_parse_start_addr = GetKeyWordAddr("<IED", file_buffer_global, file_buffer_end_global, &file_line_count_global);
    if (NULL == lv_parse_start_addr)
    {
        TRACE("\"%s\" IED start addr failed!", file_name);
        free(file_buffer_global);
        free(public_buffer_global);
        return NULL;    
    }

    lv_line_count = file_line_count_global;

    lv_parse_end_addr = GetKeyWordAddr("</IED", lv_parse_start_addr, file_buffer_end_global, &lv_line_count);
    if (NULL == lv_parse_end_addr)
    {
        TRACE("\"%s\" IED end addr failed!", file_name);
        free(file_buffer_global);
        free(public_buffer_global);
        return NULL;    
    }

    lv_p_mms_ied = NULL;
    if (NORMAL_SUCCESS != ParseMmsIed(&lv_p_mms_ied, p_mms_lnodetype, lv_parse_start_addr, lv_parse_end_addr, &lv_line_count))
    {
        TRACE("IED parse failed");        
        free(file_buffer_global);
        free(public_buffer_global);
        return NULL;
    }
    
//if (NORMAL_SUCCESS != MmsCidParseTest(lv_p_mms_ied))
//{
//    TRACE("MmsCid parse failed");        
//    free(file_buffer_global);
//    free(public_buffer_global);
//    return NULL;
//}

    free(file_buffer_global);
    free(public_buffer_global);

    return lv_p_mms_ied;    
}

//----------------------------------------------------------------------------------------------------
// 本地函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: ParseMmsIed
//      Input: 
//     Output: void
//     Return: int32 
//Description: MMS Ied解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 ParseMmsIed(MMS_IED **pp_mms_ied, MMS_LNODETYPE *p_mms_lnodetype, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count)
{
    int32 lv_parse_status;
    int32 lv_line_count;
    int32 lv_line_count_dataset;
    int32 lv_buffer_length;
    char const *lv_p_buffer;
    char const *lv_p_file_buffer;
    char const *lv_p_file_buffer_temp;
    char const *lv_p_file_buffer_dataset_start;
    char const *lv_p_file_buffer_dataset_end;
    char lv_buffer[64u];
    char *lv_p_string;
    MMS_IED *lv_p_mms_ied;
    MMS_ACCESS_POINT *lv_p_mms_access_point;
    MMS_SERVER  *lv_p_mms_server;
    MMS_LDEVICE *lv_p_mms_ldevice;
    MMS_LN *lv_p_mms_ln;
    MMS_LNODETYPE *lv_mms_lnodetype_find;
    MMS_DOI *lv_p_mms_doi_active;

    if ((NULL == pp_mms_ied) || (NULL == p_mms_lnodetype) || (NULL == src_buffer_start) || (NULL == src_buffer_end) || (NULL == p_line_count))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_ied = NULL;
    lv_p_mms_access_point = NULL;
    lv_p_mms_server = NULL;
    lv_p_mms_ldevice = NULL;
    lv_p_mms_ln = NULL;
    lv_p_file_buffer_dataset_start = NULL;
    lv_p_file_buffer_dataset_end = NULL;

    lv_p_mms_ied = calloc(1, sizeof(MMS_IED));
    if (NULL == lv_p_mms_ied)
    {
        TRACE("mms ied calloc failed!");
        return NORMAL_ERROR;
    }

    *pp_mms_ied = lv_p_mms_ied;

    lv_line_count = *p_line_count;
    lv_p_file_buffer = src_buffer_start;
    lv_line_count_dataset = lv_line_count;
    
    lv_buffer_length = public_buffer_size_global;
    lv_p_file_buffer = GetOneLine(public_buffer_global, &lv_buffer_length, lv_p_file_buffer, src_buffer_end, &lv_line_count);

    lv_p_buffer = public_buffer_global;
    lv_p_buffer = GetContentToSplit(lv_p_buffer, lv_buffer, sizeof(lv_buffer));
    if (0 != strcmp(lv_buffer, "<IED"))
    {
        TRACE("keyword IED check failed!");
        return NORMAL_ERROR;
    }
    
    GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
    if ('\0' == lv_buffer[0])
    {
        TRACE("IED get name failed!");
        return NORMAL_ERROR;
    }

    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED name calloc failed!");
        return NORMAL_ERROR;
    }

    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
    lv_p_mms_ied->name = lv_p_string;

    GetContentOfKeyword("type", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED type calloc failed!");
        return NORMAL_ERROR;
    }
          
    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
    lv_p_mms_ied->type = lv_p_string;
        

    GetContentOfKeyword("manufacturer", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED manufacturer calloc failed!");
        return NORMAL_ERROR;
    }

    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
    lv_p_mms_ied->manufacturer = lv_p_string;

    GetContentOfKeyword("configVersion", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED configVersion calloc failed!");
        return NORMAL_ERROR;
    }

    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
    lv_p_mms_ied->config_version = lv_p_string;


    GetContentOfKeyword("desc", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED desc calloc failed!");
        return NORMAL_ERROR;
    }
          
    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
    lv_p_mms_ied->desc = lv_p_string;

    lv_parse_status = 0;
    do
    {
        lv_p_file_buffer_temp = lv_p_file_buffer;
        lv_buffer_length = public_buffer_size_global;
        lv_p_file_buffer = GetOneLine(public_buffer_global, &lv_buffer_length, lv_p_file_buffer, src_buffer_end, &lv_line_count);

        lv_p_buffer = public_buffer_global;
        lv_p_buffer = GetContentToSplit(lv_p_buffer, lv_buffer, sizeof(lv_buffer));
        if (0 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<AccessPoint"))
            {
                lv_p_mms_access_point = calloc(1, sizeof(MMS_ACCESS_POINT));
                if (NULL == lv_p_mms_access_point)
                {
                    TRACE("IED access point calloc failed!");
                    return NORMAL_ERROR;
                }
                lv_p_mms_access_point->left = lv_p_mms_ied->access_point;
                lv_p_mms_ied->access_point = lv_p_mms_access_point;

                GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("IED manufacturer calloc failed!");
                    return NORMAL_ERROR;
                }

                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_access_point->name = lv_p_string;
                
                GetContentOfKeyword("desc", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("IED manufacturer calloc failed!");
                    return NORMAL_ERROR;
                }
                
                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_access_point->desc = lv_p_string;

                lv_parse_status = 1;
            }
        }
        else if (1 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<Server"))
            {
                lv_p_mms_server = calloc(1, sizeof(MMS_SERVER));
                if (NULL == lv_p_mms_server)
                {
                    TRACE("IED server calloc failed!");
                    return NORMAL_ERROR;
                }
                lv_p_mms_server->left = lv_p_mms_access_point->server;
                lv_p_mms_access_point->server = lv_p_mms_server;

                lv_parse_status = 2;
            }
            else if (0 == strcmp(lv_buffer, "</AccessPoint"))
            {
                lv_parse_status = 0;
            }
        }
        else if (2 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<LDevice"))
            {
                lv_p_mms_ldevice = calloc(1, sizeof(MMS_LDEVICE));
                if (NULL == lv_p_mms_ldevice)
                {
                    TRACE("IED LDevice calloc failed!");
                    return NORMAL_ERROR;
                }
                lv_p_mms_ldevice->left = lv_p_mms_server->ldevice;
                lv_p_mms_server->ldevice = lv_p_mms_ldevice;

                GetContentOfKeyword("inst", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("IED manufacturer calloc failed!");
                    return NORMAL_ERROR;
                }
                
                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_ldevice->inst = lv_p_string;

                lv_parse_status = 3;
            }
            else if (0 == strcmp(lv_buffer, "</Server"))
            {
                lv_parse_status = 1;
            }
        }
        else if (3 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<LN"))
            {
                if (NULL == lv_p_mms_ldevice->ln_head)
                {
                    TRACE("LLN0 must be the first LN!");
                    return NORMAL_ERROR;
                }

                GetContentOfKeyword("lnType", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                for (lv_mms_lnodetype_find = p_mms_lnodetype; NULL != lv_mms_lnodetype_find; lv_mms_lnodetype_find = lv_mms_lnodetype_find->right)
                {
                    if (0 == strcmp(lv_mms_lnodetype_find->id, lv_buffer))
                    {
                        break;
                    }
                }

                if (NULL == lv_mms_lnodetype_find)
                {
                    TRACE("lnType=%s find failed!", lv_buffer);
                    return NORMAL_ERROR;
                }

                lv_p_mms_ln = MallocLNodeInstance(lv_mms_lnodetype_find);
                if (NULL == lv_p_mms_ln)
                {
                    TRACE("lnType=%s copy failed!", lv_buffer);
                    return NORMAL_ERROR;
                }
                lv_p_mms_ldevice->ln_tail->right = lv_p_mms_ln;
                lv_p_mms_ln->left = lv_p_mms_ldevice->ln_tail;
                lv_p_mms_ldevice->ln_tail = lv_p_mms_ln;

                GetContentOfKeyword("lnClass", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                if (0 != strcmp(lv_buffer, lv_p_mms_ln->lnclass))
                {
                    TRACE("ln \"%s\"'s lnClass=%s is not equal to LNodeType's lnClass=%s!", lv_p_mms_ln->lntype, lv_buffer, lv_p_mms_ln->lnclass);
                    return NORMAL_ERROR;
                }

                GetContentOfKeyword("prefix", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("IED manufacturer calloc failed!");
                    return NORMAL_ERROR;
                }

                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_ln->prefix = lv_p_string;

                GetContentOfKeyword("inst", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("IED manufacturer calloc failed!");
                    return NORMAL_ERROR;
                }

                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_ln->inst = lv_p_string;

                GetContentOfKeyword("desc", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("IED manufacturer calloc failed!");
                    return NORMAL_ERROR;
                }

                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_ln->desc = lv_p_string;

                lv_parse_status = 4;
            }
            else if (0 == strcmp(lv_buffer, "<LN0"))
            {
                if (NULL != lv_p_mms_ldevice->ln_head)
                {
                    TRACE("only support one LLN0!");
                    return NORMAL_ERROR;
                }

                GetContentOfKeyword("lnType", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                for (lv_mms_lnodetype_find = p_mms_lnodetype; NULL != lv_mms_lnodetype_find; lv_mms_lnodetype_find = lv_mms_lnodetype_find->right)
                {
                    if (0 == strcmp(lv_mms_lnodetype_find->id, lv_buffer))
                    {
                        break;
                    }
                }

                if (NULL == lv_mms_lnodetype_find)
                {
                    TRACE("lnType=%s find failed!", lv_buffer);
                    return NORMAL_ERROR;
                }

                lv_p_mms_ldevice->ln_head = MallocLNodeInstance(lv_mms_lnodetype_find);
                if (NULL == lv_p_mms_ldevice->ln_head)
                {
                    TRACE("lnType=%s copy failed!", lv_buffer);
                    return NORMAL_ERROR;
                }
                lv_p_mms_ldevice->ln_tail = lv_p_mms_ldevice->ln_head;
                lv_p_mms_ln = lv_p_mms_ldevice->ln_head;

                GetContentOfKeyword("lnClass", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                if (0 != strcmp(lv_buffer, lv_p_mms_ln->lnclass))
                {
                    TRACE("ln \"%s\"'s lnClass=%s is not equal to LNodeType's lnClass=%s!", lv_p_mms_ln->lntype, lv_buffer, lv_p_mms_ln->lnclass);
                    return NORMAL_ERROR;
                }

                GetContentOfKeyword("prefix", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("IED prefix calloc failed!");
                    return NORMAL_ERROR;
                }

                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_ln->prefix = lv_p_string;

                GetContentOfKeyword("inst", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("IED ln inst calloc failed!");
                    return NORMAL_ERROR;
                }

                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_ln->inst = lv_p_string;
                
                GetContentOfKeyword("desc", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("IED ln desc calloc failed!");
                    return NORMAL_ERROR;
                }

                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_ln->desc = lv_p_string;

                lv_p_file_buffer_dataset_start = NULL;
                lv_parse_status = 5;
            }
            else if (0 == strcmp(lv_buffer, "</LDevice"))
            {
                if ((NULL != lv_p_file_buffer_dataset_start) && (NULL != lv_p_file_buffer_dataset_end))
                {
                    if (NORMAL_SUCCESS != ParseDataSet(lv_p_mms_ldevice, lv_p_file_buffer_dataset_start, lv_p_file_buffer_dataset_end, lv_line_count_dataset))
                    {
                        TRACE("parse dataset failed!");
                        return NORMAL_ERROR;
                    }
                }
                lv_parse_status = 2;
            }
        }
        else if ((4 == lv_parse_status) || (5 == lv_parse_status))
        {
            if (0 == strcmp(lv_buffer, "<DOI"))
            {
                GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                for (lv_p_mms_doi_active = lv_p_mms_ln->doi; NULL != lv_p_mms_doi_active; lv_p_mms_doi_active = lv_p_mms_doi_active->left)
                {
                    if (0 == strcmp(lv_buffer, lv_p_mms_doi_active->name))
                    {
                        break;
                    }
                }

                if (NORMAL_SUCCESS != ParseDoInstance(lv_p_mms_doi_active, &lv_p_file_buffer, src_buffer_end, &lv_line_count))
                {
                    TRACE("parse one doi failed!");
                    return NORMAL_ERROR;
                }
            }
            else if (0 == strcmp(lv_buffer, "</LN"))
            {
                if (4 == lv_parse_status)
                {
                    lv_parse_status = 3;
                }
                else
                {
                    TRACE("The end should be </LN>!");
                    return NORMAL_ERROR;
                }
            }
            else if (0 == strcmp(lv_buffer, "<DataSet"))
            {
                if (5 == lv_parse_status)
                {
                    if (NULL == lv_p_file_buffer_dataset_start)
                    {
                        lv_p_file_buffer_dataset_start = lv_p_file_buffer_temp;
                        lv_line_count_dataset = lv_line_count - 1;
                    }

                    lv_parse_status = 6;
                }
                else
                {
                    TRACE("DataSet should be in LN0>!");
                    return NORMAL_ERROR;
                }
            }
            else if (0 == strcmp(lv_buffer, "</LN0"))
            {
                if (5 == lv_parse_status)
                {
                    lv_parse_status = 3;
                }
                else
                {
                    TRACE("The end should be </LN0>!");
                    return NORMAL_ERROR;
                }
            }
            else
            {
                TRACE("get doi keyword failed!");
                return NORMAL_ERROR;
            }
        }
        else if (6 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "</DataSet"))
            {
                lv_p_file_buffer_dataset_end = lv_p_file_buffer;
                lv_parse_status = 5;
            }
        }
        else
        {
            TRACE("unknown status %ld!", lv_parse_status);
            return NORMAL_ERROR;
        }
            
    } while (NULL != lv_p_file_buffer);

    return NORMAL_SUCCESS;
}


static int32 ParseDataSet(MMS_LDEVICE *p_mms_ldevice, char const *src_buffer_start, char const *src_buffer_end, int32 line_count)
{
    int32 lv_index;
    int32 lv_parse_status;
    int32 lv_buffer_length;
    char const *lv_p_buffer;
    char const *lv_p_file_buffer;
    char lv_buffer[64u];
    char lv_buffer1[64u];
    char *lv_p_local_buffer;
    char *lv_p_local_buffer_temp;
    char *lv_p_string;
    int32 lv_string_length;
    int32 lv_line_count;
    MMS_FCD *lv_p_mms_fcd;
    MMS_DOI *lv_p_mms_doi_find;
    MMS_SDI *lv_p_mms_sdi_find;
    MMS_FCD_SDI *lv_p_mms_fcd_sdi;
    MMS_LN *lv_p_mms_ln_find;
    MMS_DATASET *lv_p_mms_dataset;
    MMS_LDEVICE *lv_p_mms_ldevice;

    lv_line_count = line_count;
    lv_p_file_buffer = src_buffer_start;
    lv_p_mms_ldevice = p_mms_ldevice;

    lv_parse_status = 0;
    do
    {
        lv_buffer_length = public_buffer_size_global;
        lv_p_file_buffer = GetOneLine(public_buffer_global, &lv_buffer_length, lv_p_file_buffer, src_buffer_end, &lv_line_count);

        lv_p_buffer = public_buffer_global;
        lv_p_buffer = GetContentToSplit(lv_p_buffer, lv_buffer, sizeof(lv_buffer));
        if (0 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<DataSet"))
            {
                lv_p_mms_dataset = calloc(1, sizeof(MMS_DATASET));
                if (NULL == lv_p_mms_dataset)
                {
                    TRACE("IED dataset calloc failed!");
                    return NORMAL_ERROR;
                }
                lv_p_mms_dataset->left = lv_p_mms_ldevice->dataset;
                lv_p_mms_ldevice->dataset = lv_p_mms_dataset;

                GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                if ('\0' == lv_buffer[0])
                {
                    TRACE("DataSet get name failed!");
                    return NORMAL_ERROR;
                }

                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("DataSet name calloc failed!");
                    return NORMAL_ERROR;
                }

                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_dataset->name = lv_p_string;

                GetContentOfKeyword("desc", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                if (NULL == lv_p_string)
                {
                    TRACE("DataSet desc calloc failed!");
                    return NORMAL_ERROR;
                }

                strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                lv_p_mms_dataset->desc= lv_p_string;

                lv_parse_status = 1;
            }
        }
        else if (1 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<FCDA"))
            {
                GetContentOfKeyword("ldInst", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                if (0 != strcmp(lv_buffer, p_mms_ldevice->inst))
                {
                    TRACE("ldInst=%s is accross LD, not support!", lv_buffer);
                    return NORMAL_ERROR;
                }

                lv_p_mms_fcd = calloc(1, sizeof(MMS_FCD));
                if (NULL == lv_p_mms_fcd)
                {
                    TRACE("IED FCD calloc failed!");
                    return NORMAL_ERROR;
                }

                lv_p_mms_fcd->left = lv_p_mms_dataset->fcd;
                lv_p_mms_dataset->fcd = lv_p_mms_fcd;

                lv_p_mms_fcd->ldevice = p_mms_ldevice;

                GetContentOfKeyword("prefix", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_string_length = strlen(lv_buffer);
                GetContentOfKeyword("lnClass", lv_p_buffer, &lv_buffer[lv_string_length], sizeof(lv_buffer) - lv_string_length);
                lv_string_length += strlen(&lv_buffer[lv_string_length]);
                GetContentOfKeyword("lnInst", lv_p_buffer, &lv_buffer[lv_string_length], sizeof(lv_buffer) - lv_string_length);
                for (lv_p_mms_ln_find = lv_p_mms_ldevice->ln_tail; NULL != lv_p_mms_ln_find; lv_p_mms_ln_find = lv_p_mms_ln_find->left)
                {
                    snprintf(lv_buffer1, sizeof(lv_buffer1), "%s%s%s", lv_p_mms_ln_find->prefix, lv_p_mms_ln_find->lnclass, lv_p_mms_ln_find->inst);
                    if (0 == strcmp(lv_buffer1, lv_buffer))
                    {
                        break;
                    }
                }

                if (NULL == lv_p_mms_ln_find)
                {
                    TRACE("lnInst=%s find failed!", lv_buffer);
                    return NORMAL_ERROR;
                }

                
                lv_p_mms_fcd->ln = lv_p_mms_ln_find;

                GetContentOfKeyword("fc", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                for (lv_index = 1; lv_index < mms_fc_num; lv_index++)
                {
                    if (0 == strcmp(mms_fc[lv_index].name, lv_buffer))
                    {
                        lv_p_mms_fcd->fc_index = lv_index;
                        break;
                    }
                }

                if (lv_index >= mms_fc_num)
                {
                    TRACE("unknown FCDA FC=%s", lv_buffer);
                    return NORMAL_ERROR;
                }


                GetContentOfKeyword("doName", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_local_buffer_temp = strchr(lv_buffer, '.');
                if (NULL != lv_p_local_buffer_temp)
                {
                    *lv_p_local_buffer_temp = '\0';
                    lv_p_local_buffer_temp++;
                    
                }
                for (lv_p_mms_doi_find = lv_p_mms_ln_find->doi; NULL != lv_p_mms_doi_find; lv_p_mms_doi_find = lv_p_mms_doi_find->left)
                {
                    if (0 == strcmp(lv_p_mms_doi_find->name, lv_buffer))
                    {
                        break;
                    }
                }

                if (NULL == lv_p_mms_doi_find)
                {
                    TRACE("doName=%s find failed!", lv_buffer);
                    return NORMAL_ERROR;
                }

                lv_p_mms_fcd->doi = lv_p_mms_doi_find;

                lv_p_mms_sdi_find = lv_p_mms_doi_find->sdi;
                while (NULL != lv_p_local_buffer_temp)
                {
                    lv_p_local_buffer = lv_p_local_buffer_temp;
                    lv_p_local_buffer_temp = strchr(lv_p_local_buffer, '.');
                    if (NULL != lv_p_local_buffer_temp)
                    {
                        *lv_p_local_buffer_temp = '\0';
                        lv_p_local_buffer_temp++;
                    }

                    for (; NULL != lv_p_mms_sdi_find; lv_p_mms_sdi_find = lv_p_mms_sdi_find->left)
                    {
                        if (0 == strcmp(lv_p_mms_sdi_find->name, lv_p_local_buffer))
                        {
                            break;
                        }
                    }

                    if (NULL == lv_p_mms_sdi_find)
                    {
                        TRACE("sdoName==%s find failed!", lv_buffer);
                        return NORMAL_ERROR;
                    }

                    lv_p_mms_fcd_sdi = calloc(1, sizeof(MMS_FCD_SDI));
                    if (NULL == lv_p_mms_fcd_sdi)
                    {
                        TRACE("IED FCD sdi calloc failed!");
                        return NORMAL_ERROR;
                    }
                    lv_p_mms_fcd_sdi->sdi = lv_p_mms_sdi_find;
                    lv_p_mms_sdi_find = lv_p_mms_sdi_find->sdi;
                    if (    (NULL != lv_p_mms_sdi_find)
                         && (    (MMS_BTYPE_SDO != lv_p_mms_sdi_find->btype)
                              || (MMS_BTYPE_STRUCT!= lv_p_mms_sdi_find->btype)))
                    {
                        TRACE("sdoName==%s don't have sub sdi!", lv_buffer);
                        return NORMAL_ERROR;
                    }

                    if (NULL == lv_p_mms_fcd->fcd_sdi)
                    {
                        lv_p_mms_fcd->fcd_sdi = lv_p_mms_fcd_sdi;
                    }
                }


                if (NORMAL_SUCCESS == GetContentOfKeyword("daName", lv_p_buffer, lv_buffer, sizeof(lv_buffer)))
                {
                    lv_p_local_buffer_temp = lv_buffer;
                    while (NULL != lv_p_local_buffer_temp)
                    {
                        lv_p_local_buffer = lv_p_local_buffer_temp;
                        lv_p_local_buffer_temp = strchr(lv_p_local_buffer, '.');
                        if (NULL != lv_p_local_buffer_temp)
                        {
                            *lv_p_local_buffer_temp = '\0';
                            lv_p_local_buffer_temp++;
                        }

                        for (; NULL != lv_p_mms_sdi_find; lv_p_mms_sdi_find = lv_p_mms_sdi_find->left)
                        {
                            if (0 == strcmp(lv_p_mms_sdi_find->name, lv_p_local_buffer))
                            {
                                break;
                            }
                        }

                        if (NULL == lv_p_mms_sdi_find)
                        {
                            TRACE("daName==%s find failed!", lv_buffer);
                            return NORMAL_ERROR;
                        }

                        lv_p_mms_fcd_sdi = calloc(1, sizeof(MMS_FCD_SDI));
                        if (NULL == lv_p_mms_fcd_sdi)
                        {
                            TRACE("IED FCD sdi calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_fcd_sdi->sdi = lv_p_mms_sdi_find;
                        lv_p_mms_sdi_find = lv_p_mms_sdi_find->sdi;
                        if (    (NULL != lv_p_mms_sdi_find)
                             && (    (MMS_BTYPE_SDO != lv_p_mms_sdi_find->btype)
                                  || (MMS_BTYPE_STRUCT!= lv_p_mms_sdi_find->btype)))
                        {
                            TRACE("sdoName==%s don't have sub sdi!", lv_buffer);
                            return NORMAL_ERROR;
                        }

                        if (NULL == lv_p_mms_fcd->fcd_sdi)
                        {
                            lv_p_mms_fcd->fcd_sdi = lv_p_mms_fcd_sdi;
                        }
                    }
                }
            }
            else if (0 == strcmp(lv_buffer, "</DataSet"))
            {
                lv_parse_status = 0;
            }
            else
            {
                TRACE("FCDA get failed!");
                return NORMAL_ERROR;
            }
        }
    } while (NULL != lv_p_file_buffer);
            
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: ParseDoInstance
//      Input: 
//     Output: void
//     Return: int32 
//Description: MMS DoInstace解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 ParseDoInstance(MMS_DOI *p_doi, char const **pp_file_buffer, char const *src_buffer_end, int32 *p_line_count)
{
    int32 lv_parse_status;
    int32 lv_line_count;
    int32 lv_buffer_length;
    char const *lv_p_file_buffer;
    char const *lv_p_buffer;
    char const *lv_p_buffer_temp;
    char lv_buffer[64u];
//    char *lv_p_string;
    MMS_SDI *lv_p_mms_sdi_active;

    if ((NULL == p_doi) || (NULL == pp_file_buffer) || (NULL == p_line_count))
    {
        TRACE("Function entries error!");
        return NORMAL_ERROR;
    }

    lv_parse_status = 0;
    lv_line_count = *p_line_count;
    lv_p_file_buffer = *pp_file_buffer;
    do
    {
        lv_buffer_length = public_buffer_size_global;
        lv_p_file_buffer = GetOneLine(public_buffer_global, &lv_buffer_length, lv_p_file_buffer, src_buffer_end, &lv_line_count);

        lv_p_buffer = public_buffer_global;
        lv_p_buffer = GetContentToSplit(lv_p_buffer, lv_buffer, sizeof(lv_buffer));
        if (0 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<DAI"))
            {
                GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                lv_p_mms_sdi_active = NULL;
                for (lv_p_mms_sdi_active = p_doi->sdi; NULL != lv_p_mms_sdi_active; lv_p_mms_sdi_active = lv_p_mms_sdi_active->left)
                {
                    if (0 == strcmp(lv_buffer, lv_p_mms_sdi_active->name))
                    {
                        break;
                    }
                }

                if (NULL == lv_p_mms_sdi_active)
                {
                    TRACE("\"%s\" not find in DOI!", lv_buffer);
                    return NORMAL_ERROR;
                }

                GetContentOfKeyword("sAddr", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                if ('\0' != lv_buffer[0])
                {
//                    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
//                    if (NULL == lv_p_string)
//                    {
//                        TRACE("IED ln desc calloc failed!");
//                        return NORMAL_ERROR;
//                    }
//
//                    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
//                    lv_p_mms_sdi_active->saddr = lv_p_string;
                    if (NORMAL_SUCCESS != MmsDaiGetValueAddrType(lv_p_mms_sdi_active, lv_buffer))
                    {
                        TRACE("dai=\"%s\" value addr and type get failed!", lv_p_mms_sdi_active->name);
                        return NORMAL_ERROR;
                    }
                }

                if (NORMAL_SUCCESS != CheckTheEndOfXmlElement(lv_p_buffer, "DAI"))
                {
                    lv_parse_status = 1;
                }
            }
            else if (0 == strcmp(lv_buffer, "<SDI"))
            {
                GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                for (lv_p_mms_sdi_active = p_doi->sdi; NULL != lv_p_mms_sdi_active; lv_p_mms_sdi_active = lv_p_mms_sdi_active->left)
                {
                    if (0 == strcmp(lv_buffer, lv_p_mms_sdi_active->name))
                    {
                        break;
                    }
                }

                if (NULL == lv_p_mms_sdi_active)
                {
                    TRACE("\"%s\" not find in DIO!", lv_buffer);
                    return NORMAL_ERROR;
                }

                if (NORMAL_SUCCESS != ParseSdInstance(lv_p_mms_sdi_active, &lv_p_file_buffer, src_buffer_end, &lv_line_count))
                {
                    TRACE("\"%s\" not find in LNType!", lv_buffer);
                    return NORMAL_ERROR;
                }
            }
            else if (0 == strcmp(lv_buffer, "</DOI"))
            {
                break;
            }
            else
            {
                TRACE("\"<SDI\" or \"<DAI\" keyword get failed!");
                return NORMAL_ERROR;
            }
        }
        else if (1 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<Val"))
            {
                if (NORMAL_SUCCESS != CheckTheEndOfXmlElement(lv_p_buffer, "Val"))
                {
                    lv_parse_status = 2;
                }
                else
                {
                    if (NULL == lv_p_mms_sdi_active->addr)
                    {
                        GetContentToEndStr(lv_p_buffer, "</", lv_buffer, sizeof(lv_buffer));
                        if ('\0' == lv_buffer[0])
                        {
                            TRACE("dai=\"%s\" default value should not be NULL!", lv_p_mms_sdi_active->name);
                            return NORMAL_ERROR;
                        }

                        lv_p_mms_sdi_active->default_value_addr = MmsDaiDefaultValueMalloc(lv_p_mms_sdi_active->btype, lv_buffer);
                        if (NULL == lv_p_mms_sdi_active->default_value_addr)
                        {
                            TRACE("dai=\"%s\" default value Malloc failed!", lv_p_mms_sdi_active->name);
                            return NORMAL_ERROR;
                        }
                    }
                }
            }
            else if (0 == strcmp(lv_buffer, "</DAI"))
            {
                lv_parse_status = 0;
            }
            else
            {
                TRACE("\"Val\" keyword get failed!");
                return NORMAL_ERROR;
            }
        }
        else if (2 == lv_parse_status)
        {
            if (NULL == lv_p_mms_sdi_active->addr)
            {
                lv_p_buffer_temp = public_buffer_global;
                GetContentToEndStr(lv_p_buffer_temp, "</", lv_buffer, sizeof(lv_buffer));
                if ('\0' == lv_buffer[0])
                {
                    TRACE("dai=\"%s\" default value should not be NULL!", lv_p_mms_sdi_active->name);
                    return NORMAL_ERROR;
                }

                lv_p_mms_sdi_active->default_value_addr = MmsDaiDefaultValueMalloc(lv_p_mms_sdi_active->btype, lv_buffer);
                if (NULL == lv_p_mms_sdi_active->default_value_addr)
                {
                    TRACE("dai=\"%s\" default value Malloc failed!", lv_p_mms_sdi_active->name);
                    return NORMAL_ERROR;
                }
            }
            lv_parse_status = 3;
        }
        else if (3 == lv_parse_status)
        {
            if (NORMAL_SUCCESS == CheckTheEndOfXmlElement(lv_p_buffer, "Val"))
            {
                lv_parse_status = 4;
            }
            else
            {
                TRACE("Val parse error!");
                return NORMAL_ERROR;
            }
        }
        else if (4 == lv_parse_status)
        {
            if (NORMAL_SUCCESS == CheckTheEndOfXmlElement(lv_p_buffer, "DAI"))
            {
                lv_parse_status = 0;
            }
            else
            {
                TRACE("DAI parse error!");
                return NORMAL_ERROR;
            }
        }
        else
        {
            TRACE("DIO parse status error!");
            return NORMAL_ERROR;
        }
        
    } while (NULL != lv_p_file_buffer);


    if (0 != lv_parse_status)
    {
        TRACE("DIO parse status error!");
        return NORMAL_ERROR;
    }

    *pp_file_buffer = lv_p_file_buffer;

    return NORMAL_SUCCESS;
}


//----------------------------------------------------------------------------------------------------
//   Function: ParseSdinstance
//      Input: 
//     Output: void
//     Return: int32 
//Description: MMS DoInstace解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 ParseSdInstance(MMS_SDI *p_sdi, char const **pp_file_buffer, char const *src_buffer_end, int32 *p_line_count)
{
    int32 lv_parse_status;
    int32 lv_line_count;
    int32 lv_buffer_length;
    char const *lv_p_file_buffer;
    char const *lv_p_buffer;
    char const *lv_p_buffer_temp;
    char lv_buffer[64u];
//    char *lv_p_string;
    MMS_SDI *lv_p_mms_sdi_active;

    if ((NULL == p_sdi) || (NULL == pp_file_buffer) || (NULL == p_line_count))
    {
        TRACE("Function entries error!");
        return NORMAL_ERROR;
    }

    lv_parse_status = 0;
    lv_line_count = *p_line_count;
    lv_p_file_buffer = *pp_file_buffer;
    do
    {
        lv_buffer_length = public_buffer_size_global;
        lv_p_file_buffer = GetOneLine(public_buffer_global, &lv_buffer_length, lv_p_file_buffer, src_buffer_end, &lv_line_count);

        lv_p_buffer = public_buffer_global;
        lv_p_buffer = GetContentToSplit(lv_p_buffer, lv_buffer, sizeof(lv_buffer));
        if (0 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<SDI"))
            {
                GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                for (lv_p_mms_sdi_active = p_sdi->sdi; NULL != lv_p_mms_sdi_active; lv_p_mms_sdi_active = lv_p_mms_sdi_active->left)
                {
                    if (0 == strcmp(lv_buffer, lv_p_mms_sdi_active->name))
                    {
                        break;
                    }
                }

                if (NULL == lv_p_mms_sdi_active)
                {
                    TRACE("\"%s\" not find in LNType!", lv_buffer);
                    return NORMAL_ERROR;
                }

                if (NORMAL_SUCCESS != ParseSdInstance(lv_p_mms_sdi_active, &lv_p_file_buffer, src_buffer_end, &lv_line_count))
                {
                    TRACE("\"%s\" not find in LNType!", lv_buffer);
                    return NORMAL_ERROR;
                }
            }
            else if (0 == strcmp(lv_buffer, "<DAI"))
            {
                GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                for (lv_p_mms_sdi_active = p_sdi->sdi; NULL != lv_p_mms_sdi_active; lv_p_mms_sdi_active = lv_p_mms_sdi_active->left)
                {
                    if (0 == strcmp(lv_buffer, lv_p_mms_sdi_active->name))
                    {
                        break;
                    }
                }

                if (NULL == lv_p_mms_sdi_active)
                {
                    TRACE("\"%s\" not find in SDI!", lv_buffer);
                    return NORMAL_ERROR;
                }

                GetContentOfKeyword("sAddr", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                if ('\0' != lv_buffer[0])
                {
//                    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
//                    if (NULL == lv_p_string)
//                    {
//                        TRACE("IED ln desc calloc failed!");
//                        return NORMAL_ERROR;
//                    }
//
//                    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
//                    lv_p_mms_sdi_active->saddr = lv_p_string;
                    if (NORMAL_SUCCESS != MmsDaiGetValueAddrType(lv_p_mms_sdi_active, lv_buffer))
                    {
                        TRACE("dai=\"%s\" value addr and type get failed!", lv_p_mms_sdi_active->name);
                        return NORMAL_ERROR;
                    }

                }

                if (NORMAL_SUCCESS != CheckTheEndOfXmlElement(lv_p_buffer, "DAI"))
                {
                    lv_parse_status = 1;
                }
            }
            else if (0 == strcmp(lv_buffer, "</SDI"))
            {
                break;
            }
            else
            {
                TRACE("\"<DOI\" keyword get failed!");
                return NORMAL_ERROR;
            }
        }
        else if (1 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<Val"))
            {
                if (NORMAL_SUCCESS != CheckTheEndOfXmlElement(lv_p_buffer, "Val"))
                {
                    lv_parse_status = 2;
                }
                else
                {
                    if (NULL == lv_p_mms_sdi_active->addr)
                    {
                        GetContentToEndStr(lv_p_buffer, "</", lv_buffer, sizeof(lv_buffer));
                        if ('\0' == lv_buffer[0])
                        {
                            TRACE("dai=\"%s\" default value should not be NULL!", lv_p_mms_sdi_active->name);
                            return NORMAL_ERROR;
                        }

                        lv_p_mms_sdi_active->default_value_addr = MmsDaiDefaultValueMalloc(lv_p_mms_sdi_active->btype, lv_buffer);
                        if (NULL == lv_p_mms_sdi_active->default_value_addr)
                        {
                            TRACE("dai=\"%s\" default value Malloc failed!", lv_p_mms_sdi_active->name);
                            return NORMAL_ERROR;
                        }
                    }
                }
            }
            else if (0 == strcmp(lv_buffer, "</DAI"))
            {
                lv_parse_status = 0;
            }
            else
            {
                TRACE("\"Val\" keyword get failed!");
                return NORMAL_ERROR;
            }
        }
        else if (2 == lv_parse_status)
        {
           if (NULL == lv_p_mms_sdi_active->addr)
            {
            lv_p_buffer_temp = public_buffer_global;
            GetContentToEndStr(lv_p_buffer_temp, "</", lv_buffer, sizeof(lv_buffer));
            if ('\0' == lv_buffer[0])
            {
                TRACE("dai=\"%s\" default value should not be NULL!", lv_p_mms_sdi_active->name);
                return NORMAL_ERROR;
            }

            lv_p_mms_sdi_active->default_value_addr = MmsDaiDefaultValueMalloc(lv_p_mms_sdi_active->btype, lv_buffer);
            if (NULL == lv_p_mms_sdi_active->default_value_addr)
            {
                TRACE("dai=\"%s\" default value Malloc failed!", lv_p_mms_sdi_active->name);
                return NORMAL_ERROR;
                }
            }
            lv_parse_status = 3;
        }
        else if (3 == lv_parse_status)
        {
            if (NORMAL_SUCCESS == CheckTheEndOfXmlElement(lv_p_buffer, "Val"))
            {
                lv_parse_status = 4;
            }
            else
            {
                TRACE("Val parse error!");
                return NORMAL_ERROR;
            }
        }
        else if (4 == lv_parse_status)
        {
            if (NORMAL_SUCCESS == CheckTheEndOfXmlElement(lv_p_buffer, "DAI"))
            {
                lv_parse_status = 0;
            }
            else
            {
                TRACE("DAI parse error!");
                return NORMAL_ERROR;
            }
        }
        else
        {
            TRACE("DIO parse status error!");
            return NORMAL_ERROR;
        }
        
    } while (NULL != lv_p_file_buffer);


    if (0 != lv_parse_status)
    {
        TRACE("DIO parse status error!");
        return NORMAL_ERROR;
    }

    *pp_file_buffer = lv_p_file_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: MmsBtypeToString
//      Input: 
//     Output: void
//     Return: int32 
//Description: MMS 基本类型转换成字符串
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
/*
static int32 MmsBtypeToString(void const *addr, MMS_BTYPE b_type, char *buffer, int32 buffer_length)
{
    void const *lv_addr;
    MMS_BTYPE lv_type;

    if ((NULL == addr) || (NULL == buffer))
    {
        TRACE("function entries failed!");
        return NORMAL_ERROR;
    }

    lv_type = b_type;
    lv_addr = addr;

    switch (lv_type)
    {
        case MMS_BTYPE_BOOLEAN:
        {
            snprintf(buffer, buffer_length, "%u", *((Uint8 const *)lv_addr) & 0x01u);
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_INT8:
        {
            snprintf(buffer, buffer_length, "%d", *((int8 const *)lv_addr));
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_INT16:
        {
            snprintf(buffer, buffer_length, "%d", *((int16 const *)lv_addr));
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_INT32:
        {
            snprintf(buffer, buffer_length, "%ld", *((int32 const *)lv_addr));
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_INT8U:
        {
            snprintf(buffer, buffer_length, "%u", *((Uint8 const *)lv_addr) & 0xFFu);
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_INT16U:
        {
            snprintf(buffer, buffer_length, "%u", *((Uint16 const *)lv_addr) & 0xFFFFu);
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_INT32U:
        {
            snprintf(buffer, buffer_length, "%lu", *((Uint32 const *)lv_addr) & 0xFFFFu);
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_FLOAT32:
        {
            snprintf(buffer, buffer_length, "%f", *((float const *)lv_addr));
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_FLOAT64:
        {
            snprintf(buffer, buffer_length, "%lf", *((double const *)lv_addr));
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_ENUM:
        {
            snprintf(buffer, buffer_length, "%lu", *((Uint32 const *)lv_addr));
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_DBPOS:
        {
            snprintf(buffer, buffer_length, "%u", *((Uint8 const *)lv_addr) & 0xFFu);
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_TCMD:
        {
            snprintf(buffer, buffer_length, "%u", *((Uint8 const *)lv_addr) & 0xFFu);
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_QUALITY:
        {
            return NORMAL_ERROR;
        }
        case MMS_BTYPE_TIMESTAMP:
        {
            return NORMAL_ERROR;
        }
        case MMS_BTYPE_VISSTRING64:
        {
            snprintf(buffer, buffer_length, "%s", ((char const *)lv_addr));
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_VISSTRING255:
        {
            snprintf(buffer, buffer_length, "%s", ((char const *)lv_addr));
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_OCTET64:
        case MMS_BTYPE_ENTRYTIME:
        {
            return NORMAL_ERROR;
        }
        case MMS_BTYPE_UNICODE255:
        {
            snprintf(buffer, buffer_length, "%s", ((char const *)lv_addr));
            return NORMAL_SUCCESS;
        }
        case MMS_BTYPE_CHECK:
        {
            snprintf(buffer, buffer_length, "%u", *((Uint8 const *)lv_addr) & 0xFFu);
            return NORMAL_SUCCESS;
        }
        default:
        {
            TRACE("unknown data type %d!", lv_type);
            return NORMAL_ERROR;
        }
    }

    return NORMAL_SUCCESS;
}
*/
//----------------------------------------------------------------------------------------------------
//   Function: MmsDaiGetValueAddrType
//      Input: 
//     Output: void
//     Return: int32 
//Description: MMS 获取saddr的地址及类型
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 MmsDaiGetValueAddrType(MMS_SDI *p_mms_sdi, char const *p_saddr)
{
    extern Uint8 *mms_dsp_base_addr_va;

    char lv_buffer[64u];
    char const *lv_p_buffer;
    char *lv_p_buffer_temp;
    MMS_SDI *lv_p_mms_sdi;
    int32 lv_index;
    Uint32 lv_offset;
    
    if ((NULL == p_mms_sdi) || (NULL == p_saddr))
    {
        TRACE("function entrys error!");
        return NORMAL_ERROR;
    }

    lv_p_buffer = p_saddr;
    lv_p_mms_sdi = p_mms_sdi;
    lv_p_buffer = GetContentToEndStr(lv_p_buffer, ":", lv_buffer, sizeof(lv_buffer));
    if (NULL == lv_p_buffer)
    {
        TRACE("sAddr=%s parse error, no addr", p_saddr);
        return NORMAL_ERROR;
    }

    for (lv_index = 0; lv_index < sizeof(mms_user_btype) / sizeof(mms_user_btype[0]); lv_index++)
    {
        if (0 == strcmp(mms_user_btype[lv_index].name, lv_buffer))
        {
            lv_p_mms_sdi->user_btype = mms_user_btype[lv_index].btype;
            break;
        }
    }

    if (sizeof(mms_user_btype) / sizeof(mms_user_btype[0]) == lv_index)
    {
        TRACE("unknown mms_btype=%s", lv_buffer);
        return NORMAL_ERROR;
    }

    lv_p_buffer = GetContentToEndStr(lv_p_buffer, ":", lv_buffer, sizeof(lv_buffer));
    if ('\0' == lv_buffer)
    {
        TRACE("get addr from %s failed!", p_saddr);
        return NORMAL_ERROR;
    }

    lv_offset = strtoul(lv_buffer, &lv_p_buffer_temp, 0);
    if ((NULL != lv_p_buffer_temp) && ('\0' != *lv_p_buffer_temp))
    {
        TRACE("get addr from %s failed, it is not a digit!", p_saddr);
        return NORMAL_ERROR;
    }

    if ((0 != mms_user_btype[lv_index].align) && (0 != (lv_offset % mms_user_btype[lv_index].align)))
    {
        TRACE("get addr from %s failed, it is not a %d align!", p_saddr, mms_user_btype[lv_index].align);
        return NORMAL_ERROR;
    }

    p_mms_sdi->addr = mms_dsp_base_addr_va + lv_offset;
    if (NULL != lv_p_buffer)
    {
        if (('w' == *lv_p_buffer) || ('W' == *lv_p_buffer))
        {
            lv_p_mms_sdi->property |= 0x01u;
        }
    }

    return NORMAL_SUCCESS;
}


//----------------------------------------------------------------------------------------------------
//   Function: MmsDaiDefaultValueMalloc
//      Input: 
//     Output: void
//     Return: void * 
//Description: Mms DAI默认值处理
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static void *MmsDaiDefaultValueMalloc(MMS_BTYPE type, char const *src)
{
    char *lv_p_string;
    int32 lv_string_length;
    Uint32 lv_ulong_temp;
    int32  lv_long_temp;
    float  lv_float_temp;
    long double lv_ldouble_temp;
    void *lv_p_void;

    if (NULL == src)
    {
        TRACE("function entrys error!");
        return NULL;
    }

    switch (type)
    {
        case MMS_BTYPE_BOOLEAN:
        {
            lv_ulong_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(Uint8));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((Uint8 *)lv_p_void) = lv_ulong_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_INT8:
        {
            lv_long_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(int8));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((int8 *)lv_p_void) = lv_long_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_INT16:
        {
            lv_long_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(int16));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((int16 *)lv_p_void) = lv_long_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_INT32:
        {
            lv_long_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(int32));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((int32 *)lv_p_void) = lv_long_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_INT8U:
        {
            lv_ulong_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(Uint8));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((Uint8 *)lv_p_void) = lv_ulong_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_INT16U:
        {
            lv_ulong_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(Uint16));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((Uint16 *)lv_p_void) = lv_ulong_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_INT32U:
        {
            lv_ulong_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(Uint32));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((Uint32 *)lv_p_void) = lv_ulong_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_FLOAT32:
        {
            lv_float_temp = strtof(src, NULL);
            lv_p_void = calloc(1, sizeof(float));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((float *)lv_p_void) = lv_float_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_FLOAT64:
        {
            lv_ldouble_temp = strtold(src, NULL);
            lv_p_void = calloc(1, sizeof(double));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((long double *)lv_p_void) = lv_ldouble_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_TCMD:
        case MMS_BTYPE_ENUM:
        {
            lv_ulong_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(Uint32));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((Uint32 *)lv_p_void) = lv_ulong_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_DBPOS:
        {
            lv_ulong_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(Uint8));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((Uint8 *)lv_p_void) = lv_ulong_temp;
            return lv_p_void;
        }
        case MMS_BTYPE_QUALITY:
        {
            return NULL;
        }
        case MMS_BTYPE_TIMESTAMP:
        {
            return NULL;
        }
        case MMS_BTYPE_VISSTRING64:
        {
            lv_string_length = strlen(src) + 1;
            if (lv_string_length > 64)
            {
                TRACE("default string length=%ld is greater than VISSTRING64!", lv_string_length);
                return NULL;
            }

            lv_p_string = calloc(lv_string_length, 1);
            if (NULL == lv_p_string)
            {
                TRACE("default_value calloc failed!");
                return NULL;
            }
            strncpy(lv_p_string, src, lv_string_length);
            return lv_p_string;
        }
        case MMS_BTYPE_VISSTRING255:
        {
            lv_string_length = strlen(src) + 1;
            if (lv_string_length > 255)
            {
                TRACE("default string length=%ld is greater than VISSTRING64!", lv_string_length);
                return NULL;
            }

            lv_p_string = calloc(lv_string_length, 1);
            if (NULL == lv_p_string)
            {
                TRACE("default_value calloc failed!");
                return NULL;
            }
            strncpy(lv_p_string, src, lv_string_length);
            return lv_p_string;
        }
        case MMS_BTYPE_ENTRYTIME:
        case MMS_BTYPE_OCTET64:
        {
            return NULL;
        }
        case MMS_BTYPE_UNICODE255:
        {
            lv_string_length = strlen(src) + 1;
            if (lv_string_length > 255)
            {
                TRACE("default string length=%ld is greater than VISSTRING64!", lv_string_length);
                return NULL;
            }

            lv_p_string = calloc(lv_string_length, 1);
            if (NULL == lv_p_string)
            {
                TRACE("default_value calloc failed!");
                return NULL;
            }
            strncpy(lv_p_string, src, lv_string_length);
            return lv_p_string;
        }
        case MMS_BTYPE_CHECK:
        {
            lv_ulong_temp = strtoul(src, NULL, 0);
            lv_p_void = calloc(1, sizeof(int8));
            if (NULL == lv_p_void)
            {
                TRACE("default value calloc failed!");
                return NULL;
            }

            *((Uint8 *)lv_p_void) = lv_ulong_temp;
            return lv_p_void;
        }
        default:
        {
            TRACE("unknown data type %d!", type);
            return NULL;
        }
    }
}

//----------------------------------------------------------------------------------------------------
//   Function: MallocLNodeInstance
//      Input: 
//     Output: void
//     Return: MMS_LN * 
//Description: LN实例化
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static MMS_LN *MallocLNodeInstance(MMS_LNODETYPE *p_lnodetype)
{
    MMS_LN *lv_p_ln;
    MMS_DOI *lv_p_doi_new;
    MMS_LNODETYPE_ELEMENT *lv_p_lnodetype_element;
    char *lv_p_string;

    if ((NULL == p_lnodetype) || (NULL == p_lnodetype->element))
    {
        TRACE("IED lnode element should not null!");
        return NULL;
    }

    lv_p_ln = calloc(1, sizeof(MMS_LN));
    if (NULL == lv_p_ln)
    {
        TRACE("IED ln calloc failed!");
        return NULL;
    }

    lv_p_string = calloc(strlen(p_lnodetype->id) + 1, 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED ln name calloc failed!");
        return NULL;
    }

    strncpy(lv_p_string, p_lnodetype->id, strlen(p_lnodetype->id) + 1);
    lv_p_ln->lntype = lv_p_string;

    lv_p_string = calloc(strlen(p_lnodetype->lnclass) + 1, 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED lnode inst calloc failed!");
        return NULL;
    }

    strncpy(lv_p_string, p_lnodetype->lnclass, strlen(p_lnodetype->lnclass) + 1);
    lv_p_ln->lnclass = lv_p_string;

    for (lv_p_lnodetype_element = p_lnodetype->element; NULL != lv_p_lnodetype_element; lv_p_lnodetype_element = lv_p_lnodetype_element->right)
    {
        lv_p_doi_new = MallocDoInstance(lv_p_lnodetype_element);
        if (NULL == lv_p_doi_new)
        {
            TRACE("IED doi calloc failed!");
            return NULL;
        }
        lv_p_doi_new->left = lv_p_ln->doi;
        lv_p_ln->doi = lv_p_doi_new;
    }

    return lv_p_ln;
}

//----------------------------------------------------------------------------------------------------
//   Function: MallocDoInstance
//      Input: 
//     Output: void
//     Return: MMS_DOI * 
//Description: DO实例化
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static MMS_DOI *MallocDoInstance(MMS_LNODETYPE_ELEMENT *p_lnodetype_element)
{
    
    MMS_DOI *lv_p_doi;
    MMS_SDI *lv_p_sdi_new;
    MMS_DOTYPE_ELEMENT *lv_p_dotype_element;
    char *lv_p_string;

    if (NULL == p_lnodetype_element)
    {
        TRACE("Function entries error!");
        return NULL;
    }

    lv_p_doi = calloc(1, sizeof(MMS_DOI));
    if (NULL == lv_p_doi)
    {
        TRACE("IED doi calloc failed!");
        return NULL;
    }

    lv_p_string = calloc(strlen(p_lnodetype_element->name) + 1, 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED doi name calloc failed!");
        return NULL;
    }

    strncpy(lv_p_string, p_lnodetype_element->name, strlen(p_lnodetype_element->name) + 1);
    lv_p_doi->name = lv_p_string;

    for (lv_p_dotype_element = p_lnodetype_element->do_inst->element; NULL != lv_p_dotype_element; lv_p_dotype_element = lv_p_dotype_element->right)
    {
        lv_p_sdi_new = MallocSdInstance(lv_p_dotype_element);
        if (NULL == lv_p_sdi_new)
        {
            TRACE("IED sdi calloc failed!");
            return NULL;
        }
        lv_p_sdi_new->left = lv_p_doi->sdi;
        lv_p_doi->sdi = lv_p_sdi_new;
    }

    return lv_p_doi;
}

//----------------------------------------------------------------------------------------------------
//   Function: MallocSdInstance
//      Input: 
//     Output: void
//     Return: MMS_SDI * 
//Description: SDO实例化
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static MMS_SDI *MallocSdInstance(MMS_DOTYPE_ELEMENT *p_dotype_element)
{
    
    MMS_SDI *lv_p_sdi;
    MMS_SDI *lv_p_sdi_new;
    MMS_DOTYPE_ELEMENT *lv_p_dotype_element;
    MMS_BDA *lv_p_sdi_element_dai;
    char *lv_p_string;

    if (NULL == p_dotype_element)
    {
        TRACE("Function entries error!");
        return NULL;
    }

    lv_p_sdi = calloc(1, sizeof(MMS_SDI));
    if (NULL == lv_p_sdi)
    {
        TRACE("IED sdi calloc failed!");
        return NULL;
    }

    lv_p_string = calloc(1, strlen(p_dotype_element->name) + 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED sdi name calloc failed!");
        return NULL;
    }

    strncpy(lv_p_string, p_dotype_element->name, strlen(p_dotype_element->name) + 1);
    lv_p_sdi->name = lv_p_string;

    lv_p_sdi->btype = p_dotype_element->btype;
    lv_p_sdi->fc_index = p_dotype_element->fc_index;
    if (MMS_BTYPE_SDO == p_dotype_element->btype)
    {
        for (lv_p_dotype_element = p_dotype_element->sdo->element; NULL != lv_p_dotype_element; lv_p_dotype_element = lv_p_dotype_element->right)
        {
            lv_p_sdi_new = MallocSdInstance(lv_p_dotype_element);
            if (NULL == lv_p_sdi_new)
            {
                TRACE("IED sdi calloc failed!");
                return NULL;
            }
            lv_p_sdi_new->left = lv_p_sdi->sdi;
            lv_p_sdi->sdi = lv_p_sdi_new;
        }
    }
    else if (MMS_BTYPE_STRUCT == p_dotype_element->btype)
    {
        for (lv_p_sdi_element_dai = p_dotype_element->da_inst->bda; NULL != lv_p_sdi_element_dai; lv_p_sdi_element_dai = lv_p_sdi_element_dai->right)
        {
            lv_p_sdi_new = MallocDaInstance(lv_p_sdi_element_dai);
            if (NULL == lv_p_sdi_new)
            {
                TRACE("IED sdi calloc failed!");
                return NULL;
            }
            lv_p_sdi_new->left = lv_p_sdi->sdi;
            lv_p_sdi->sdi = lv_p_sdi_new;
        }
    }

    return lv_p_sdi;
}

//----------------------------------------------------------------------------------------------------
//   Function: MallocDaInstance
//      Input: 
//     Output: void
//     Return: MMS_SDI * 
//Description: DA实例化
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static MMS_SDI *MallocDaInstance(MMS_BDA *p_bda)
{
    
    MMS_SDI *lv_p_sdi;
    MMS_SDI *lv_p_sdi_new;
    MMS_BDA *lv_p_sdi_sub_dai;
    char *lv_p_string;

    if (NULL == p_bda)
    {
        TRACE("Function entries error!");
        return NULL;
    }

    lv_p_sdi = calloc(1, sizeof(MMS_SDI));
    if (NULL == lv_p_sdi)
    {
        TRACE("IED sdi calloc failed!");
        return NULL;
    }

    lv_p_string = calloc(1, strlen(p_bda->name) + 1);
    if (NULL == lv_p_string)
    {
        TRACE("IED sdi name calloc failed!");
        return NULL;
    }

    strncpy(lv_p_string, p_bda->name, strlen(p_bda->name) + 1);
    lv_p_sdi->name = lv_p_string;

    lv_p_sdi->btype = p_bda->btype;
    if (MMS_BTYPE_STRUCT == p_bda->btype)
    {
        for (lv_p_sdi_sub_dai = p_bda->down->bda; NULL != lv_p_sdi_sub_dai; lv_p_sdi_sub_dai = lv_p_sdi_sub_dai->right)
        {
            lv_p_sdi_new = MallocDaInstance(lv_p_sdi_sub_dai);
            if (NULL == lv_p_sdi_new)
            {
                TRACE("IED sdi calloc failed!");
                return NULL;
            }
            lv_p_sdi_new->left = lv_p_sdi->sdi;
            lv_p_sdi->sdi = lv_p_sdi_new;
        }
    }

    return lv_p_sdi;
}

//----------------------------------------------------------------------------------------------------
//   Function: ParseMmsLNodeType
//      Input: 
//     Output: void
//     Return: int32 
//Description: MMS LNodeType解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 ParseMmsLNodeType(MMS_LNODETYPE **pp_mms_lnodetype, MMS_DOTYPE *p_mms_dotype, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count)
{
    int32 lv_buffer_length;
    char const *lv_p_buffer;
    int32 lv_line_count;
    char lv_buffer[64u];
    char *lv_p_string;
    char const *lv_p_file_buffer;
    int32 lv_parse_status;
    MMS_LNODETYPE *lv_p_mms_lnodetype_active;
    MMS_LNODETYPE_ELEMENT *lv_p_mms_lnodetype_element_active;
    MMS_DOTYPE *lv_p_mms_dotype_find;
    
    if ((NULL == pp_mms_lnodetype) || (NULL == p_mms_dotype) || (NULL == src_buffer_start) || (NULL == src_buffer_end) || (NULL == p_line_count))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    *pp_mms_lnodetype = NULL;
    lv_line_count = *p_line_count;

    lv_line_count = *p_line_count;
    lv_p_file_buffer = src_buffer_start;
    lv_parse_status = 0;
    lv_p_mms_lnodetype_active = NULL;
    lv_p_mms_lnodetype_element_active = NULL;
    do
    {
        lv_buffer_length = public_buffer_size_global;
        lv_p_file_buffer = GetOneLine(public_buffer_global, &lv_buffer_length, lv_p_file_buffer, src_buffer_end, &lv_line_count);

        lv_p_buffer = public_buffer_global;
        lv_p_buffer = GetContentToSplit(lv_p_buffer, lv_buffer, sizeof(lv_buffer));
        if (0 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<LNodeType"))
            {
                lv_parse_status = 1;
                GetContentOfKeyword("id", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                if ('\0' != lv_buffer[0])
                {
                    if (NULL != lv_p_mms_lnodetype_active)
                    {
                        lv_p_mms_lnodetype_active->right = calloc(1, sizeof(MMS_LNODETYPE));
                        if (NULL == lv_p_mms_lnodetype_active->right)
                        {
                            TRACE("MMS_LNODETYPE calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_lnodetype_active = lv_p_mms_lnodetype_active->right;
                    }
                    else
                    {
                        lv_p_mms_lnodetype_active = calloc(1, sizeof(MMS_LNODETYPE));
                        if (NULL == lv_p_mms_lnodetype_active)
                        {
                            TRACE("MMS_LNODETYPE calloc failed!");
                            return NORMAL_ERROR;
                        }
                        *pp_mms_lnodetype = lv_p_mms_lnodetype_active;
                    }

                    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                    if (NULL == lv_p_string)
                    {
                        TRACE("MMS_LNODETYPE id calloc failed!");
                        return NORMAL_ERROR;
                    }
                    
                    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                    lv_p_mms_lnodetype_active->id = lv_p_string;
                    
                    GetContentOfKeyword("lnClass", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    if ('\0' == lv_buffer[0])
                    {
                        TRACE("LNodeType %s get lnClass failed!", lv_p_mms_lnodetype_active->id);
                        return NORMAL_ERROR;
                    }

                    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                    if (NULL == lv_p_string)
                    {
                        TRACE("LN lnClass calloc failed!");
                        return NORMAL_ERROR;
                    }
                        
                    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                    lv_p_mms_lnodetype_active->lnclass = lv_p_string;
                       
                    lv_p_mms_lnodetype_element_active = NULL;
                }
                else
                {
                    TRACE("MMS_LNODETYPE get id failed!");
                    return NORMAL_ERROR;
                }
            }
        }
        else
        {
            if (0 == strcmp(lv_buffer, "</LNodeType"))
            {
                lv_parse_status = 0;
                lv_p_mms_lnodetype_element_active = NULL;
            }
            else
            {
                if (0 == strcmp(lv_buffer, "<DO"))
                {
                    if (NULL == lv_p_mms_lnodetype_element_active)
                    {
                        lv_p_mms_lnodetype_element_active = calloc(1, sizeof(MMS_LNODETYPE_ELEMENT));
                        if (NULL == lv_p_mms_lnodetype_element_active)
                        {
                            TRACE("LN ELEMENT calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_lnodetype_active->element = lv_p_mms_lnodetype_element_active;
                    }
                    else
                    {
                        lv_p_mms_lnodetype_element_active->right = calloc(1, sizeof(MMS_LNODETYPE_ELEMENT));
                        if (NULL == lv_p_mms_lnodetype_element_active->right)
                        {
                            TRACE("LN ELEMENT calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_lnodetype_element_active = lv_p_mms_lnodetype_element_active->right;
                    }

                    GetContentOfKeyword("type", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    for (lv_p_mms_dotype_find = p_mms_dotype; NULL != lv_p_mms_dotype_find; lv_p_mms_dotype_find = lv_p_mms_dotype_find->right)
                    {
                        if (0 == strcmp(lv_buffer, lv_p_mms_dotype_find->id))
                        {
                            break;
                        }
                    }

                    if (NULL == lv_p_mms_dotype_find)
                    {
                        TRACE("don't support bType=%s!", lv_buffer);
                        return NORMAL_ERROR;
                    }

                    lv_p_mms_lnodetype_element_active->do_inst = lv_p_mms_dotype_find;

                    GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    if ('\0' == lv_buffer[0])
                    {
                        TRACE("LNodeType %s element get name failed!", lv_p_mms_lnodetype_active->id);
                        return NORMAL_ERROR;
                    }
                    
                    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                    if (NULL == lv_p_string)
                    {
                        TRACE("LNodeType element name calloc failed!");
                        return NORMAL_ERROR;
                    }
                    
                    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                    lv_p_mms_lnodetype_element_active->name = lv_p_string;
                }
                else
                {
                    TRACE("unknown keyword \"%s\"!", lv_buffer);
                    return NORMAL_ERROR;
                }
            }
            
        }
    } while (NULL != lv_p_file_buffer);

    if (0 != lv_parse_status)
    {
        TRACE("LNodeType parse not finish!");
        return NORMAL_ERROR;
    }

    *p_line_count = lv_line_count;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: ParseMmsDoType
//      Input: 
//     Output: void
//     Return: int32 
//Description: MMS DOType解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 ParseMmsDoType(MMS_DOTYPE **pp_mms_dotype, MMS_DATYPE *p_mms_datype, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count)
{
    int32 lv_index;
    int32 lv_buffer_length;
    char const *lv_p_buffer;
    int32 lv_line_count;
    char lv_buffer[64u];
    char *lv_p_string;
    char const *lv_p_file_buffer;
    int32 lv_parse_status;
    MMS_DOTYPE *lv_p_mms_dotype_active;
    MMS_DOTYPE *lv_p_mms_dotype_find;
    MMS_DOTYPE *lv_p_mms_dotype_find_temp;
    MMS_DOTYPE *lv_p_mms_dotype_not_initial;
    MMS_DOTYPE *lv_p_mms_dotype_not_initial_header;
    MMS_DOTYPE_ELEMENT *lv_p_mms_dotype_element_active;
    MMS_DATYPE *lv_p_mms_datype_find;
    
    if ((NULL == pp_mms_dotype) || (NULL == p_mms_datype) || (NULL == src_buffer_start) || (NULL == src_buffer_end) || (NULL == p_line_count))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    *pp_mms_dotype = NULL;
    lv_line_count = *p_line_count;

    lv_p_buffer = public_buffer_global;
    lv_line_count = *p_line_count;
    lv_p_file_buffer = src_buffer_start;
    lv_parse_status = 0;
    lv_p_mms_dotype_active = NULL;
    lv_p_mms_dotype_element_active = NULL;
    lv_p_mms_dotype_find_temp = NULL;
    lv_p_mms_dotype_not_initial = NULL;
    lv_p_mms_dotype_not_initial_header = NULL;
    do
    {
        lv_buffer_length = public_buffer_size_global;
        lv_p_file_buffer = GetOneLine(public_buffer_global, &lv_buffer_length, lv_p_file_buffer, src_buffer_end, &lv_line_count);

        lv_p_buffer = public_buffer_global;
        lv_p_buffer = GetContentToSplit(lv_p_buffer, lv_buffer, sizeof(lv_buffer));
        if (0 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<DOType"))
            {
                lv_parse_status = 1;
                GetContentOfKeyword("id", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                if ('\0' != lv_buffer[0])
                {
                    lv_p_mms_dotype_find_temp = lv_p_mms_dotype_not_initial_header;
                    for (lv_p_mms_dotype_find = lv_p_mms_dotype_not_initial_header; NULL != lv_p_mms_dotype_find; lv_p_mms_dotype_find = lv_p_mms_dotype_find->right)
                    {
                        if (0 == strcmp(lv_p_mms_dotype_find->id, lv_buffer))
                        {
                            if (lv_p_mms_dotype_find == lv_p_mms_dotype_not_initial_header)
                            {
                                lv_p_mms_dotype_not_initial_header = lv_p_mms_dotype_not_initial_header->right;
                            }
                            else
                            {
                                lv_p_mms_dotype_find_temp->right = lv_p_mms_dotype_find->right;
                            }
                            break;
                        }
                        lv_p_mms_dotype_find_temp = lv_p_mms_dotype_find;
                    }

                    if (NULL != lv_p_mms_dotype_find)
                    {
                        lv_p_mms_dotype_active->right = lv_p_mms_dotype_find;
                        lv_p_mms_dotype_active = lv_p_mms_dotype_active->right;
                        lv_p_mms_dotype_element_active = NULL;
                    }
                    else
                    {
                        if (NULL != lv_p_mms_dotype_active)
                        {
                            lv_p_mms_dotype_active->right = calloc(1, sizeof(MMS_DOTYPE));
                            if (NULL == lv_p_mms_dotype_active->right)
                            {
                                TRACE("MMS_DOTYPE calloc failed!");
                                return NORMAL_ERROR;
                            }
                            lv_p_mms_dotype_active = lv_p_mms_dotype_active->right;
                        }
                        else
                        {
                            lv_p_mms_dotype_active = calloc(1, sizeof(MMS_DOTYPE));
                            if (NULL == lv_p_mms_dotype_active)
                            {
                                TRACE("MMS_DOTYPE calloc failed!");
                                return NORMAL_ERROR;
                            }
                            *pp_mms_dotype = lv_p_mms_dotype_active;
                        }

                        lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                        if (NULL == lv_p_string)
                        {
                            TRACE("MMS_DOTYPE id calloc failed!");
                            return NORMAL_ERROR;
                        }
                    
                        strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                        lv_p_mms_dotype_active->id = lv_p_string;
                        lv_p_mms_dotype_element_active = NULL;
                    }
                }
                else
                {
                    TRACE("MMS_DOTYPE get id failed!");
                    return NORMAL_ERROR;
                }
            }
        }
        else if (1 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "</DOType"))
            {
                lv_parse_status = 0;
                lv_p_mms_dotype_element_active = NULL;
            }
            else
            {
                if (0 == strcmp(lv_buffer, "<DA"))
                {
                    if (NULL == lv_p_mms_dotype_element_active)
                    {
                        lv_p_mms_dotype_element_active = calloc(1, sizeof(MMS_DOTYPE_ELEMENT));
                        if (NULL == lv_p_mms_dotype_element_active)
                        {
                            TRACE("DO ELEMENT calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_dotype_active->element = lv_p_mms_dotype_element_active;
                    }
                    else
                    {
                        lv_p_mms_dotype_element_active->right = calloc(1, sizeof(MMS_DOTYPE_ELEMENT));
                        if (NULL == lv_p_mms_dotype_element_active->right)
                        {
                            TRACE("DO ELEMENT calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_dotype_element_active = lv_p_mms_dotype_element_active->right;
                    }

                    GetContentOfKeyword("bType", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    for (lv_index = 0;  lv_index < sizeof(mms_btype) / sizeof(mms_btype[0]); lv_index++)
                    {
                        if (0 == strcmp(mms_btype[lv_index].name, lv_buffer))
                        {
                            lv_p_mms_dotype_element_active->btype = mms_btype[lv_index].btype;
                            if (MMS_BTYPE_STRUCT == lv_p_mms_dotype_element_active->btype)
                            {
                                GetContentOfKeyword("type", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                                for (lv_p_mms_datype_find = p_mms_datype; NULL != lv_p_mms_datype_find; lv_p_mms_datype_find = lv_p_mms_datype_find->right)
                                {
                                    if (0 == strcmp(lv_buffer, lv_p_mms_datype_find->id))
                                    {
                                        break;
                                    }
                                }

                                if (NULL == lv_p_mms_datype_find)
                                {
                                    TRACE("don't support bType=%s!", lv_buffer);
                                    return NORMAL_ERROR;
                                }

                                lv_p_mms_dotype_element_active->da_inst = lv_p_mms_datype_find;
                            }
                            break;
                        }
                    }

                    if (sizeof(mms_btype) / sizeof(mms_btype[0]) == lv_index)
                    {
                        TRACE("don't support bType=%s!", lv_buffer);
                        return NORMAL_ERROR;
                    }

                    GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    if ('\0' == lv_buffer[0])
                    {
                        TRACE("DOType %s get element name failed!", lv_p_mms_dotype_active->id);
                        return NORMAL_ERROR;
                    }

                    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                    if (NULL == lv_p_string)
                    {
                        TRACE("DOType element name calloc failed!");
                        return NORMAL_ERROR;
                    }
                    
                    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                    lv_p_mms_dotype_element_active->name = lv_p_string;

                    GetContentOfKeyword("fc", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    if ('\0' == lv_buffer[0])
                    {
                        TRACE("DOType \"%s\" get FC failed!", lv_p_mms_dotype_active->id);
                        return NORMAL_ERROR;
                    }

                    for (lv_index = 1; lv_index < mms_fc_num; lv_index++)
                    {
                        if (0 == strcmp(mms_fc[lv_index].name, lv_buffer))
                        {
                            lv_p_mms_dotype_element_active->fc_index = lv_index;
                            break;
                        }
                    }

                    if (mms_fc_num == lv_index)
                    {
                        TRACE("unknown mms_fc=%s", lv_buffer);
                        return NORMAL_ERROR;
                    }
                }
                else if (0 == strcmp(lv_buffer, "<SDO"))
                {
                    if (NULL == lv_p_mms_dotype_element_active)
                    {
                        lv_p_mms_dotype_element_active = calloc(1, sizeof(MMS_DOTYPE_ELEMENT));
                        if (NULL == lv_p_mms_dotype_element_active)
                        {
                            TRACE("DO ELEMENT calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_dotype_active->element = lv_p_mms_dotype_element_active;
                    }
                    else
                    {
                        lv_p_mms_dotype_element_active->right = calloc(1, sizeof(MMS_DOTYPE_ELEMENT));
                        if (NULL == lv_p_mms_dotype_element_active->right)
                        {
                            TRACE("DO ELEMENT calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_dotype_element_active = lv_p_mms_dotype_element_active->right;
                    }
                    
                    GetContentOfKeyword("type", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    for (lv_p_mms_dotype_find = *pp_mms_dotype; NULL != lv_p_mms_dotype_find; lv_p_mms_dotype_find = lv_p_mms_dotype_find->right)
                    {
                        if (0 == strcmp(lv_buffer, lv_p_mms_dotype_find->id))
                        {
                            lv_p_mms_dotype_element_active->sdo = lv_p_mms_dotype_find;
                            break;
                        }
                    }

                    if (NULL == lv_p_mms_dotype_find)
                    {
                        for (lv_p_mms_dotype_find = lv_p_mms_dotype_not_initial_header; NULL != lv_p_mms_dotype_find; lv_p_mms_dotype_find = lv_p_mms_dotype_find->right)
                        {
                            if (0 == strcmp(lv_buffer, lv_p_mms_dotype_find->id))
                            {
                                lv_p_mms_dotype_element_active->sdo = lv_p_mms_dotype_find;
                                break;
                            }
                        }
                    }

                    if (NULL == lv_p_mms_dotype_find)
                    {
                        if (NULL != lv_p_mms_dotype_not_initial)
                        {
                            lv_p_mms_dotype_not_initial->right = calloc(1, sizeof(MMS_DOTYPE));
                            if (NULL == lv_p_mms_dotype_not_initial->right)
                            {
                                TRACE("MMS_DOTYPE calloc failed!");
                                return NORMAL_ERROR;
                            }
                            lv_p_mms_dotype_not_initial = lv_p_mms_dotype_not_initial->right;
                        }
                        else
                        {
                            lv_p_mms_dotype_not_initial_header = calloc(1, sizeof(MMS_DOTYPE));
                            if (NULL == lv_p_mms_dotype_not_initial_header)
                            {
                                TRACE("MMS_DOTYPE calloc failed!");
                                return NORMAL_ERROR;
                            }
                            
                            lv_p_mms_dotype_not_initial = lv_p_mms_dotype_not_initial_header;
                        }
                    
                        lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                        if (NULL == lv_p_string)
                        {
                            TRACE("MMS_DATYPE id calloc failed!");
                            return NORMAL_ERROR;
                        }
                        
                        strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                        lv_p_mms_dotype_not_initial->id = lv_p_string;
                    
                        lv_p_mms_dotype_element_active->sdo = lv_p_mms_dotype_not_initial;
                    }
                    lv_p_mms_dotype_element_active->btype = MMS_BTYPE_SDO;

                    GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    if ('\0' == lv_buffer[0])
                    {
                        TRACE("DOType %s get element name failed!", lv_p_mms_dotype_active->id);
                        return NORMAL_ERROR;
                    }

                    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                    if (NULL == lv_p_string)
                    {
                        TRACE("DO element name calloc failed!");
                        return NORMAL_ERROR;
                    }
                
                    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                    lv_p_mms_dotype_element_active->name = lv_p_string;
                }
                else if (0 == strcmp(lv_buffer, "<Val"))
                {
                    lv_parse_status = 2;
                }                
                else
                {
                    TRACE("unknown keyword \"%s\"!", lv_buffer);
                    return NORMAL_ERROR;
                }
            }
            
        }
        else
        {
            if (0 == strcmp(lv_buffer, "</DA"))
            {
                lv_parse_status = 1;
            }                
            else
            {
                TRACE("unknown keyword \"%s\"!", lv_buffer);
                return NORMAL_ERROR;
            }
        }
    } while (NULL != lv_p_file_buffer);

    if (NULL != lv_p_mms_dotype_not_initial_header)
    {
        TRACE("there has not initial dotypes:");
        for (lv_p_mms_dotype_find = lv_p_mms_dotype_not_initial_header; NULL != lv_p_mms_dotype_find; lv_p_mms_dotype_find = lv_p_mms_dotype_find->right)
        {
            printf("    \"%s\" not initial\n", lv_p_mms_dotype_find->id);
        }
        return NORMAL_ERROR;
    }

    if (0 != lv_parse_status)
    {
        TRACE("DOType parse not finish!");
        return NORMAL_ERROR;
    }

    *p_line_count = lv_line_count;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: ParseMmsDaType
//      Input: 
//     Output: void
//     Return: int32 
//Description: MMS DAType解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 ParseMmsDaType(MMS_DATYPE **pp_mms_datype, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count)
{
    int32 lv_index;
    int32 lv_buffer_length;
    char const *lv_p_buffer;
    int32 lv_line_count;
    char lv_buffer[64u];
    char *lv_p_string;
    char const *lv_p_file_buffer;
    int32 lv_parse_status;
    MMS_DATYPE *lv_p_mms_datype_active;
    MMS_DATYPE *lv_p_mms_datype_find;
    MMS_DATYPE *lv_p_mms_datype_find_temp;
    MMS_DATYPE *lv_p_mms_datype_not_initial;
    MMS_DATYPE *lv_p_mms_datype_not_initial_header;
    MMS_BDA *lv_p_mms_bda_active;

    if ((NULL == pp_mms_datype) || (NULL == src_buffer_start) || (NULL == src_buffer_end) || (NULL == p_line_count))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    *pp_mms_datype = NULL;
    lv_line_count = *p_line_count;

    lv_p_buffer = public_buffer_global;
    lv_line_count = *p_line_count;
    lv_p_file_buffer = src_buffer_start;
    lv_parse_status = 0;
    lv_p_mms_datype_active = NULL;
    lv_p_mms_bda_active = NULL;
    lv_p_mms_datype_find_temp = NULL;
    lv_p_mms_datype_not_initial = NULL;
    lv_p_mms_datype_not_initial_header = NULL;
    do
    {
        lv_buffer_length = public_buffer_size_global;
        lv_p_file_buffer = GetOneLine(public_buffer_global, &lv_buffer_length, lv_p_file_buffer, src_buffer_end, &lv_line_count);

        lv_p_buffer = public_buffer_global;
        lv_p_buffer = GetContentToSplit(lv_p_buffer, lv_buffer, sizeof(lv_buffer));
        if (0 == lv_parse_status)
        {
            if (0 == strcmp(lv_buffer, "<DAType"))
            {
                lv_parse_status = 1;
                GetContentOfKeyword("id", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                if ('\0' != lv_buffer[0])
                {
                    lv_p_mms_datype_find_temp = lv_p_mms_datype_not_initial_header;
                    for (lv_p_mms_datype_find = lv_p_mms_datype_not_initial_header; NULL != lv_p_mms_datype_find; lv_p_mms_datype_find = lv_p_mms_datype_find->right)
                    {
                        if (0 == strcmp(lv_p_mms_datype_find->id, lv_buffer))
                        {
                            if (lv_p_mms_datype_find == lv_p_mms_datype_not_initial_header)
                            {
                                lv_p_mms_datype_not_initial_header = lv_p_mms_datype_not_initial_header->right;
                            }
                            else
                            {
                                lv_p_mms_datype_find_temp->right = lv_p_mms_datype_find->right;
                            }
                            break;
                        }
                        lv_p_mms_datype_find_temp = lv_p_mms_datype_find;
                    }

                    if (NULL != lv_p_mms_datype_find)
                    {
                        lv_p_mms_datype_active->right = lv_p_mms_datype_find;
                        lv_p_mms_datype_active = lv_p_mms_datype_active->right;
                    }
                    else
                    {
                        if (NULL != lv_p_mms_datype_active)
                        {
                            lv_p_mms_datype_active->right = calloc(1, sizeof(MMS_DATYPE));
                            if (NULL == lv_p_mms_datype_active->right)
                            {
                                TRACE("MMS_DATYPE calloc failed!");
                                return NORMAL_ERROR;
                            }
                            lv_p_mms_datype_active = lv_p_mms_datype_active->right;
                        }
                        else
                        {
                            lv_p_mms_datype_active = calloc(1, sizeof(MMS_DATYPE));
                            if (NULL == lv_p_mms_datype_active)
                            {
                                TRACE("MMS_DATYPE calloc failed!");
                                return NORMAL_ERROR;
                            }
                            *pp_mms_datype = lv_p_mms_datype_active;
                        }

                        lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                        if (NULL == lv_p_string)
                        {
                            TRACE("MMS_DATYPE id calloc failed!");
                            return NORMAL_ERROR;
                        }
                    
                        strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                        lv_p_mms_datype_active->id = lv_p_string;
                        lv_p_mms_bda_active = NULL;
                    }

                }
                else
                {
                    TRACE("MMS_DATYPE get id failed!");
                    return NORMAL_ERROR;
                }
            }
        }
        else
        {
            if (0 == strcmp(lv_buffer, "</DAType"))
            {
                lv_parse_status = 0;
                lv_p_mms_bda_active = NULL;
            }
            else
            {
                if (0 == strcmp(lv_buffer, "<BDA"))
                {
                    if (NULL != lv_p_mms_bda_active)
                    {
                        lv_p_mms_bda_active->right = calloc(1, sizeof(MMS_BDA));
                        if (NULL == lv_p_mms_bda_active->right)
                        {
                            TRACE("BDA calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_bda_active = lv_p_mms_bda_active->right;
                    }
                    else
                    {
                        lv_p_mms_datype_active->bda = calloc(1, sizeof(MMS_BDA));
                        if (NULL == lv_p_mms_datype_active->bda)
                        {
                            TRACE("BDA calloc failed!");
                            return NORMAL_ERROR;
                        }
                        lv_p_mms_bda_active = lv_p_mms_datype_active->bda;
                    }

                    GetContentOfKeyword("bType", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    for (lv_index = 0;  lv_index < sizeof(mms_btype) / sizeof(mms_btype[0]); lv_index++)
                    {
                        if (0 == strcmp(mms_btype[lv_index].name, lv_buffer))
                        {
                            lv_p_mms_bda_active->btype = mms_btype[lv_index].btype;
                            if (MMS_BTYPE_STRUCT == lv_p_mms_bda_active->btype)
                            {
                                GetContentOfKeyword("type", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                                for (lv_p_mms_datype_find = *pp_mms_datype; NULL != lv_p_mms_datype_find; lv_p_mms_datype_find = lv_p_mms_datype_find->right)
                                {
                                    if (0 == strcmp(lv_buffer, lv_p_mms_datype_find->id))
                                    {
                                        lv_p_mms_bda_active->down = lv_p_mms_datype_find;
                                        break;
                                    }
                                }

                                if (NULL == lv_p_mms_datype_find)
                                {
                                    for (lv_p_mms_datype_find = lv_p_mms_datype_not_initial_header; NULL != lv_p_mms_datype_find; lv_p_mms_datype_find = lv_p_mms_datype_find->right)
                                    {
                                        if (0 == strcmp(lv_buffer, lv_p_mms_datype_find->id))
                                        {
                                            lv_p_mms_bda_active->down = lv_p_mms_datype_find;
                                            break;
                                        }
                                    }
                                }

                                if (NULL == lv_p_mms_datype_find)
                                {
                                    if (NULL != lv_p_mms_datype_not_initial)
                                    {
                                        lv_p_mms_datype_not_initial->right = calloc(1, sizeof(MMS_DATYPE));
                                        if (NULL == lv_p_mms_datype_not_initial->right)
                                        {
                                            TRACE("MMS_DATYPE calloc failed!");
                                            return NORMAL_ERROR;
                                        }
                                        lv_p_mms_datype_not_initial = lv_p_mms_datype_not_initial->right;
                                    }
                                    else
                                    {
                                        lv_p_mms_datype_not_initial_header = calloc(1, sizeof(MMS_DATYPE));
                                        if (NULL == lv_p_mms_datype_not_initial_header)
                                        {
                                            TRACE("MMS_DATYPE calloc failed!");
                                            return NORMAL_ERROR;
                                        }
                                        
                                        lv_p_mms_datype_not_initial = lv_p_mms_datype_not_initial_header;
                                    }

                                    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                                    if (NULL == lv_p_string)
                                    {
                                        TRACE("MMS_DATYPE id calloc failed!");
                                        return NORMAL_ERROR;
                                    }
                                    
                                    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                                    lv_p_mms_datype_not_initial->id = lv_p_string;

                                    lv_p_mms_bda_active->down = lv_p_mms_datype_not_initial;
                                }
                            }
                            break;
                        }
                    }

                    if (sizeof(mms_btype) / sizeof(mms_btype[0]) == lv_index)
                    {
                        TRACE("don't support bType=%s!", lv_buffer);
                        return NORMAL_ERROR;
                    }

                    GetContentOfKeyword("name", lv_p_buffer, lv_buffer, sizeof(lv_buffer));
                    lv_p_string = calloc(strlen(lv_buffer) + 1, 1);
                    if (NULL == lv_p_string)
                    {
                        TRACE("BDA name calloc failed!");
                        return NORMAL_ERROR;
                    }
                
                    strncpy(lv_p_string, lv_buffer, strlen(lv_buffer) + 1);
                    lv_p_mms_bda_active->name = lv_p_string;
                }
                else
                {
                    TRACE("keyword BDA get failed!");
                    return NORMAL_ERROR;
                }
            }
        }
    } while (NULL != lv_p_file_buffer);

    if (NULL != lv_p_mms_datype_not_initial_header)
    {
        TRACE("there has not initial datypes:");
        for (lv_p_mms_datype_find = lv_p_mms_datype_not_initial_header; NULL != lv_p_mms_datype_find; lv_p_mms_datype_find = lv_p_mms_datype_find->right)
        {
            printf("    \"%s\" not initial\n", lv_p_mms_datype_find->id);
        }
        return NORMAL_ERROR;
    }

    if (0 != lv_parse_status)
    {
        TRACE("DAType parse not finish!");
        return NORMAL_ERROR;
    }

    *p_line_count = lv_line_count;

    return NORMAL_SUCCESS;
}

























/*
static int32 SdiPrintf(MMS_SDI *p_mms_sdi, int32 *nest_count);
int32 MmsCidParseTest(MMS_IED *p_mms_ied)
{
    MMS_IED *lv_p_mms_ied_test;
    MMS_ACCESS_POINT *lv_p_access_point_test;
    MMS_SERVER *lv_p_server_test;
    MMS_LDEVICE *lv_p_ldevice_test;
    MMS_LN *lv_p_ln_test;
    MMS_DOI *lv_p_doi_test;
    MMS_SDI *lv_p_sdi_test;
    int32 lv_nest_index;
    
    lv_p_mms_ied_test = p_mms_ied;
    printf("<IED name=\"%s\" type=\"%s\" manufacturer=\"%s\" configVersion=\"%s\">\n"
    , lv_p_mms_ied_test->name
    , lv_p_mms_ied_test->type
    , lv_p_mms_ied_test->manufacturer
    , lv_p_mms_ied_test->config_version
    );
    
    
    for (lv_p_access_point_test = lv_p_mms_ied_test->access_point; NULL != lv_p_access_point_test; lv_p_access_point_test = lv_p_access_point_test->right)
    {
        printf("    <AccessPoint>\n");
        for (lv_p_server_test = lv_p_access_point_test->server; NULL != lv_p_server_test; lv_p_server_test= lv_p_server_test->right)
        {
            printf("        <Server>\n");
            for (lv_p_ldevice_test = lv_p_server_test->ldevice; NULL != lv_p_ldevice_test; lv_p_ldevice_test= lv_p_ldevice_test->right)
            {
                printf("            <LDevice\n");
                for (lv_p_ln_test = lv_p_ldevice_test->ln; NULL != lv_p_ln_test; lv_p_ln_test = lv_p_ln_test->right)
                {
                    printf("                <LN lnClass=\"%s\" lnType=\"%s\" prefix=\"%s\" inst=\"%s\" desc=\"%s\">\n"
                    , lv_p_ln_test->lnclass
                    , lv_p_ln_test->lntype
                    , lv_p_ln_test->prefix
                    , lv_p_ln_test->inst
                    , lv_p_ln_test->desc
                    );
                    for (lv_p_doi_test = lv_p_ln_test->doi; NULL != lv_p_doi_test; lv_p_doi_test = lv_p_doi_test->right)
                    {
                        printf("                    <DOI name=\"%s\">\n", lv_p_doi_test->name);
                        for (lv_p_sdi_test = lv_p_doi_test->sdi; NULL != lv_p_sdi_test; lv_p_sdi_test = lv_p_sdi_test->right)
                        {
                            lv_nest_index = 1;
                            SdiPrintf(lv_p_sdi_test, &lv_nest_index);
                        }
                        printf("                    </DOI>\n");
                    }
                    
                    printf("                </LN>\n");
                }
                
                printf("            </LDevice>\n");
            }
            printf("         </Server>\n");
        }
        printf("    </AccessPoint>\n");
    }
    printf("</IED>\n");
    
    int32 lv_index;
    MMS_LNODETYPE *lv_p_lnodetype_test;
    MMS_DOTYPE *lv_p_dotype_test;
    MMS_DATYPE *lv_p_datype_test;
    MMS_LNODETYPE_ELEMENT *lv_p_lnode_element;
    MMS_DOTYPE_ELEMENT *lv_p_do_element;
    MMS_BDA *lv_p_bda_test;
    
    printf("<DataTypeTemplates>\n");
    for (lv_p_lnodetype_test = p_mms_lnodetype; NULL != lv_p_lnodetype_test; lv_p_lnodetype_test = lv_p_lnodetype_test->right)
    {
        printf("    <LNodeType id=\"%s\" lnClass=\"%s\">\n", lv_p_lnodetype_test->id, lv_p_lnodetype_test->lnclass);
        for (lv_p_lnode_element = lv_p_lnodetype_test->element; NULL != lv_p_lnode_element; lv_p_lnode_element= lv_p_lnode_element->right)
        {
            printf("        <DO name=%s type=%s/>\n", lv_p_lnode_element->name, lv_p_lnode_element->do_inst->id);
        }
        printf("    </LNodeType>\n");
    }
    
    for (lv_p_dotype_test = p_mms_dotype; NULL != lv_p_dotype_test; lv_p_dotype_test = lv_p_dotype_test->right)
    {
        printf("    <DOType id=%s>\n", lv_p_dotype_test->id);
        for (lv_p_do_element = lv_p_dotype_test->element; NULL != lv_p_do_element; lv_p_do_element= lv_p_do_element->right)
        {
            if (MMS_BTYPE_SDO == lv_p_do_element->btype)
            {
                printf("        <SDO name=%s type=%s/>\n", lv_p_do_element->name, lv_p_do_element->sdo->id);
                if (NULL == lv_p_do_element->sdo)
                {
                    TRACE("SDO should not be null");
                    return NORMAL_ERROR;
                }
            }
            else
            {
                for (lv_index = 0;  lv_index < sizeof(mms_btype) / sizeof(mms_btype[0]); lv_index++)
                {
                    if (mms_btype[lv_index].btype == lv_p_do_element->btype)
                    {
                        if (MMS_BTYPE_STRUCT == lv_p_do_element->btype)
                        {
                            printf("        <DA name=%s bType=%s type=%s/>\n", lv_p_do_element->name, mms_btype[lv_index].name, lv_p_do_element->da_inst->id);
                            if (NULL == lv_p_do_element->da_inst)
                            {
                                TRACE("SDO should not be null");
                                return NORMAL_ERROR;
                            }
                        }
                        else
                        {
                            printf("        <DA name=%s bType=%s/>\n", lv_p_do_element->name, mms_btype[lv_index].name);
                        }
                        break;
                    }
                }
                if (sizeof(mms_btype) / sizeof(mms_btype[0]) == lv_index)
                {
                    TRACE("unknown mms_btype=%d", lv_p_do_element->btype);
                    return NORMAL_ERROR;
                }
            }
        }
        printf("    </DOType>\n");
    }
    
    for (lv_p_datype_test = p_mms_datype; (NULL != lv_p_datype_test); lv_p_datype_test = lv_p_datype_test->right)
    {
        printf("    <DAType id=%s>\n", lv_p_datype_test->id);
        for (lv_p_bda_test = lv_p_datype_test->bda; (NULL != lv_p_bda_test); lv_p_bda_test = lv_p_bda_test->right)
        {
            for (lv_index = 0;  lv_index < sizeof(mms_btype) / sizeof(mms_btype[0]); lv_index++)
            {
                if (mms_btype[lv_index].btype == lv_p_bda_test->btype)
                {
                    if (MMS_BTYPE_STRUCT == lv_p_bda_test->btype)
                    {
                        printf("        <BDA name=%s bType=%s type=%s/>\n", lv_p_bda_test->name, mms_btype[lv_index].name, lv_p_bda_test->down->id);
                        if (NULL == lv_p_bda_test->down)
                        {
                            TRACE("bda down should not be null");
                            return NORMAL_ERROR;
                        }
                    }
                    else
                    {
                        printf("        <BDA name=%s bType=%s/>\n", lv_p_bda_test->name, mms_btype[lv_index].name);
                    }
                    break;
                }
            }
            if (sizeof(mms_btype) / sizeof(mms_btype[0]) == lv_index)
            {
                TRACE("unknown mms_btype=%d", lv_p_do_element->btype);
                return NORMAL_ERROR;
            }
        }
        printf("    </DAType>\n");
    }
    printf("</DataTypeTemplates>\n");
    return NORMAL_SUCCESS;
}

static int32 SdiPrintf(MMS_SDI *p_mms_sdi, int32 *nest_count)
{
    int32 i;
    int32 lv_index;
    MMS_SDI *lv_p_sdi_sdi;
    char lv_buffer[64u];

    for (i = 0; i < *nest_count; i++)
    {
        printf("    ");
    }


    if ((MMS_BTYPE_SDO == p_mms_sdi->btype) || (MMS_BTYPE_STRUCT == p_mms_sdi->btype))
    {
        (*nest_count)++;

        printf("                    <SDI name=\"%s\">\n", p_mms_sdi->name);
        for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->right)
        {
            SdiPrintf(lv_p_sdi_sdi, nest_count);
        }

        (*nest_count)--;

        for (i = 0; i < *nest_count; i++)
        {
            printf("    ");
        }
        printf("                    </SDI>\n");
    }
    else
    {
        if (NULL != p_mms_sdi->saddr)
        {
            printf("                    <DAI name=\"%s\" sAddr=\"%s\"", p_mms_sdi->name,  p_mms_sdi->saddr);
        }
        else
        {
            printf("                    <DAI name=\"%s\"", p_mms_sdi->name);
        }

        for (lv_index = 0;  lv_index < sizeof(mms_btype) / sizeof(mms_btype[0]); lv_index++)
        {
            if (mms_btype[lv_index].btype == p_mms_sdi->btype)
            {
                printf(" bType=%s>\n", mms_btype[lv_index].name);
                break;
            }
        }

        if (sizeof(mms_btype) / sizeof(mms_btype[0]) == lv_index)
        {
            TRACE("unknown mms_btype=0x%04X", p_mms_sdi->btype);
            return NORMAL_ERROR;
        }

        if (NULL != p_mms_sdi->default_value_addr)
        {
            for (i = 0; i < *nest_count; i++)
            {
                printf("    ");
            }
            if (NORMAL_ERROR != MmsBtypeToString(p_mms_sdi->default_value_addr, p_mms_sdi->btype, lv_buffer, sizeof(lv_buffer)))
            {
                printf("                        <Val>%s</Val>\n", lv_buffer);
            }
            else
            {
                TRACE("get sdi \"%s\"default value failed!", p_mms_sdi->name);
                return NORMAL_ERROR;
            }
        }

        for (i = 0; i < *nest_count; i++)
        {
            printf("    ");
        }
        printf("                    </DAI>\n");
    }

    return NORMAL_SUCCESS;
}
*/








