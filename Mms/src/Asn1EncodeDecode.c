/*****************************************************************************************************
* FileName:                    Asn1EncodeDecode.c
*
* Description:                 asn.1编解码
*
* Author:                      YanDengxue
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2010-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
//====================================================================================================
// 本文件使用的头文件
//====================================================================================================
// 库头文件
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// 自定义头文件
#include "UserTypesDef.h"
#include "Asn1EncodeDecode.h"

//====================================================================================================
// 本地变量声明,此处声明的变量不与外部接口
//====================================================================================================
static Uint32 const utc_fraction_bit[] =
{
    0x800000u, 0x400000u, 0x200000u, 0x100000u,
    0x080000u, 0x040000u, 0x020000u, 0x010000u,
    0x008000u, 0x004000u, 0x002000u, 0x001000u,
    0x000800u, 0x000400u, 0x000200u, 0x000100u,
    0x000080u, 0x000040u, 0x000020u, 0x000010u,
};

static Uint32 const utc_fraction_value[] =
{
    500000u, 250000u, 125000u, 62500u,
    31250u,  15625u,  7812u,   3906u,
    1953u,   977u,    488u,    244u,
    122u,    61u,     31u,     15u,
    8u,      4u,      2u,      1u
};

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
// 接口函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: Length2Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 tag
//             Uint16 length
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: 长度asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Length2Asn1r(Uint8 **pp_buffer, Uint8 tag, int32 length)
{
    Uint8 *lv_p_buffer;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;

    if (length > 65536u)
    {
        *(--lv_p_buffer) = length;
        *(--lv_p_buffer) = length >> 8;
        *(--lv_p_buffer) = length >> 16u;
        *(--lv_p_buffer) = 0x83u;
    }
    else if (length > 255u)
    {
        *(--lv_p_buffer) = length;
        *(--lv_p_buffer) = length >> 8;
        *(--lv_p_buffer) = 0x82u;
    }
    else if (length > 127u)
    {
        *(--lv_p_buffer) = length;
        *(--lv_p_buffer) = 0x81u;
    }
    else
    {
        *(--lv_p_buffer) = length;
    }

    *(--lv_p_buffer) = tag;

    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Asn12Length
//      Input: Uint8 **pp_buffer: 输入buffer的首地址指针
//     Output: Uint8 **pp_buffer: 输出buffer的尾地址指针
//     Return: int32: 长度
//Description: 长度asn.1解码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12Length(Uint8 const **pp_buffer)
{
    Uint8 const *lv_p_buffer;
    Uint8 lv_length_tag;
    int32 lv_length;
    Uint8 lv_length_bytes;
    Uint8 lv_index;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;

    lv_length_tag = *(lv_p_buffer++);
    if (lv_length_tag < 128u)
    {
        lv_length = lv_length_tag;
    }
    else
    {
        lv_length_bytes = lv_length_tag & 0x7Fu;
        lv_length = *(lv_p_buffer++);
        for (lv_index = 1; lv_index < lv_length_bytes; lv_index++)
        {
            lv_length = (lv_length << 8u) | *(lv_p_buffer++);
        }
    }

    *pp_buffer = lv_p_buffer;

    return lv_length;
}

//----------------------------------------------------------------------------------------------------
//   Function: Bool2Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 tag
//             Uint8 src
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: Bool的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Bool2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint8 src)
{
    Uint8 *lv_p_buffer;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;

    *(--lv_p_buffer) = (0 == (src & 0x01u)) ? 0x00u : 0xFFu;
    *(--lv_p_buffer) = 0x01u;
    *(--lv_p_buffer) = tag;

    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Asn12Bool
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 *p_dst
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: Bool的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12Bool(Uint8 const **pp_frame, Uint8 *p_dst)
{
    int32 lv_length;
    Uint8 lv_uchar_temp;
    Uint8 const *lv_p_frame;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else if (1u != lv_length)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_uchar_temp = *(lv_p_frame++);

    if (NULL != p_dst)
    {
        *p_dst = (0u == (lv_uchar_temp & 0x01u)) ? 0x00u : 0xFFu;
    }
    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Dbpos2Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 tag
//             Uint8 src
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: Dbpos的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Dbpos2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint8 src)
{
    Uint8 *lv_p_buffer;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;

    *(--lv_p_buffer) = src;
    *(--lv_p_buffer) = 0x06;
    *(--lv_p_buffer) = 0x02;
    *(--lv_p_buffer) = tag;

    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Asn12Dbpos
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 *p_dst
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: Dbpos的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12Dbpos(Uint8 const **pp_frame, Uint8 *p_dst)
{
    int32 lv_length;
    Uint8 lv_uchar_temp;
    Uint8 const *lv_p_frame;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else if (2u != lv_length)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_uchar_temp = (*lv_p_frame);
    if (6u != lv_uchar_temp)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_p_frame++;
    lv_uchar_temp = (*lv_p_frame++);

    if (NULL != p_dst)
    {
        *p_dst = lv_uchar_temp;
    }
    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Check2Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 tag
//             Uint8 src
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: Dbpos的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Check2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint8 src)
{
    Uint8 *lv_p_buffer;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;

    *(--lv_p_buffer) = src;
    *(--lv_p_buffer) = 0x06;
    *(--lv_p_buffer) = 0x02;
    *(--lv_p_buffer) = tag;

    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Asn12Check
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 *p_dst
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: Dbpos的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12Check(Uint8 const **pp_frame, Uint8 *p_dst)
{
    int32 lv_length;
    Uint8 lv_uchar_temp;
    Uint8 const *lv_p_frame;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else if (2u != lv_length)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_uchar_temp = (*lv_p_frame);
    if (6u != lv_uchar_temp)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_p_frame++;
    lv_uchar_temp = (*lv_p_frame++);

    if (NULL != p_dst)
    {
        *p_dst = lv_uchar_temp;
    }
    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Quality2Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 tag
//             Uint8 src
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: Quality的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Quality2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint16 src)
{
    Uint8 *lv_p_buffer;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;

    *(--lv_p_buffer) = src;
    *(--lv_p_buffer) = src >> 8;
    *(--lv_p_buffer) = 0x01;
    *(--lv_p_buffer) = 0x03;
    *(--lv_p_buffer) = tag;

    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Asn12Quality
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint16 *p_dst
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: Quality的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12Quality(Uint8 const **pp_frame, Uint16 *p_dst)
{
    int32 lv_length;
    int32 lv_index;
    Uint16 lv_ushort_temp;
    Uint8 const *lv_p_frame;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else if (3u != lv_length)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_p_frame++;
    lv_ushort_temp = 0;
    for (lv_index = 0; lv_index < lv_length; lv_index++)
    {
        lv_ushort_temp = (lv_ushort_temp << 8u) | *(lv_p_frame++);
    }

    if (NULL != p_dst)
    {
        *p_dst = lv_ushort_temp;
    }
    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Byte2Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             int32 string_max_length
//             Uint8 tag
//             char const *src: 字符串首地址
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: 字符串的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Bytes2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint8 const *src, int32 byte_length)
{
    int32 lv_length;
    Uint8 *lv_p_buffer;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    if (byte_length < 0)
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;
    if (NULL == src)
    {
        *(--lv_p_buffer) = 0x00u;
        *(--lv_p_buffer) = tag;
    }
    else
    {
        lv_length = byte_length;
        lv_p_buffer -= lv_length;
        memcpy(lv_p_buffer, src, lv_length);
        if (lv_length > 255)
        {
            *(--lv_p_buffer) = lv_length;
            *(--lv_p_buffer) = lv_length >> 8;
            *(--lv_p_buffer) = 0x82;
        }
        else if (lv_length > 127)
        {
            *(--lv_p_buffer) = LLSB(lv_length);
            *(--lv_p_buffer) = 0x81;
        }
        else
        {
            *(--lv_p_buffer) = LLSB(lv_length);
        }
        *(--lv_p_buffer) = tag;
    }
    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Asn12Bytes
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 *buffer
//             int32 buffer_length
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: 字符串的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12Bytes(Uint8 const **pp_frame, Uint8 *buffer, int32 buffer_length)
{
    int32 lv_length;
    Uint8 const *lv_p_frame;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else
    {
        if ((buffer_length <= 0) || (NULL == buffer))
        {
            *pp_frame = lv_p_frame + lv_length;
            return NORMAL_SUCCESS;
        }
        else if (lv_length > buffer_length)
        {
            memcpy(buffer, lv_p_frame, buffer_length);
            *pp_frame = lv_p_frame + lv_length;
            return NORMAL_ERROR;
        }
        else
        {
            memcpy(buffer, lv_p_frame, lv_length);
            *pp_frame = lv_p_frame + lv_length;
            return NORMAL_SUCCESS;
        }
    }
}

//----------------------------------------------------------------------------------------------------
//   Function: String2Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             int32 string_max_length
//             Uint8 tag
//             char const *src: 字符串首地址
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: 字符串的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 String2Asn1r(Uint8 **pp_buffer, int32 string_max_length, Uint8 tag, char const *src)
{
    int32 lv_length;
    Uint8 *lv_p_buffer;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    if (string_max_length < 0)
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;
    if (NULL == src)
    {
        *(--lv_p_buffer) = 0x00u;
        *(--lv_p_buffer) = tag;
    }
    else
    {
        lv_length = strlen(src);
        if ((0 != string_max_length) && (lv_length > string_max_length))
        {
            lv_length = string_max_length;
        }
        lv_p_buffer -= lv_length;
        memcpy(lv_p_buffer, src, lv_length);
        if (lv_length > 255)
        {
            *(--lv_p_buffer) = lv_length;
            *(--lv_p_buffer) = lv_length >> 8;
            *(--lv_p_buffer) = 0x82;
        }
        else if (lv_length > 127)
        {
            *(--lv_p_buffer) = LLSB(lv_length);
            *(--lv_p_buffer) = 0x81;
        }
        else
        {
            *(--lv_p_buffer) = LLSB(lv_length);
        }
        *(--lv_p_buffer) = tag;
    }
    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Asn12String
//      Input: Uint8 **pp_buffer: 输入buffer的首地址指针
//             char *buffer: 字符串首地址
//             int32 buffer_length
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: 字符串的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12String(Uint8 const **pp_frame, char *buffer, int32 buffer_length)
{
    Uint8 const *lv_p_frame;
    int32 lv_length;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    buffer[0] = '\0';
    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else
    {
        if ((buffer_length <= 0) || (NULL == buffer))
        {
            *pp_frame = lv_p_frame + lv_length;
            return NORMAL_SUCCESS;
        }
        else if (lv_length >= buffer_length)
        {
            memcpy(buffer, lv_p_frame, buffer_length - 1);
            buffer[buffer_length - 1] = '\0';
            *pp_frame = lv_p_frame + lv_length;
            return NORMAL_ERROR;
        }
        else
        {

            memcpy(buffer, lv_p_frame, lv_length);
            buffer[lv_length] = '\0';
            *pp_frame = lv_p_frame + lv_length;
            return NORMAL_SUCCESS;
        }
    }
}

//----------------------------------------------------------------------------------------------------
//   Function: Uint322Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 tag
//             Uint32 src
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: Uint32的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Uint322Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint32 src)
{
    Uint8 lv_length;
    Uint8 *lv_p_buffer;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;
    if (src > 0x7fffffffu)
    {
        lv_length = 5;
        *(--lv_p_buffer) = src;
        *(--lv_p_buffer) = src >> 8;
        *(--lv_p_buffer) = src >> 16;
        *(--lv_p_buffer) = src >> 24;
        *(--lv_p_buffer) = 0;
    }
    else if (src > 0x7fffffu)
    {
        lv_length = 4;
        *(--lv_p_buffer) = src;
        *(--lv_p_buffer) = src >> 8;
        *(--lv_p_buffer) = src >> 16;
        *(--lv_p_buffer) = src >> 24;
    }
    else if (src > 0x7fffu)
    {
        lv_length = 3;
        *(--lv_p_buffer) = src;
        *(--lv_p_buffer) = src >> 8;
        *(--lv_p_buffer) = src >> 16;
    }
    else if (src > 0x7f)
    {
        lv_length = 2;
        *(--lv_p_buffer) = src;
        *(--lv_p_buffer) = src >> 8;
    }
    else
    {
        lv_length = 1;
        *(--lv_p_buffer) = src;
    }

    *(--lv_p_buffer) = lv_length;
    *(--lv_p_buffer) = tag;

    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Asn12Uint32
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//             int32 *p_dst
//     Return: int32: 函数执行情况
//Description: int32的asn.1解码码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12Uint32(Uint8 const **pp_frame, Uint32 *p_dst)
{
    Uint8 const *lv_p_frame;
    int32 lv_length;
    Uint8 lv_index;
    Uint32 lv_ulong_temp;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else if (lv_length > 5u)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_ulong_temp = 0;
    for (lv_index = 0; lv_index < lv_length; lv_index++)
    {
        lv_ulong_temp = (lv_ulong_temp << 8u) | *(lv_p_frame++);
    }

    if (NULL != p_dst)
    {
        *p_dst = lv_ulong_temp;
    }
    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Int322Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 tag
//             int32 src
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: int32的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Int322Asn1r(Uint8 **pp_buffer, Uint8 tag, int32 src)
{
    Uint8 lv_length;
    Uint8 *lv_p_buffer;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;
    if ((src > 8388607) || (src < -8388608))
    {
        lv_length = 4;
        *(--lv_p_buffer) = src;
        *(--lv_p_buffer) = src >> 8;
        *(--lv_p_buffer) = src >> 16;
        *(--lv_p_buffer) = src >> 24;
    }
    else if ((src > 32767) || (src < -32768))
    {
        lv_length = 3;
        *(--lv_p_buffer) = src;
        *(--lv_p_buffer) = src >> 8;
        *(--lv_p_buffer) = src >> 16;
    }
    else if ((src > 127) || (src < -128))
    {
        lv_length = 2;
        *(--lv_p_buffer) = src;
        *(--lv_p_buffer) = src >> 8;
    }
    else
    {
        lv_length = 1;
        *(--lv_p_buffer) = src;
    }

    *(--lv_p_buffer) = lv_length;
    *(--lv_p_buffer) = tag;

    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Asn12Int32
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//             int32 *p_dst
//     Return: int32: 函数执行情况
//Description: int32的asn.1解码码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12Int32(Uint8 const **pp_frame, int32 *p_dst)
{
    Uint8 const *lv_p_frame;
    int32 lv_length;
    Uint8 lv_index;
    int32 lv_long_temp;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else if (lv_length > 4u)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_long_temp = 0;
    for (lv_index = 0; lv_index < lv_length; lv_index++)
    {
        lv_long_temp = (lv_long_temp << 8u) | *(lv_p_frame++);
    }

    if (NULL != p_dst)
    {
        *p_dst = lv_long_temp;
    }
    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Float322Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 tag
//             float src
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: float的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Float322Asn1r(Uint8 **pp_buffer, Uint8 tag, float src)
{
    Uint8 *lv_p_buffer;
    union
    {
        Uint32 lv_uint32_temp;
        float  lv_float_temp;
    } u;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;

    u.lv_float_temp = src;
    *(--lv_p_buffer) = u.lv_uint32_temp;
    *(--lv_p_buffer) = u.lv_uint32_temp >> 8;
    *(--lv_p_buffer) = u.lv_uint32_temp >> 16;
    *(--lv_p_buffer) = u.lv_uint32_temp >> 24;
    *(--lv_p_buffer) = 0x08u;
    *(--lv_p_buffer) = 0x05u;
    *(--lv_p_buffer) = tag;

    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Float322Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             float *p_dst
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: float的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12Float32(Uint8 const **pp_frame, float *p_dst)
{
    Uint8 const *lv_p_frame;
    int32 lv_length;
    Uint8 lv_uchar_temp;
    int32 lv_index;
    union
    {
        Uint32 lv_uint32_temp;
        float  lv_float_temp;
    } u;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else if (0x5u != lv_length)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_uchar_temp = (*lv_p_frame++);
    if (8u != lv_uchar_temp)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    u.lv_uint32_temp = 0;
    for (lv_index = 0; lv_index < lv_length; lv_index++)
    {
        u.lv_uint32_temp = (u.lv_uint32_temp << 8u) | *(lv_p_frame++);
    }

    if (NULL != p_dst)
    {
        *p_dst = u.lv_float_temp;
    }
    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: UTCTime2Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint8 tag
//             Uint64 usecond
//             Uint32 time_zone
//             Uint8 time_flag
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: UTCTime的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 UTCTime2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint64 usecond, Uint32 time_zone, Uint8 time_flag)
{
    Uint32 i;
    Uint8 *lv_p_buffer;
    Uint32 lv_ulong_temp;
    Uint32 lv_fraction_bit;

    if ((NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;

    *(--lv_p_buffer) = time_flag | (sizeof(utc_fraction_value) / sizeof(utc_fraction_value[0]));

    lv_fraction_bit = 0;
    lv_ulong_temp = usecond % 1000000;
    for (i = 0; i < sizeof(utc_fraction_value) / sizeof(utc_fraction_value[0]); i++)
    {
        if (lv_ulong_temp >= utc_fraction_value[i])
        {
            lv_fraction_bit |= utc_fraction_bit[i];
            lv_ulong_temp -= utc_fraction_value[i];
        }
    }

    *(--lv_p_buffer) = lv_fraction_bit;
    *(--lv_p_buffer) = lv_fraction_bit >> 8;
    *(--lv_p_buffer) = lv_fraction_bit >> 16;

    lv_ulong_temp = usecond / 1000000u - time_zone;
    *(--lv_p_buffer) = lv_ulong_temp;
    *(--lv_p_buffer) = lv_ulong_temp >> 8;
    *(--lv_p_buffer) = lv_ulong_temp >> 16;
    *(--lv_p_buffer) = lv_ulong_temp >> 24;

    *(--lv_p_buffer) = 8u;
    *(--lv_p_buffer) = tag;

    *pp_buffer = lv_p_buffer;
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: UTCTime2Asn1r
//      Input: Uint8 **pp_buffer: 输出buffer的尾地址指针
//             Uint64 *p_dst
//             Uint32 time_zone
//     Output: Uint8 **pp_buffer: 输出buffer的首地址指针
//     Return: int32: 函数执行情况
//Description: UTCTime的asn.1编码
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 Asn12UTCTime(Uint8 const **pp_frame, Uint64 *p_dst, Uint32 time_zone)
{
    int32 lv_index;
    int32 lv_length;
    Uint32 lv_ulong_temp;
    Uint32 lv_fraction_bit;
    Uint64 lv_ullong_temp;
    Uint8 const *lv_p_frame;

    if ((NULL == pp_frame) || (NULL == *pp_frame))
    {
        return NORMAL_ERROR;
    }

    lv_p_frame = *pp_frame;

    lv_length = Asn12Length(&lv_p_frame);
    if (lv_length < 0)
    {
        return NORMAL_ERROR;
    }
    else if (8u != lv_length)
    {
        *pp_frame = lv_p_frame + lv_length;
        return NORMAL_ERROR;
    }

    lv_ulong_temp = 0;
    for (lv_index = 0; lv_index < 4u; lv_index++)
    {
        lv_ulong_temp = (lv_ulong_temp << 8u) | *(lv_p_frame++);
    }
    lv_ullong_temp = (Uint64)lv_ulong_temp * 1000000u + time_zone;

    lv_fraction_bit = 0;
    for (lv_index = 0; lv_index < 3u; lv_index++)
    {
        lv_fraction_bit = (lv_fraction_bit << 8u) | *(lv_p_frame++);
    }

    lv_ulong_temp = 0;
    for (lv_index = 0; lv_index < sizeof(utc_fraction_value) / sizeof(utc_fraction_value[0]); lv_index++)
    {
        if (0 != (lv_fraction_bit & utc_fraction_value[lv_index]))
        {
            lv_ulong_temp += utc_fraction_value[lv_index];
        }
    }
    lv_ullong_temp += lv_ulong_temp;

    if (NULL != p_dst)
    {
        *p_dst = lv_ullong_temp;
    }
    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

