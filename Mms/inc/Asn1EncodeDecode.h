/*****************************************************************************************************
* FileName:                    Asn1EncodeDecode.h
*
* Description:                 Asn1编解码
*
* Author:                      YanDengxue
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2010-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
#ifndef _Asn1_Encode_Decode_H_
#define _Asn1_Encode_Decode_H_

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================================================
// 外部函数声明
//====================================================================================================
extern int32 Length2Asn1r(Uint8 **pp_buffer, Uint8 tag, int32 length);
extern int32 Asn12Length(Uint8 const **pp_buffer);
extern int32 Bool2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint8 src);
extern int32 Asn12Bool(Uint8 const **pp_buffer, Uint8 *p_dst);
extern int32 Dbpos2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint8 src);
extern int32 Asn12Dbpos(Uint8 const **pp_buffer, Uint8 *p_dst);
extern int32 Check2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint8 src);
extern int32 Asn12Check(Uint8 const **pp_buffer, Uint8 *p_dst);
extern int32 Quality2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint16 src);
extern int32 Asn12Quality(Uint8 const **pp_buffer, Uint16 *p_dst);
extern int32 Bytes2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint8 const *src, int32 byte_length);
extern int32 Asn12Bytes(Uint8 const **pp_buffer, Uint8 *buffer, int32 buffer_length);
extern int32 String2Asn1r(Uint8 **pp_buffer, int32 string_max_length, Uint8 tag, char const *src);
extern int32 Asn12String(Uint8 const **pp_buffer, char *buffer, int32 buffer_length);
extern int32 Uint322Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint32 src);
extern int32 Asn12Uint32(Uint8 const **pp_buffer, Uint32 *p_dst);
extern int32 Int322Asn1r(Uint8 **pp_buffer, Uint8 tag, int32 src);
extern int32 Asn12Int32(Uint8 const **pp_buffer, int32 *p_dst);
extern int32 Float322Asn1r(Uint8 **pp_buffer, Uint8 tag, float src);
extern int32 Asn12Float32(Uint8 const **pp_buffer, float *p_dst);
extern int32 UTCTime2Asn1r(Uint8 **pp_buffer, Uint8 tag, Uint64 usecond, Uint32 time_zone, Uint8 time_flag);
extern int32 Asn12UTCTime(Uint8 const **pp_buffer, Uint64 *p_dst, Uint32 time_zone);

#ifdef __cplusplus
}
#endif

#endif // End _DEVICE_H_

