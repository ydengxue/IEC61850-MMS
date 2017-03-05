/*****************************************************************************************************
* FileName:                    Debug.h
*
* Description:                 调试用定义
*
* Author:                      YanDengxue, Fiberhome-Fuhua
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
#ifndef _Debug_H
#define _Debug_H

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================================================
// 调试用公用宏定义 Create by YanDengxue at 2010-01-25 10:00
//====================================================================================================
#define TRACE(fmt, args...) printf("%s:%s:%d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##args)

#ifdef __cplusplus
}
#endif

#endif


