/*
 * Copyright (c) 2017 Jimmy-Yj Huang <jimmy-yj.huang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MFD_MT6351_CORE_H__
#define __MFD_MT6351_CORE_H__

enum mt6351_irq_numbers {
	RG_INT_STATUS_PWRKEY = 0,
	RG_INT_STATUS_HOMEKEY,
	RG_INT_STATUS_PWRKEY_R,
	RG_INT_STATUS_HOMEKEY_R,
	RG_INT_STATUS_THR_H,
	RG_INT_STATUS_THR_L,
	RG_INT_STATUS_BAT_H,
	RG_INT_STATUS_BAT_L,
	RG_INT_STATUS_NO_USE1,
	RG_INT_STATUS_RTC,
	RG_INT_STATUS_AUDIO,
	RG_INT_STATUS_MAD,
	RG_INT_STATUS_ACCDET,
	RG_INT_STATUS_ACCDET_EINT,
	RG_INT_STATUS_ACCDET_NEGV,
	RG_INT_STATUS_NI_LBAT_INT,
	RG_INT_STATUS_VCORE_OC,
	RG_INT_STATUS_VGPU_OC,
	RG_INT_STATUS_VSRAM_MD_OC,
	RG_INT_STATUS_VMODEM_OC,
	RG_INT_STATUS_VM1_OC,
	RG_INT_STATUS_VS1_OC,
	RG_INT_STATUS_VS2_OC,
	RG_INT_STATUS_VPA_OC,
	RG_INT_STATUS_VCORE_PREOC,
	RG_INT_STATUS_VGPU_PREOC,
	RG_INT_STATUS_VSRAM_MD_PREOC,
	RG_INT_STATUS_VMODEM_PREOC,
	RG_INT_STATUS_VM1_PREOC,
	RG_INT_STATUS_VS1_PREOC,
	RG_INT_STATUS_VS2_PREOC,
	RG_INT_STATUS_LDO_OC,
	RG_INT_STATUS_JEITA_HOT,
	RG_INT_STATUS_JEITA_WARM,
	RG_INT_STATUS_JEITA_COOL,
	RG_INT_STATUS_JEITA_COLD,
	RG_INT_STATUS_AUXADC_IMP,
	RG_INT_STATUS_NAG_C_DLTV,
	RG_INT_STATUS_NO_USE2,
	RG_INT_STATUS_NO_USE3,
	RG_INT_STATUS_OV,
	RG_INT_STATUS_BVALID_DET,
	RG_INT_STATUS_RGS_BATON_HV,
	RG_INT_STATUS_VBATON_UNDET,
	RG_INT_STATUS_WATCHDOG,
	RG_INT_STATUS_PCHR_CM_VDEC,
	RG_INT_STATUS_CHRDET,
	RG_INT_STATUS_PCHR_CM_VINC,
	RG_INT_STATUS_FG_BAT_H,
	RG_INT_STATUS_FG_BAT_L,
	RG_INT_STATUS_FG_CUR_H,
	RG_INT_STATUS_FG_CUR_L,
	RG_INT_STATUS_FG_ZCV,
	RG_INT_STATUS_NO_USE4,
	RG_INT_STATUS_NO_USE5,
	RG_INT_STATUS_NO_USE6,
	RG_INT_STATUS_NO_USE7,
	RG_INT_STATUS_NO_USE8,
	RG_INT_STATUS_NO_USE9,
	RG_INT_STATUS_NO_USE10,
	RG_INT_STATUS_NO_USE11,
	RG_INT_STATUS_NO_USE12,
	RG_INT_STATUS_NO_USE13,
	RG_INT_STATUS_NO_USE14,
	RG_INT_STATUS_NR,
};

#endif /* __MFD_MT6351_CORE_H__ */