/*****************************************************************************************************
* FileName:                    ParseConfig.h
*
* Description:                 解析配置文件相关定义
*
* Author:                      YanDengxue, Fiberhome-Fuhua
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-10-10 11:00       --           1.00             Create
*****************************************************************************************************/
#ifndef _Parse_Config_H_
#define _Parse_Config_H_
    
#ifdef __cplusplus
extern "C" {
#endif
    
//====================================================================================================
// 外部函数声明
//====================================================================================================
extern char const *GetKeyWordAddr(char const *keyword, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count);
extern char const *GetOneLine(char *buffer, int32 *p_length, char const *src_buffer_start, char const *src_buffer_end, int32 *p_line_count);

#ifdef __cplusplus
}
#endif
    
#endif
    
    

