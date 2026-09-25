# INA237

https://docs.google.com/spreadsheets/d/1-vjs-lUSrl7nsLS4U-ToBjx40_OiCmG_iyD1ISLh_T4/edit?usp=sharing
This sheet above helps in calculating the limit values of following registers (gives values in Hex Value):
SOVL, SUVL, BOVL, BUVL, TEMP_LIMIT, PWR_LIMIT
CONV_DLY
SHUNT_CAL

Tested Conditions:
Shunt Resistor - 0.1Ω
Exp Current - 0.5A

I2C MODE - Standard Mode

FLT_DETECT.Init.op_mode = INA237_OP_MODE_SH_BUS_TEMP_CONT;
FLT_DETECT.Init.adc_range = INA237_ADCRANGE_163_84_MV;
FLT_DETECT.Init.avg = INA237_AVG_1024;
FLT_DETECT.Init.conv_dly = 0x00U;
FLT_DETECT.Init.vbusct = INA237_VBUSCT_50US;
FLT_DETECT.Init.vshct = INA237_VSHCT_50US;
FLT_DETECT.Init.vtct = INA237_VTCT_50US;

RESET
LATCH
APOL
SLOW ALERT

Tested against values:
BUSUL - 0xA00
BUSOL - 0xA00
SHNTUL - 0x3E8
SHNTOL - 0x3E8
TMPOL - 0x190
POL - 0x500
Conversion DLY - 0xFF

Other Configurations Tested:
FLT_DETECT.Init.adc_range = INA237_ADCRANGE_40_96_MV;
FLT_DETECT.Init.op_mode = INA237_OP_MODE_SH_BUS_TEMP_TRG;
Other I2C Modes Tested:
I2C MODE - Standard Mode, Fast Mode, Fast Mode Plus
