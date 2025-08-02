#ifndef __RTL8372N_SMI_H__
#define __RTL8372N_SMI_H__

#include "rtl8372n_types.h"
#include "rtk_error.h"


#define MDC_MDIO_ADDRESS_REG        0x16
#define MDC_MDIO_DATAL_REG          0x17
#define MDC_MDIO_DATAH_REG          0x18

#define MDC_MDIO_READ_OP           0x1B
#define MDC_MDIO_WRITE_OP          0x19

#define DELAY                        10000
#define CLK_DURATION(clk)            { int i; for(i=0; i<clk; i++); }


rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData);
rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData);

#endif


