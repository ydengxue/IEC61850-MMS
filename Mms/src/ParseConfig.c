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

// 自定义头文件
#include "UserTypesDef.h"
#include "ParseString.h"
#include "ParseConfig.h"
#include "Debug.h"

//----------------------------------------------------------------------------------------------------
//   Function: GetKeyWordAddr
//      Input: 
//     Output: void
//     Return: char const *
//Description: 获取关键字的起始地址
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
char const *GetKeyWordAddr(char const *keyword, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count)
{
    int32 lv_buffer_length;
    char *lv_p_buffer;
    char lv_buffer[64u];
    char const *lv_p_file_buffer;
    char const *lv_p_file_buffer_temp;
    int32 lv_line_count;
    char lv_buffer_line[128u];

    if ((NULL == keyword) || (NULL == src_buffer_start) || (NULL == src_buffer_end) || (NULL == p_line_count))
    {
        TRACE("function entries error!");
        return NULL;
    }

    lv_p_buffer = lv_buffer_line;
    lv_line_count = *p_line_count;
    lv_p_file_buffer = src_buffer_start;
    lv_p_file_buffer_temp = lv_p_file_buffer;
    do
    {
        lv_buffer_length = sizeof(lv_buffer_line);
        lv_p_file_buffer = GetOneLine(lv_p_buffer, &lv_buffer_length, lv_p_file_buffer, src_buffer_end, &lv_line_count);
        GetContentToSplit(lv_p_buffer, lv_buffer, sizeof(lv_buffer));
        if (0 == strcmp(lv_buffer, keyword))
        {
            break;
        }
        lv_p_file_buffer_temp = lv_p_file_buffer;
    } while (NULL != lv_p_file_buffer_temp);

    *p_line_count = lv_line_count;

    return lv_p_file_buffer_temp;
}


//----------------------------------------------------------------------------------------------------
//   Function: GetOneLine
//      Input: 
//     Output: buffer
//     Return: char const *
//Description: 获取一行数据
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
char const *GetOneLine(char *buffer, int32 *p_length, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count)
{
    int32 i;
    int32 lv_length;
    Uint8 lv_byte_temp;
    char const *lv_p_src;
    char const *lv_p_burrer;
    int32 lv_line_count;

    if ((NULL == buffer) || (NULL == p_length) || (NULL == src_buffer_start) || (NULL == src_buffer_end) || (NULL == p_line_count))
    {
        TRACE("function entrys error!");
        return NULL;
    }

    if (0 == *p_length)
    {
        buffer[0] = '\0';
        return src_buffer_start;
    }

    lv_length = *p_length;
    lv_p_src  = src_buffer_start;
    i = 0;
    lv_line_count = *p_line_count;
    while (lv_p_src != src_buffer_end)
    {
        while ((' ' == *lv_p_src) || ('\t' == *lv_p_src))
        {
            lv_p_src++;
        } 

        for (i = 0; ((i < lv_length - 1) && (lv_p_src != src_buffer_end)); i++, lv_p_src++)
        {
            lv_byte_temp = *lv_p_src;
            if ('\n' == lv_byte_temp)
            {
                lv_line_count++;
                if ((i > 0) && ('\r' == buffer[i - 1]))
                {
                    i--;
                }

                if ((i > 0) && ('\\' == buffer[i - 1]))
                {
                    i--;
                }
                else
                {
                    lv_p_src++;
                    buffer[i] = '\0';
                    if (lv_p_src >= src_buffer_end)
                    {
                        lv_p_src = NULL;
                    }
                    break;
                }
            }
            else
            {
                buffer[i] = lv_byte_temp;
            }
        }

        lv_p_burrer = buffer;
        if ('\0' == *lv_p_burrer)
        {
            continue;
        }
        else if (('/' == *lv_p_burrer) && ('/' == *(lv_p_burrer + 1)))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    buffer[i] = '\0';
    *p_length = i;
    *p_line_count = lv_line_count;

    return lv_p_src;
}

