/*****************************************************************************************************
* FileName:                    main.c
*
* Description:                 系统主函数
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
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <unistd.h>
#include <bits/signum.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>

// 自定义头文件
#include "UserTypesDef.h"
#include "MmsBaseData.h"
#include "ParseCidConfig.h"
#include "MmsSocketHandle.h"
#include "Debug.h"

//====================================================================================================
// 本地函数声明,此处声明的函数不与外部接口
//====================================================================================================
static void MainExit(int signum);

//====================================================================================================
// 本文件定义的与外部的接口变量
//====================================================================================================
Uint8 *mms_dsp_base_addr_va = (Uint8 *)0x0;

//====================================================================================================
// 本地变量声明,此处声明的变量不与外部接口
//====================================================================================================
static MMS_IED *p_mms_ied;

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
// 接口函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: main
//      Input: int32 
//     Output: void
//     Return: int32: 函数执行情况
//Description: 系统程序入口
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    char const *lv_p_cid_file_name;

    lv_p_cid_file_name = argv[1];
    if ((argc < 1) || (NULL == argv[1]))
    {
        lv_p_cid_file_name = "Device.cid";
    }

mms_dsp_base_addr_va = calloc(1, 0x100000);
if (NULL == mms_dsp_base_addr_va)
{
    TRACE("mms_dsp_base_addr_va calloc failed!");
    return NORMAL_ERROR;
}

    p_mms_ied = ParseCidConfig(lv_p_cid_file_name);
    if (NULL == p_mms_ied)
    {
        TRACE("%s parse \"%s\" failed!", argv[0], lv_p_cid_file_name);
        return NORMAL_ERROR;
    }

    printf("\n%s parse \"%s\" successful and enter to server process!\n", argv[0], lv_p_cid_file_name);

    signal(SIGTERM, MainExit);

    if (NORMAL_SUCCESS != MmsSocketHandle(p_mms_ied))
    {
        TRACE("%s server process failed!", argv[0]);
        return NORMAL_ERROR;
    }

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
// 本地函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: MainExit
//      Input: int signum
//     Output: void
//     Return: void
//Description: SIGTERM信号处理
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static void MainExit(int signum)
{
    signal(SIGCHLD, SIG_DFL);

    MmsSocketExitHandle();

    exit(NORMAL_SUCCESS);
}

