/*****************************************************************************************************
* FileName:                    MmsSocketHandle.c
*
* Description:                 Mms Socket 处理函数
*
* Author:                      YanDengxue, Fiberhome-Fuhua
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
// 库头文件
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <bits/signum.h>

// 自定义头文件
#include "UserTypesDef.h"
#include "MmsBaseData.h"
#include "ParseCidConfig.h"
#include "Asn1EncodeDecode.h"
#include "MmsSocketHandle.h"
#include "Debug.h"

//====================================================================================================
// 宏定义
//====================================================================================================
#define MMS_CONSOLE_MAX_NUM 16u
#define MMS_SOCKET_PORT     0x66u
typedef enum
{
    MMS_DATAACCESSERROR_OBJECT_INVALIDATED            = 0x00u,
    MMS_DATAACCESSERROR_HARDWARE_FAULT                = 0x01u,
    MMS_DATAACCESSERROR_TEMPORARILY_UNAVAILABLE       = 0x02u,
    MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED          = 0x03u,
    MMS_DATAACCESSERROR_OBJECT_UNDEFINED              = 0x04u,
    MMS_DATAACCESSERROR_INVALID_ADDRESS               = 0x05u,
    MMS_DATAACCESSERROR_TYPE_UNSUPPORTED              = 0x06u,
    MMS_DATAACCESSERROR_TYPE_INCONSISTENT             = 0x07u,
    MMS_DATAACCESSERROR_OBJECT_ATTRIBUTE_INCONSISTENT = 0x08u,
    MMS_DATAACCESSERROR_OBJECT_ACCESS_UNSUPPORTED     = 0x09u,
    MMS_DATAACCESSERROR_OBJECT_NON_EXISTENT           = 0x0Au,
    MMS_DATAACCESSERROR_OBJECT_VALUE_INVALID          = 0x0Bu,
} MMS_DATAACCESSERROR;

//====================================================================================================
// 数据类型定义及宏定义
//====================================================================================================
typedef struct
{
    int fd;
    Uint32 client_ip;
    MMS_IED const *p_ied;
} MMS_SOCKET_THREAD_DATA;

typedef struct
{
    int32  status;
    int32  frame_receive_addr;
    Uint16 cotp_dst_ref;
    Uint16 cotp_src_ref;
    Uint8  const *cotp_client_id;
    Uint8  const *cotp_server_id;
    Uint8  const *p_frame;
    Uint8  const *p_frame_end;
    Uint8  cotp_client_id_length;
    Uint8  cotp_server_id_length;
    Uint8  *frame_receive_buffer;
    Uint16 cotp_client_tpdu_size;
    Uint16 cotp_server_tpdu_size;
    Uint32 invoke_id;
} MMS_COMMUNICATION;


//====================================================================================================
// 本地函数声明,此处声明的函数不与外部接口
//====================================================================================================
static int MmsSocketHandleThread(MMS_SOCKET_THREAD_DATA *p_mms_socket_data);
static int32 CotpSocketParse(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 CotpSocketSendCC(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 CotpSocketSendDR(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm, Uint8 reason);
static int32 SessionSocketParse(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 SessionSocketSendAC(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsSocketParse(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsParseConfirmedRequestpdu(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsParseConfirmedServiceRequest(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsParseGetNameList(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsSendNameListVmd(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsSendNameListDomain(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm, Uint32 lv_object_class);
static int32 MmsGetLnNameList(MMS_LN const *p_mms_ln,  char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr);
static int32 MmsGetFcNameList(MMS_LN const *p_mms_ln, int16 fc_index, Uint8 *p_fc_pre, char *buffer_start, char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr);
static int32 MmsGetDoiNameList(MMS_DOI const *p_mms_doi, int16 fc_index, Uint8 *p_fc_pre, char *buffer_start, char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr);
static int32 MmsGetSdiNameList(MMS_SDI const *p_mms_sdi, int16 fc_index, Uint8 *p_fc_pre, char *buffer_start, char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr);
static int32 MmsGetSdiNameListDirect(MMS_SDI const *p_mms_sdi, char *buffer_start, char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr);
static int32 MmsParseGetVariableAccessAttributes(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsGetAttributesName(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsGetLnAttributesName(MMS_LN const *p_mms_ln,  Uint8 **p_send_buffer_ptr);
static int32 MmsGetFcAttributesName(MMS_LN const *p_mms_ln, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr);
static int32 MmsGetDoiAttributesName(MMS_DOI const *p_mms_doi, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr);
static int32 MmsGetSdiAttributesName(MMS_SDI const *p_mms_sdi, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr);
static int32 MmsGetSdiAttributesNameDirect(MMS_SDI const *p_mms_sdi, Uint8 **p_send_buffer_ptr);
static int32 MmsParseGetNamedVariableListAttributes(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsGetNamedVariableListAttributes(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsParseRead(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsReadByName(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsReadLn(MMS_LN const *p_mms_ln,  Uint8 **p_send_buffer_ptr);
static int32 MmsReadFc(MMS_LN const *p_mms_ln, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr);
static int32 MmsReadDoi(MMS_DOI const *p_mmd_doi, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr);
static int32 MmsReadSdi(MMS_SDI const *p_mms_sdi, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr);
static int32 MmsReadSdiDirect(MMS_SDI const *p_mms_sdi, Uint8 **p_send_buffer_ptr);
static int32 MmsParseWrite(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsWriteByName(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm);
static int32 MmsWriteError(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm, int32 error_value);
static int32 MmsWriteDoi(MMS_DOI const *p_mms_doi, int16 fc_index, Uint8 const **pp_frame, Uint8 const *p_frame_end);
static int32 MmsWriteSdi(MMS_SDI const *p_mms_sdi, int16 fc_index, Uint8 const **pp_frame, Uint8 const *p_frame_end);
static int32 MmsWriteSdiDirect(MMS_SDI const *p_mms_sdi, Uint8 const const **pp_frame, Uint8 const *p_frame_end);
static int32 MmsBasicTypeDescriptionToBuffer(MMS_SDI const *p_mms_sdi, Uint8 **pp_buffer);
static int32 MmsBasicTypeReadToBuffer(MMS_SDI const *p_mms_sdi, Uint8 **pp_buffer);
static int32 MmsBasicTypeWrite(MMS_SDI const *p_mms_sdi, Uint8 const **pp_frame);
static int32 TpckPacketSend(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, int32 tpdu_size, Uint8 const *buffer, int32 buffer_length);

//====================================================================================================
// 本文件引用的外部变量声明
//====================================================================================================
extern MMS_FC_DEFINE const mms_fc[];
extern int32 const mms_fc_num;

//====================================================================================================
// 本地全局变量
//====================================================================================================
static int mms_tcplisten_socket;
static MMS_SOCKET_THREAD_DATA mms_socket_thread_data[MMS_CONSOLE_MAX_NUM];

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
// 接口函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: MmsSocketHandle
//      Input: MMS_IED const *p_mms_ied
//     Output: void
//     Return: int32: 函数执行情况
//Description: MMS socket处理函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 MmsSocketHandle(MMS_IED const *p_mms_ied)
{
    struct sockaddr_in lv_sockaddr;
    Uint32 lv_ulong_temp;
    int32 lv_index;
    int lv_temp;
    int lv_new_socket;
    char lv_buffer[64u];
    socklen_t lv_sockaddr_length;
	pthread_t lv_new_thread;
	pthread_attr_t lv_thread_attr;

    if (NULL == p_mms_ied)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    if ((mms_tcplisten_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        return NORMAL_ERROR;
    }

    setsockopt(mms_tcplisten_socket, SOL_SOCKET, SO_REUSEADDR, &lv_temp, sizeof(lv_temp));
    lv_sockaddr.sin_family      = AF_INET;
    lv_sockaddr.sin_port        = htons(MMS_SOCKET_PORT);
    lv_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(mms_tcplisten_socket, (struct sockaddr *)&lv_sockaddr, sizeof(lv_sockaddr))<0)
    {
        perror("bind()");
        return NORMAL_ERROR;
    }

//    lv_temp = fcntl(lv_tcplisten_socket, F_GETFL, 0);
//    fcntl(lv_tcplisten_socket, F_SETFL, lv_temp | O_NONBLOCK);
    lv_temp = listen(mms_tcplisten_socket, MMS_CONSOLE_MAX_NUM);
    if (lv_temp < 0)
    {
        perror("listen()");
        return NORMAL_ERROR;
    }

    while (1)
    {
        lv_sockaddr_length = sizeof(lv_sockaddr);
        lv_new_socket = accept(mms_tcplisten_socket, (struct sockaddr *)&lv_sockaddr, &lv_sockaddr_length);
        if (lv_new_socket <= 0)
        {
            perror("accept()");
            continue;
        }
        else
        {
            inet_ntop(AF_INET, &lv_sockaddr.sin_addr.s_addr, lv_buffer, sizeof(lv_buffer));
            for (lv_index = 0; lv_index < MMS_CONSOLE_MAX_NUM; lv_index++)
            {
                if (mms_socket_thread_data[lv_index].fd > 0)
                {
                    lv_ulong_temp = lv_sockaddr.sin_addr.s_addr;
                    if (lv_ulong_temp == mms_socket_thread_data[lv_index].client_ip)
                    {
                        printf("The MMS CONSOLE %ld is serving for client %s, and reject the new connection from it!\n", lv_index, lv_buffer);
                        close(lv_new_socket);
                        break;
                    }
                }
            }

            if (lv_index == MMS_CONSOLE_MAX_NUM)
            {
                for (lv_index = 0; lv_index < MMS_CONSOLE_MAX_NUM; lv_index++)
                {
                    if (mms_socket_thread_data[lv_index].fd  <= 0)
                    {
                        mms_socket_thread_data[lv_index].fd = lv_new_socket;
                        mms_socket_thread_data[lv_index].p_ied = p_mms_ied;
                        mms_socket_thread_data[lv_index].client_ip  = lv_sockaddr.sin_addr.s_addr;
                        printf("Receive a MMS request from client %s, and the CONSOLE %ld serve for it!\n", lv_buffer, lv_index);
                        break;
                    }
                }

                if (lv_index == MMS_CONSOLE_MAX_NUM)
                {
                    printf("The MMS CONSOLE num has arrived to the MAX num, and can't serve the MMS request for client %s!\n", lv_buffer);
                    close(lv_new_socket);
                }
                else
                {
	                pthread_attr_init(&lv_thread_attr);
                    pthread_attr_setdetachstate(&lv_thread_attr,PTHREAD_CREATE_DETACHED);
                    pthread_attr_setscope(&lv_thread_attr, PTHREAD_SCOPE_PROCESS);
	                if (0 != pthread_create(&lv_new_thread, &lv_thread_attr, (void *)MmsSocketHandleThread, &mms_socket_thread_data[lv_index]))
                    {
                        perror("newThread:");
                        close(lv_new_socket);
                        continue;
                    }
                }
            }
        }
    }

    return MmsSocketExitHandle();
}

//----------------------------------------------------------------------------------------------------
//   Function: MmsSocketHandle
//      Input: MMS_IED const *p_mms_ied
//     Output: void
//     Return: int32: 函数执行情况
//Description: MMS socket处理函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 MmsSocketExitHandle(void)
{
    int32 lv_index;
    char lv_buffer[64u];

    for (lv_index = 0; lv_index < MMS_CONSOLE_MAX_NUM; lv_index++)
    {
        if (mms_socket_thread_data[lv_index].fd > 0)
        {
            inet_ntop(AF_INET, &mms_socket_thread_data[lv_index].client_ip, lv_buffer, sizeof(lv_buffer));
            printf("Close MMS CONSOLE %ld for client %s!\n", lv_index, lv_buffer);
            close(mms_socket_thread_data[lv_index].fd);
        }
    }

    if (mms_tcplisten_socket > 0)
    {
        printf("Close MMS listen socket!\n");
        close(mms_tcplisten_socket);
    }

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
// 本地函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: MmsSocketHandleThread
//      Input: MMS_SOCKET_THREAD_DATA *p_mms_socket_data
//     Output: void
//     Return: int: 函数执行情况
//Description: MMS socket线程处理函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#define COTP_TPDU_SIZE 1024u

static Uint8 const COTP_DEFAULT_CLIENT_ID[] = {0x00u, 0x01u};
static Uint8 const COTP_DEFAULT_SERVER_ID[] = {0x00u, 0x01u};
static Uint16 const COTP_DEFAULT_SRC_REF = 0x0C01u;
static Uint16 const COTP_DEFAULT_DST_REF = 0x0004u;
static int MmsSocketHandleThread(MMS_SOCKET_THREAD_DATA *p_mms_socket_data)
{
#define MMS_READ_BUFFER_LENGTH (COTP_TPDU_SIZE + 4u)
    int32 lv_ret;
    int  lv_socket_fd;
    char lv_addr_buffer[16u];
    char lv_buffer[64u];
    int32 lv_receive_buffer_size;
    int32 lv_receive_bytes;
    MMS_COMMUNICATION lv_mms_comm;
    MMS_COMMUNICATION *lv_p_mms_comm;
    MMS_SOCKET_THREAD_DATA *lv_p_local_data;

    if (NULL == p_mms_socket_data)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_local_data = p_mms_socket_data;
    inet_ntop(AF_INET, &lv_p_local_data->client_ip, lv_addr_buffer, sizeof(lv_addr_buffer));

    lv_p_mms_comm = &lv_mms_comm;

    lv_p_mms_comm->frame_receive_buffer = malloc(MMS_READ_BUFFER_LENGTH);
    if (NULL == lv_p_mms_comm->frame_receive_buffer)
    {
        TRACE("client %s read buffer malloc failed!", lv_addr_buffer);
        close(lv_p_local_data->fd);
        lv_p_local_data->fd = -1;
        return NORMAL_ERROR;
    }

    lv_p_mms_comm->status = 0;

    lv_p_mms_comm->cotp_src_ref = COTP_DEFAULT_SRC_REF;
    lv_p_mms_comm->cotp_dst_ref = COTP_DEFAULT_DST_REF;
    lv_p_mms_comm->cotp_client_id = COTP_DEFAULT_CLIENT_ID;
    lv_p_mms_comm->cotp_server_id = COTP_DEFAULT_SERVER_ID;
    lv_p_mms_comm->cotp_client_id_length = 2u;
    lv_p_mms_comm->cotp_server_id_length = 2u;
    lv_p_mms_comm->cotp_client_tpdu_size = 0x0400u;
    lv_p_mms_comm->cotp_server_tpdu_size = COTP_TPDU_SIZE;
    lv_p_mms_comm->frame_receive_addr = 0;
    lv_socket_fd = lv_p_local_data->fd;
    while (1)
    {
        lv_receive_buffer_size = MMS_READ_BUFFER_LENGTH - lv_p_mms_comm->frame_receive_addr;
        if (lv_receive_buffer_size <= 0)
        {
            lv_p_mms_comm->frame_receive_addr = 0;
            lv_receive_buffer_size = MMS_READ_BUFFER_LENGTH;
        }

        lv_receive_bytes = read(lv_socket_fd, &lv_p_mms_comm->frame_receive_buffer[lv_p_mms_comm->frame_receive_addr], lv_receive_buffer_size);
        if (lv_receive_bytes < 0)
        {
            snprintf(lv_buffer, sizeof(lv_buffer), "read from CLIENT %s failed, and force to close it", lv_addr_buffer);
            perror(lv_buffer);
            close(lv_p_local_data->fd);
            lv_p_local_data->fd = -1;
            free(lv_p_mms_comm->frame_receive_buffer);
            return NORMAL_ERROR;
        }
        else if (0 == lv_receive_bytes)
        {
            printf("CLIENT %s is closed!\n", lv_addr_buffer);
            close(lv_p_local_data->fd);
            lv_p_local_data->fd = -1;
            free(lv_p_mms_comm->frame_receive_buffer);
            return NORMAL_SUCCESS;
        }
        else
        {
//int32 i;
//printf("receive %ld bytes:\n", lv_receive_bytes);
//for (i = 0; i < lv_receive_bytes; i++)
//{
//    printf("%02X ", lv_p_mms_comm->frame_receive_buffer[i]);
//}
//printf("\n");
            lv_p_mms_comm->frame_receive_addr += lv_receive_bytes;
            lv_ret = CotpSocketParse(lv_p_local_data, lv_p_mms_comm);
            if (-1 == lv_ret)
            {
                printf("CLIENT %s is socket parse failed!\n", lv_addr_buffer);
                close(lv_p_local_data->fd);
                lv_p_local_data->fd = -1;
                free(lv_p_mms_comm->frame_receive_buffer);
                return NORMAL_SUCCESS;
            }
            else if ((0 == lv_ret) || (2 == lv_ret))
            {
                lv_p_mms_comm->frame_receive_addr = 0;
            }
        }
    }

    close(lv_p_local_data->fd);
    lv_p_local_data->fd = -1;
    free(lv_p_mms_comm->frame_receive_buffer);

    return NORMAL_SUCCESS;
}


//----------------------------------------------------------------------------------------------------
//   Function: CotpSocketParse
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况: 0 : 完整数据帧解析成功
//                                  1 : 不完整帧
//                                  2 : 数据校验未通过,丢弃本帧
//                                  3 : 错误,需关闭连接
//Description: COTP CC报文发送
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#define TP_VERSION               0x03u
#define TP_PAR_CODE_TSAP_CALLING 0xC1u
#define TP_PAR_CODE_TSAP_CALLED	 0xC2u
#define TP_PAR_CODE_TPDU_SIZE    0xC0u
#define TP_PAR_CODE_VERSION	     0xC4u
#define TP_PAR_CODE_ADD_OPTIONS  0xC6u

#define TP_PDU_TYPE_CR           0xE0u/* 1110 xxxx */
#define TP_PDU_TYPE_CC           0xD0u/* 1101 xxxx */
#define TP_PDU_TYPE_DR           0x80u/* 1000 0000 */
#define TP_PDU_TYPE_DC           0xC0u/* 1100 0000 */
#define TP_PDU_TYPE_DT           0xF0u/* 1111 0000 */
#define TP_PDU_TYPE_ED           0x10u/* 0001 0000 */
#define TP_PDU_TYPE_AK           0x60u/* 0110 zzzz */
#define TP_PDU_TYPE_EA           0x20u/* 0010 0000 */
#define TP_PDU_TYPE_RJ           0x50u/* 0101 zzzz */
#define TP_PDU_TYPE_ER           0x70u/* 0111 0000 */
#define TP_PDU_TYPE_UD           0x40u/* 0100 0000 for CLTP	*/

#define SPDU_TYPE_GIVE_TOKENS    0x01u
#define SPDU_TYPE_DATA_TRANSFER  0x01u

static int32 CotpSocketParse(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    Uint8  lv_tag_length;
    Uint16 lv_tp_length;
    Uint8 const *lv_p_frame;
    MMS_COMMUNICATION *lv_p_mms_comm;
    Uint8 const *lv_p_cotp_variable;
    int32 lv_cotp_variable_length_all;
    Uint8 lv_cotp_variable_code;
    Uint8 lv_cotp_variable_length;

    if (NULL == p_mms_comm)
    {
        TRACE("function entries error!");
        return -1;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->frame_receive_buffer;

    if (0 == lv_p_mms_comm->status)
    {
        lv_p_mms_comm->cotp_dst_ref = (lv_p_frame[8u] << 8u) | lv_p_frame[9u];
        if (TP_VERSION != lv_p_frame[0])
        {
            CotpSocketSendDR(p_mms_socket_data, lv_p_mms_comm, 0);
            return -1;
        }

        if (lv_p_mms_comm->frame_receive_addr < 11u)
        {
            return 1;
        }

        lv_tp_length = (lv_p_frame[2u] << 8u) | lv_p_frame[3u];
        if (lv_tp_length > lv_p_mms_comm->cotp_server_tpdu_size)
        {
            return 2;
        }

        if (lv_p_mms_comm->frame_receive_addr < lv_tp_length)
        {
            return 1;
        }

        lv_tag_length = lv_p_frame[4u];
        if (lv_tag_length < 6u)
        {
            CotpSocketSendDR(p_mms_socket_data, lv_p_mms_comm, 0);
            return -1;
        }

        if (TP_PDU_TYPE_CR != (lv_p_frame[5u] & 0xF0u))
        {
            CotpSocketSendDR(p_mms_socket_data, lv_p_mms_comm, 0);
            return -1;
        }

        lv_p_cotp_variable = &lv_p_frame[11u];
        lv_cotp_variable_length_all = lv_tag_length - 7u;
        while (lv_cotp_variable_length_all > 0)
        {
            lv_cotp_variable_code   = lv_p_cotp_variable[0u];
            lv_cotp_variable_length = lv_p_cotp_variable[1u];
            switch (lv_cotp_variable_code)
            {
                case (TP_PAR_CODE_TSAP_CALLING) :
                {
                    lv_p_mms_comm->cotp_client_id = &lv_p_cotp_variable[2u];
                    lv_p_mms_comm->cotp_client_id_length = lv_cotp_variable_length;
                    break;
                }
                case (TP_PAR_CODE_TSAP_CALLED) :
                {
                    lv_p_mms_comm->cotp_server_id = &lv_p_cotp_variable[2u];
                    lv_p_mms_comm->cotp_server_id_length = lv_cotp_variable_length;
                    break;
                }
                case (TP_PAR_CODE_TPDU_SIZE) :
                {
                    if (lv_p_cotp_variable[2u] > 0x07u && lv_p_cotp_variable[2u] <= 0x0Du)
                    {
                        lv_p_mms_comm->cotp_client_tpdu_size = (1 << lv_p_cotp_variable[2u]);
                    }
                    break;
                }
                default :
                {
                    break;
                }
            }
            lv_p_cotp_variable += (lv_cotp_variable_length + 2u);
            lv_cotp_variable_length_all -= (lv_cotp_variable_length + 2u);
        }

        if (NORMAL_SUCCESS != CotpSocketSendCC(p_mms_socket_data, lv_p_mms_comm))
        {
            TRACE("Cotp socket send CC error!");
            return -1;
        }
        lv_p_mms_comm->status = 1;
    }
    else
    {
        if (TP_VERSION != *lv_p_frame)
        {
            return 2;
        }

        if (lv_p_mms_comm->frame_receive_addr < 7u)
        {
            return 1;
        }
        lv_p_frame += 2u;

        lv_tp_length = (lv_p_frame[0u] << 8u) | lv_p_frame[1u];
        if (lv_tp_length > lv_p_mms_comm->cotp_server_tpdu_size)
        {
            return 2;
        }
        
        if (lv_p_mms_comm->frame_receive_addr < lv_tp_length)
        {
            return 1;
        }
        lv_p_frame += 2u;

        lv_p_mms_comm->p_frame_end = lv_p_frame + lv_tp_length - 4u;

        lv_tag_length = *lv_p_frame;
        if (lv_tag_length < 2u)
        {
            return 2;
        }
        lv_p_frame++;

        lv_p_mms_comm->p_frame = lv_p_frame + lv_tag_length;

        if (TP_PDU_TYPE_DT != (lv_p_frame[0u] & 0xF0u))
        {
            return 2;
        }

        if (0x80u != (lv_p_frame[1u]))
        {
            TRACE("Only supply one Cotp frame!");
            return 2;
        }

        if (NORMAL_SUCCESS != SessionSocketParse(p_mms_socket_data, lv_p_mms_comm))
        {
            return 2;
        }
    }

    return 0;
}

//----------------------------------------------------------------------------------------------------
//   Function: CotpSocketSendCC
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: COTP CC报文发送
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 CotpSocketSendCC(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    int32 lv_send_bytes;
    int32 lv_send_bytes_index;
    MMS_COMMUNICATION *lv_p_mms_comm;
    Uint8 *lv_p_send_buffer;
    Uint16 lv_server_tpdu_size;
    Uint8  lv_server_tpdu_size_code;

    if (NULL == p_mms_comm)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;

    lv_send_bytes = 4u + 7u + lv_p_mms_comm->cotp_client_id_length + lv_p_mms_comm->cotp_server_id_length + 4u + 3u;
    lv_p_send_buffer = malloc(lv_send_bytes);
    if (NULL == lv_p_send_buffer)
    {
        TRACE("MMS send buffer malloc failed!");
        return NORMAL_ERROR;
    }

    lv_send_bytes_index = 0;
    lv_p_send_buffer[lv_send_bytes_index++] = TP_VERSION;
    lv_p_send_buffer[lv_send_bytes_index++] = 0x00u;
    lv_p_send_buffer[lv_send_bytes_index++] = LHSB(lv_send_bytes);
    lv_p_send_buffer[lv_send_bytes_index++] = LLSB(lv_send_bytes);
    lv_p_send_buffer[lv_send_bytes_index++] = lv_send_bytes - 5u;
    lv_p_send_buffer[lv_send_bytes_index++] = TP_PDU_TYPE_CC;
    lv_p_send_buffer[lv_send_bytes_index++] = LHSB(lv_p_mms_comm->cotp_dst_ref);
    lv_p_send_buffer[lv_send_bytes_index++] = LLSB(lv_p_mms_comm->cotp_dst_ref);
    lv_p_send_buffer[lv_send_bytes_index++] = LHSB(lv_p_mms_comm->cotp_src_ref);
    lv_p_send_buffer[lv_send_bytes_index++] = LLSB(lv_p_mms_comm->cotp_src_ref);
    lv_p_send_buffer[lv_send_bytes_index++] = 0x00u;

    lv_p_send_buffer[lv_send_bytes_index++] = TP_PAR_CODE_TSAP_CALLING;
    lv_p_send_buffer[lv_send_bytes_index++] = lv_p_mms_comm->cotp_client_id_length;
    memcpy(&lv_p_send_buffer[lv_send_bytes_index], lv_p_mms_comm->cotp_client_id, lv_p_mms_comm->cotp_client_id_length);
    lv_send_bytes_index += lv_p_mms_comm->cotp_client_id_length;

    lv_p_send_buffer[lv_send_bytes_index++] = TP_PAR_CODE_TSAP_CALLED;
    lv_p_send_buffer[lv_send_bytes_index++] = lv_p_mms_comm->cotp_server_id_length;
    memcpy(&lv_p_send_buffer[lv_send_bytes_index], lv_p_mms_comm->cotp_server_id, lv_p_mms_comm->cotp_server_id_length);
    lv_send_bytes_index += lv_p_mms_comm->cotp_server_id_length;

    lv_p_send_buffer[lv_send_bytes_index++] = TP_PAR_CODE_TPDU_SIZE;
    lv_p_send_buffer[lv_send_bytes_index++] = 1u;
    lv_server_tpdu_size = (lv_p_mms_comm->cotp_server_tpdu_size >> 7u);
    for (lv_server_tpdu_size_code = 0x07u; lv_server_tpdu_size_code < 0x0Eu; lv_server_tpdu_size_code++)
    {
        if (0 != (lv_server_tpdu_size & 0x01u))
        {
            break;
        }
        lv_server_tpdu_size = (lv_server_tpdu_size >> 1u);
    }
    if (lv_server_tpdu_size_code >= 0x0Eu)
    {
        lv_server_tpdu_size_code = 0x07u;
    }
    lv_p_send_buffer[lv_send_bytes_index++] = lv_server_tpdu_size_code;

    write(p_mms_socket_data->fd, lv_p_send_buffer, lv_send_bytes);

    free(lv_p_send_buffer);

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: CotpSocketSendDR
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: COTP DR报文发送
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 CotpSocketSendDR(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm, Uint8 reason)
{
    int32 lv_send_bytes;
    int32 lv_send_bytes_index;
    MMS_COMMUNICATION *lv_p_mms_comm;
    Uint8 *lv_p_send_buffer;

    if (NULL == p_mms_comm)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;

    lv_send_bytes = 4u + 7u;
    lv_p_send_buffer = malloc(lv_send_bytes);
    if (NULL == lv_p_send_buffer)
    {
        TRACE("MMS send buffer malloc failed!");
        return NORMAL_ERROR;
    }

    lv_send_bytes_index = 0;
    lv_p_send_buffer[lv_send_bytes_index++] = TP_VERSION;
    lv_p_send_buffer[lv_send_bytes_index++] = 0x00u;
    lv_p_send_buffer[lv_send_bytes_index++] = LHSB(lv_send_bytes);
    lv_p_send_buffer[lv_send_bytes_index++] = LLSB(lv_send_bytes);
    lv_p_send_buffer[lv_send_bytes_index++] = lv_send_bytes - 5u;
    lv_p_send_buffer[lv_send_bytes_index++] = TP_PDU_TYPE_DR;
    lv_p_send_buffer[lv_send_bytes_index++] = LHSB(lv_p_mms_comm->cotp_dst_ref);
    lv_p_send_buffer[lv_send_bytes_index++] = LLSB(lv_p_mms_comm->cotp_dst_ref);
    lv_p_send_buffer[lv_send_bytes_index++] = LHSB(lv_p_mms_comm->cotp_src_ref);
    lv_p_send_buffer[lv_send_bytes_index++] = LLSB(lv_p_mms_comm->cotp_src_ref);
    lv_p_send_buffer[lv_send_bytes_index++] = reason;

    write(p_mms_socket_data->fd, lv_p_send_buffer, lv_send_bytes);

    free(lv_p_send_buffer);

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: SessionSocketParse
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: Session报文解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#define SESSION_SPDU_SI_CN          0x0Du
#define SESSION_SPDU_SI_AC          0x0Eu
#define SESSION_CONNECT_ACCEPT_ITEM 0x05u
#define SESSION_PROTOCAL_OPTIONS    0x13u
#define SESSION_VERSION_NUMBER      0x16u
#define SESSION_USER_REQUIREMENT    0x14u
#define SESSION_USER_DATA           0xC1u
#define PRESENTATION_CPA_PPDU       0x31u
#define PRESENTATION_MODE_SELECTOR  0xA0u
#define PRESENTATION_MODE_VALUE     0x80u
#define PRESENTATION_NORMAL_MODE_PARAMETER     0xA2u
#define PRESENTATION_RESPONDING_PRESENTATION_SELECTOR 0x83u
#define PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST   0xA5u
#define PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST_RESULT   0x30u
#define PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST_RESULT_ACCEPTANCE   0x80u
#define PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST_RESULT_PROVIDER_REJECTION 0x81u
#define PRESENTATION_REQUIREMENTS 0x88u
#define PRESENTATION_USER_DATA_FULLY_ENCODED_DATA    0x61u
#define PRESENTATION_PDV_LIST    0x30u
#define PRESENTATION_CONTEXT_IDENTIFIER    0x02u
#define PRESENTATION_DATA_VALUES    0xA0u

#define ACSE_TAG    0x61u
#define ACSE_PROTOCOL_VERSION    0x80u
#define ACSE_APPLICATION_CONTEXT_NAME 0xA1u
#define ACSE_RESULT 0xA2u
#define ACSE_RESULT_SOURCE_DIAGNOSTIC 0xA3u
#define ACSE_SERVICE_USER 0xA1u
#define ACSE_RESPONDING_AP_TITLE 0xA4u
#define ACSE_RESPONDING_AE_QUALIFIER 0xA5u
#define ACSE_USER_INFORMATION  0xBEu
#define ACSE_ASSOCIATION_DATA  0x28u
#define ACSE_INDIRECT_REFERENCE 0x02u
#define ACSE_SIGNLE_ASN1_TYPE  0xA0u

#define MMS_INITIATE_RESPONSEPDU 0xA9u
#define MMS_LOCALDETAILCALLED 0x80u
#define MMS_NEGOTIATEDMAXSERVOUTSTANDINGCALLING 0x81u
#define MMS_NEGOTIATEDMAXSERVOUTSTANDINGCALLED  0x82u
#define MMS_NEGOTIATEDDATASTRUCTURENESTINGLEVEL 0x83u
#define MMS_INITRESPONSEDETAIL 0xA4u
#define MMS_NEGOTIATEDVERSIONNUMBER 0x80u
#define MMS_NEGOTIATEDPARAMETERCBB  0x81u
#define MMS_SERVICESSUPPORTEDCALLING 0x82u

static int32 SessionSocketParse(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    Uint8 const *lv_p_frame;
    Uint8 lv_tag;
    Uint8 lv_tag_length;
    int32 lv_length;
    MMS_COMMUNICATION *lv_p_mms_comm;

    if (NULL == p_mms_comm)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;
    if (1 == lv_p_mms_comm->status)
    {
        if (SESSION_SPDU_SI_CN == lv_p_frame[0])
        {
            SessionSocketSendAC(p_mms_socket_data, lv_p_mms_comm);
            lv_p_mms_comm->status = 2;
        }
        else
        {
            return -1;
        }
    }
    else 
    {
        if (SPDU_TYPE_GIVE_TOKENS != *lv_p_frame)
        {
            return 2;
        }
        lv_p_frame++;

        lv_tag_length = *lv_p_frame++;

        lv_p_frame += lv_tag_length;

        if (SPDU_TYPE_DATA_TRANSFER != (*lv_p_frame))
        {
            return 2;
        }
        lv_p_frame++;
        
        lv_tag_length = *lv_p_frame++;

        lv_p_frame += lv_tag_length;

        if (ACSE_TAG != *lv_p_frame)
        {
            return 2;
        }
        lv_p_frame++;

        lv_length = Asn12Length(&lv_p_frame);
        if (NORMAL_ERROR == lv_length)
        {
            return 2;
        }

        if (PRESENTATION_PDV_LIST != *lv_p_frame)
        {
            return 2;
        }
        lv_p_frame++;

        lv_length = Asn12Length(&lv_p_frame);
        if (NORMAL_ERROR == lv_length)
        {
            return 2;
        }

        while (lv_p_frame < lv_p_mms_comm->p_frame_end)
        {
            lv_tag = *lv_p_frame++;

            lv_length = Asn12Length(&lv_p_frame);
            if (NORMAL_ERROR == lv_length)
            {
                return 2;
            }

            if (PRESENTATION_DATA_VALUES == lv_tag)
            {
                lv_p_mms_comm->p_frame = lv_p_frame;
                return MmsSocketParse(p_mms_socket_data, lv_p_mms_comm);
            }

            lv_p_frame += lv_length;
        }
        return NORMAL_ERROR;
    }

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: SessionSocketSendAC
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: COTP CC报文发送
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 SessionSocketSendAC(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    int32 lv_send_bytes;
    MMS_COMMUNICATION *lv_p_mms_comm;
    Uint8 *lv_p_send_buffer;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_end;
    int32 lv_length;

    if (NULL == p_mms_comm)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;

    lv_p_send_buffer = malloc(lv_p_mms_comm->cotp_client_tpdu_size + 4u);
    if (NULL == lv_p_send_buffer)
    {
        TRACE("MMS send buffer malloc failed!");
        return NORMAL_ERROR;
    }


    lv_p_send_buffer_ptr_end = lv_p_send_buffer + lv_p_mms_comm->cotp_client_tpdu_size + 4u;
    lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_end;

    *(--lv_p_send_buffer_ptr) = 0x18u;
    *(--lv_p_send_buffer_ptr) = 0xFDu;
    *(--lv_p_send_buffer_ptr) = 0x59u;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = 0x04u;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x1Cu;
    *(--lv_p_send_buffer_ptr) = 0xEEu;
    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = 0x0Cu;
    *(--lv_p_send_buffer_ptr) = MMS_SERVICESSUPPORTEDCALLING;

    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0xFBu;
    *(--lv_p_send_buffer_ptr) = 0x05u;
    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = MMS_NEGOTIATEDPARAMETERCBB;

    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = MMS_NEGOTIATEDVERSIONNUMBER;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, MMS_INITRESPONSEDETAIL, lv_length);

    *(--lv_p_send_buffer_ptr) = 0x05u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = MMS_NEGOTIATEDDATASTRUCTURENESTINGLEVEL;

    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = MMS_NEGOTIATEDMAXSERVOUTSTANDINGCALLED;

    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = MMS_NEGOTIATEDMAXSERVOUTSTANDINGCALLING;

    *(--lv_p_send_buffer_ptr) = 0xE8u;
    *(--lv_p_send_buffer_ptr) = 0xFDu;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = MMS_LOCALDETAILCALLED;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, MMS_INITIATE_RESPONSEPDU, lv_length);

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, ACSE_SIGNLE_ASN1_TYPE, lv_length);

    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = ACSE_INDIRECT_REFERENCE;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, ACSE_ASSOCIATION_DATA, lv_length);

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, ACSE_USER_INFORMATION, lv_length);

    *(--lv_p_send_buffer_ptr) = 0x21u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = ACSE_RESPONDING_AE_QUALIFIER;

    *(--lv_p_send_buffer_ptr) = 0x6Au;
    *(--lv_p_send_buffer_ptr) = 0x0Fu;
    *(--lv_p_send_buffer_ptr) = 0xCEu;
    *(--lv_p_send_buffer_ptr) = 0x2Bu;
    *(--lv_p_send_buffer_ptr) = 0x04u;
    *(--lv_p_send_buffer_ptr) = 0x06u;
    *(--lv_p_send_buffer_ptr) = 0x06u;
    *(--lv_p_send_buffer_ptr) = ACSE_RESPONDING_AP_TITLE;

    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = ACSE_SERVICE_USER;

    *(--lv_p_send_buffer_ptr) = 0x05u;
    *(--lv_p_send_buffer_ptr) = ACSE_RESULT_SOURCE_DIAGNOSTIC;

    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = ACSE_RESULT;

    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = 0x22u;
    *(--lv_p_send_buffer_ptr) = 0xCAu;
    *(--lv_p_send_buffer_ptr) = 0x28u;
    *(--lv_p_send_buffer_ptr) = 0x05u;
    *(--lv_p_send_buffer_ptr) = 0x06u;
    *(--lv_p_send_buffer_ptr) = 0x07u;
    *(--lv_p_send_buffer_ptr) = ACSE_APPLICATION_CONTEXT_NAME;

    *(--lv_p_send_buffer_ptr) = 0x80u;
    *(--lv_p_send_buffer_ptr) = 0x07u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = ACSE_PROTOCOL_VERSION;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, ACSE_TAG, lv_length);

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, PRESENTATION_DATA_VALUES, lv_length);

    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_CONTEXT_IDENTIFIER;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, PRESENTATION_PDV_LIST, lv_length);

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, PRESENTATION_USER_DATA_FULLY_ENCODED_DATA, lv_length);


    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x06u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_REQUIREMENTS;

    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x51u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST_RESULT_PROVIDER_REJECTION;

    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST_RESULT_ACCEPTANCE;

    *(--lv_p_send_buffer_ptr) = 0x07u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST_RESULT;

    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x51u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST_RESULT_PROVIDER_REJECTION;

    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST_RESULT_ACCEPTANCE;

    *(--lv_p_send_buffer_ptr) = 0x07u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST_RESULT;

    *(--lv_p_send_buffer_ptr) = 0x12u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_CONTEXT_DEFINITION_RESULT_LIST;

    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x04u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_RESPONDING_PRESENTATION_SELECTOR;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, PRESENTATION_NORMAL_MODE_PARAMETER, lv_length);

    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_MODE_VALUE;
    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_MODE_SELECTOR;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, PRESENTATION_CPA_PPDU, lv_length);

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    *(--lv_p_send_buffer_ptr) = LLSB(lv_length);
    *(--lv_p_send_buffer_ptr) = SESSION_USER_DATA;

    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = SESSION_USER_REQUIREMENT;

    *(--lv_p_send_buffer_ptr) = 0x02u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = SESSION_VERSION_NUMBER;
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = SESSION_PROTOCAL_OPTIONS;
    *(--lv_p_send_buffer_ptr) = 0x06u;
    *(--lv_p_send_buffer_ptr) = SESSION_CONNECT_ACCEPT_ITEM;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    *(--lv_p_send_buffer_ptr) = LLSB(lv_length);
    *(--lv_p_send_buffer_ptr) = SESSION_SPDU_SI_AC;

    *(--lv_p_send_buffer_ptr) = 0x80u;
    *(--lv_p_send_buffer_ptr) = TP_PDU_TYPE_DT;
    *(--lv_p_send_buffer_ptr) = 0x02u;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr + 4u;
    *(--lv_p_send_buffer_ptr) = LLSB(lv_length);
    *(--lv_p_send_buffer_ptr) = LHSB(lv_length);
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = TP_VERSION;

    lv_send_bytes = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    write(p_mms_socket_data->fd, lv_p_send_buffer_ptr, lv_send_bytes);

    free(lv_p_send_buffer);

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: MmsSocketParse
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: COTP CC报文发送
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#define MMS_CONFIRMED_REQUESTPDU 0xA0u
static int32 MmsSocketParse(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    Uint8 const *lv_p_frame;
    Uint8 lv_tag;
    int32 lv_length;
    MMS_COMMUNICATION *lv_p_mms_comm;

    if (NULL == p_mms_comm)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;

    while (lv_p_frame < lv_p_mms_comm->p_frame_end)
    {
        lv_tag = *lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if (NORMAL_ERROR == lv_length)
        {
            return 2;
        }

        switch (lv_tag)
        {
            case MMS_CONFIRMED_REQUESTPDU:
            {
                lv_p_mms_comm->p_frame = lv_p_frame;
                MmsParseConfirmedRequestpdu(p_mms_socket_data, lv_p_mms_comm);
                lv_p_frame = lv_p_mms_comm->p_frame;
                break;
            }
            default:
            {
                break;
            }
        }
        lv_p_frame += lv_length;
    }

    lv_p_mms_comm->p_frame = lv_p_frame;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: MmsParseConfirmedRequestpdu
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: ConfirmedRequestpdu解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#define MMS_BASICOBJECTCLASS 0x80u
static int32 MmsParseConfirmedRequestpdu(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    Uint8 const *lv_p_frame;
    int32 lv_length;
    int32 lv_tag_count;
    int32 lv_ret;
    MMS_COMMUNICATION *lv_p_mms_comm;

    if (NULL == p_mms_comm)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;

    lv_tag_count = 0;
    while (lv_p_frame < lv_p_mms_comm->p_frame_end)
    {
        if (0 == lv_tag_count)
        {
            lv_p_frame++;
            if (NORMAL_SUCCESS != Asn12Uint32(&lv_p_frame, &lv_p_mms_comm->invoke_id))
            {
                return 2;
            }
        }
        else if (1 == lv_tag_count)
        {

            lv_p_mms_comm->p_frame = lv_p_frame;
            lv_ret = MmsParseConfirmedServiceRequest(p_mms_socket_data, lv_p_mms_comm);
            if (NORMAL_SUCCESS != lv_ret)
            {
                return lv_ret;
            }
            lv_p_frame = lv_p_mms_comm->p_frame;
        }
        else
        {
            lv_p_frame++;
            lv_length = Asn12Length(&lv_p_frame);
            if (NORMAL_ERROR == lv_length)
            {
                return 2;
            }
            lv_p_frame += lv_length;
        }
        
        lv_tag_count++;
    }

    lv_p_mms_comm->p_frame = lv_p_frame;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: MmsParseConfirmedServiceRequest
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: ConfirmedServiceRequest解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#define MMS_GETNAMELIST 0xA1u
#define MMS_READ 0xA4u
#define MMS_WRITE 0xA5u
#define MMS_GETVARIABLEACCESSATTRIBUTES 0xA6
#define MMS_GETNAMEDVARIABLELISTATTRIBUTES 0xAC
#define MMS_LISTOFIDENTIFIER 0xA0u
#define MMS_MOREFOLLOWS 0x81u
static int32 MmsParseConfirmedServiceRequest(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    Uint8 const *lv_p_frame;
    Uint8 lv_tag;
    int32 lv_length;
    int32 lv_ret;
    MMS_COMMUNICATION *lv_p_mms_comm;

    if (NULL == p_mms_comm)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;

    while (lv_p_frame < lv_p_mms_comm->p_frame_end)
    {
        lv_tag = *lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if (NORMAL_ERROR == lv_length)
        {
            return 2;
        }

        switch (lv_tag)
        {
            case MMS_GETNAMELIST:
            {
                lv_p_mms_comm->p_frame = lv_p_frame;
                lv_ret = MmsParseGetNameList(p_mms_socket_data, lv_p_mms_comm);
                if (NORMAL_SUCCESS != lv_ret)
                {
                    return lv_ret;
                }
                break;
            }
            case MMS_READ:
            {
                lv_p_mms_comm->p_frame = lv_p_frame;
                lv_ret = MmsParseRead(p_mms_socket_data, lv_p_mms_comm);
                if (NORMAL_SUCCESS != lv_ret)
                {
                    return lv_ret;
                }
                break;
            }
            case MMS_WRITE:
            {
                lv_p_mms_comm->p_frame = lv_p_frame;
                lv_ret = MmsParseWrite(p_mms_socket_data, lv_p_mms_comm);
                if (NORMAL_SUCCESS != lv_ret)
                {
                    return lv_ret;
                }
                break;
            }
            case MMS_GETVARIABLEACCESSATTRIBUTES:
            {
                lv_p_mms_comm->p_frame = lv_p_frame;
                lv_ret = MmsParseGetVariableAccessAttributes(p_mms_socket_data, lv_p_mms_comm);
                if (NORMAL_SUCCESS != lv_ret)
                {
                    return lv_ret;
                }
                break;
            }
            case MMS_GETNAMEDVARIABLELISTATTRIBUTES:
            {
                lv_p_mms_comm->p_frame = lv_p_frame;
                lv_ret = MmsParseGetNamedVariableListAttributes(p_mms_socket_data, lv_p_mms_comm);
                if (NORMAL_SUCCESS != lv_ret)
                {
                    return lv_ret;
                }
                break;
            }
            default:
            {
                break;
            }
        }
        lv_p_frame += lv_length;
    }

    lv_p_mms_comm->p_frame = lv_p_frame;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: MmsParseGetNameList
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: ParseGetNameList解析
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#define MMS_OBJECTCLASS 0xA0u
#define MMS_OBJECTSCOPE 0xA1u
#define MMS_OBJECTSCOPE_VMDSPECIFIC 0x80u
#define MMS_OBJECTSCOPE_DOMAINSPECIFIC 0x81u
static int32 MmsParseGetNameList(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    Uint8 const *lv_p_frame;
    Uint8 const *lv_p_frame_temp;
    Uint8 lv_tag;
    int32 lv_length;
    int32 lv_length_temp;
    Uint32 lv_object_class;
    char lv_buffer[256u];
    char *lv_p_buffer;
    MMS_COMMUNICATION *lv_p_mms_comm;

    if (NULL == p_mms_comm)
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;

    while (lv_p_frame < lv_p_mms_comm->p_frame_end)
    {
        lv_tag = *lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if (NORMAL_ERROR == lv_length)
        {
            return 2;
        }
        switch (lv_tag)
        {
            case MMS_OBJECTCLASS:
            {
                lv_p_frame_temp = lv_p_frame + 1;
                if (NORMAL_SUCCESS != Asn12Uint32(&lv_p_frame_temp, &lv_object_class))
                {
                    return 2;
                }
                break;
            }
            case MMS_OBJECTSCOPE:
            {
                if (MMS_OBJECTSCOPE_VMDSPECIFIC == *lv_p_frame)
                {
                    MmsSendNameListVmd(p_mms_socket_data, lv_p_mms_comm);
                }
                else if (MMS_OBJECTSCOPE_DOMAINSPECIFIC == *lv_p_frame)
                {
                    lv_p_frame_temp = lv_p_frame + 1;
                    lv_length_temp = Asn12Length(&lv_p_frame_temp);
                    lv_p_frame_temp += lv_length_temp;
                    if ((lv_p_frame_temp < lv_p_mms_comm->p_frame_end) && (0x82u == *lv_p_frame_temp))
                    {
                        lv_p_frame_temp++;
                        if (NORMAL_SUCCESS != Asn12String(&lv_p_frame_temp, lv_buffer, sizeof(lv_buffer)))
                        {
                            return 2;
                        }
                        lv_p_buffer = lv_buffer;
                    }
                    else
                    {
                        lv_p_buffer = NULL;
                    }
                    MmsSendNameListDomain(lv_p_buffer, p_mms_socket_data, lv_p_mms_comm, lv_object_class);
                }
                break;
            }
            default:
            {
                break;
            }
        }
        lv_p_frame += lv_length;
    }

    lv_p_mms_comm->p_frame = lv_p_frame;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: MmsSendNameListVmd
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: MmsSendNameListVmd
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#define MMS_CONFIRMED_RESPONSEPDU 0xA1u
static int32 MmsSendNameListVmd(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    int32 lv_send_bytes;
    char  lv_buffer[256u];
    MMS_COMMUNICATION *lv_p_mms_comm;
    Uint8 *lv_p_send_buffer;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_end;
    Uint8 *lv_p_send_buffer_ptr_temp;
    int32 lv_length;

    if ((NULL == p_mms_comm) || (NULL == p_mms_socket_data))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;

    lv_p_send_buffer = malloc(lv_p_mms_comm->cotp_client_tpdu_size + 4u);
    if (NULL == lv_p_send_buffer)
    {
        TRACE("MMS send buffer malloc failed!");
        return NORMAL_ERROR;
    }

    lv_p_send_buffer_ptr_end = lv_p_send_buffer + lv_p_mms_comm->cotp_client_tpdu_size + 4u;
    lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_end;

    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = MMS_MOREFOLLOWS;

    lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
    snprintf(lv_buffer, sizeof(lv_buffer), "%s%s%s", p_mms_socket_data->p_ied->name
                                                   , p_mms_socket_data->p_ied->access_point->name
                                                   , p_mms_socket_data->p_ied->access_point->server->ldevice->inst);
    String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x1Au, lv_buffer);

    lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, MMS_LISTOFIDENTIFIER, lv_length);

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, MMS_GETNAMELIST, lv_length);

    Uint322Asn1r(&lv_p_send_buffer_ptr, 0x02u, lv_p_mms_comm->invoke_id);

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, MMS_CONFIRMED_RESPONSEPDU, lv_length);

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, PRESENTATION_DATA_VALUES, lv_length);

    *(--lv_p_send_buffer_ptr) = 0x03u;
    *(--lv_p_send_buffer_ptr) = 0x01u;
    *(--lv_p_send_buffer_ptr) = PRESENTATION_CONTEXT_IDENTIFIER;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, PRESENTATION_PDV_LIST, lv_length);

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, PRESENTATION_USER_DATA_FULLY_ENCODED_DATA, lv_length);

    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = SPDU_TYPE_DATA_TRANSFER;

    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = SPDU_TYPE_GIVE_TOKENS;

    *(--lv_p_send_buffer_ptr) = 0x80u;
    *(--lv_p_send_buffer_ptr) = TP_PDU_TYPE_DT;
    *(--lv_p_send_buffer_ptr) = 0x02u;

    lv_length = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr + 4u;
    *(--lv_p_send_buffer_ptr) = LLSB(lv_length);
    *(--lv_p_send_buffer_ptr) = LHSB(lv_length);
    *(--lv_p_send_buffer_ptr) = 0x00u;
    *(--lv_p_send_buffer_ptr) = TP_VERSION;

    lv_send_bytes = lv_p_send_buffer_ptr_end - lv_p_send_buffer_ptr;
    write(p_mms_socket_data->fd, lv_p_send_buffer_ptr, lv_send_bytes);

    free(lv_p_send_buffer);

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: MmsSendNameListDomain
//      Input: int socket_fd
//             MMS_COMMUNICATION *p_mms_comm
//     Output: void
//     Return: int32: 函数执行情况
//Description: MmsSendNameListVmd
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#define MMS_TEMP_BUFFER_SIZE 0x40000
static int32 MmsSendNameListDomain(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm, Uint32 object_class)
{
    MMS_COMMUNICATION *lv_p_mms_comm;
    char *lv_p_name;
    char *lv_p_name_temp;
    char lv_buffer[256u];
    char *lv_p_local_buffer;
    int32 lv_local_buffer_length;
    Uint8 *lv_p_temp_buffer;
    Uint8 *lv_p_buffer_tail;
    Uint8 *lv_p_buffer_ptr;
    Uint8 *lv_p_buffer_ptr_temp;
    int32 lv_length;
    MMS_LN  const *lv_p_ln_active;
    MMS_DATASET const *lv_p_dataset_active;

    if ((NULL == p_mms_comm) || (NULL == p_mms_socket_data))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;

    lv_p_temp_buffer = malloc(MMS_TEMP_BUFFER_SIZE);
    if (NULL == lv_p_temp_buffer)
    {
        TRACE("MMS temp buffer malloc failed!");
        return NORMAL_ERROR;
    }

    lv_p_buffer_tail = lv_p_temp_buffer + MMS_TEMP_BUFFER_SIZE;
    lv_p_buffer_ptr = lv_p_buffer_tail;
    lv_p_buffer_ptr_temp = lv_p_buffer_ptr;

    if (0 == object_class)
    {
        lv_p_ln_active = p_mms_socket_data->p_ied->access_point->server->ldevice->ln_head;
        if (NULL != name)
        {
            lv_p_name = name;
            lv_p_name_temp = strchr(lv_p_name, '$');
            if (NULL != lv_p_name_temp)
            {
                *lv_p_name_temp = '\0';
            }

            for (; NULL != lv_p_ln_active; lv_p_ln_active = lv_p_ln_active->right)
            {
                snprintf(lv_buffer, sizeof(lv_buffer), "%s%s%s", lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
                if (0 == strcmp(lv_p_name, lv_buffer))
                {
                    break;
                }
            }

            if (NULL == lv_p_ln_active)
            {
                TRACE("ln \"%s\" found failed!", lv_p_name);
                free(lv_p_temp_buffer);
                return NORMAL_ERROR;
            }

            if (NULL == lv_p_ln_active->right)
            {
                TRACE("after ln \"%s\" has no item!", lv_p_name);
                free(lv_p_temp_buffer);
                return NORMAL_ERROR;
            }

            lv_p_ln_active = lv_p_ln_active->right;
        }

        if (NULL == lv_p_ln_active->right)
        {
            *(--lv_p_buffer_ptr) = 0x00u;
            *(--lv_p_buffer_ptr) = 0x01u;
            *(--lv_p_buffer_ptr) = MMS_MOREFOLLOWS;
        }

        lv_p_buffer_ptr_temp = lv_p_buffer_ptr;
        if (NORMAL_SUCCESS != MmsGetLnNameList(lv_p_ln_active, lv_buffer, sizeof(lv_buffer), &lv_p_buffer_ptr))
        {
            TRACE("ln name list get failed!");
            free(lv_p_temp_buffer);
            return NORMAL_ERROR;
        }
    }
    else if (2 == object_class)
    {
        *(--lv_p_buffer_ptr) = 0x00u;
        *(--lv_p_buffer_ptr) = 0x01u;
        *(--lv_p_buffer_ptr) = MMS_MOREFOLLOWS;

        lv_p_buffer_ptr_temp = lv_p_buffer_ptr;

        lv_p_ln_active = p_mms_socket_data->p_ied->access_point->server->ldevice->ln_head;
        snprintf(lv_buffer, sizeof(lv_buffer), "%s%s%s", lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
        lv_local_buffer_length = strlen(lv_buffer);
        lv_p_local_buffer = lv_buffer + lv_local_buffer_length;
        lv_local_buffer_length = sizeof(lv_buffer) - lv_local_buffer_length;
        for (lv_p_dataset_active = p_mms_socket_data->p_ied->access_point->server->ldevice->dataset; NULL != lv_p_dataset_active; lv_p_dataset_active = lv_p_dataset_active->left)
        {
            snprintf(lv_p_local_buffer, lv_local_buffer_length, "$%s", lv_p_dataset_active->name);
            String2Asn1r(&lv_p_buffer_ptr, 0, 0x1Au, lv_buffer);
        }
    }
    else
    {
        TRACE("unknown object class %lu!", object_class);
        free(lv_p_temp_buffer);
        return NORMAL_ERROR;
    }
    

    lv_length = lv_p_buffer_ptr_temp - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_LISTOFIDENTIFIER, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_GETNAMELIST, lv_length);

    Uint322Asn1r(&lv_p_buffer_ptr, 0x02u, lv_p_mms_comm->invoke_id);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_CONFIRMED_RESPONSEPDU, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_DATA_VALUES, lv_length);

    *(--lv_p_buffer_ptr) = 0x03u;
    *(--lv_p_buffer_ptr) = 0x01u;
    *(--lv_p_buffer_ptr) = PRESENTATION_CONTEXT_IDENTIFIER;

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_PDV_LIST, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_USER_DATA_FULLY_ENCODED_DATA, lv_length);

    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_DATA_TRANSFER;
    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_GIVE_TOKENS;

    if (NORMAL_SUCCESS != TpckPacketSend(p_mms_socket_data, lv_p_mms_comm->cotp_client_tpdu_size ,lv_p_buffer_ptr, lv_p_buffer_tail - lv_p_buffer_ptr))
    {
        TRACE("MMS tpck packet send failed!");
        free(lv_p_temp_buffer);
        return NORMAL_ERROR;
    }

    free(lv_p_temp_buffer);

    return NORMAL_SUCCESS;
}

static int32 MmsGetLnNameList(MMS_LN const *p_mms_ln,  char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr)
{
    int16 lv_fc_index;
    int32 lv_string_length;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_ln_temp;
    MMS_LN  const *lv_p_ln_active;
    Uint8 lv_fc_flag;

    if ((NULL == p_mms_ln) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_ln_active = p_mms_ln;
    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    snprintf(buffer, buffer_length, "%s%s%s", lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
    lv_string_length = strlen(buffer);
    for (lv_fc_index = (mms_fc_num - 1); lv_fc_index > 0; lv_fc_index--)
    {
        lv_fc_flag = 0;
        lv_p_send_buffer_ptr_ln_temp = lv_p_send_buffer_ptr;
        if (NORMAL_SUCCESS != MmsGetFcNameList(lv_p_ln_active, lv_fc_index, &lv_fc_flag, buffer, buffer + lv_string_length, buffer_length - lv_string_length, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }

        if (0 == lv_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_ln_temp;
        }
    }

    buffer[lv_string_length] = '\0';
    String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x1Au, buffer);

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsGetFcNameList(MMS_LN const *p_mms_ln, int16 fc_index, Uint8 *p_fc_pre, char *buffer_start, char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr)
{
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_doi_temp;
    MMS_DOI const *lv_p_doi;
    Uint8 lv_fc_flag;
    Uint8 lv_doi_fc_flag;
    int32 lv_string_length;

    if ((NULL == p_mms_ln) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr) || (NULL == p_fc_pre) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_fc_flag = 0;
    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
    snprintf(buffer, buffer_length, "$%s", mms_fc[fc_index].name);
    lv_string_length = strlen(buffer);
    for (lv_p_doi = p_mms_ln->doi; NULL != lv_p_doi; lv_p_doi = lv_p_doi->left)
    {
        lv_doi_fc_flag = 0;
        lv_p_send_buffer_ptr_doi_temp = lv_p_send_buffer_ptr;
        if (NORMAL_SUCCESS != MmsGetDoiNameList(lv_p_doi, fc_index, &lv_doi_fc_flag, buffer_start, buffer + lv_string_length, buffer_length - lv_string_length, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }
                
        if (0 == lv_doi_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_doi_temp;
        }
        else
        {
            lv_fc_flag = 1;
        }       
    }

    if (0 == lv_fc_flag)
    {
        lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_temp;
    }
    else
    {
        buffer[lv_string_length] = '\0';
        String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x1Au, buffer_start);

        *p_fc_pre = 1u;
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsGetDoiNameList(MMS_DOI const *p_mms_doi, int16 fc_index, Uint8 *p_fc_pre, char *buffer_start, char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr)
{
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_sdi_temp;
    MMS_SDI const *lv_p_sdi;
    Uint8 lv_fc_flag;
    Uint8 lv_sdi_fc_flag;
    int32 lv_string_length;

    if ((NULL == p_mms_doi) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr) || (NULL == p_fc_pre) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_fc_flag = 0u;
    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
    snprintf(buffer, buffer_length, "$%s", p_mms_doi->name);
    lv_string_length = strlen(buffer);
    for (lv_p_sdi = p_mms_doi->sdi; NULL != lv_p_sdi; lv_p_sdi = lv_p_sdi->left)
    {
        lv_sdi_fc_flag = 0u;
        lv_p_send_buffer_ptr_sdi_temp = lv_p_send_buffer_ptr;
        if (NORMAL_SUCCESS != MmsGetSdiNameList(lv_p_sdi, fc_index, &lv_sdi_fc_flag, buffer_start, buffer + lv_string_length, buffer_length - lv_string_length, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }
        
        if (0u == lv_sdi_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_sdi_temp;
        }
        else
        {
            lv_fc_flag = 1u;
        }
    }

    if (0u == lv_fc_flag)
    {
        lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_temp;
    }
    else
    {
        buffer[lv_string_length] = '\0';
        String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x1Au, buffer_start);

        *p_fc_pre = 1u;
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsGetSdiNameList(MMS_SDI const *p_mms_sdi, int16 fc_index, Uint8 *p_fc_pre, char *buffer_start, char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr)
{
    Uint8 lv_fc_flag;
    Uint8 lv_sdi_fc_flag;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_sdi_temp;
    MMS_SDI const *lv_p_sdi_sdi;
    int32 lv_string_length;

    if ((NULL == p_mms_sdi) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr) || (NULL == p_fc_pre) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    if (MMS_BTYPE_SDO == p_mms_sdi->btype)
    {
        lv_fc_flag = 0u;
        lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
        snprintf(buffer, buffer_length, "$%s", p_mms_sdi->name);
        lv_string_length = strlen(buffer);
        for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->left)
        {
            lv_sdi_fc_flag = 0u;
            lv_p_send_buffer_ptr_sdi_temp = lv_p_send_buffer_ptr;
            MmsGetSdiNameList(lv_p_sdi_sdi, fc_index, &lv_sdi_fc_flag, buffer_start, buffer + lv_string_length, buffer_length - lv_string_length, &lv_p_send_buffer_ptr);
            if (0u == lv_sdi_fc_flag)
            {
                lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_sdi_temp;
            }
            else
            {
                lv_fc_flag = 1u;
            }

        }

        if (0u == lv_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_temp;
        }
        else
        {
            buffer[lv_string_length] = '\0';
            String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x1Au, buffer_start);

            *p_fc_pre = 1u;
        }
    }
    else if ((MMS_BTYPE_STRUCT == p_mms_sdi->btype))
    {
        if (fc_index == p_mms_sdi->fc_index)
        {
            snprintf(buffer, buffer_length, "$%s", p_mms_sdi->name);
            lv_string_length = strlen(buffer);
            for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->left)
            {
                MmsGetSdiNameListDirect(lv_p_sdi_sdi, buffer_start, buffer + lv_string_length, buffer_length - lv_string_length, &lv_p_send_buffer_ptr);
            }
            buffer[lv_string_length] = '\0';
            String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x1Au, buffer_start);

            *p_fc_pre = 1u;
        }
    }
    else
    {
        if (fc_index == p_mms_sdi->fc_index)
        {
            snprintf(buffer, buffer_length, "$%s", p_mms_sdi->name);
            String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x1Au, buffer_start);

            *p_fc_pre = 1u;
        }
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsGetSdiNameListDirect(MMS_SDI const *p_mms_sdi, char *buffer_start, char *buffer, int32 buffer_length, Uint8 **p_send_buffer_ptr)
{
    MMS_SDI const *lv_p_sdi_sdi;
    int32 lv_string_length;
    Uint8 *lv_p_send_buffer_ptr;

    if ((NULL == p_mms_sdi) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    if ((MMS_BTYPE_STRUCT == p_mms_sdi->btype))
    {
        snprintf(buffer, buffer_length, "$%s", p_mms_sdi->name);
        lv_string_length = strlen(buffer);
        for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->left)
        {
            MmsGetSdiNameListDirect(lv_p_sdi_sdi, buffer_start, buffer + lv_string_length, buffer_length - lv_string_length, &lv_p_send_buffer_ptr);
        }
        buffer[lv_string_length] = '\0';
        String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x1Au, buffer_start);
    }
    else
    {
        snprintf(buffer, buffer_length, "$%s", p_mms_sdi->name);
        String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x1Au, buffer_start);
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}


#define MMS_GETVARIABLEACCESSATTRIBUTES_NAME 0xA0u
#define MMS_GETVARIABLEACCESSATTRIBUTES_NAME_DOMAIN 0xA1u
static int32 MmsParseGetVariableAccessAttributes(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{

    Uint8 const *lv_p_frame;
    Uint8 lv_tag;
    int32 lv_length;
    Uint8 const *lv_p_frame_temp;
    Uint8 lv_tag_temp;
    int32 lv_length_temp;
    char lv_buffer[256u];
    MMS_COMMUNICATION *lv_p_mms_comm;

    if ((NULL == p_mms_comm) || (NULL == p_mms_socket_data))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;

    while (lv_p_frame < lv_p_mms_comm->p_frame_end)
    {
        lv_tag = *lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if (NORMAL_ERROR == lv_length)
        {
            return 2;
        }
        switch (lv_tag)
        {
            case MMS_GETVARIABLEACCESSATTRIBUTES_NAME:
            {
                lv_p_frame_temp = lv_p_frame;
                lv_tag_temp = *lv_p_frame_temp++;
                lv_length_temp = Asn12Length(&lv_p_frame_temp);
                if (lv_tag_temp == MMS_GETVARIABLEACCESSATTRIBUTES_NAME_DOMAIN)
                {
                    lv_tag_temp = *lv_p_frame_temp++;
                    lv_length_temp = Asn12Length(&lv_p_frame_temp);
                    lv_p_frame_temp += lv_length_temp;
                    lv_p_frame_temp++;
                    if (NORMAL_SUCCESS != Asn12String(&lv_p_frame_temp, lv_buffer, sizeof(lv_buffer)))
                    {
                        return 2;
                    }
                    if (NORMAL_SUCCESS != MmsGetAttributesName(lv_buffer, p_mms_socket_data, p_mms_comm))
                    {
                        return 2;
                    }
                }
                break;
            }
            default:
            {
                break;
            }
        }
        lv_p_frame += lv_length;
    }

    lv_p_mms_comm->p_frame = lv_p_frame;

    return NORMAL_SUCCESS;
}

static int32 MmsGetAttributesName(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    MMS_COMMUNICATION *lv_p_mms_comm;
    char *lv_p_name;
    char *lv_p_name_temp;
    char lv_buffer[32u];
    int16 lv_fc_index;
    MMS_LN  const *lv_p_ln_active;
    MMS_DOI const *lv_p_doi_active;
    MMS_SDI const *lv_p_sdi_active;
    Uint8 *lv_p_temp_buffer;
    Uint8 *lv_p_buffer_ptr;
    Uint8 lv_mms_fc_flag;
    int32 lv_length;
    Uint8 *lv_p_buffer_tail;

    if ((NULL == p_mms_comm) || (NULL == p_mms_socket_data))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;

    lv_p_name = name;
    lv_p_name_temp = strchr(lv_p_name, '$');
    if (NULL != lv_p_name_temp)
    {
        *lv_p_name_temp = '\0';
        lv_p_name_temp++;
    }

    for (lv_p_ln_active = p_mms_socket_data->p_ied->access_point->server->ldevice->ln_head; NULL != lv_p_ln_active; lv_p_ln_active = lv_p_ln_active->right)
    {
        snprintf(lv_buffer, sizeof(lv_buffer), "%s%s%s", lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
        if (0 == strcmp(lv_p_name, lv_buffer))
        {
            break;
        }
    }

    if (NULL == lv_p_ln_active)
    {
        TRACE("ln \"%s\" found failed!", lv_p_name);
        return NORMAL_ERROR;
    }

    lv_p_temp_buffer = malloc(MMS_TEMP_BUFFER_SIZE);
    if (NULL == lv_p_temp_buffer)
    {
        TRACE("MMS temp buffer malloc failed!");
        return NORMAL_ERROR;
    }

    lv_p_buffer_tail = lv_p_temp_buffer + MMS_TEMP_BUFFER_SIZE;
    lv_p_buffer_ptr = lv_p_buffer_tail;
    if (NULL == lv_p_name_temp)
    {
        if (NORMAL_SUCCESS != MmsGetLnAttributesName(lv_p_ln_active, &lv_p_buffer_ptr))
        {
            free(lv_p_temp_buffer);
            return NORMAL_ERROR;
        }
    }
    else
    {
        lv_p_name = lv_p_name_temp;
        lv_p_name_temp = strchr(lv_p_name, '$');
        if (NULL != lv_p_name_temp)
        {
            *lv_p_name_temp = '\0';
            lv_p_name_temp++;
        }

        for (lv_fc_index = 1; lv_fc_index < mms_fc_num; lv_fc_index++)
        {
            if (0 == strcmp(lv_p_name, mms_fc[lv_fc_index].name))
            {
                break;
            }
        }

        if (lv_fc_index >= mms_fc_num)
        {
            TRACE("unknown mms_fc=%s", lv_buffer);
            free(lv_p_temp_buffer);
            return NORMAL_ERROR;
        }

        if (NULL == lv_p_name_temp)
        {
            if (NORMAL_SUCCESS != MmsGetFcAttributesName(lv_p_ln_active, lv_fc_index, &lv_mms_fc_flag, &lv_p_buffer_ptr))
            {
                free(lv_p_temp_buffer);
                return NORMAL_ERROR;
            }
        }
        else
        {
            lv_p_name = lv_p_name_temp;
            lv_p_name_temp = strchr(lv_p_name, '$');
            if (NULL != lv_p_name_temp)
            {
                *lv_p_name_temp = '\0';
                lv_p_name_temp++;
            }

            for (lv_p_doi_active = lv_p_ln_active->doi; NULL != lv_p_doi_active; lv_p_doi_active = lv_p_doi_active->left)
            {
                if (0 == strcmp(lv_p_name, lv_p_doi_active->name))
                {
                    break;
                }
            }

            if (NULL == lv_p_doi_active)
            {
                TRACE("doi \"%s\" found in ln \"%s%s%s\" failed!", lv_p_name, lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
                free(lv_p_temp_buffer);
                return NORMAL_ERROR;
            }

            if (NULL == lv_p_name_temp)
            {
                if (NORMAL_SUCCESS != MmsGetDoiAttributesName(lv_p_doi_active, lv_fc_index, &lv_mms_fc_flag, &lv_p_buffer_ptr))
                {
                    free(lv_p_temp_buffer);
                    return NORMAL_ERROR;
                }
            }
            else
            {
                lv_p_sdi_active = lv_p_doi_active->sdi;
                while (NULL != lv_p_name_temp)
                {
                    lv_p_name = lv_p_name_temp;
                    lv_p_name_temp = strchr(lv_p_name, '$');
                    if (NULL != lv_p_name_temp)
                    {
                        *lv_p_name_temp = '\0';
                        lv_p_name_temp++;
                    }

                    for (; NULL != lv_p_sdi_active; lv_p_sdi_active = lv_p_sdi_active->left)
                    {
                        if (0 == strcmp(lv_p_name, lv_p_sdi_active->name))
                        {
                            break;
                        }
                    }

                    if (NULL == lv_p_sdi_active)
                    {
                        TRACE("sdi \"%s\" found in ln \"%s%s%s\" failed!", lv_p_name, lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
                        free(lv_p_temp_buffer);
                        return NORMAL_ERROR;
                    }

                    if ((NULL != lv_p_name_temp) && (NULL != lv_p_sdi_active->sdi))
                    {
                        lv_p_sdi_active = lv_p_sdi_active->sdi;
                    }
                }

                if (MMS_BTYPE_SDO == lv_p_sdi_active->btype)
                {
                    if (NORMAL_SUCCESS != MmsGetSdiAttributesName(lv_p_sdi_active, lv_fc_index, &lv_mms_fc_flag, &lv_p_buffer_ptr))
                    {
                        free(lv_p_temp_buffer);
                        return NORMAL_ERROR;
                    }
                }
                else
                {
                    if (NORMAL_SUCCESS != MmsGetSdiAttributesNameDirect(lv_p_sdi_active, &lv_p_buffer_ptr))
                    {
                        free(lv_p_temp_buffer);
                        return NORMAL_ERROR;
                    }
                }
            }
        }
    }

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, 0xA2u, lv_length);

    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = 0x01u;
    *(--lv_p_buffer_ptr) = 0x80u;

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_GETVARIABLEACCESSATTRIBUTES, lv_length);

    Uint322Asn1r(&lv_p_buffer_ptr, 0x02u, lv_p_mms_comm->invoke_id);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_CONFIRMED_RESPONSEPDU, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_DATA_VALUES, lv_length);

    *(--lv_p_buffer_ptr) = 0x03u;
    *(--lv_p_buffer_ptr) = 0x01u;
    *(--lv_p_buffer_ptr) = PRESENTATION_CONTEXT_IDENTIFIER;

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_PDV_LIST, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_USER_DATA_FULLY_ENCODED_DATA, lv_length);

    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_DATA_TRANSFER;
    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_GIVE_TOKENS;

    if (NORMAL_SUCCESS != TpckPacketSend(p_mms_socket_data, lv_p_mms_comm->cotp_client_tpdu_size ,lv_p_buffer_ptr, lv_p_buffer_tail - lv_p_buffer_ptr))
    {
        TRACE("MMS tpck packet send failed!");
        free(lv_p_temp_buffer);
        return NORMAL_ERROR;
    }

    free(lv_p_temp_buffer);

    return NORMAL_SUCCESS;
}

static int32 MmsGetLnAttributesName(MMS_LN const *p_mms_ln,  Uint8 **p_send_buffer_ptr)
{
    int16 lv_fc_index;
    int32 lv_length;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_ln_temp;
    MMS_LN  const *lv_p_ln_active;
    Uint8 lv_fc_flag;

    if ((NULL == p_mms_ln) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_ln_active = p_mms_ln;
    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
    for (lv_fc_index = (mms_fc_num - 1); lv_fc_index > 0; lv_fc_index--)
    {
        lv_fc_flag = 0;
        lv_p_send_buffer_ptr_ln_temp = lv_p_send_buffer_ptr;
        if (NORMAL_SUCCESS != MmsGetFcAttributesName(lv_p_ln_active, lv_fc_index, &lv_fc_flag, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }
                
        if (0 == lv_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_ln_temp;
        }
        else
        {
            lv_length = lv_p_send_buffer_ptr_ln_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

            String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x80u, mms_fc[lv_fc_index].name);

            lv_length = lv_p_send_buffer_ptr_ln_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0x30u, lv_length);
        }
    }

    lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

    lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsGetFcAttributesName(MMS_LN const *p_mms_ln, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr)
{
    Uint8 lv_fc_flag;
    Uint8 lv_doi_fc_flag;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_doi_temp;
    MMS_DOI const *lv_p_doi;
    int32 lv_length;

    lv_fc_flag = 0u;
    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
    for (lv_p_doi = p_mms_ln->doi; NULL != lv_p_doi; lv_p_doi = lv_p_doi->left)
    {
        lv_doi_fc_flag = 0u;
        lv_p_send_buffer_ptr_doi_temp = lv_p_send_buffer_ptr;
        if (NORMAL_SUCCESS != MmsGetDoiAttributesName(lv_p_doi, fc_index, &lv_doi_fc_flag, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }
                
        if (0u == lv_doi_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_doi_temp;
        }
        else
        {
            lv_length = lv_p_send_buffer_ptr_doi_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

            String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x80u, lv_p_doi->name);

            lv_length = lv_p_send_buffer_ptr_doi_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0x30u, lv_length);

            lv_fc_flag = 1u;
        }
    }

    if (0u == lv_fc_flag)
    {
       lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_temp;
    }
    else
    {
        lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
        Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

        lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
        Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

        *p_fc_pre = 1u;
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsGetDoiAttributesName(MMS_DOI const *p_mms_doi, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr)
{
    Uint8 lv_fc_flag;
    Uint8 lv_sdi_fc_flag;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_sdi_temp;
    MMS_SDI const *lv_p_sdi;
    int32 lv_length;

    if ((NULL == p_mms_doi) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr) || (NULL == p_fc_pre) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_fc_flag = 0u;
    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
    for (lv_p_sdi = p_mms_doi->sdi; NULL != lv_p_sdi; lv_p_sdi = lv_p_sdi->left)
    {
        lv_sdi_fc_flag = 0u;
        lv_p_send_buffer_ptr_sdi_temp = lv_p_send_buffer_ptr;
        if (NORMAL_SUCCESS != MmsGetSdiAttributesName(lv_p_sdi, fc_index, &lv_sdi_fc_flag, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }
                
        if (0u == lv_sdi_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_sdi_temp;
        }
        else
        {
            lv_length = lv_p_send_buffer_ptr_sdi_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

            String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x80u, lv_p_sdi->name);

            lv_length = lv_p_send_buffer_ptr_sdi_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0x30u, lv_length);

            lv_fc_flag = 1u;
        }
    }

    if (0u == lv_fc_flag)
    {
        lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_temp;
    }
    else
    {
        lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
        Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

        lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
        Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

        *p_fc_pre = 1u;
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;

}

static int32 MmsGetSdiAttributesName(MMS_SDI const *p_mms_sdi, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr)
{
    Uint8 lv_fc_flag;
    Uint8 lv_sdi_fc_flag;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_sdi_temp;
    MMS_SDI const *lv_p_sdi_sdi;
    int32 lv_length;

    if ((NULL == p_mms_sdi) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr) || (NULL == p_fc_pre) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    if (MMS_BTYPE_SDO == p_mms_sdi->btype)
    {
        lv_fc_flag = 0u;
        lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
        for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->left)
        {
            lv_sdi_fc_flag = 0u;
            lv_p_send_buffer_ptr_sdi_temp = lv_p_send_buffer_ptr;
            if (NORMAL_SUCCESS != MmsGetSdiAttributesName(lv_p_sdi_sdi, fc_index, &lv_sdi_fc_flag, &lv_p_send_buffer_ptr))
            {
                return NORMAL_ERROR;
            }

            if (0u == lv_sdi_fc_flag)
            {
                lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_sdi_temp;
            }
            else
            {
                lv_length = lv_p_send_buffer_ptr_sdi_temp - lv_p_send_buffer_ptr;
                Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

                String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x80u, lv_p_sdi_sdi->name);

                lv_length = lv_p_send_buffer_ptr_sdi_temp - lv_p_send_buffer_ptr;
                Length2Asn1r(&lv_p_send_buffer_ptr, 0x30u, lv_length);

                lv_fc_flag = 1u;
            }
        }

        if (0u == lv_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_temp;
        }
        else
        {
            lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

            lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

            *p_fc_pre = 1u;
        }
    }
    else if ((MMS_BTYPE_STRUCT == p_mms_sdi->btype))
    {
        if (fc_index == p_mms_sdi->fc_index)
        {
            lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
            for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->left)
            {
                lv_p_send_buffer_ptr_sdi_temp = lv_p_send_buffer_ptr;
                if (NORMAL_SUCCESS != MmsGetSdiAttributesNameDirect(lv_p_sdi_sdi, &lv_p_send_buffer_ptr))
                {
                    return NORMAL_ERROR;
                }

                lv_length = lv_p_send_buffer_ptr_sdi_temp - lv_p_send_buffer_ptr;
                Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

                String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x80u, lv_p_sdi_sdi->name);

                lv_length = lv_p_send_buffer_ptr_sdi_temp - lv_p_send_buffer_ptr;
                Length2Asn1r(&lv_p_send_buffer_ptr, 0x30u, lv_length);
            }

            lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

            lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

            *p_fc_pre = 1u;
        }
    }
    else
    {
        if (fc_index == p_mms_sdi->fc_index)
        {
            if (NORMAL_SUCCESS != MmsBasicTypeDescriptionToBuffer(p_mms_sdi, &lv_p_send_buffer_ptr))
            {
                return NORMAL_ERROR;
            }

            *p_fc_pre = 1u;
        }
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsGetSdiAttributesNameDirect(MMS_SDI const *p_mms_sdi, Uint8 **p_send_buffer_ptr)
{
    MMS_SDI const *lv_p_sdi_sdi;
    int32 lv_length;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_sdi_temp;

    if ((NULL == p_mms_sdi) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    if ((MMS_BTYPE_STRUCT == p_mms_sdi->btype))
    {
        lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
        for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->left)
        {
            lv_p_send_buffer_ptr_sdi_temp = lv_p_send_buffer_ptr;
            if (NORMAL_SUCCESS != MmsGetSdiAttributesNameDirect(lv_p_sdi_sdi, &lv_p_send_buffer_ptr))
            {
                return NORMAL_ERROR;
            }

            lv_length = lv_p_send_buffer_ptr_sdi_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

            String2Asn1r(&lv_p_send_buffer_ptr, 0, 0x80u, lv_p_sdi_sdi->name);

            lv_length = lv_p_send_buffer_ptr_sdi_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0x30u, lv_length);
        }

        lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
        Length2Asn1r(&lv_p_send_buffer_ptr, 0xA1u, lv_length);

        lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
        Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);
    }
    else
    {
        if (NORMAL_SUCCESS != MmsBasicTypeDescriptionToBuffer(p_mms_sdi, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsParseGetNamedVariableListAttributes(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{

    Uint8 const *lv_p_frame;
    Uint8 lv_tag;
    int32 lv_length;
    Uint8 const *lv_p_frame_temp;
    int32 lv_length_temp;
    char lv_buffer[256u];
    MMS_COMMUNICATION *lv_p_mms_comm;

    if ((NULL == p_mms_comm) || (NULL == p_mms_socket_data))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;

    while (lv_p_frame < lv_p_mms_comm->p_frame_end)
    {
        lv_tag = *lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if (NORMAL_ERROR == lv_length)
        {
            return 2;
        }

        if (lv_tag == MMS_GETVARIABLEACCESSATTRIBUTES_NAME_DOMAIN)
        {
            lv_p_frame_temp = lv_p_frame + 1;
            lv_length_temp = Asn12Length(&lv_p_frame_temp);
            lv_p_frame_temp += lv_length_temp;
            lv_p_frame_temp++;
            if (NORMAL_SUCCESS != Asn12String(&lv_p_frame_temp, lv_buffer, sizeof(lv_buffer)))
            {
                return 2;
            }

            if (NORMAL_SUCCESS != MmsGetNamedVariableListAttributes(lv_buffer, p_mms_socket_data, p_mms_comm))
            {
                return 2;
            }
        }

        lv_p_frame += lv_length;
    }

    lv_p_mms_comm->p_frame = lv_p_frame;

    return NORMAL_SUCCESS;
}

static int32 MmsGetNamedVariableListAttributes(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    MMS_COMMUNICATION *lv_p_mms_comm;
    char *lv_p_name;
    char *lv_p_name_temp;
    char lv_buffer[256u];
    int32 lv_length;
    int32 lv_buffer_length;
    char *lv_p_local_buffer;
    MMS_LN  const *lv_p_ln_active;
    MMS_DATASET const *lv_p_dataset_active;
    MMS_FCD const *lv_p_mms_fcd;
    MMS_FCD_SDI const *lv_p_mms_fcd_sdi;
    Uint8 *lv_p_temp_buffer;
    Uint8 *lv_p_buffer_ptr;
    Uint8 *lv_p_buffer_tail;
    Uint8 *lv_p_buffer_ptr_temp;

    if ((NULL == p_mms_comm) || (NULL == p_mms_socket_data))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;

    lv_p_name = name;
    lv_p_name_temp = strchr(lv_p_name, '$');
    if (NULL != lv_p_name_temp)
    {
        *lv_p_name_temp = '\0';
    }

    for (lv_p_ln_active = p_mms_socket_data->p_ied->access_point->server->ldevice->ln_head; NULL != lv_p_ln_active; lv_p_ln_active = lv_p_ln_active->right)
    {
        snprintf(lv_buffer, sizeof(lv_buffer), "%s%s%s", lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
        if (0 == strcmp(lv_p_name, lv_buffer))
        {
            break;
        }
    }

    if (NULL == lv_p_ln_active)
    {
        TRACE("ln \"%s\" found failed!", lv_p_name);
        return NORMAL_ERROR;
    }

    lv_p_temp_buffer = malloc(MMS_TEMP_BUFFER_SIZE);
    if (NULL == lv_p_temp_buffer)
    {
        TRACE("MMS temp buffer malloc failed!");
        return NORMAL_ERROR;
    }

    lv_p_buffer_tail = lv_p_temp_buffer + MMS_TEMP_BUFFER_SIZE;
    lv_p_buffer_ptr = lv_p_buffer_tail;
    if (NULL == lv_p_name_temp)
    {
        TRACE("%s need sub domain!", name);
        free(lv_p_temp_buffer);
        return NORMAL_ERROR;
    }
    else
    {
        lv_p_name = lv_p_name_temp + 1;
        lv_p_name_temp = strstr(lv_p_name, "$");
        if (NULL != lv_p_name_temp)
        {
            *lv_p_name_temp = '\0';
        }

        for (lv_p_dataset_active = p_mms_socket_data->p_ied->access_point->server->ldevice->dataset; NULL != lv_p_dataset_active; lv_p_dataset_active = lv_p_dataset_active->left)
        {
            if (0 == strcmp(lv_p_name, lv_p_dataset_active->name))
            {
                break;
            }
        }

        if (NULL == lv_p_dataset_active)
        {
            TRACE("ln \"%s\" dataset \"%s\"found failed!", name, lv_p_name);
            free(lv_p_temp_buffer);
            return NORMAL_ERROR;
        }

        for (lv_p_mms_fcd = lv_p_dataset_active->fcd; NULL != lv_p_mms_fcd; lv_p_mms_fcd = lv_p_mms_fcd->left)
        {
            lv_p_buffer_ptr_temp = lv_p_buffer_ptr;
            snprintf(lv_buffer, sizeof(lv_buffer), "%s%s%s$%s$%s", lv_p_mms_fcd->ln->prefix
                                                                 , lv_p_mms_fcd->ln->lnclass
                                                                 , lv_p_mms_fcd->ln->inst
                                                                 , mms_fc[lv_p_mms_fcd->fc_index].name
                                                                 , lv_p_mms_fcd->doi->name);
            lv_length = strlen(lv_buffer);
            lv_p_local_buffer = lv_buffer + lv_length;
            lv_buffer_length = sizeof(lv_buffer) - lv_length;
            for (lv_p_mms_fcd_sdi = lv_p_mms_fcd->fcd_sdi; NULL != lv_p_mms_fcd_sdi; lv_p_mms_fcd_sdi = lv_p_mms_fcd_sdi->down)
            {
                snprintf(lv_p_local_buffer, lv_buffer_length, "$%s", lv_p_mms_fcd_sdi->sdi->name);
                lv_length = strlen(lv_p_local_buffer);
                lv_p_local_buffer = lv_p_local_buffer + lv_length;
                lv_buffer_length = lv_buffer_length - lv_length;
            }
            String2Asn1r(&lv_p_buffer_ptr, 0, 0x1Au, lv_buffer);

            snprintf(lv_buffer, sizeof(lv_buffer), "%s%s%s", p_mms_socket_data->p_ied->name
                                                           , p_mms_socket_data->p_ied->access_point->name
                                                           , p_mms_socket_data->p_ied->access_point->server->ldevice->inst);

            String2Asn1r(&lv_p_buffer_ptr, 0, 0x1Au, lv_buffer);

            lv_length = lv_p_buffer_ptr_temp - lv_p_buffer_ptr;
            Length2Asn1r(&lv_p_buffer_ptr, 0xA1, lv_length);

            lv_length = lv_p_buffer_ptr_temp - lv_p_buffer_ptr;
            Length2Asn1r(&lv_p_buffer_ptr, 0xA0, lv_length);

            lv_length = lv_p_buffer_ptr_temp - lv_p_buffer_ptr;
            Length2Asn1r(&lv_p_buffer_ptr, 0x30, lv_length);
        }

        lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
        Length2Asn1r(&lv_p_buffer_ptr, 0xA1, lv_length);

        *(--lv_p_buffer_ptr) = 0x00u;
        *(--lv_p_buffer_ptr) = 0x01u;
        *(--lv_p_buffer_ptr) = 0x80u;
    }

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, 0xAC, lv_length);

    Uint322Asn1r(&lv_p_buffer_ptr, 0x02u, lv_p_mms_comm->invoke_id);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_CONFIRMED_RESPONSEPDU, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_DATA_VALUES, lv_length);

    *(--lv_p_buffer_ptr) = 0x03u;
    *(--lv_p_buffer_ptr) = 0x01u;
    *(--lv_p_buffer_ptr) = PRESENTATION_CONTEXT_IDENTIFIER;

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_PDV_LIST, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_USER_DATA_FULLY_ENCODED_DATA, lv_length);

    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_DATA_TRANSFER;
    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_GIVE_TOKENS;

    if (NORMAL_SUCCESS != TpckPacketSend(p_mms_socket_data, lv_p_mms_comm->cotp_client_tpdu_size ,lv_p_buffer_ptr, lv_p_buffer_tail - lv_p_buffer_ptr))
    {
        TRACE("MMS tpck packet send failed!");
        free(lv_p_temp_buffer);
        return NORMAL_ERROR;
    }

    free(lv_p_temp_buffer);

    return NORMAL_SUCCESS;
}

#define MMS_VARIABLEACCESSSPECIFICATION 0xA1u
#define MMS_LISTOFVARIABLE 0xA0u
#define MMS_VARIABLESPECIFICATION 0x30u
#define MMS_VARIABLESPECIFICATION_NAME 0xA0u
#define MMS_OBJECTNAME_DOMAIN_SPECIFIC  0xA1u
static int32 MmsParseRead(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    Uint8 const *lv_p_frame;
    Uint8 lv_tag;
    int32 lv_length;
    Uint8 const *lv_p_frame_temp;
    Uint8 lv_tag_temp;
    int32 lv_length_temp;
    char lv_buffer[256u];
    MMS_COMMUNICATION *lv_p_mms_comm;

    if ((NULL == p_mms_comm) || (NULL == p_mms_socket_data))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;

    while (lv_p_frame < lv_p_mms_comm->p_frame_end)
    {
        lv_tag = *lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if (NORMAL_ERROR == lv_length)
        {
            return 2;
        }
        switch (lv_tag)
        {
            case MMS_VARIABLEACCESSSPECIFICATION:
            {
                lv_p_frame_temp = lv_p_frame;
                lv_tag_temp = *lv_p_frame_temp++;
                lv_length_temp = Asn12Length(&lv_p_frame_temp);
                if (lv_tag_temp == MMS_LISTOFVARIABLE)
                {
                    lv_tag_temp = *lv_p_frame_temp++;
                    lv_length_temp = Asn12Length(&lv_p_frame_temp);
                    if (lv_tag_temp == MMS_VARIABLESPECIFICATION)
                    {
                        lv_tag_temp = *lv_p_frame_temp++;
                        lv_length_temp = Asn12Length(&lv_p_frame_temp);
                        if (lv_tag_temp == MMS_VARIABLESPECIFICATION_NAME)
                        {
                            lv_tag_temp = *lv_p_frame_temp++;
                            lv_length_temp = Asn12Length(&lv_p_frame_temp);
                            if (lv_tag_temp == MMS_OBJECTNAME_DOMAIN_SPECIFIC)
                            {
                                lv_tag_temp = *lv_p_frame_temp++;
                                lv_length_temp = Asn12Length(&lv_p_frame_temp);
                                lv_p_frame_temp += lv_length_temp;
                                lv_p_frame_temp++;
                                if (NORMAL_SUCCESS != Asn12String(&lv_p_frame_temp, lv_buffer, sizeof(lv_buffer)))
                                {
                                    return 2;
                                }

                                if (NORMAL_SUCCESS != MmsReadByName(lv_buffer, p_mms_socket_data, p_mms_comm))
                                {
                                    return 2;
                                }
                            }
                            else
                            {
                                TRACE("unknow OBJECTNAME!");
                                return NORMAL_ERROR;
                            }
                        }
                        else
                        {
                            TRACE("unknow VARIABLESPECIFICATION!");
                            return NORMAL_ERROR;
                        }
                    }
                    else
                    {
                        TRACE("unknow VARIABLESPECIFICATION!");
                        return NORMAL_ERROR;
                    }
                }
                else
                {
                    TRACE("unknow tag!");
                    return NORMAL_ERROR;
                }
                break;
            }
            default:
            {
                break;
            }
        }
        lv_p_frame += lv_length;
    }

    lv_p_mms_comm->p_frame = lv_p_frame;

    return NORMAL_SUCCESS;
}

#define MMS_READ_LISTOFACCESSRESULT 0xA1u
static int32 MmsReadByName(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    MMS_COMMUNICATION *lv_p_mms_comm;
    char *lv_p_name;
    char *lv_p_name_temp;
    char lv_buffer[32u];
    MMS_LN  const *lv_p_ln_active;
    MMS_DOI const *lv_p_doi_active;
    MMS_SDI const *lv_p_sdi_active;
    Uint8 *lv_p_temp_buffer;
    Uint8 *lv_p_buffer_ptr;
    int32 lv_length;
    Uint8 *lv_p_buffer_tail;
    int16  lv_fc_index;
    Uint8  lv_mms_fc_flag;

    if ((NULL == p_mms_comm) || (NULL == p_mms_socket_data))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;

    lv_p_name = name;
    lv_p_name_temp = strchr(lv_p_name, '$');
    if (NULL != lv_p_name_temp)
    {
        *lv_p_name_temp = '\0';
        lv_p_name_temp++;
    }

    for (lv_p_ln_active = p_mms_socket_data->p_ied->access_point->server->ldevice->ln_head; NULL != lv_p_ln_active; lv_p_ln_active = lv_p_ln_active->right)
    {
        snprintf(lv_buffer, sizeof(lv_buffer), "%s%s%s", lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
        if (0 == strcmp(lv_p_name, lv_buffer))
        {
            break;
        }
    }

    if (NULL == lv_p_ln_active)
    {
        TRACE("ln \"%s\" found failed!", lv_p_name);
        return NORMAL_ERROR;
    }

    lv_p_temp_buffer = malloc(MMS_TEMP_BUFFER_SIZE);
    if (NULL == lv_p_temp_buffer)
    {
        TRACE("MMS temp buffer malloc failed!");
        return NORMAL_ERROR;
    }

    lv_p_buffer_tail = lv_p_temp_buffer + MMS_TEMP_BUFFER_SIZE;
    lv_p_buffer_ptr = lv_p_buffer_tail;
    if (NULL == lv_p_name_temp)
    {
        if (NORMAL_SUCCESS != MmsReadLn(lv_p_ln_active, &lv_p_buffer_ptr))
        {
            free(lv_p_temp_buffer);
            return NORMAL_ERROR;
        }
    }
    else
    {
        lv_p_name = lv_p_name_temp;
        lv_p_name_temp = strchr(lv_p_name, '$');
        if (NULL != lv_p_name_temp)
        {
            *lv_p_name_temp = '\0';
            lv_p_name_temp++;
        }

        for (lv_fc_index = 1; lv_fc_index < mms_fc_num; lv_fc_index++)
        {
            if (0 == strcmp(lv_p_name, mms_fc[lv_fc_index].name))
            {
                break;
            }
        }

        if (lv_fc_index >= mms_fc_num)
        {
            TRACE("unknown mms_fc=%s", lv_buffer);
            free(lv_p_temp_buffer);
            return NORMAL_ERROR;
        }

        if (NULL == lv_p_name_temp)
        {
            if (NORMAL_SUCCESS != MmsReadFc(lv_p_ln_active, lv_fc_index, &lv_mms_fc_flag, &lv_p_buffer_ptr))
            {
                free(lv_p_temp_buffer);
                return NORMAL_ERROR;
            }
        }
        else
        {
            lv_p_name = lv_p_name_temp;
            lv_p_name_temp = strchr(lv_p_name, '$');
            if (NULL != lv_p_name_temp)
            {
                *lv_p_name_temp = '\0';
                lv_p_name_temp++;
            }

            for (lv_p_doi_active = lv_p_ln_active->doi; NULL != lv_p_doi_active; lv_p_doi_active = lv_p_doi_active->left)
            {
                if (0 == strcmp(lv_p_name, lv_p_doi_active->name))
                {
                    break;
                }
            }

            if (NULL == lv_p_doi_active)
            {
                TRACE("doi \"%s\" found in ln \"%s%s%s\" failed!", lv_p_name, lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
                free(lv_p_temp_buffer);
                return NORMAL_ERROR;
            }

            if (NULL == lv_p_name_temp)
            {
                if (NORMAL_SUCCESS != MmsReadDoi(lv_p_doi_active, lv_fc_index, &lv_mms_fc_flag, &lv_p_buffer_ptr))
                {
                    free(lv_p_temp_buffer);
                    return NORMAL_ERROR;
                }
            }
            else
            {
                lv_p_sdi_active = lv_p_doi_active->sdi;
                while (NULL != lv_p_name_temp)
                {
                    lv_p_name = lv_p_name_temp;
                    lv_p_name_temp = strchr(lv_p_name, '$');
                    if (NULL != lv_p_name_temp)
                    {
                        *lv_p_name_temp = '\0';
                        lv_p_name_temp++;
                    }

                    for (; NULL != lv_p_sdi_active; lv_p_sdi_active = lv_p_sdi_active->left)
                    {
                        if (0 == strcmp(lv_p_name, lv_p_sdi_active->name))
                        {
                            break;
                        }
                    }

                    if (NULL == lv_p_sdi_active)
                    {
                        TRACE("sdi \"%s\" found in ln \"%s%s%s\" failed!", lv_p_name, lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
                        free(lv_p_temp_buffer);
                        return NORMAL_ERROR;
                    }

                    if ((NULL != lv_p_name_temp) && (NULL != lv_p_sdi_active->sdi))
                    {
                        lv_p_sdi_active = lv_p_sdi_active->sdi;
                    }
                }

                if (MMS_BTYPE_SDO == lv_p_sdi_active->btype)
                {
                    if (NORMAL_SUCCESS != MmsReadSdi(lv_p_sdi_active, lv_fc_index, &lv_mms_fc_flag, &lv_p_buffer_ptr))
                    {
                        free(lv_p_temp_buffer);
                        return NORMAL_ERROR;
                    }
                }
                else
                {
                    if (NORMAL_SUCCESS != MmsReadSdiDirect(lv_p_sdi_active, &lv_p_buffer_ptr))
                    {
                        free(lv_p_temp_buffer);
                        return NORMAL_ERROR;
                    }
                }
            }
        }
    }

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_READ_LISTOFACCESSRESULT, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_READ, lv_length);

    Uint322Asn1r(&lv_p_buffer_ptr, 0x02u, lv_p_mms_comm->invoke_id);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_CONFIRMED_RESPONSEPDU, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_DATA_VALUES, lv_length);

    *(--lv_p_buffer_ptr) = 0x03u;
    *(--lv_p_buffer_ptr) = 0x01u;
    *(--lv_p_buffer_ptr) = PRESENTATION_CONTEXT_IDENTIFIER;

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_PDV_LIST, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_USER_DATA_FULLY_ENCODED_DATA, lv_length);

    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_DATA_TRANSFER;
    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_GIVE_TOKENS;

    if (NORMAL_SUCCESS != TpckPacketSend(p_mms_socket_data, lv_p_mms_comm->cotp_client_tpdu_size ,lv_p_buffer_ptr, lv_p_buffer_tail - lv_p_buffer_ptr))
    {
        TRACE("MMS tpck packet send failed!");
        free(lv_p_temp_buffer);
        return NORMAL_ERROR;
    }

    free(lv_p_temp_buffer);

    return NORMAL_SUCCESS;
}

static int32 MmsReadLn(MMS_LN const *p_mms_ln,  Uint8 **p_send_buffer_ptr)
{
    int32 lv_index;
    int32 lv_length;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_ln_temp;
    MMS_LN  const *lv_p_ln_active;
    Uint8 lv_fc_flag;

    if ((NULL == p_mms_ln) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_ln_active = p_mms_ln;
    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
    for (lv_index = (mms_fc_num - 1); lv_index > 0; lv_index--)
    {
        lv_fc_flag = 0;
        lv_p_send_buffer_ptr_ln_temp = lv_p_send_buffer_ptr;
        if (NORMAL_SUCCESS != MmsReadFc(lv_p_ln_active, lv_index, &lv_fc_flag, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }

        if (0 == lv_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_ln_temp;
        }
        else
        {
            lv_length = lv_p_send_buffer_ptr_ln_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);
        }
    }

    lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
    Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsReadFc(MMS_LN const *p_mms_ln, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr)
{
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_doi_temp;
    MMS_DOI const *lv_p_doi;
    Uint8 lv_fc_flag;
    Uint8 lv_doi_fc_flag;
    int32 lv_length;

    if ((NULL == p_mms_ln) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr) || (NULL == p_fc_pre) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_fc_flag = 0u;
    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
    for (lv_p_doi = p_mms_ln->doi; NULL != lv_p_doi; lv_p_doi = lv_p_doi->left)
    {
        lv_doi_fc_flag = 0u;
        lv_p_send_buffer_ptr_doi_temp = lv_p_send_buffer_ptr;
        if (NORMAL_SUCCESS != MmsReadDoi(lv_p_doi, fc_index, &lv_doi_fc_flag, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }

        if (0u == lv_doi_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_doi_temp;
        }
        else
        {
            lv_fc_flag = 1u;
        }
    }

    if (0u == lv_fc_flag)
    {
        lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_temp;
    }
    else
    {
        lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
        Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

        *p_fc_pre = 1u;
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}


static int32 MmsReadDoi(MMS_DOI const *p_mms_doi, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr)
{
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_sdi_temp;
    MMS_SDI const *lv_p_sdi;
    Uint8 lv_fc_flag;
    Uint8 lv_sdi_fc_flag;
    int32 lv_length;

    if ((NULL == p_mms_doi) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr) || (NULL == p_fc_pre) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_fc_flag = 0u;
    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
    for (lv_p_sdi = p_mms_doi->sdi; NULL != lv_p_sdi; lv_p_sdi = lv_p_sdi->left)
    {
        lv_sdi_fc_flag = 0u;
        lv_p_send_buffer_ptr_sdi_temp = lv_p_send_buffer_ptr;
        if (NORMAL_SUCCESS != MmsReadSdi(lv_p_sdi, fc_index, &lv_sdi_fc_flag, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }
                
        if (0u == lv_sdi_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_sdi_temp;
        }
        else
        {
            lv_fc_flag = 1u;
        }
    }

    if (0 == lv_fc_flag)
    {
        lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_temp;
    }
    else
    {
        lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
        Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

        *p_fc_pre = 1u;
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsReadSdi(MMS_SDI const *p_mms_sdi, int16 fc_index, Uint8 *p_fc_pre, Uint8 **p_send_buffer_ptr)
{
    Uint8 lv_fc_flag;
    Uint8 lv_sdi_fc_flag;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;
    Uint8 *lv_p_send_buffer_ptr_sdi_temp;
    MMS_SDI const *lv_p_sdi_sdi;
    int32 lv_length;

    if ((NULL == p_mms_sdi) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr) || (NULL == p_fc_pre) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    if (MMS_BTYPE_SDO == p_mms_sdi->btype)
    {
        lv_fc_flag = 0u;
        lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
        for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->left)
        {
            lv_sdi_fc_flag = 0u;
            lv_p_send_buffer_ptr_sdi_temp = lv_p_send_buffer_ptr;
            if (NORMAL_SUCCESS != MmsReadSdi(lv_p_sdi_sdi, fc_index, &lv_sdi_fc_flag, &lv_p_send_buffer_ptr))
            {
                return NORMAL_ERROR;
            }

            if (0u == lv_sdi_fc_flag)
            {
                lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_sdi_temp;
            }
            else
            {
                lv_fc_flag = 1u;
            }
        }

        if (0 == lv_fc_flag)
        {
            lv_p_send_buffer_ptr = lv_p_send_buffer_ptr_temp;
        }
        else
        {
            lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

            *p_fc_pre = 1u;
        }
    }
    else if ((MMS_BTYPE_STRUCT == p_mms_sdi->btype))
    {
        if (fc_index == p_mms_sdi->fc_index)
        {
            lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
            for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->left)
            {
                if (NORMAL_SUCCESS != MmsReadSdiDirect(lv_p_sdi_sdi, &lv_p_send_buffer_ptr))
                {
                    return NORMAL_ERROR;
                }
            }

            lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
            Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);

            *p_fc_pre = 1u;
        }
    }
    else
    {
        if (fc_index == p_mms_sdi->fc_index)
        {
            if (NORMAL_SUCCESS != MmsBasicTypeReadToBuffer(p_mms_sdi, &lv_p_send_buffer_ptr))
            {
                return NORMAL_ERROR;
            }

            *p_fc_pre = 1u;
        }
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsReadSdiDirect(MMS_SDI const *p_mms_sdi, Uint8 **p_send_buffer_ptr)
{
    MMS_SDI const *lv_p_sdi_sdi;
    int32 lv_length;
    Uint8 *lv_p_send_buffer_ptr;
    Uint8 *lv_p_send_buffer_ptr_temp;

    if ((NULL == p_mms_sdi) || (NULL == p_send_buffer_ptr) || (NULL == *p_send_buffer_ptr))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_send_buffer_ptr = *p_send_buffer_ptr;
    if ((MMS_BTYPE_STRUCT == p_mms_sdi->btype))
    {
        lv_p_send_buffer_ptr_temp = lv_p_send_buffer_ptr;
        for (lv_p_sdi_sdi = p_mms_sdi->sdi; NULL != lv_p_sdi_sdi; lv_p_sdi_sdi = lv_p_sdi_sdi->left)
        {
            if (NORMAL_SUCCESS != MmsReadSdiDirect(lv_p_sdi_sdi, &lv_p_send_buffer_ptr))
            {
                return NORMAL_ERROR;
            }
        }

        lv_length = lv_p_send_buffer_ptr_temp - lv_p_send_buffer_ptr;
        Length2Asn1r(&lv_p_send_buffer_ptr, 0xA2u, lv_length);
    }
    else
    {
        if (NORMAL_SUCCESS != MmsBasicTypeReadToBuffer(p_mms_sdi, &lv_p_send_buffer_ptr))
        {
            return NORMAL_ERROR;
        }
    }

    *p_send_buffer_ptr = lv_p_send_buffer_ptr;
    return NORMAL_SUCCESS;
}

static int32 MmsParseWrite(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    Uint8 const *lv_p_frame;
    Uint8 const *lv_p_frame_temp;
    Uint8 lv_tag;
    int32 lv_length;
    Uint8 lv_tag_temp;
    int32 lv_length_temp;
    char lv_buffer[256u];
    MMS_COMMUNICATION *lv_p_mms_comm;

    if ((NULL == p_mms_comm) || (NULL == p_mms_socket_data))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;

    while (lv_p_frame < lv_p_mms_comm->p_frame_end)
    {
        lv_tag = *lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if (NORMAL_ERROR == lv_length)
        {
            return 2;
        }
        switch (lv_tag)
        {
            case MMS_LISTOFVARIABLE:
            {
                lv_p_frame_temp = lv_p_frame;
                lv_tag_temp = *lv_p_frame_temp++;
                lv_length_temp = Asn12Length(&lv_p_frame_temp);
                if (lv_tag_temp == MMS_VARIABLESPECIFICATION)
                {
                    lv_tag_temp = *lv_p_frame_temp++;
                    lv_length_temp = Asn12Length(&lv_p_frame_temp);
                    if (lv_tag_temp == MMS_VARIABLESPECIFICATION_NAME)
                    {
                        lv_tag_temp = *lv_p_frame_temp++;
                        lv_length_temp = Asn12Length(&lv_p_frame_temp);
                        if (lv_tag_temp == MMS_OBJECTNAME_DOMAIN_SPECIFIC)
                        {
                            lv_tag_temp = *lv_p_frame_temp++;
                            lv_length_temp = Asn12Length(&lv_p_frame_temp);
                            lv_p_frame_temp += lv_length_temp;
                            lv_p_frame_temp++;
                            if (NORMAL_SUCCESS != Asn12String(&lv_p_frame_temp, lv_buffer, sizeof(lv_buffer)))
                            {
                                return 2;
                            }

                            lv_p_mms_comm->p_frame = lv_p_frame_temp;
                            if (NORMAL_SUCCESS != MmsWriteByName(lv_buffer, p_mms_socket_data, lv_p_mms_comm))
                            {
                                return 2;
                            }
                            lv_p_frame = lv_p_mms_comm->p_frame;
                        }
                        else
                        {
                            TRACE("unknow OBJECTNAME!");
                            return NORMAL_ERROR;
                        }
                    }
                    else
                    {
                        TRACE("unknow VARIABLESPECIFICATION!");
                        return NORMAL_ERROR;
                    }
                }
                else
                {
                    TRACE("unknow tag!");
                    return NORMAL_ERROR;
                }
                break;
            }
            default:
            {
                lv_p_frame += lv_length;
                break;
            }
        }
    }

    lv_p_mms_comm->p_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

#define MMS_WRITE_SUCCESS   0x81u
#define MMS_WRITE_FAILURE   0x80u
static int32 MmsWriteByName(char *name, MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm)
{
    char *lv_p_name;
    char *lv_p_name_temp;
    char lv_buffer[32u];
    int32 lv_length;
    int32 lv_ret;
    int16  lv_fc_index;
    Uint8 const *lv_p_frame;
    Uint8 const *lv_p_frame_temp;
    Uint8 const *lv_p_frame_end;
    MMS_LN  const *lv_p_ln_active;
    MMS_DOI const *lv_p_doi_active;
    MMS_SDI const *lv_p_sdi_active;
    Uint8 *lv_p_temp_buffer;
    Uint8 *lv_p_buffer_ptr;
    Uint8 *lv_p_buffer_tail;
    MMS_COMMUNICATION *lv_p_mms_comm;

    if ((NULL == p_mms_socket_data) || (NULL == p_mms_comm))
    {
        TRACE("function entries error!");
        MmsWriteError(p_mms_socket_data, lv_p_mms_comm, MMS_DATAACCESSERROR_INVALID_ADDRESS);
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;
    lv_p_frame = lv_p_mms_comm->p_frame;
    lv_p_frame_end = lv_p_mms_comm->p_frame_end;

    lv_p_name = name;
    lv_p_name_temp = strchr(lv_p_name, '$');
    if (NULL == lv_p_name_temp)
    {
        TRACE("Can't support LN write");
        MmsWriteError(p_mms_socket_data, lv_p_mms_comm, MMS_DATAACCESSERROR_OBJECT_ACCESS_UNSUPPORTED);
        return NORMAL_ERROR;
    }
    else
    {
        *lv_p_name_temp = '\0';
        lv_p_name_temp++;
    }

    for (lv_p_ln_active = p_mms_socket_data->p_ied->access_point->server->ldevice->ln_head; NULL != lv_p_ln_active; lv_p_ln_active = lv_p_ln_active->right)
    {
        snprintf(lv_buffer, sizeof(lv_buffer), "%s%s%s", lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
        if (0 == strcmp(lv_p_name, lv_buffer))
        {
            break;
        }
    }

    if (NULL == lv_p_ln_active)
    {
        TRACE("ln \"%s\" found failed!", lv_p_name);
        MmsWriteError(p_mms_socket_data, lv_p_mms_comm, MMS_DATAACCESSERROR_OBJECT_NON_EXISTENT);
        return NORMAL_ERROR;
    }

    lv_p_name = lv_p_name_temp;
    lv_p_name_temp = strchr(lv_p_name, '$');
    if (NULL == lv_p_name_temp)
    {
        TRACE("Can't support FC write");
        MmsWriteError(p_mms_socket_data, lv_p_mms_comm, MMS_DATAACCESSERROR_OBJECT_ACCESS_UNSUPPORTED);
        return NORMAL_ERROR;
    }
    else
    {
        *lv_p_name_temp = '\0';
        lv_p_name_temp++;
    }

    for (lv_fc_index = 1; lv_fc_index < mms_fc_num; lv_fc_index++)
    {
        if (0 == strcmp(lv_p_name, mms_fc[lv_fc_index].name))
        {
            break;
        }
    }

    if (lv_fc_index >= mms_fc_num)
    {
        TRACE("unknown mms_fc=%s", lv_buffer);
        MmsWriteError(p_mms_socket_data, lv_p_mms_comm, MMS_DATAACCESSERROR_OBJECT_NON_EXISTENT);
        return NORMAL_ERROR;
    }

    lv_p_name = lv_p_name_temp;
    lv_p_name_temp = strchr(lv_p_name, '$');
    if (NULL != lv_p_name_temp)
    {
        *lv_p_name_temp = '\0';
        lv_p_name_temp++;
    }

    for (lv_p_doi_active = lv_p_ln_active->doi; NULL != lv_p_doi_active; lv_p_doi_active = lv_p_doi_active->left)
    {
        if (0 == strcmp(lv_p_name, lv_p_doi_active->name))
        {
            break;
        }
    }

    if (NULL == lv_p_doi_active)
    {
        TRACE("doi \"%s\" found in ln \"%s%s%s\" failed!", lv_p_name, lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
        MmsWriteError(p_mms_socket_data, lv_p_mms_comm, MMS_DATAACCESSERROR_OBJECT_NON_EXISTENT);
        return NORMAL_ERROR;
    }

    lv_p_frame++;
    lv_length = Asn12Length(&lv_p_frame);
    if ((lv_length < 0) || ((lv_p_frame + lv_length) > lv_p_frame_end))
    {
        TRACE("write value parse error!");
        MmsWriteError(p_mms_socket_data, lv_p_mms_comm, MMS_DATAACCESSERROR_OBJECT_VALUE_INVALID);
        return NORMAL_ERROR;
    }
    lv_p_frame_temp = lv_p_frame;
    if (NULL == lv_p_name_temp)
    {
        lv_ret = MmsWriteDoi(lv_p_doi_active, lv_fc_index, &lv_p_frame_temp, lv_p_frame_end);
        if (NORMAL_SUCCESS != lv_ret)
        {
            MmsWriteError(p_mms_socket_data, lv_p_mms_comm, lv_ret - 1u);
            return NORMAL_ERROR;
        }
    }
    else
    {
        lv_p_sdi_active = lv_p_doi_active->sdi;
        while (NULL != lv_p_name_temp)
        {
            lv_p_name = lv_p_name_temp;
            lv_p_name_temp = strchr(lv_p_name, '$');
            if (NULL != lv_p_name_temp)
            {
                *lv_p_name_temp = '\0';
                lv_p_name_temp++;
            }

            for (; NULL != lv_p_sdi_active; lv_p_sdi_active = lv_p_sdi_active->left)
            {
                if (0 == strcmp(lv_p_name, lv_p_sdi_active->name))
                {
                    break;
                }
            }

            if (NULL == lv_p_sdi_active)
            {
                TRACE("sdi \"%s\" found in ln \"%s%s%s\" failed!", lv_p_name, lv_p_ln_active->prefix, lv_p_ln_active->lnclass, lv_p_ln_active->inst);
                MmsWriteError(p_mms_socket_data, lv_p_mms_comm, MMS_DATAACCESSERROR_OBJECT_NON_EXISTENT);
                return NORMAL_ERROR;
            }

            if ((NULL != lv_p_name_temp) && (NULL != lv_p_sdi_active->sdi))
            {
                lv_p_sdi_active = lv_p_sdi_active->sdi;
            }
        }

        if (MMS_BTYPE_SDO == lv_p_sdi_active->btype)
        {
            lv_ret = MmsWriteSdi(lv_p_sdi_active, lv_fc_index, &lv_p_frame_temp, lv_p_frame_end);
            if (NORMAL_SUCCESS != lv_ret)
            {
                MmsWriteError(p_mms_socket_data, lv_p_mms_comm, lv_ret - 1u);
                return NORMAL_ERROR;
            }
        }
        else
        {
            lv_ret = MmsWriteSdiDirect(lv_p_sdi_active, &lv_p_frame_temp, lv_p_frame_end);
            if (NORMAL_SUCCESS != lv_ret)
            {
                MmsWriteError(p_mms_socket_data, lv_p_mms_comm, lv_ret - 1u);
                return NORMAL_ERROR;
            }
        }
    }
    lv_p_mms_comm->p_frame = lv_p_frame + lv_length;

    lv_p_temp_buffer = malloc(MMS_TEMP_BUFFER_SIZE);
    if (NULL == lv_p_temp_buffer)
    {
        TRACE("MMS temp buffer malloc failed!");
        MmsWriteError(p_mms_socket_data, lv_p_mms_comm, MMS_DATAACCESSERROR_TEMPORARILY_UNAVAILABLE);
        return NORMAL_ERROR;
    }

    lv_p_buffer_tail = lv_p_temp_buffer + MMS_TEMP_BUFFER_SIZE;
    lv_p_buffer_ptr = lv_p_buffer_tail;

    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = MMS_WRITE_SUCCESS;

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_WRITE, lv_length);

    Uint322Asn1r(&lv_p_buffer_ptr, 0x02u, lv_p_mms_comm->invoke_id);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_CONFIRMED_RESPONSEPDU, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_DATA_VALUES, lv_length);

    *(--lv_p_buffer_ptr) = 0x03u;
    *(--lv_p_buffer_ptr) = 0x01u;
    *(--lv_p_buffer_ptr) = PRESENTATION_CONTEXT_IDENTIFIER;

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_PDV_LIST, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_USER_DATA_FULLY_ENCODED_DATA, lv_length);

    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_DATA_TRANSFER;
    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_GIVE_TOKENS;

    if (NORMAL_SUCCESS != TpckPacketSend(p_mms_socket_data, lv_p_mms_comm->cotp_client_tpdu_size ,lv_p_buffer_ptr, lv_p_buffer_tail - lv_p_buffer_ptr))
    {
        TRACE("MMS tpck packet send failed!");
        free(lv_p_temp_buffer);
        return NORMAL_ERROR;
    }

    free(lv_p_temp_buffer);

    return NORMAL_SUCCESS;
}

static int32 MmsWriteError(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, MMS_COMMUNICATION *p_mms_comm, int32 error_value)
{
    int32 lv_length;
    Uint8 *lv_p_temp_buffer;
    Uint8 *lv_p_buffer_ptr;
    Uint8 *lv_p_buffer_tail;
    MMS_COMMUNICATION *lv_p_mms_comm;

    if ((NULL == p_mms_socket_data) || (NULL == p_mms_comm))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_mms_comm = p_mms_comm;

    lv_p_temp_buffer = malloc(MMS_TEMP_BUFFER_SIZE);
    if (NULL == lv_p_temp_buffer)
    {
        TRACE("MMS temp buffer malloc failed!");
        return NORMAL_ERROR;
    }

    lv_p_buffer_tail = lv_p_temp_buffer + MMS_TEMP_BUFFER_SIZE;
    lv_p_buffer_ptr = lv_p_buffer_tail;

    Int322Asn1r(&lv_p_buffer_ptr, MMS_WRITE_FAILURE, error_value);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_WRITE, lv_length);

    Uint322Asn1r(&lv_p_buffer_ptr, 0x02u, lv_p_mms_comm->invoke_id);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, MMS_CONFIRMED_RESPONSEPDU, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_DATA_VALUES, lv_length);

    *(--lv_p_buffer_ptr) = 0x03u;
    *(--lv_p_buffer_ptr) = 0x01u;
    *(--lv_p_buffer_ptr) = PRESENTATION_CONTEXT_IDENTIFIER;

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_PDV_LIST, lv_length);

    lv_length = lv_p_buffer_tail - lv_p_buffer_ptr;
    Length2Asn1r(&lv_p_buffer_ptr, PRESENTATION_USER_DATA_FULLY_ENCODED_DATA, lv_length);

    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_DATA_TRANSFER;
    *(--lv_p_buffer_ptr) = 0x00u;
    *(--lv_p_buffer_ptr) = SPDU_TYPE_GIVE_TOKENS;

    if (NORMAL_SUCCESS != TpckPacketSend(p_mms_socket_data, lv_p_mms_comm->cotp_client_tpdu_size ,lv_p_buffer_ptr, lv_p_buffer_tail - lv_p_buffer_ptr))
    {
        TRACE("MMS tpck packet send failed!");
        free(lv_p_temp_buffer);
        return NORMAL_ERROR;
    }

    free(lv_p_temp_buffer);

    return NORMAL_SUCCESS;
}
static int32 MmsWriteDoi(MMS_DOI const *p_mms_doi, int16 fc_index, Uint8 const **pp_frame, Uint8 const *p_frame_end)
{
    Uint8 const *lv_p_frame;
    Uint8 const *lv_p_frame_temp;
    MMS_SDI const *lv_p_sdi;
    MMS_SDI const *lv_p_sdi_current;
    int32 lv_length;
    int32 lv_ret;

    if ((NULL == p_mms_doi) || (NULL == pp_frame) || (NULL == *pp_frame) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
    }

    lv_p_frame = *pp_frame;

    lv_p_frame++;
    lv_length = Asn12Length(&lv_p_frame);
    if ((lv_length < 0) || ((lv_p_frame + lv_length) > p_frame_end))
    {
        TRACE("doi value parse error!");
        return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
    }

    lv_p_frame_temp = lv_p_frame;
    lv_p_sdi_current = NULL;
    while (lv_p_sdi_current != p_mms_doi->sdi)
    {
        for (lv_p_sdi = p_mms_doi->sdi; lv_p_sdi_current != lv_p_sdi->left; lv_p_sdi = lv_p_sdi->left);
        lv_p_sdi_current = lv_p_sdi;
        lv_ret = MmsWriteSdi(lv_p_sdi, fc_index, &lv_p_frame_temp, p_frame_end);
        if (NORMAL_SUCCESS != lv_ret)
        {
            return lv_ret;
        }
    }

    lv_p_frame += lv_length;
    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

static int32 MmsWriteSdi(MMS_SDI const *p_mms_sdi, int16 fc_index, Uint8 const **pp_frame, Uint8 const *p_frame_end)
{
    Uint8 const *lv_p_frame;
    Uint8 const *lv_p_frame_temp;
    MMS_SDI const *lv_p_sdi_sdi;
    MMS_SDI const *lv_p_sdi_sdi_current;
    int32 lv_length;
    int32 lv_ret;

    if ((NULL == p_mms_sdi) || (NULL == pp_frame) || (NULL == *pp_frame) || (fc_index <= 0) || (fc_index >= mms_fc_num))
    {
        TRACE("function entries error!");
        return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
    }

    lv_p_frame = *pp_frame;
    if (MMS_BTYPE_SDO == p_mms_sdi->btype)
    {
        lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if ((lv_length < 0) || ((lv_p_frame + lv_length) > p_frame_end))
        {
            TRACE("sdo value parse error!");
            return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
        }

        lv_p_frame_temp = lv_p_frame;
        lv_p_sdi_sdi_current = NULL;
        while (lv_p_sdi_sdi_current != p_mms_sdi->sdi)
        {
            for (lv_p_sdi_sdi = p_mms_sdi->sdi; lv_p_sdi_sdi_current != lv_p_sdi_sdi->left; lv_p_sdi_sdi = lv_p_sdi_sdi->left);
            lv_p_sdi_sdi_current = lv_p_sdi_sdi;
            lv_ret = MmsWriteSdi(lv_p_sdi_sdi, fc_index, &lv_p_frame_temp, p_frame_end);
            if (NORMAL_SUCCESS != lv_ret)
            {
                return lv_ret;
            }
        }
        lv_p_frame += lv_length;
    }
    else if ((MMS_BTYPE_STRUCT == p_mms_sdi->btype))
    {
        if (fc_index == p_mms_sdi->fc_index)
        {
            lv_p_frame++;
            lv_length = Asn12Length(&lv_p_frame);
            if ((lv_length < 0) || ((lv_p_frame + lv_length) > p_frame_end))
            {
                TRACE("doi value parse error!");
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }

            lv_p_frame_temp = lv_p_frame;
            lv_p_sdi_sdi_current = NULL;
            while (lv_p_sdi_sdi_current != p_mms_sdi->sdi)
            {
                for (lv_p_sdi_sdi = p_mms_sdi->sdi; lv_p_sdi_sdi_current != lv_p_sdi_sdi->left; lv_p_sdi_sdi = lv_p_sdi_sdi->left);
                lv_p_sdi_sdi_current = lv_p_sdi_sdi;
                lv_ret = MmsWriteSdiDirect(lv_p_sdi_sdi, &lv_p_frame_temp, p_frame_end);
                if (NORMAL_SUCCESS != lv_ret)
                {
                    return lv_ret;
                }
            } 
            lv_p_frame += lv_length;
        }
    }
    else
    {
        if (fc_index == p_mms_sdi->fc_index)
        {
            lv_p_frame_temp = lv_p_frame;

            lv_p_frame++;
            lv_length = Asn12Length(&lv_p_frame);
            if ((lv_length < 0) || ((lv_p_frame + lv_length) > p_frame_end))
            {
                TRACE("frame write basic type length parse error!");
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }

            lv_ret = MmsBasicTypeWrite(p_mms_sdi, &lv_p_frame_temp);
            if (NORMAL_SUCCESS != lv_ret)
            {
                return lv_ret;
            }

            lv_p_frame += lv_length;
        }
    }

    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

static int32 MmsWriteSdiDirect(MMS_SDI const *p_mms_sdi, Uint8 const **pp_frame, Uint8 const *p_frame_end)
{
    int32 lv_ret;
    int32 lv_length;
    Uint8 const *lv_p_frame;
    Uint8 const *lv_p_frame_temp;
    MMS_SDI const *lv_p_sdi_sdi;
    MMS_SDI const *lv_p_sdi_sdi_current;

    if ((NULL == p_mms_sdi) || (NULL == pp_frame) || (NULL == *pp_frame))
    {
        TRACE("function entries error!");
        return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
    }

    lv_p_frame = *pp_frame;
    if ((MMS_BTYPE_STRUCT == p_mms_sdi->btype))
    {
        lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if ((lv_length < 0) || ((lv_p_frame + lv_length) > p_frame_end))
        {
            TRACE("doi value parse error!");
            return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
        }

        lv_p_frame_temp = lv_p_frame;
        lv_p_sdi_sdi_current = NULL;
        while (lv_p_sdi_sdi_current != p_mms_sdi->sdi)
        {
            for (lv_p_sdi_sdi = p_mms_sdi->sdi; lv_p_sdi_sdi_current != lv_p_sdi_sdi->left; lv_p_sdi_sdi = lv_p_sdi_sdi->left);
            lv_p_sdi_sdi_current = lv_p_sdi_sdi;
            lv_ret = MmsWriteSdiDirect(lv_p_sdi_sdi, &lv_p_frame_temp, p_frame_end);
            if (NORMAL_SUCCESS != lv_ret)
            {
                return lv_ret;
            }
        }
        lv_p_frame += lv_length;
    }
    else
    {
        lv_p_frame_temp = lv_p_frame;

        lv_p_frame++;
        lv_length = Asn12Length(&lv_p_frame);
        if ((lv_length < 0) || ((lv_p_frame + lv_length) > p_frame_end))
        {
            TRACE("frame write basic type length parse error!");
            return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
        }

        lv_ret = MmsBasicTypeWrite(p_mms_sdi, &lv_p_frame_temp);
        if (NORMAL_SUCCESS != lv_ret)
        {
            return lv_ret;
        }

        lv_p_frame += lv_length;
    }

    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

static int32 MmsBasicTypeDescriptionToBuffer(MMS_SDI const *p_mms_sdi, Uint8 **pp_buffer)
{
    Uint8 *lv_p_buffer;

    if ((NULL == p_mms_sdi) || (NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_buffer = *pp_buffer;
    switch (p_mms_sdi->btype)
    {
        case MMS_BTYPE_BOOLEAN :
        {
            *(--lv_p_buffer) = 0x00u;
            *(--lv_p_buffer) = 0x83u;
            break;
        }
        case MMS_BTYPE_DBPOS :
        {
            *(--lv_p_buffer) = 0x08u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x85u;
            break;
        }
        case MMS_BTYPE_INT8 :
        {
            *(--lv_p_buffer) = 0x08u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x85u;
            break;
        }
        case MMS_BTYPE_INT16 :
        {
            *(--lv_p_buffer) = 0x10u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x85u;
            break;
        }
        case MMS_BTYPE_INT32 :
        {
            *(--lv_p_buffer) = 0x20u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x85u;
            break;
        }
        case MMS_BTYPE_INT8U :
        {
            *(--lv_p_buffer) = 0x08u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x86u;
            break;
        }
        case MMS_BTYPE_INT16U :
        {
            *(--lv_p_buffer) = 0x10u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x86u;
            break;
        }
        case MMS_BTYPE_INT32U :
        {
            *(--lv_p_buffer) = 0x20u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x86u;
            break;
        }
        case MMS_BTYPE_FLOAT32 :
        {
            *(--lv_p_buffer) = 0x08u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x02u;
            *(--lv_p_buffer) = 0x20u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x02u;
            *(--lv_p_buffer) = 0x06u;
            *(--lv_p_buffer) = 0xA7u;
            break;
        }
        case MMS_BTYPE_TCMD :
        case MMS_BTYPE_ENUM :
        {
            *(--lv_p_buffer) = 0x08u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x85u;
            break;
        }
        case MMS_BTYPE_QUALITY :
        {
            *(--lv_p_buffer) = 0x0Du;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x84u;
            break;
        }
        case MMS_BTYPE_TIMESTAMP :
        {
            *(--lv_p_buffer) = 0x00u;
            *(--lv_p_buffer) = 0x91u;
            break;
        }
        case MMS_BTYPE_VISSTRING64 :
        {
            *(--lv_p_buffer) = 0xC0u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x8Au;
            break;
        }
        case MMS_BTYPE_VISSTRING255 :
        {
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0xFFu;
            *(--lv_p_buffer) = 0x02u;
            *(--lv_p_buffer) = 0x8Au;
            break;
        }
        case MMS_BTYPE_ENTRYTIME:
        case MMS_BTYPE_OCTET64 :
        {
            *(--lv_p_buffer) = 0xC0u;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x89u;
            break;
        }
//        case MMS_BTYPE_ENTRYTIME :
//        {
//            *(--lv_p_buffer) = 0x08u;
//            *(--lv_p_buffer) = 0x01u;
//            *(--lv_p_buffer) = 0x8Bu;
//            break;
//        }
        case MMS_BTYPE_UNICODE255 :
        {
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0xFFu;
            *(--lv_p_buffer) = 0x02u;
            *(--lv_p_buffer) = 0x90u;
            break;
        }
        case MMS_BTYPE_CHECK :
        {
            *(--lv_p_buffer) = 0xFEu;
            *(--lv_p_buffer) = 0x01u;
            *(--lv_p_buffer) = 0x84u;
            break;
        }
        default:
        {
            TRACE("unknow mms btype=0x%04X", p_mms_sdi->btype);
            return NORMAL_ERROR;
        }
    }

    *pp_buffer = lv_p_buffer;

    return NORMAL_SUCCESS;
}

static int32 MmsBasicTypeReadToBuffer(MMS_SDI const *p_mms_sdi, Uint8 **pp_buffer)
{
    Uint8 *lv_p_buffer;
    Uint8 lv_uint8_temp;
    int32 lv_int32_temp;
    Uint32 lv_uint32_temp;
    Uint64 lv_uint64_temp;
    float lv_float_temp;
    Uint16 lv_ushort_temp;
    char const *lv_p_string;
    Uint8 const *lv_p_byte;
    void *lv_addr;

    if ((NULL == p_mms_sdi) || (NULL == pp_buffer) || (NULL == *pp_buffer))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    if (NULL != p_mms_sdi->addr)
    {
        lv_addr = p_mms_sdi->addr;
    }
    else
    {
        lv_addr = p_mms_sdi->default_value_addr;
    }

    lv_p_buffer = *pp_buffer;
    switch (p_mms_sdi->btype)
    {
        case MMS_BTYPE_BOOLEAN :
        {
            lv_uint8_temp = 0;
            if (NULL != lv_addr)
            {
                if (0 != ((*(Uint8 const *)lv_addr) & 0x01u))
                {
                    lv_uint8_temp = 0xFFu;
                }
            }
            Bool2Asn1r(&lv_p_buffer, 0x83u, lv_uint8_temp);
            break;
        }
        case MMS_BTYPE_DBPOS :
        {
            lv_uint8_temp = 0;
            if (NULL != lv_addr)
            {
                lv_uint8_temp = *(Uint8 const *)lv_addr;
            }
            Dbpos2Asn1r(&lv_p_buffer, 0x83u, lv_uint8_temp);
            break;
        }
        case MMS_BTYPE_INT8 :
        {
            lv_int32_temp = 0;
            if (NULL != lv_addr)
            {
                lv_int32_temp = *(int8 const *)lv_addr;
            }
            Int322Asn1r(&lv_p_buffer, 0x85u, lv_int32_temp);
            break;
        }
        case MMS_BTYPE_INT16 :
        {
            lv_int32_temp = 0;
            if (NULL != lv_addr)
            {
                lv_int32_temp = *(int16 const *)lv_addr;
            }
            Int322Asn1r(&lv_p_buffer, 0x85u, lv_int32_temp);
            break;
        }
        case MMS_BTYPE_INT32 :
        {
            lv_int32_temp = 0;
            if (NULL != lv_addr)
            {
                lv_int32_temp = *(int32 const *)lv_addr;
            }
            Int322Asn1r(&lv_p_buffer, 0x85u, lv_int32_temp);
            break;
        }
        case MMS_BTYPE_INT8U :
        {
            lv_uint32_temp = 0;
            if (NULL != lv_addr)
            {
                lv_uint32_temp = *(Uint8 const *)lv_addr;
            }
            Uint322Asn1r(&lv_p_buffer, 0x86u, lv_uint32_temp);
            break;
        }
        case MMS_BTYPE_INT16U :
        {
            lv_uint32_temp = 0;
            if (NULL != lv_addr)
            {
                lv_uint32_temp = *(Uint16 const *)lv_addr;
            }
            Uint322Asn1r(&lv_p_buffer, 0x86u, lv_uint32_temp);
            break;
        }
        case MMS_BTYPE_INT32U :
        {
            lv_uint32_temp = 0;
            if (NULL != lv_addr)
            {
                lv_uint32_temp = *(Uint32 const *)lv_addr;
            }
            Uint322Asn1r(&lv_p_buffer, 0x86u, lv_uint32_temp);
            break;
        }
        case MMS_BTYPE_FLOAT32 :
        {
            lv_float_temp = 0;
            if (NULL != lv_addr)
            {
                lv_float_temp = *(float const *)lv_addr;
            }
            Float322Asn1r(&lv_p_buffer, 0x87u, lv_float_temp);
            break;
        }
        case MMS_BTYPE_TCMD :
        case MMS_BTYPE_ENUM :
        {
            lv_int32_temp = 0;
            if (NULL != lv_addr)
            {
                lv_int32_temp = *(int8 const *)lv_addr;
            }
            Int322Asn1r(&lv_p_buffer, 0x85u, lv_int32_temp);
            break;
        }
        case MMS_BTYPE_QUALITY :
        {
            lv_ushort_temp = 0;
            if (NULL != lv_addr)
            {
                lv_ushort_temp = *(Uint16 const *)lv_addr;
            }
            Quality2Asn1r(&lv_p_buffer, 0x84u, lv_ushort_temp);
            break;
        }
        case MMS_BTYPE_TIMESTAMP :
        {
            lv_uint64_temp = 0;
            if (NULL != lv_addr)
            {
                lv_uint64_temp = *(Uint64 const *)lv_addr;
            }
            UTCTime2Asn1r(&lv_p_buffer, 0x91u, lv_uint64_temp, 0, 0);
            break;
        }
        case MMS_BTYPE_VISSTRING64 :
        {
            lv_p_string = NULL;
            if (NULL != lv_addr)
            {
                lv_p_string = (char const *)lv_addr;
            }
            String2Asn1r(&lv_p_buffer, 64, 0x8Au, lv_p_string);            
            break;
        }
        case MMS_BTYPE_VISSTRING255 :
        {
            lv_p_string = NULL;
            if (NULL != lv_addr)
            {
                lv_p_string = (char const *)lv_addr;
            }
            String2Asn1r(&lv_p_buffer, 255, 0x8Au, lv_p_string);            
            break;
        }
        case MMS_BTYPE_ENTRYTIME:
        case MMS_BTYPE_OCTET64 :
        {
            lv_p_byte = NULL;
            if (NULL != lv_addr)
            {
                lv_p_byte = (Uint8 const *)lv_addr;
            }
            Bytes2Asn1r(&lv_p_buffer, 0x89u, lv_p_byte, 8u);            
            break;
        }
//        case MMS_BTYPE_ENTRYTIME :
//        {
//            *(--lv_p_buffer) = 0x08u;
//            *(--lv_p_buffer) = 0x01u;
//            *(--lv_p_buffer) = 0x8Bu;
//            break;
//        }
        case MMS_BTYPE_UNICODE255 :
        {
            lv_p_string = NULL;
            if (NULL != lv_addr)
            {
                lv_p_string = (char const *)lv_addr;
            }
            String2Asn1r(&lv_p_buffer, 255, 0x90u, lv_p_string);            
            break;
        }
        case MMS_BTYPE_CHECK :
        {
            lv_uint8_temp = 0;
            if (NULL != lv_addr)
            {
                lv_uint8_temp = *(Uint8 const *)lv_addr;
            }
            Check2Asn1r(&lv_p_buffer, 0x84u, lv_uint8_temp);
            break;
        }
        default:
        {
            TRACE("unknow mms btype=0x%04X", p_mms_sdi->btype);
            return NORMAL_ERROR;
        }
    }

    *pp_buffer = lv_p_buffer;
    return NORMAL_SUCCESS;
}

static int32 MmsBasicTypeWrite(MMS_SDI const *p_mms_sdi, Uint8 const **pp_frame)
{
    Uint8 const *lv_p_frame;
    Uint8 const *lv_p_frame_temp;
    int32 lv_length;
    void *lv_p_wr_addr;
    void const *lv_p_rd_addr;
    union
    {
        Uint8  uint8_val;
        int32  int32_val;
        Uint32 uint32_val;
        Uint64 uint64_val;
        float  float_val;
        Uint16 ushort_val;
        char   *p_string;
        Uint8  *p_bytes;
    } lv_write_temp, lv_read_temp;

    if ((NULL == p_mms_sdi) || (NULL == pp_frame) || (NULL == *pp_frame))
    {
        TRACE("function entries error!");
        return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
    }

    lv_p_wr_addr = p_mms_sdi->addr;
    lv_p_rd_addr = p_mms_sdi->addr;
    if (NULL == lv_p_rd_addr)
    {
        lv_p_rd_addr = p_mms_sdi->default_value_addr;
    }

    lv_p_frame = *pp_frame;

    lv_p_frame++;
    switch (p_mms_sdi->btype)
    {
        case MMS_BTYPE_BOOLEAN :
        {
            lv_write_temp.uint8_val = 0;
            if (NORMAL_SUCCESS != Asn12Bool(&lv_p_frame, &lv_write_temp.uint8_val))
            {
                TRACE("get BOOL value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                if (0 != lv_write_temp.uint8_val)
                {
                    (*(Uint8 *)lv_p_wr_addr) |= 0x01u;
                }
                else
                {
                    (*(Uint8 *)lv_p_wr_addr) &= ~0x01u;
                }
            }
            else
            {
                lv_read_temp.uint8_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.uint8_val = (*(Uint8 const *)lv_p_rd_addr) & 0x01u;
                }

                if (lv_read_temp.uint8_val != lv_write_temp.uint8_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_DBPOS :
        {
            lv_write_temp.uint8_val = 0;
            if (NORMAL_SUCCESS != Asn12Dbpos(&lv_p_frame, &lv_write_temp.uint8_val))
            {
                TRACE("get DBPOS value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                (*(Uint8 *)lv_p_wr_addr) = lv_write_temp.uint8_val;
            }
            else
            {
                lv_read_temp.uint8_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.uint8_val = *(Uint8 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.uint8_val != lv_write_temp.uint8_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_INT8 :
        {
            lv_write_temp.int32_val = 0;
            if (NORMAL_SUCCESS != Asn12Int32(&lv_p_frame, &lv_write_temp.int32_val))
            {
                TRACE("get INT8 value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(int8 *)lv_p_wr_addr = lv_write_temp.int32_val;
            }
            else
            {
                lv_read_temp.int32_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.int32_val = *(int8 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.int32_val != lv_write_temp.int32_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_INT16 :
        {
            lv_write_temp.int32_val = 0;
            if (NORMAL_SUCCESS != Asn12Int32(&lv_p_frame, &lv_write_temp.int32_val))
            {
                TRACE("get INT16 value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(int16 *)lv_p_wr_addr = lv_write_temp.int32_val;
            }
            else
            {
                lv_read_temp.int32_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.int32_val = *(int16 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.int32_val != lv_write_temp.int32_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_INT32 :
        {
            lv_write_temp.int32_val = 0;
            if (NORMAL_SUCCESS != Asn12Int32(&lv_p_frame, &lv_write_temp.int32_val))
            {
                TRACE("get INT32 value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(int32 *)lv_p_wr_addr = lv_write_temp.int32_val;
            }
            else
            {
                lv_read_temp.int32_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.int32_val = *(int32 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.int32_val != lv_write_temp.int32_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_INT8U :
        {
            lv_write_temp.uint32_val = 0;
            if (NORMAL_SUCCESS != Asn12Uint32(&lv_p_frame, &lv_write_temp.uint32_val))
            {
                TRACE("get INT8U value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(Uint8 *)lv_p_wr_addr = lv_write_temp.uint32_val;
            }
            else
            {
                lv_read_temp.uint32_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.uint32_val = *(Uint8 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.uint32_val != lv_write_temp.uint32_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_INT16U :
        {
            lv_write_temp.uint32_val = 0;
            if (NORMAL_SUCCESS != Asn12Uint32(&lv_p_frame, &lv_write_temp.uint32_val))
            {
                TRACE("get INT16U value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(Uint16 *)lv_p_wr_addr = lv_write_temp.uint32_val;
            }
            else
            {
                lv_read_temp.uint32_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.uint32_val = *(Uint16 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.uint32_val != lv_write_temp.uint32_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_INT32U :
        {
            lv_write_temp.uint32_val = 0;
            if (NORMAL_SUCCESS != Asn12Uint32(&lv_p_frame, &lv_write_temp.uint32_val))
            {
                TRACE("get INT32U value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(Uint32 *)lv_p_wr_addr = lv_write_temp.uint32_val;
            }
            else
            {
                lv_read_temp.uint32_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.uint32_val = *(Uint32 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.uint32_val != lv_write_temp.uint32_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_FLOAT32 :
        {
            lv_write_temp.float_val = 0;
            if (NORMAL_SUCCESS != Asn12Float32(&lv_p_frame, &lv_write_temp.float_val))
            {
                TRACE("get FLOAT32 value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(float *)lv_p_wr_addr = lv_write_temp.float_val;
            }
            else
            {
                lv_read_temp.float_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.float_val = *(float const *)lv_p_rd_addr;
                }

                if (lv_read_temp.float_val != lv_write_temp.float_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_TCMD :
        case MMS_BTYPE_ENUM :
        {
            lv_write_temp.int32_val = 0;
            if (NORMAL_SUCCESS != Asn12Int32(&lv_p_frame, &lv_write_temp.int32_val))
            {
                TRACE("get ENUM value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(int8 *)lv_p_wr_addr = lv_write_temp.int32_val;
            }
            else
            {
                lv_read_temp.int32_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.int32_val = *(int8 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.int32_val != lv_write_temp.int32_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_QUALITY :
        {
            lv_write_temp.ushort_val = 0;
            if (NORMAL_SUCCESS != Asn12Quality(&lv_p_frame, &lv_write_temp.ushort_val))
            {
                TRACE("get QUALITY value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(Uint16 *)lv_p_wr_addr = lv_write_temp.ushort_val;
            }
            else
            {
                lv_read_temp.ushort_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.ushort_val = *(Uint16 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.ushort_val != lv_write_temp.ushort_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_TIMESTAMP :
        {
            lv_write_temp.uint64_val = 0;
            if (NORMAL_SUCCESS != Asn12UTCTime(&lv_p_frame, &lv_write_temp.uint64_val, 0))
            {
                TRACE("get TIMESTAMP value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                *(Uint64 *)lv_p_wr_addr = lv_write_temp.uint64_val;
            }
            else
            {
                lv_read_temp.uint64_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.uint64_val = *(Uint64 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.uint64_val != lv_write_temp.uint64_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        case MMS_BTYPE_VISSTRING64 :
        {
            if ((NULL != lv_write_temp.p_string) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                lv_write_temp.p_string = (char *)lv_p_wr_addr;            
                if (NORMAL_SUCCESS != Asn12String(&lv_p_frame, lv_write_temp.p_string, 64))
                {
                    TRACE("get VISSTRING64 value failed!");
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
                }
            }
            else
            {
                lv_p_frame_temp = lv_p_frame;
                if (NORMAL_SUCCESS != Asn12String(&lv_p_frame, lv_write_temp.p_string, 64))
                {
                    TRACE("get VISSTRING64 value failed!");
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
                }
                if (NULL != lv_p_rd_addr)
                {
                    lv_length = strlen((char const *)lv_p_rd_addr);
                    if (lv_length > 64)
                    {
                        lv_length = 64;
                    }
                    if (0 != (memcmp(lv_p_rd_addr, &lv_p_frame_temp[2], lv_length)))
                    {
                        *pp_frame = lv_p_frame;
                        return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                    }
                }
                else
                {
                    if (0u != lv_p_frame_temp[1])
                    {
                        *pp_frame = lv_p_frame;
                        return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                    }
                }

            }
            break;
        }
        case MMS_BTYPE_VISSTRING255 :
        {
            if ((NULL != lv_write_temp.p_string) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                lv_write_temp.p_string = (char *)lv_p_wr_addr;            
                if (NORMAL_SUCCESS != Asn12String(&lv_p_frame, lv_write_temp.p_string, 255))
                {
                    TRACE("get VISSTRING255 value failed!");
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
                }
            }
            else
            {
                lv_p_frame_temp = lv_p_frame;
                if (NORMAL_SUCCESS != Asn12String(&lv_p_frame, lv_write_temp.p_string, 255))
                {
                    TRACE("get VISSTRING255 value failed!");
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
                }
                if (NULL != lv_p_rd_addr)
                {
                    lv_length = strlen((char const *)lv_p_rd_addr);
                    if (lv_length > 255)
                    {
                        lv_length = 255;
                    }
                    if (0 != (memcmp(lv_p_rd_addr, &lv_p_frame_temp[2], lv_length)))
                    {
                        *pp_frame = lv_p_frame;
                        return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                    }
                }
                else
                {
                    if (0u != lv_p_frame_temp[1])
                    {
                        *pp_frame = lv_p_frame;
                        return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                    }
                }
            }
            break;
        }
        case MMS_BTYPE_ENTRYTIME:
        case MMS_BTYPE_OCTET64 :
        {
            if ((NULL != lv_write_temp.p_bytes) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                lv_write_temp.p_bytes = (Uint8 *)lv_p_wr_addr;            
                if (NORMAL_SUCCESS != Asn12Bytes(&lv_p_frame, lv_write_temp.p_bytes, 8u))
                {
                    TRACE("get VISSTRING64 value failed!");
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
                }
            }
            else
            {
                lv_p_frame_temp = lv_p_frame;
                if (NORMAL_SUCCESS != Asn12Bytes(&lv_p_frame, lv_write_temp.p_bytes, 8u))
                {
                    TRACE("get VISSTRING64 value failed!");
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
                }
                if (NULL != lv_p_rd_addr)
                {
                    if (0 != (memcmp(lv_p_rd_addr, &lv_p_frame_temp[2], 8u)))
                    {
                        *pp_frame = lv_p_frame;
                        return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                    }
                }
                else
                {
                    if (0u != lv_p_frame_temp[1])
                    {
                        *pp_frame = lv_p_frame;
                        return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                    }
                }
            }
            break;
        }
//        case MMS_BTYPE_ENTRYTIME :
//        {
//            *(--lv_p_buffer) = 0x08u;
//            *(--lv_p_buffer) = 0x01u;
//            *(--lv_p_buffer) = 0x8Bu;
//            break;
//        }
        case MMS_BTYPE_UNICODE255 :
        {
            if ((NULL != lv_write_temp.p_string) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                lv_write_temp.p_string = (char *)lv_p_wr_addr;            
                if (NORMAL_SUCCESS != Asn12String(&lv_p_frame, lv_write_temp.p_string, 255))
                {
                    TRACE("get UNICODE255 value failed!");
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
                }
            }
            else
            {
                lv_p_frame_temp = lv_p_frame;
                if (NORMAL_SUCCESS != Asn12String(&lv_p_frame, lv_write_temp.p_string, 255))
                {
                    TRACE("get UNICODE255 value failed!");
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
                }
                if (NULL != lv_p_rd_addr)
                {
                    lv_length = strlen((char const *)lv_p_rd_addr);
                    if (lv_length > 255)
                    {
                        lv_length = 255;
                    }
                    if (0 != (memcmp(lv_p_rd_addr, &lv_p_frame_temp[2], lv_length)))
                    {
                        *pp_frame = lv_p_frame;
                        return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                    }
                }
                else
                {
                    if (0u != lv_p_frame_temp[1])
                    {
                        *pp_frame = lv_p_frame;
                        return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                    }
                }
            }
            break;
        }
        case MMS_BTYPE_CHECK :
        {
            lv_write_temp.uint8_val = 0;
            if (NORMAL_SUCCESS != Asn12Check(&lv_p_frame, &lv_write_temp.uint8_val))
            {
                TRACE("get DBPOS value failed!");
                *pp_frame = lv_p_frame;
                return MMS_DATAACCESSERROR_INVALID_ADDRESS + 1u;
            }
            if ((NULL != lv_p_wr_addr) && (0 != (p_mms_sdi->property & 0x01u)))
            {
                (*(Uint8 *)lv_p_wr_addr) = lv_write_temp.uint8_val;
            }
            else
            {
                lv_read_temp.uint8_val = 0;
                if (NULL != lv_p_rd_addr)
                {
                    lv_read_temp.uint8_val = *(Uint8 const *)lv_p_rd_addr;
                }

                if (lv_read_temp.uint8_val != lv_write_temp.uint8_val)
                {
                    *pp_frame = lv_p_frame;
                    return MMS_DATAACCESSERROR_OBJECT_ACCESS_DENIED + 1u;
                }
            }
            break;
        }
        default:
        {
            TRACE("unknow mms btype=0x%04X", p_mms_sdi->btype);
            return MMS_DATAACCESSERROR_TYPE_UNSUPPORTED + 1u;
        }
    }

    *pp_frame = lv_p_frame;
    return NORMAL_SUCCESS;
}

static int32 TpckPacketSend(MMS_SOCKET_THREAD_DATA const *p_mms_socket_data, int32 tpdu_size, Uint8 const *buffer, int32 buffer_length)
{
    int32 lv_length;
    int32 lv_send_length;
    int32 lv_send_length_current;
    int32 lv_retry_times;
    int32 lv_mms_send_length_current;
    int32 lv_mms_max_send_size;
    Uint8 *lv_p_send_buffer;
    Uint8 const *lv_p_send_buffer_temp;
    Uint8 const *lv_p_buffer;
    char lv_addr_buffer[16u];

    if ((NULL == p_mms_socket_data) || (NULL == buffer))
    {
        TRACE("function entries error!");
        return NORMAL_ERROR;
    }

    lv_p_send_buffer = malloc(tpdu_size + 4u);
    if (NULL == lv_p_send_buffer)
    {
        TRACE("MMS send buffer malloc failed!");
        return NORMAL_ERROR;
    }

    lv_length = buffer_length;
    lv_p_buffer = buffer;
    lv_mms_max_send_size = tpdu_size - 3u;

    lv_p_send_buffer[0]  = TP_VERSION;
    lv_p_send_buffer[1]  = 0x00u;
    lv_p_send_buffer[4]  = 0x02u;
    lv_p_send_buffer[5]  = TP_PDU_TYPE_DT;
    while (lv_length > 0)
    {
        if (lv_length > lv_mms_max_send_size)
        {
            lv_mms_send_length_current = lv_mms_max_send_size;
            lv_p_send_buffer[6] = 0x00u;
        }
        else
        {
            lv_mms_send_length_current = lv_length;
            lv_p_send_buffer[6] = 0x80u;
        }

        lv_send_length = lv_mms_send_length_current + 7u;
        lv_p_send_buffer[2]  = LHSB(lv_send_length);
        lv_p_send_buffer[3]  = LLSB(lv_send_length);

        memcpy(&lv_p_send_buffer[7u], lv_p_buffer, lv_mms_send_length_current);

        lv_retry_times = 0;
        lv_p_send_buffer_temp = lv_p_send_buffer;
        while (lv_send_length > 0)
        {
            lv_send_length_current = write(p_mms_socket_data->fd, lv_p_send_buffer_temp, lv_send_length);
            if (lv_send_length_current < 0)
            {
                lv_retry_times++;
                inet_ntop(AF_INET, &p_mms_socket_data->client_ip, lv_addr_buffer, sizeof(lv_addr_buffer));
                printf("client %s send retry %ld times!\n", lv_addr_buffer, lv_retry_times);
                if (lv_retry_times >= 3)
                {
                    free(lv_p_send_buffer);
                    return NORMAL_ERROR;
                }
                usleep(5000);
            }
            else if (0 == lv_send_length_current)
            {
                inet_ntop(AF_INET, &p_mms_socket_data->client_ip, lv_addr_buffer, sizeof(lv_addr_buffer));
                printf("client %s has closed!\n", lv_addr_buffer);
                free(lv_p_send_buffer);
                return NORMAL_ERROR;
            }
            else
            {
                lv_p_send_buffer_temp += lv_send_length_current;
                lv_send_length -= lv_send_length_current;
            }
        } 
        
        lv_p_buffer += lv_mms_send_length_current;
        lv_length -= lv_mms_send_length_current;
    }
    free(lv_p_send_buffer);

    return NORMAL_SUCCESS;
}

