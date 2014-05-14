/* Copyright (c) 2012, The Linux Foundation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "msm_sensor.h"
#include "msm.h"
#include "msm_ispif.h"
#include "msm_camera_i2c_mux.h"

#define SENSOR_NAME "mt9v115"
#define INIT_ARRAY_CONT sizeof(mt9v115_init_settings)/sizeof(msm_camera_i2c_reg_delay_conf)

typedef struct msm_camera_i2c_reg_delay_conf {
	uint16_t addr;
	uint16_t data;
	enum msm_camera_i2c_data_type dt;
	uint16_t delay;
} msm_camera_i2c_reg_delay_conf;


DEFINE_MUTEX(mt9v115_mut);
static struct msm_sensor_ctrl_t mt9v115_s_ctrl;

static struct msm_camera_i2c_reg_conf mt9v115_start_settings[] = {
	{0x3C40, 0x7834},
};

static struct msm_camera_i2c_reg_conf mt9v115_stop_settings[] = {
	{0x3C40, 0x7836},
};

static struct msm_camera_i2c_reg_delay_conf mt9v115_init_settings[] = {
	{0x0010, 0x052C, MSM_CAMERA_I2C_WORD_DATA, 0},	//PLL_DIVIDERS
	{0x0012, 0x0800, MSM_CAMERA_I2C_WORD_DATA, 0},	//PLL_P_DIVIDERS->0x0800
	{0x0014, 0x2047, MSM_CAMERA_I2C_WORD_DATA, 0},	//PLL_DIVIDERS//add
	{0x0014, 0x2046, MSM_CAMERA_I2C_WORD_DATA, 0},	//PLL_P_DIVIDERS//add
	{0x0018, 0x4505, MSM_CAMERA_I2C_WORD_DATA, 0},	//STANDBY_CONTROL_AND_STATUS//add
	{0x0018, 0x4504, MSM_CAMERA_I2C_WORD_DATA, 10},	//STANDBY_CONTROL_AND_STATUS//add
	{0x0042, 0xFFF3, MSM_CAMERA_I2C_WORD_DATA, 0},	//COMMAND_RW
	{0x3C00, 0x5004, MSM_CAMERA_I2C_WORD_DATA, 0},	//TX_CONTROL
	{0x001A, 0x0520, MSM_CAMERA_I2C_WORD_DATA, 0},	//RESET_AND_MISC_CONTROL
	{0x001A, 0x0564, MSM_CAMERA_I2C_WORD_DATA, 10},	//RESET_AND_MISC_CONTROL
	{0x0012, 0x0200, MSM_CAMERA_I2C_WORD_DATA, 0},	//PLL_P_DIVIDERS
	//timing,fixed30fps
	{0x300A, 0x01F9, MSM_CAMERA_I2C_WORD_DATA, 0},	//FRAME_LENGTH_LINES
	{0x300C, 0x02D6, MSM_CAMERA_I2C_WORD_DATA, 0},	//LINE_LENGTH_PCK
	{0x3010, 0x0012, MSM_CAMERA_I2C_WORD_DATA, 0},	//FINE_CORRECTION
	{0x3040, 0x0041, MSM_CAMERA_I2C_WORD_DATA, 0},	//READ_MODE
	{0x098E, 0x9803, MSM_CAMERA_I2C_WORD_DATA, 0},	//LOGICAL_ADDRESS_ACCESS[STAT_FD_ZONE_HEIGHT]
	{0x9803, 0x07, MSM_CAMERA_I2C_BYTE_DATA, 0},	//STAT_FD_ZONE_HEIGHT
	{0xA06E, 0x0098, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_FD_CONFIG_FDPERIOD_50HZ
	{0xA070, 0x007E, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_FD_CONFIG_FDPERIOD_60HZ
	{0xA072, 0x11, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_FD_CONFIG_SEARCH_F1_50
	{0xA073, 0x13, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_FD_CONFIG_SEARCH_F2_50
	{0xA074, 0x14, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_FD_CONFIG_SEARCH_F1_60
	{0xA075, 0x16, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_FD_CONFIG_SEARCH_F2_60
	{0xA076, 0x000E, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_FD_CONFIG_MAX_FDZONE_50HZ
	{0xA078, 0x0010, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_FD_CONFIG_MAX_FDZONE_60HZ
	{0xA01A, 0x000E, MSM_CAMERA_I2C_WORD_DATA, 1},	//CAM_AE_CONFIG_TARGET_FDZONE
	//Step3-Recommended//Charsettings
	//char
	{0x3168, 0x84F8, MSM_CAMERA_I2C_WORD_DATA, 0},	//DAC_ECL_VAALO
	{0x316A, 0x028A, MSM_CAMERA_I2C_WORD_DATA, 0},	//DAC_RSTLO
	{0x316C, 0xB477, MSM_CAMERA_I2C_WORD_DATA, 0},	//DAC_TXLO
	{0x316E, 0x8268, MSM_CAMERA_I2C_WORD_DATA, 0},	//DAC_ECL8A
	{0x3180, 0x87FF, MSM_CAMERA_I2C_WORD_DATA, 0},	//DELTA_DK_CONTROL
	{0x3E02, 0x0600, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_RSTX1
	{0x3E04, 0x221C, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_RSTX2
	{0x3E06, 0x3632, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_RSTX3
	{0x3E08, 0x3204, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_VLN_HOLD
	{0x3E0A, 0x3106, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_SAMP_EN
	{0x3E0C, 0x3025, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_SAMP_SIG
	{0x3E0E, 0x190B, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_SAMP_RST
	{0x3E10, 0x0700, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_COL_PUP
	{0x3E12, 0x24FF, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_COL_PDN1
	{0x3E14, 0x3731, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_COL_PDN2
	{0x3E16, 0x0401, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_BOOST1_EN
	{0x3E18, 0x211E, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_BOOST2_EN
	{0x3E1A, 0x3633, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_BOOST3_EN
	{0x3E1C, 0x3107, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_BOOST_MUX
	{0x3E1E, 0x1A16, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_BOOST1_HLT
	{0x3E20, 0x312D, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_BOOST2_HLT
	{0x3E22, 0x3303, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_BOOST_ROW
	{0x3E24, 0x1401, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_SH_VCL
	{0x3E26, 0x0600, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_SPARE
	{0x3E30, 0x0037, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_READOUT
	{0x3E32, 0x1638, MSM_CAMERA_I2C_WORD_DATA, 0},	//SAMP_RESET_DONE
	{0x3E90, 0x0E05, MSM_CAMERA_I2C_WORD_DATA, 0},	//RST_RSTX1
	{0x3E92, 0x1310, MSM_CAMERA_I2C_WORD_DATA, 0},	//RST_RSTX2
	{0x3E94, 0x0904, MSM_CAMERA_I2C_WORD_DATA, 0},	//RST_SHUTTER
	{0x3E96, 0x0B00, MSM_CAMERA_I2C_WORD_DATA, 0},	//RST_COL_PUP
	{0x3E98, 0x130B, MSM_CAMERA_I2C_WORD_DATA, 0},	//RST_COL_PDN
	{0x3E9A, 0x0C06, MSM_CAMERA_I2C_WORD_DATA, 0},	//RST_BOOST1_EN
	{0x3E9C, 0x1411, MSM_CAMERA_I2C_WORD_DATA, 0},	//RST_BOOST2_EN
	{0x3E9E, 0x0E01, MSM_CAMERA_I2C_WORD_DATA, 0},	//RST_BOOST_MUX
	{0x3ECC, 0x4091, MSM_CAMERA_I2C_WORD_DATA, 0},	//DAC_LD_0_1
	{0x3ECE, 0x430D, MSM_CAMERA_I2C_WORD_DATA, 0},	//DAC_LD_2_3
	{0x3ED0, 0x1817, MSM_CAMERA_I2C_WORD_DATA, 0},	//DAC_LD_4_5
	{0x3ED2, 0x8504, MSM_CAMERA_I2C_WORD_DATA, 0},	//DAC_LD_6_7
	//patch
	{0x0982, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//ACCESS_CTL_STAT
	{0x098A, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//PHYSICAL_ADDRESS_ACCESS
	{0x8251, 0x3C3C, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8253, 0xBDD1, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8255, 0xF2D6, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8257, 0x15C1, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8259, 0x0126, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x825B, 0x3ADC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x825D, 0x0A30, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x825F, 0xED02, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8261, 0xDC08, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8263, 0xED00, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8265, 0xFC01, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8267, 0xFCBD, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8269, 0xF5FC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x826B, 0x30EC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x826D, 0x02FD, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x826F, 0x0344, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8271, 0xB303, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8273, 0x4025, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8275, 0x0DCC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8277, 0x3180, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8279, 0xED00, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x827B, 0xCCA0, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x827D, 0x00BD, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x827F, 0xFBFB, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8281, 0x2013, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8283, 0xFC03, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8285, 0x44B3, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8287, 0x0342, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8289, 0x220B, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x828B, 0xCC31, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x828D, 0x80ED, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x828F, 0x00CC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8291, 0xA000, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8293, 0xBDFC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8295, 0x1738, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8297, 0x3839, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8299, 0x3CD6, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x829B, 0x15C1, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x829D, 0x0126, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x829F, 0x70FC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82A1, 0x0344, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82A3, 0xB303, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82A5, 0x4025, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82A7, 0x13FC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82A9, 0x7E26, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82AB, 0x83FF, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82AD, 0xFF27, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82AF, 0x0BFC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82B1, 0x7E26, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82B3, 0xFD03, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82B5, 0x4CCC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82B7, 0xFFFF, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82B9, 0x2013, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82BB, 0xFC03, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82BD, 0x44B3, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82BF, 0x0342, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82C1, 0x220E, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82C3, 0xFC7E, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82C5, 0x2683, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82C7, 0xFFFF, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82C9, 0x2606, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82CB, 0xFC03, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82CD, 0x4CFD, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82CF, 0x7E26, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82D1, 0xFC7E, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82D3, 0x2683, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82D5, 0xFFFF, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82D7, 0x2605, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82D9, 0xFC03, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82DB, 0x4A20, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82DD, 0x03FC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82DF, 0x0348, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82E1, 0xFD7E, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82E3, 0xD0FC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82E5, 0x7ED2, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82E7, 0x5F84, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82E9, 0xF030, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82EB, 0xED00, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82ED, 0xDC0A, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82EF, 0xB303, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82F1, 0x4625, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82F3, 0x10EC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82F5, 0x0027, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82F7, 0x0CFD, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82F9, 0x034E, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82FB, 0xFC7E, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82FD, 0xD284, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x82FF, 0x0FED, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8301, 0x0020, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8303, 0x19DC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8305, 0x0AB3, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8307, 0x0346, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8309, 0x2415, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x830B, 0xEC00, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x830D, 0x8300, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x830F, 0x0026, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8311, 0x0EFC, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8313, 0x7ED2, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8315, 0x840F, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8317, 0xFA03, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8319, 0x4FBA, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x831B, 0x034E, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x831D, 0xFD7E, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x831F, 0xD2BD, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8321, 0xD2AD, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8323, 0x3839, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x098E, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//LOGICAL_ADDRESS_ACCESS
	{0x0982, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//ACCESS_CTL_STAT
	{0x098A, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//PHYSICAL_ADDRESS_ACCESS
	{0x8340, 0x0048, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8342, 0x0040, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8344, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8346, 0x0040, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x8348, 0x1817, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x834A, 0x1857, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x098E, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//LOGICAL_ADDRESS_ACCESS
	{0x0982, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//ACCESS_CTL_STAT
	{0x098A, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//PHYSICAL_ADDRESS_ACCESS
	{0x824D, 0x0251, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x824F, 0x0299, MSM_CAMERA_I2C_WORD_DATA, 0},
	{0x098E, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 1},	//LOGICAL_ADDRESS_ACCESS
	//step4-PGA
	{0x3210, 0x00B0, MSM_CAMERA_I2C_WORD_DATA, 0},	//COLOR_PIPELINE_CONTROL
	{0x3640, 0x0030, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P0Q0
	{0x3642, 0x3608, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P0Q1
	{0x3644, 0x24F0, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P0Q2
	{0x3646, 0xFC0D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P0Q3
	{0x3648, 0xB94C, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P0Q4
	{0x364A, 0x0090, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P0Q0
	{0x364C, 0x26A8, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P0Q1
	{0x364E, 0x3250, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P0Q2
	{0x3650, 0xA5AD, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P0Q3
	{0x3652, 0xB82A, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P0Q4
	{0x3654, 0x0090, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P0Q0
	{0x3656, 0x88AC, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P0Q1
	{0x3658, 0x0190, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P0Q2
	{0x365A, 0xA76B, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P0Q3
	{0x365C, 0x872C, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P0Q4
	{0x365E, 0x01F0, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P0Q0
	{0x3660, 0x0D0A, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P0Q1
	{0x3662, 0x26B0, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P0Q2
	{0x3664, 0xF10D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P0Q3
	{0x3666, 0x988C, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P0Q4
	{0x3680, 0x670B, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P1Q0
	{0x3682, 0xEDC9, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P1Q1
	{0x3684, 0x720A, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P1Q2
	{0x3686, 0xD92D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P1Q3
	{0x3688, 0xDCED, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P1Q4
	{0x368A, 0x54EB, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P1Q0
	{0x368C, 0x1084, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P1Q1
	{0x368E, 0x3408, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P1Q2
	{0x3690, 0xE00D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P1Q3
	{0x3692, 0x8E6E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P1Q4
	{0x3694, 0x230B, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P1Q0
	{0x3696, 0x8826, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P1Q1
	{0x3698, 0x418A, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P1Q2
	{0x369A, 0xC6CD, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P1Q3
	{0x369C, 0x950B, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P1Q4
	{0x369E, 0x510B, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P1Q0
	{0x36A0, 0xE5A4, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P1Q1
	{0x36A2, 0x48EB, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P1Q2
	{0x36A4, 0xE9AD, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P1Q3
	{0x36A6, 0xAE4D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P1Q4
	{0x36C0, 0x1AB0, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P2Q0
	{0x36C2, 0xA1AE, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P2Q1
	{0x36C4, 0x9650, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P2Q2
	{0x36C6, 0xCB0E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P2Q3
	{0x36C8, 0x3AF1, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P2Q4
	{0x36CA, 0x2330, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P2Q0
	{0x36CC, 0xCC2C, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P2Q1
	{0x36CE, 0x7C0E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P2Q2
	{0x36D0, 0xCCCC, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P2Q3
	{0x36D2, 0x57AE, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P2Q4
	{0x36D4, 0x0D10, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P2Q0
	{0x36D6, 0x336D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P2Q1
	{0x36D8, 0x8A4E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P2Q2
	{0x36DA, 0xE590, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P2Q3
	{0x36DC, 0x8AB0, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P2Q4
	{0x36DE, 0x1670, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P2Q0
	{0x36E0, 0xE24E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P2Q1
	{0x36E2, 0x9C2E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P2Q2
	{0x36E4, 0x196E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P2Q3
	{0x36E6, 0x0470, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P2Q4
	{0x3700, 0xF98A, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P3Q0
	{0x3702, 0x060A, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P3Q1
	{0x3704, 0x45CA, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P3Q2
	{0x3706, 0xE64A, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P3Q3
	{0x3708, 0x7ACD, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P3Q4
	{0x370A, 0xB64C, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P3Q0
	{0x370C, 0x9D0D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P3Q1
	{0x370E, 0x87ED, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P3Q2
	{0x3710, 0x2F8E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P3Q3
	{0x3712, 0x0BF0, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P3Q4
	{0x3714, 0xAF6A, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P3Q0
	{0x3716, 0x83ED, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P3Q1
	{0x3718, 0x576C, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P3Q2
	{0x371A, 0x088D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P3Q3
	{0x371C, 0xDD8F, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P3Q4
	{0x371E, 0x080C, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P3Q0
	{0x3720, 0x950D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P3Q1
	{0x3722, 0xAE6E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P3Q2
	{0x3724, 0x08EF, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P3Q3
	{0x3726, 0x0670, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P3Q4
	{0x3740, 0xE86D, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P4Q0
	{0x3742, 0x4E4E, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P4Q1
	{0x3744, 0x2B92, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P4Q2
	{0x3746, 0x5F2F, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P4Q3
	{0x3748, 0x83D4, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G1_P4Q4
	{0x374A, 0x30A9, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P4Q0
	{0x374C, 0x9CCD, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P4Q1
	{0x374E, 0xD2AF, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P4Q2
	{0x3750, 0xBF30, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P4Q3
	{0x3752, 0xC872, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_R_P4Q4
	{0x3754, 0xCBAF, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P4Q0
	{0x3756, 0x90B0, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P4Q1
	{0x3758, 0x222B, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P4Q2
	{0x375A, 0x3C72, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P4Q3
	{0x375C, 0x11D1, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_B_P4Q4
	{0x375E, 0xAAAC, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P4Q0
	{0x3760, 0x30D0, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P4Q1
	{0x3762, 0x5EB0, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P4Q2
	{0x3764, 0xF051, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P4Q3
	{0x3766, 0xA4F3, MSM_CAMERA_I2C_WORD_DATA, 0},	//P_G2_P4Q4
	{0x3782, 0x00F0, MSM_CAMERA_I2C_WORD_DATA, 0},	//CENTER_ROW
	{0x3784, 0x0174, MSM_CAMERA_I2C_WORD_DATA, 0},	//CENTER_COLUMN
	{0x3210, 0x00B8, MSM_CAMERA_I2C_WORD_DATA, 0},	//COLOR_PIPELINE_CONTROL
	{0x3210, 0x00B8, MSM_CAMERA_I2C_WORD_DATA, 0},	//COLOR_PIPELINE_CONTROL
	//Step5-AWB_CCM//AWB&CCM
	{0xA02F, 0x0176, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_0
	{0xA031, 0xFF62, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_1
	{0xA033, 0x0042, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_2
	{0xA035, 0xFF96, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_3
	{0xA037, 0x00FE, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_4
	{0xA039, 0x004F, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_5
	{0xA03B, 0xFFAB, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_6
	{0xA03D, 0xFF07, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_7
	{0xA03F, 0x01F1, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_8
	{0xA041, 0x001D, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_9
	{0xA043, 0x0046, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_L_10
	{0xA045, 0x00C5, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_0
	{0xA047, 0xFFB7, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_1
	{0xA049, 0xFFD8, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_2
	{0xA04B, 0x002F, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_3
	{0xA04D, 0x005D, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_4
	{0xA04F, 0xFF9E, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_5
	{0xA051, 0x0058, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_6
	{0xA053, 0x006B, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_7
	{0xA055, 0xFFEE, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_8
	{0xA057, 0x0010, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_9
	{0xA059, 0xFFE0, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_CCM_RL_10
	{0x2112, 0x051E, MSM_CAMERA_I2C_WORD_DATA, 0},	//AWB_WEIGHT_R0
	{0x2114, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//AWB_WEIGHT_R1
	{0x2116, 0xAE40, MSM_CAMERA_I2C_WORD_DATA, 0},	//AWB_WEIGHT_R2
	{0x2118, 0x7AE1, MSM_CAMERA_I2C_WORD_DATA, 0},	//AWB_WEIGHT_R3
	{0x211A, 0x1996, MSM_CAMERA_I2C_WORD_DATA, 0},	//AWB_WEIGHT_R4
	{0x211C, 0x0F5A, MSM_CAMERA_I2C_WORD_DATA, 0},	//AWB_WEIGHT_R5
	{0x211E, 0xFDD8, MSM_CAMERA_I2C_WORD_DATA, 0},	//AWB_WEIGHT_R6
	{0x2120, 0x5000, MSM_CAMERA_I2C_WORD_DATA, 0},	//AWB_WEIGHT_R7
	{0xA065, 0x03, MSM_CAMERA_I2C_BYTE_DATA, 1},	//CAM_AWB_CONFIG_X_SCALE
	{0xA06B, 0x70, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_AWB_CONFIG_K_R_R//default0x80
	//Step6-CPIPE_Calibration//ColorPipeCalibrationsettings,ifany
	{0x9418, 0x2E, MSM_CAMERA_I2C_BYTE_DATA, 0},	//AWB_B_SCENE_RATIO_LOWER
	//Step7-CPIPE_Preference//ColorPipepreferencesettings,ifany
	{0x326E, 0x0006, MSM_CAMERA_I2C_WORD_DATA, 0},	//LOW_PASS_YUV_FILTER
	{0x33F4, 0x000B, MSM_CAMERA_I2C_WORD_DATA, 0},	//KERNEL_CONFIG
	{0xA07A, 0x04, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_AP_THRESH_START
	{0xA07C, 0x06, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_AP_GAIN_START
	{0xA081, 0x1E, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_DM_EDGE_TH_START
	{0xA082, 0x50, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_DM_EDGE_TH_STOP
	{0xA0B1, 0x10, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_NR_RED_START
	{0xA0B2, 0x2D, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_NR_RED_STOP
	{0xA0B3, 0x10, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_NR_GREEN_START
	{0xA0B4, 0x2D, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_NR_GREEN_STOP
	{0xA0B5, 0x10, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_NR_BLUE_START
	{0xA0B6, 0x2D, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_NR_BLUE_STOP
	{0xA0B7, 0x10, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_NR_MIN_MAX_START
	{0xA0B8, 0x2D, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_NR_MIN_MAX_STOP
	{0xA05F, 0x80, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_AWB_CONFIG_START_SATURATION
	{0xA060, 0x05, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_AWB_CONFIG_END_SATURATION
	{0xA0B9, 0x0026, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_LL_CONFIG_START_GAIN_METRIC
	{0xA0BB, 0x00B4, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_LL_CONFIG_STOP_GAIN_METRIC
	{0xA07E, 0x001E, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_LL_CONFIG_CDC_THRESHOLD_BM
	{0x9C00, 0xBF, MSM_CAMERA_I2C_BYTE_DATA, 0},	//LL_MODE
	{0xA085, 0x0078, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_LL_CONFIG_FTB_AVG_YSUM_STOP
	{0xA087, 0x00, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_0
	{0xA088, 0x07, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_1
	{0xA089, 0x16, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_2
	{0xA08A, 0x30, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_3
	{0xA08B, 0x52, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_4
	{0xA08C, 0x6D, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_5
	{0xA08D, 0x86, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_6
	{0xA08E, 0x9B, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_7
	{0xA08F, 0xAB, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_8
	{0xA090, 0xB9, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_9
	{0xA091, 0xC5, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_10
	{0xA092, 0xCF, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_11
	{0xA093, 0xD8, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_12
	{0xA094, 0xE0, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_13
	{0xA095, 0xE7, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_14
	{0xA096, 0xEE, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_15
	{0xA097, 0xF4, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_16
	{0xA098, 0xFA, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_LL_CONFIG_GAMMA_CONTRAST_CURVE_17
	{0xA0AD, 0x0005, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_LL_CONFIG_GAMMA_START_BM
	{0xA0AF, 0x0021, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_LL_CONFIG_GAMMA_STOP_BM
	{0xA020, 0x3C, MSM_CAMERA_I2C_BYTE_DATA, 0},	//CAM_AE_CONFIG_BASE_TARGET
	{0xA027, 0x002A, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AE_CONFIG_MIN_VIRT_AGAIN
	{0xA029, 0x0180, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AE_CONFIG_MAX_VIRT_AGAIN
	{0xA01C, 0x0140, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AE_CONFIG_TARGET_AGAIN
	{0xA023, 0x0080, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AE_CONFIG_MIN_VIRT_DGAIN
	{0xA025, 0x0080, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AE_CONFIG_MAX_VIRT_DGAIN
	{0xA01E, 0x0100, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AE_CONFIG_TARGET_DGAIN
	{0x8C03, 0x1, MSM_CAMERA_I2C_BYTE_DATA, 0}, //FD_STAT_MIN
	{0x8C04, 0x03, MSM_CAMERA_I2C_BYTE_DATA, 0},	//FD_STAT_MAX
	{0x8C05, 0x05, MSM_CAMERA_I2C_BYTE_DATA, 1},	//FD_MIN_AMPLITUDE
	//Step8-Features//Ports,specialfeatures,etc.,ifany
	//None
	{0x329e, 0x0000, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_K_R_R//default0x80
	{0x0018, 0x0002, MSM_CAMERA_I2C_WORD_DATA, 0},	//CAM_AWB_CONFIG_K_R_R//default0x80
	{0x337C, 0x0004, MSM_CAMERA_I2C_WORD_DATA, 10},
};

int32_t mt9v115_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0, i = 0;
	static int csi_config;
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(10);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		for (i = 0; i < INIT_ARRAY_CONT; i++) {
			msm_camera_i2c_write(s_ctrl->sensor_i2c_client, mt9v115_init_settings[i].addr,
				mt9v115_init_settings[i].data, mt9v115_init_settings[i].dt);
			if(mt9v115_init_settings[i].delay)
				msleep(mt9v115_init_settings[i].delay);
		}
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
		msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->mode_settings, res);
		if (!csi_config) {
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			CDBG("CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			CDBG("CSI config is done\n");
			mb();
			msleep(10);
			csi_config = 1;
		}
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE,
			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
	}
	return rc;
}


static struct msm_camera_i2c_reg_conf mt9v115_full_settings[] = {
};


static struct v4l2_subdev_info mt9v115_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array mt9v115_confs[] = {
	{&mt9v115_full_settings[0],
	ARRAY_SIZE(mt9v115_full_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_sensor_output_info_t mt9v115_dimensions[] = {
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		.line_length_pclk = 0x2D6,
		.frame_length_lines = 0x1F9,
		.vt_pixel_clk = 14000000,
		.op_pixel_clk = 14000000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params mt9v115_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x06,
};

static struct msm_camera_csi_params *mt9v115_csi_params_array[] = {
	&mt9v115_csi_params,
};

static struct msm_sensor_output_reg_addr_t mt9v115_reg_addr = {
	.x_output = 0x3008,
	.y_output = 0x3006,
	.line_length_pclk = 0x300C,
	.frame_length_lines = 0x300A,
};

static struct msm_sensor_id_info_t mt9v115_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x2284,
};

static const struct i2c_device_id mt9v115_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9v115_s_ctrl},
	{ }
};


static struct i2c_driver mt9v115_i2c_driver = {
	.id_table = mt9v115_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9v115_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	int rc = 0;
	CDBG("mt9v115\n");

	rc = i2c_add_driver(&mt9v115_i2c_driver);

	return rc;
}

static struct v4l2_subdev_core_ops mt9v115_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9v115_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9v115_subdev_ops = {
	.core = &mt9v115_subdev_core_ops,
	.video  = &mt9v115_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9v115_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_csi_setting = mt9v115_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t mt9v115_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
	.start_stream_conf = mt9v115_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(mt9v115_start_settings),
	.stop_stream_conf = mt9v115_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(mt9v115_stop_settings),
	//.init_settings = &mt9v115_init_conf1[0],
	//.init_size = ARRAY_SIZE(mt9v115_init_conf1),
	.mode_settings = &mt9v115_confs[0],
	//.no_effect_settings = &mt9v115_no_effect_confs[0],
	.output_settings = &mt9v115_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9v115_confs),
};

static struct msm_sensor_ctrl_t mt9v115_s_ctrl = {
	.msm_sensor_reg = &mt9v115_regs,
	.sensor_i2c_client = &mt9v115_sensor_i2c_client,
	.sensor_i2c_addr = 0x7a,
	.sensor_output_reg_addr = &mt9v115_reg_addr,
	.sensor_id_info = &mt9v115_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &mt9v115_csi_params_array[0],
	.msm_sensor_mutex = &mt9v115_mut,
	.sensor_i2c_driver = &mt9v115_i2c_driver,
	.sensor_v4l2_subdev_info = mt9v115_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9v115_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9v115_subdev_ops,
	.func_tbl = &mt9v115_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivision VGA YUV sensor driver");
MODULE_LICENSE("GPL v2");