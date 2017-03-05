/*****************************************************************************************************
* FileName:                    ParseString.c
*
* Description:                 配置解析相关函数
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
#include "Debug.h"

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
// 接口函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: CheckTheEndOfXmlElement
//      Input: Uint8 const *src:
//             int8 const *keyword:
//     Output: void
//     Return: int32:
//Description: 检测XML元素结束符
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 CheckTheEndOfXmlElement(int8 const *src, int8 const *keyword)
{
    char const *lv_p_string;
    char lv_buffer[64u];

    if (NULL == src)
    {
        TRACE("Function entry error!");
        return NORMAL_ERROR;
    }

    if (NULL != strstr(src, "/>"))
    {
        return NORMAL_SUCCESS;
    }

    if (NULL == keyword)
    {
        return NORMAL_ERROR;
    }
    else
    {
        lv_p_string = strstr(src, "</");
        if (NULL != lv_p_string)
        {
            lv_p_string += 2;
            while ((' ' == *lv_p_string) || ('\t' == *lv_p_string))
            {
                lv_p_string++;
            }

            GetContentToSplit(lv_p_string, lv_buffer, sizeof(lv_buffer));
            if (0 == strcmp(lv_buffer, keyword))
            {
                return NORMAL_SUCCESS;
            }
            else
            {
                return NORMAL_ERROR;
            }
        }
        else
        {
            return NORMAL_ERROR;
        }
    }
}

//----------------------------------------------------------------------------------------------------
//   Function: GetContentToSplit
//      Input: Uint8 const *src:
//             Uint8 *buffer:
//             int32 buffer_length:
//     Output: void
//     Return: int8 const *:
//Description: 获取分隔符之间的内容
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int8 const *GetContentToSplit(int8 const *src, int8 *buffer, int32 buffer_length)
{
    Uint8 lv_escape_flag;
    Uint8 lv_quote_flag;
    Uint8 lv_byte_temp;
    int32 lv_index;

    if ((NULL == src) || (NULL == buffer))
    {
        TRACE("Function entry error!");
        return NULL;
    }

    while ((' ' == *src) || ('\t' == *src))
    {
        src++;
    }

    if (('/' == *src) && ('>' == *(src + 1)))
    {
        if (buffer_length >= 3)
        {
            buffer[0] = '/';
            buffer[1] = '>';
            buffer[2] = '\0';
        }
        else if (2 == buffer_length)
        {
            buffer[0] = '/';
            buffer[1] = '\0';
        }
        else
        {
            buffer[0] = '\0';
        }

        return NULL;
    }

    lv_index = 0;
    if (('<' == *src) && ('/' == *(src + 1)))
    {
        if (buffer_length >= 3)
        {
            buffer[lv_index++] = '<';
            buffer[lv_index++] = '/';
        }
        src += 2;
    }

    buffer[lv_index] = '\0';

    while ((' ' == *src) || ('\t' == *src) || ('>' == *src))
    {
        src++;
    }

    if ('"' == *src)
    {
        lv_quote_flag = 1;
        src++;
    }
    else
    {
        lv_quote_flag = 0;
    }

    lv_escape_flag = 0;
    while (lv_index < (buffer_length - 1))
    {
        lv_byte_temp = *src;
        if ('\0' == lv_byte_temp)
        {
            break;
        }

        if (0 != lv_escape_flag)
        {
            lv_escape_flag = 0;
        }
        else
        {
            if ('\\' == lv_byte_temp)
            {
                lv_escape_flag = 1;
                src++;
                continue;
            }
            else if ('"' == lv_byte_temp)
            {
                if (0 != lv_quote_flag)
                {
                    lv_quote_flag = 0;
                }
                else
                {
                    lv_quote_flag = 1;
                }
            }
            else if (0 == lv_quote_flag)
            {
                if ((' ' == lv_byte_temp) || ('\t' == lv_byte_temp) || ('=' == lv_byte_temp) || ('>' == lv_byte_temp))
                {
                    break;
                }
                else if (('<' == lv_byte_temp) && ('/' == *(src + 1)))
                {
                    break;
                }
                else if (('/' == lv_byte_temp) && ('>' == *(src + 1)))
                {
                    break;
                }
            }
        }
        buffer[lv_index++] = lv_byte_temp;
        src++;
    }

    if ((lv_index > 0) && ('"' == buffer[lv_index - 1]))
    {
        lv_index--;
    }
    buffer[lv_index] = '\0';

    while ((' ' == *src) || ('\t' == *src) || ('>' == *src))
    {
        src++;
    }

    if ('\0' == *src)
    {
        return NULL;
    }
    else
    {
        return src;
    }
}

//----------------------------------------------------------------------------------------------------
//   Function: GetContentOfKeyword
//      Input: char const *buffer : 待查询的字符首地址
//             char const *key_word : 待查询的关键字
//     Output: unsigned long *result : 关键字的值
//     Return: int : 函数执行情况
//Description: 获取关键字之后的值
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2009-10-22 09:00           Create
//----------------------------------------------------------------------------------------------------
int32 GetContentOfKeyword(int8 const *key_word, int8 const *src, int8 *buffer, int32 buffer_length)
{
    char const *lv_p_str;

    if ((NULL == buffer) || (NULL == key_word) || (NULL == src))
    {
        TRACE("entry parameter is null");
        return NORMAL_ERROR;
    }

    lv_p_str = src;

    do
    {
        if ('=' == *lv_p_str)
        {
            lv_p_str++;
            continue;
        }
        lv_p_str = GetContentToSplit(lv_p_str, buffer, buffer_length);
        if (NULL == lv_p_str)
        {
            buffer[0] = '\0';
            return NORMAL_ERROR;
        }
        else if ('=' != *lv_p_str)
        {
            continue;
        }
    } while (0 != strcmp(key_word, buffer));


    lv_p_str++;
    lv_p_str = GetContentToSplit(lv_p_str, buffer, buffer_length);
    if (NULL != lv_p_str)
    {
        // 跳过空格或制表符
        while ((' ' == *lv_p_str) || ('\t' == *lv_p_str))
        {
            lv_p_str++;
        }

        if ('=' == *lv_p_str)
        {
            buffer[0] = '\0';
        }
    }

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: GetContentToEndStr
//      Input: char const *src : 待查询的字符首地址
//             char const *key_word : 待查询的关键字
//     Output: char *buffer : 
//     Return: int : 函数执行情况
//Description: 获取至关键字处的字符串
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2009-10-22 09:00           Create
//----------------------------------------------------------------------------------------------------
char const *GetContentToEndStr(char const *src, char const *end_str, char *buffer, int32 buffer_lentgh)
{

    int32 lv_byte_index;
    int32 lv_dest_index;
    Uint8 lv_escape_flag;
    Uint8 lv_byte_temp;
    char const *lv_end_addr;
    char const *lv_return_addr;

    if ((NULL == src) || (NULL == buffer))
    {
        TRACE("function entries error!");
        return NULL;
    }

    while ((' ' == *src) || ('\t' == *src))
    {
        src++;
    }

    buffer[0] = '\0';
    lv_return_addr = NULL;
    lv_end_addr = NULL;
    if (NULL != end_str)
    {
        lv_end_addr = strstr(src, end_str);
        if (NULL != lv_end_addr)
        {
            lv_return_addr = lv_end_addr + strlen(end_str);
            if ('\0' == *lv_return_addr)
            {
                lv_return_addr = NULL;
            }
            
            if (src == lv_end_addr)
            {
                return lv_return_addr;
            }
            else
            {
                while ((' ' == *(lv_end_addr - 1)) || ('\t' == *(lv_end_addr - 1)))
                {
                    lv_end_addr--;
                    if (src == lv_end_addr)
                    {
                        return lv_return_addr;
                    }
                }
            }
        }
    }

    lv_escape_flag = 0;
    lv_dest_index = 0;
    for (lv_byte_index = 0; lv_byte_index < (buffer_lentgh - 1); lv_byte_index++)
    {
        lv_byte_temp = src[lv_byte_index];
        if (('\0' == lv_byte_temp) ||  (&src[lv_byte_index] == lv_end_addr))
        {
            break;
        }

        if (0 != lv_escape_flag)
        {
            buffer[lv_dest_index++] = lv_byte_temp;
        }
        else
        {
            if ('\\' == lv_byte_temp)
            {
                lv_escape_flag = 1;
                continue;
            }
            else
            {
                buffer[lv_dest_index++] = lv_byte_temp;
            }
        }
    }

    buffer[lv_dest_index] = '\0';
    return lv_return_addr;
}



