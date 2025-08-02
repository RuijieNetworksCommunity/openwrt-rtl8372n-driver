#ifndef __RTL8372N_SWITCH_H__
#define __RTL8372N_SWITCH_H__

#include "rtl8372n_types.h"

#define UNDEFINE_PHY_PORT   (0xFF)
#define RTK_SWITCH_PORT_NUM (10) //why this is 10?

#define MAXPKTLEN_CFG_ID_MAX (1)

#define RTK_SWITCH_MAX_PKTLEN (0x3FFF)

/* UTIL MACRO */
#define RTK_CHK_INIT_STATE()                                \
    do                                                      \
    {                                                       \
        if(rtk_switch_initialState_get() != INIT_COMPLETED) \
        {                                                   \
            return RT_ERR_NOT_INIT;                         \
        }                                                   \
    }while(0)

#define RTK_CHK_PORT_VALID(__port__)                            \
    do                                                          \
    {                                                           \
        if(rtk_switch_logicalPortCheck(__port__) != RT_ERR_OK)  \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORT_IS_UTP(__port__)                           \
    do                                                          \
    {                                                           \
        if(rtk_switch_isUtpPort(__port__) != RT_ERR_OK)         \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORT_IS_EXT(__port__)                           \
    do                                                          \
    {                                                           \
        if(rtk_switch_isExtPort(__port__) != RT_ERR_OK)         \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORT_IS_COMBO(__port__)                         \
    do                                                          \
    {                                                           \
        if(rtk_switch_isComboPort(__port__) != RT_ERR_OK)       \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORT_IS_PTP(__port__)                           \
    do                                                          \
    {                                                           \
        if(rtk_switch_isPtpPort(__port__) != RT_ERR_OK)         \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORTMASK_VALID(__portmask__)                        \
    do                                                              \
    {                                                               \
        if(rtk_switch_isPortMaskValid(__portmask__) != RT_ERR_OK)   \
        {                                                           \
            return RT_ERR_PORT_MASK;                                \
        }                                                           \
    }while(0)

#define RTK_CHK_PORTMASK_VALID_ONLY_UTP(__portmask__)               \
    do                                                              \
    {                                                               \
        if(rtk_switch_isPortMaskUtp(__portmask__) != RT_ERR_OK)     \
        {                                                           \
            return RT_ERR_PORT_MASK;                                \
        }                                                           \
    }while(0)

#define RTK_CHK_PORTMASK_VALID_ONLY_EXT(__portmask__)               \
    do                                                              \
    {                                                               \
        if(rtk_switch_isPortMaskExt(__portmask__) != RT_ERR_OK)     \
        {                                                           \
            return RT_ERR_PORT_MASK;                                \
        }                                                           \
    }while(0)

#define RTK_CHK_TRUNK_GROUP_VALID(__grpId__)                        \
    do                                                              \
    {                                                               \
        if(rtk_switch_isValidTrunkGrpId(__grpId__) != RT_ERR_OK)    \
        {                                                           \
            return RT_ERR_LA_TRUNK_ID;                              \
        }                                                           \
    }while(0)

#define RTK_PORTMASK_IS_PORT_SET(__portmask__, __port__)    (((__portmask__).bits[0] & (0x00000001 << __port__)) ? 1 : 0)
#define RTK_PORTMASK_IS_EMPTY(__portmask__)                 (((__portmask__).bits[0] == 0) ? 1 : 0)
#define RTK_PORTMASK_CLEAR(__portmask__)                    ((__portmask__).bits[0] = 0)
#define RTK_PORTMASK_PORT_SET(__portmask__, __port__)       ((__portmask__).bits[0] |= (0x00000001 << __port__))
#define RTK_PORTMASK_PORT_CLEAR(__portmask__, __port__)     ((__portmask__).bits[0] &= ~(0x00000001 << __port__))
#define RTK_PORTMASK_ALLPORT_SET(__portmask__)              (rtk_switch_logPortMask_get(&__portmask__))
#define RTK_PORTMASK_SCAN(__portmask__, __port__)           for(__port__ = 0; __port__ < RTK_SWITCH_PORT_NUM; __port__++)  if(RTK_PORTMASK_IS_PORT_SET(__portmask__, __port__))
#define RTK_PORTMASK_COMPARE(__portmask_A__, __portmask_B__)    ((__portmask_A__).bits[0] - (__portmask_B__).bits[0])

#define RTK_SCAN_ALL_PHY_PORTMASK(__port__)                 for(__port__ = 0; __port__ < RTK_SWITCH_PORT_NUM; __port__++)  if( (rtk_switch_phyPortMask_get() & (0x00000001 << __port__)))
#define RTK_SCAN_ALL_LOG_PORT(__port__)                     for(__port__ = 0; __port__ < RTK_SWITCH_PORT_NUM; __port__++)  if( rtk_switch_logicalPortCheck(__port__) == RT_ERR_OK)
#define RTK_SCAN_ALL_LOG_PORTMASK(__portmask__)             for((__portmask__).bits[0] = 0; (__portmask__).bits[0] < 0x7FFFF; (__portmask__).bits[0]++)  if( rtk_switch_isPortMaskValid(&__portmask__) == RT_ERR_OK)

/* Port mask definition */
#define RTK_PHY_PORTMASK_ALL                                (rtk_switch_phyPortMask_get())

/* Port definition*/
#define RTK_MAX_LOGICAL_PORT_ID                             (rtk_switch_maxLogicalPort_get())

typedef enum switch_chip_e
{
    CHIP_RTL8373 = 0,
    CHIP_RTL8372,//1
    CHIP_RTL8224,//2
    CHIP_RTLxxxx,//3
    CHIP_RTL8373N,//4
    CHIP_RTL8372N,//5
    CHIP_RTL8224N,//6
    CHIP_RTL8366,// ? 7
    CHIP_END
}switch_chip_t;

typedef enum init_state_e
{
    INIT_NOT_COMPLETED = 0,
    INIT_COMPLETED,
    INIT_STATE_END
} init_state_t;

typedef enum port_type_e
{
    UTP_PORT = 0,
    EXT_PORT,
    UNKNOWN_PORT = 0xFF,
    PORT_TYPE_END
}port_type_t;

typedef struct rtk_switch_halCtrl_s
{
    switch_chip_t   switch_type;
    rtk_uint32      l2p_port[RTK_SWITCH_PORT_NUM];
    rtk_uint32      p2l_port[RTK_SWITCH_PORT_NUM];
    port_type_t     log_port_type[RTK_SWITCH_PORT_NUM];
    rtk_uint32      ptp_port[RTK_SWITCH_PORT_NUM];
    rtk_uint32      valid_portmask;
    rtk_uint32      valid_utp_portmask;
    rtk_uint32      valid_ext_portmask;
    rtk_uint32      valid_cpu_portmask;
    rtk_uint32      min_phy_port;
    rtk_uint32      max_phy_port;
    rtk_uint32      phy_portmask;
    rtk_uint32      combo_logical_port;
    rtk_uint32      hsg_logical_port;
    rtk_uint32      sg_logical_portmask;
    rtk_uint32      max_meter_id;
    rtk_uint32      max_lut_addr_num;
    rtk_uint32      trunk_group_mask;

}rtk_switch_halCtrl_t;

typedef enum rtk_vlan_acceptFrameType_e
{
    ACCEPT_FRAME_TYPE_ALL = 0,             /* untagged, priority-tagged and tagged */
    ACCEPT_FRAME_TYPE_TAG_ONLY,         /* tagged */
    ACCEPT_FRAME_TYPE_UNTAG_ONLY,     /* untagged and priority-tagged */
    ACCEPT_FRAME_TYPE_END
} rtk_vlan_acceptFrameType_t;

/* Function Name:
 *      rtk_switch_probe
 * Description:
 *      Probe switch
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Switch probed
 *      RT_ERR_FAILED   - Switch Unprobed.
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_probe(switch_chip_t *pSwitchChip);

/* Function Name:
 *      rtk_switch_initialState_set
 * Description:
 *      Set initial status
 * Input:
 *      state   - Initial state;
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Initialized
 *      RT_ERR_FAILED   - Uninitialized
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_initialState_set(init_state_t state);

/* Function Name:
 *      rtk_switch_initialState_get
 * Description:
 *      Get initial status
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      INIT_COMPLETED     - Initialized
 *      INIT_NOT_COMPLETED - Uninitialized
 * Note:
 *
 */
extern init_state_t rtk_switch_initialState_get(void);

/* Function Name:
 *      rtk_switch_init
 * Description:
 *      Set chip to default configuration environment
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can set chip registers to default configuration for different release chip model.
 */
extern rtk_api_ret_t rtk_switch_init(void);

/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize VLAN.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      VLAN is disabled by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set
 *      all ports PVID to the default VLAN.
 */
extern rtk_api_ret_t rtk_vlan_init(void);

/* Function Name:
 *      rtk_switch_logicalPortCheck
 * Description:
 *      Check logical port ID.
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is correct
 *      RT_ERR_FAILED   - Port ID is not correct
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_logicalPortCheck(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_phyPortMask_get
 * Description:
 *      Get physical portmask
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Physical port mask
 * Note:
 *
 */
rtk_uint32 rtk_switch_phyPortMask_get(void);

/* Function Name:
 *      rtk_switch_isPortMaskValid
 * Description:
 *      Check portmask is valid or not
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - port mask is valid
 *      RT_ERR_FAILED       - port mask is not valid
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isPortMaskValid(rtk_portmask_t *pPmask);

/* Function Name:
 *      rtk_switch_port_L2P_get
 * Description:
 *      Get physical port ID
 * Input:
 *      logicalPort       - logical port ID
 * Output:
 *      None
 * Return:
 *      Physical port ID
 * Note:
 *
 */
extern rtk_uint32 rtk_switch_port_L2P_get(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_portmask_L2P_get
 * Description:
 *      Get physical portmask from logical portmask
 * Input:
 *      pLogicalPmask       - logical port mask
 * Output:
 *      pPhysicalPortmask   - physical port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 *      RT_ERR_PORT_MASK    - Error port mask
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_portmask_L2P_get(rtk_portmask_t *pLogicalPmask, rtk_uint32 *pPhysicalPortmask);

/* Function Name:
 *      rtk_switch_portmask_P2L_get
 * Description:
 *      Get logical portmask from physical portmask
 * Input:
 *      physicalPortmask    - physical port mask
 * Output:
 *      pLogicalPmask       - logical port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 *      RT_ERR_PORT_MASK    - Error port mask
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_portmask_P2L_get(rtk_uint32 physicalPortmask, rtk_portmask_t *pLogicalPmask);

/* Function Name:
 *      rtk_switch_port_P2L_get
 * Description:
 *      Get logical port ID
 * Input:
 *      physicalPort       - physical port ID
 * Output:
 *      None
 * Return:
 *      logical port ID
 * Note:
 *
 */
extern rtk_port_t rtk_switch_port_P2L_get(rtk_uint32 physicalPort);

extern ret_t rtk_portMib_read(rtk_uint32 port, rtk_uint32 mib_index, rtk_uint64 *counter_value);

extern ret_t rtk_globalMib_rst(void);

extern ret_t rtk_portMib_rst(rtk_uint32 port);

extern char* rtk_chipid_to_chip_name(switch_chip_t id);

#endif