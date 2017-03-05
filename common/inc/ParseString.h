/*****************************************************************************************************
* FileName:                    ParseString.h
*
* Description:                 配置解析头文件
*                              
* Author:                      YanDengxue, Fiberhome-Fuhua
*                              
* Rev History:  
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
#ifndef _Parse_String_H
#define _Parse_String_H

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================================================
// 外部函数声明
//====================================================================================================
extern int32 CheckTheEndOfXmlElement(int8 const *src, int8 const *keyword);
extern int8 const *GetContentToSplit(int8 const *src, int8 *buffer, int32 buffer_length);
extern int32 GetContentOfKeyword(int8 const *key_word, int8 const *src, int8 *buffer, int32 buffer_length);
extern char const *GetContentToEndStr(char const *src, char const *end_str, char *buffer, int32 buffer_lentgh);


#ifdef __cplusplus
}
#endif

#endif

