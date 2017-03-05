/*****************************************************************************************************
* FileName:                    MmsBaseData.h
*
* Description:                 MMS相关数据定义
*
* Author:                      YanDengxue, Fiberhome-Fuhua
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-10-10 11:00       --           1.00             Create
*****************************************************************************************************/
#ifndef _Mms_Base_Data_H_
#define _Mms_Base_Data_H_

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================================================
// enum定义
//====================================================================================================
typedef enum
{
    MMS_BTYPE_BOOLEAN      = 0x0001,
    MMS_BTYPE_INT8         = 0x0002,
    MMS_BTYPE_INT16        = 0x0003,
    MMS_BTYPE_INT32        = 0x0005,
    MMS_BTYPE_INT8U        = 0x0006,
    MMS_BTYPE_INT16U       = 0x0007,
    MMS_BTYPE_INT32U       = 0x0008,
    MMS_BTYPE_FLOAT32      = 0x0009,
    MMS_BTYPE_FLOAT64      = 0x000A,
    MMS_BTYPE_ENUM         = 0x000B,
    MMS_BTYPE_DBPOS        = 0x000C,
    MMS_BTYPE_TCMD         = 0x000D,
    MMS_BTYPE_QUALITY      = 0x000E,
    MMS_BTYPE_TIMESTAMP    = 0x000F,
    MMS_BTYPE_VISSTRING64  = 0x0010,
    MMS_BTYPE_VISSTRING255 = 0x0011,
    MMS_BTYPE_OCTET64      = 0x0012,
    MMS_BTYPE_STRUCT       = 0x0013,
    MMS_BTYPE_ENTRYTIME    = 0x0014,
    MMS_BTYPE_UNICODE255   = 0x0015,
    MMS_BTYPE_CHECK        = 0x0016,
    MMS_BTYPE_SDO          = 0x0080,
} MMS_BTYPE;

typedef enum
{
    MMS_USER_BTYPE_BOOLEAN = 0x0001,
    MMS_USER_BTYPE_INT8    = 0x0002,
    MMS_USER_BTYPE_INT16   = 0x0003,
    MMS_USER_BTYPE_INT32   = 0x0004,
    MMS_USER_BTYPE_INT8U   = 0x0005,
    MMS_USER_BTYPE_INT16U  = 0x0006,
    MMS_USER_BTYPE_INT32U  = 0x0007,
    MMS_USER_BTYPE_FLOAT32 = 0x0008,
    MMS_USER_BTYPE_FLOAT64 = 0x0009,
    MMS_USER_BTYPE_STRING  = 0x000A,
} MMS_USER_BTYPE;

// FUNCTIONAL_CONSTRAINT
typedef enum
{
    MMS_FC_ST    = 0x0001,
    MMS_FC_MX    = 0x0002,
    MMS_FC_CO    = 0x0003,
    MMS_FC_SP    = 0x0005,
    MMS_FC_SV    = 0x0006,
    MMS_FC_CF    = 0x0007,
    MMS_FC_DC    = 0x0008,
    MMS_FC_SG    = 0x0009,
    MMS_FC_SE    = 0x000A,
    MMS_FC_EX    = 0x000B,
    MMS_FC_BR    = 0x000C,
    MMS_FC_RP    = 0x000D,
    MMS_FC_LG    = 0x000E,
    MMS_FC_GO    = 0x000F,
    MMS_FC_GS    = 0x0010,
    MMS_FC_MS    = 0x0011,
    MMS_FC_US    = 0x0012,
} MMS_FC;

//====================================================================================================
// 结构声明
//====================================================================================================
typedef struct
{
    char const *const name;
    MMS_FC  fc;
} MMS_FC_DEFINE;

struct MMS_DATYPE;
typedef struct MMS_BDA
{
    char const *name;
    MMS_BTYPE btype;
    struct MMS_DATYPE *down;
    struct MMS_BDA *right;
} MMS_BDA;

struct MMS_DOTYPE;
typedef struct MMS_DATYPE
{
    char const *id;
    struct MMS_BDA *bda;
    struct MMS_DATYPE *right;
} MMS_DATYPE;

typedef struct MMS_DOTYPE_ELEMENT
{
    char const *name;
    MMS_BTYPE btype;
    int16 fc_index;
    union
    {
        struct MMS_DATYPE *da_inst;
        struct MMS_DOTYPE *sdo;
    };
    struct MMS_DOTYPE_ELEMENT *right;
} MMS_DOTYPE_ELEMENT;

typedef struct MMS_DOTYPE
{
    char const *id;
    struct MMS_DOTYPE_ELEMENT *element;
    struct MMS_DOTYPE *right;
} MMS_DOTYPE;

typedef struct MMS_LNODETYPE_ELEMENT
{
    char const *name;
    struct MMS_DOTYPE *do_inst;
    struct MMS_LNODETYPE_ELEMENT *right;
} MMS_LNODETYPE_ELEMENT;

typedef struct MMS_LNODETYPE
{
    char const *id;
    char const *lnclass;
    struct MMS_LNODETYPE_ELEMENT *element;
    struct MMS_LNODETYPE *right;
} MMS_LNODETYPE;

typedef struct MMS_SDI
{
    char const *name;
    char const *desc;
//    char const *saddr;
    int16      fc_index;
    MMS_BTYPE  btype;
    MMS_USER_BTYPE user_btype;
    union
    {
        void *addr;
        struct MMS_SDI *sdi;
    };
    union
    {
        void *default_value_addr;
        Uint8 property;
    };    
    struct MMS_SDI *left;
} MMS_SDI;

typedef struct MMS_DOI
{
    char const *name;
    char const *desc;
    struct MMS_SDI *sdi;
    struct MMS_DOI *left;
} MMS_DOI;

typedef struct MMS_LN
{
    char const *prefix;
    char const *lnclass;
    char const *inst;
    char const *lntype;
    char const *desc;
    struct MMS_DOI *doi;
    struct MMS_LN *left;
    struct MMS_LN *right;
} MMS_LN;

struct MMS_DATASET;
typedef struct MMS_LDEVICE
{
    char const *inst;
    struct MMS_LN *ln_head;
    struct MMS_LN *ln_tail;
    struct MMS_LDEVICE *left;
    struct MMS_DATASET *dataset;
} MMS_LDEVICE;

typedef struct MMS_SERVER
{
    struct MMS_LDEVICE *ldevice;
    struct MMS_SERVER *left;
} MMS_SERVER;

typedef struct MMS_FCD_SDI
{
    struct MMS_SDI *sdi;
    struct MMS_FCD_SDI *down;
} MMS_FCD_SDI;

typedef struct MMS_FCD
{
    int16  fc_index;
    struct MMS_LDEVICE *ldevice;
    struct MMS_LN  *ln;
    struct MMS_DOI *doi;
    struct MMS_FCD_SDI *fcd_sdi;
    struct MMS_FCD *left;
} MMS_FCD;

typedef struct MMS_DATASET
{
    char const *name;
    char const *desc;
    struct MMS_FCD *fcd;
    struct MMS_DATASET *left;
} MMS_DATASET;

typedef struct MMS_ACCESS_POINT
{
    char const *name;
    char const *desc;
    struct MMS_SERVER *server;
    struct MMS_ACCESS_POINT *left;
} MMS_ACCESS_POINT;

typedef struct MMS_IED
{
    char const *name;
    char const *desc;
    char const *type;
    char const *manufacturer;
    char const *config_version;
    struct MMS_ACCESS_POINT *access_point;
} MMS_IED;

#ifdef __cplusplus
}
#endif

#endif


