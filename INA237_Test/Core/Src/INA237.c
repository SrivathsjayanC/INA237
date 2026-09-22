/*
 * 	@file INA237.c
 *  @date 04 Sep 2026
 *  @author: SRIVATHSJAYAN
 */
#include "INA237.h"

static HAL_StatusTypeDef INA237_WriteReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t Data);
static HAL_StatusTypeDef INA237_ReadReg_24(INA237_Handle_TypeDef_t *hfault,INA237_Register_t reg,uint32_t *pData);
static HAL_StatusTypeDef INA237_ReadReg_Signed(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,int16_t *pData);

/**
 * @brief  Performs a software reset on the INA237 device over I2C.
 * @details Sets the reset bit (RST, bit 15) in the Configuration Register (00h),
 *          restoring all internal registers to their default power-on reset states.
 *
 * @param[in] hfault Pointer to the INA237 handle structure containing device
 *                   and I2C configuration.
 *
 * @retval HAL_OK       Reset command was successfully transmitted.
 * @retval HAL_ERROR    I2C communication failure or transmission error.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA237_Reset(INA237_Handle_TypeDef_t *hfault)
{
	uint16_t reset_bit = 0x01 << __INA237_RST_POS;

	return INA237_WriteReg(hfault, INA237_REG_CONFIG,reset_bit);
}
/**
 * @brief  Initializes the INA237 power/current monitor over I2C.
 * @details Validates the handle pointers, peripheral instance, device address, and
 *          initialization parameters. Issues a software reset to restore the device to its
 *          power-on defaults, waits 5 ms for internal reboot completion, and configures both
 *          the CONFIG (00h) and ADC_CONFIG (01h) registers with the requested ADC range,
 *          conversion delay, operating mode, conversion times, and averaging count.
 *
 * @param[in] hfault Pointer to the INA237 handle structure containing device settings,
 *                   I2C peripheral instance, and initialization parameters.
 *
 * @retval HAL_OK       Device successfully reset and configured.
 * @retval HAL_ERROR    A pointer argument is NULL, the target device I2C address is 0,
 *                      an initialization parameter is out of valid range or set to an
 *                      unsupported power-down state, or an I2C communication error occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C communication timed out.
 *
 * @note   The device is reset before applying the configuration.
 * @note   The function configures the ADC range, conversion delay,
 *         operating mode, bus voltage conversion time, shunt voltage
 *         conversion time, temperature conversion time, and averaging.
 */
HAL_StatusTypeDef INA237_Init(INA237_Handle_TypeDef_t *hfault)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	uint16_t reg=0;
	HAL_StatusTypeDef status;
	INA237_Reset(hfault);
	HAL_Delay(5);
	if(hfault->Init.adc_range > INA237_ADCRANGE_40_96_MV)
	{
		return HAL_ERROR;
	}
	reg = hfault->Init.adc_range << __INA237_ADCRANGE_POS;

	if(hfault->Init.conv_dly > __INA237_CONVDLY_MAX)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.conv_dly << __INA237_CONVDLY_POS;
	status = INA237_WriteReg(hfault,INA237_REG_CONFIG,reg);

	if(status != HAL_OK)
	{
		return status;
	}

	reg = 0;

	if(hfault->Init.op_mode > INA237_OP_MODE_SH_BUS_TEMP_CONT)
	{
		return HAL_ERROR;
	}
	if(hfault->Init.op_mode == INA237_OP_MODE_PWR_DWN_0 || hfault->Init.op_mode == INA237_OP_MODE_PWR_DWN_8)
	{
		return HAL_ERROR;
	}
	reg = hfault->Init.op_mode << __INA237_OP_MODE_POS;

	if(hfault->Init.vbusct > INA237_VBUSCT_4120US)
	{
		return HAL_ERROR;
	}

	reg |= hfault->Init.vbusct << __INA237_VBUSCT_POS;

	if(hfault->Init.vshct > INA237_VSHCT_4120US)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.vshct << __INA237_VSHCT_POS;

	if(hfault->Init.vtct > INA237_VTCT_4120US)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.vtct << __INA237_VTCT_POS;

	if(hfault->Init.avg > INA237_AVG_1024)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.avg << __INA237_AVG_POS;

	return INA237_WriteReg(hfault,INA237_REG_ADC_CONFIG,reg);
}
/**
 * @brief  Writes a 16-bit value to an INA237 register over I2C.
 * @details Splits the 16-bit data word into MSB and LSB (transmitted MSB first)[cite: 1, 3]
 *          and transmits it along with the 8-bit register address to the target
 *          device using the HAL memory write API[cite: 1].
 *
 * @param[in] hfault Pointer to the INA237 handle structure containing device
 *                   configuration and the I2C peripheral instance[cite: 1].
 * @param[in] Reg    Target register address to write to (type ::INA237_Register_t).
 * @param[in] Data   16-bit data value to write into the register[cite: 1, 3].
 *
 * @retval HAL_OK       Register write completed successfully over I2C[cite: 1, 3].
 * @retval HAL_ERROR    A transmission error or I2C communication failure occurred[cite: 1, 3].
 * @retval HAL_BUSY     The I2C peripheral is currently busy[cite: 1, 3].
 * @retval HAL_TIMEOUT  The I2C write operation timed out[cite: 1, 3].
 */
static HAL_StatusTypeDef INA237_WriteReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t Data)
{
	uint8_t tx[2];
	tx[0] = (uint8_t)(Data>>8);
	tx[1] = (uint8_t)Data;

	return HAL_I2C_Mem_Write(hfault->hi2c, hfault->Init.dev_i2c_addr, Reg, 1, tx, 2, HAL_MAX_DELAY);
}
/**
 * @brief  Reads a 16-bit register from the INA237 over I2C.
 * @details Validates the handle pointer, peripheral instance, target device address,
 *          and output buffer pointer. Reads 2 consecutive data bytes (MSB first)
 *          from the specified target register using the HAL I2C memory read API
 *          and reconstructs them into an unsigned 16-bit word.
 *
 * @note   This function is intended for reading unsigned 16-bit registers (e.g., CONFIG,
 *         ADC_CONFIG, BUS_VLTG, PWR_LIMIT). For registers containing signed two's
 *         complement data (such as Shunt Voltage and Current), use the dedicated signed
 *         read routine to prevent sign interpretation issues.
 *
 * @param[in]  hfault Pointer to the INA237 handle structure containing device
 *                    configuration and the I2C peripheral instance.
 * @param[in]  Reg    Target register address to read from (type ::INA237_Register_t).
 * @param[out] pData  Pointer to a 16-bit unsigned variable where the reconstructed
 *                    register value will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Register read completed successfully over I2C.
 * @retval HAL_ERROR    A pointer argument is NULL, the target device address is 0,
 *                      or an I2C communication failure occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read operation timed out.
 */
HAL_StatusTypeDef INA237_ReadReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint8_t rx_buff[2];

	status = HAL_I2C_Mem_Read(hfault->hi2c, hfault->Init.dev_i2c_addr, Reg, 1, rx_buff, 2,HAL_MAX_DELAY);
	if(status!=HAL_OK)
	{
		return status;
	}
	*pData = (((uint16_t)rx_buff[0]<<8) |(uint16_t)rx_buff[1]);

	return status;
}
/**
 * @brief  Reads the raw 16-bit signed value from an INA237 register over I2C.
 * @details Reads 2 consecutive data bytes (MSB first) from the specified target
 *          register using the HAL I2C memory read API and combines them into
 *          a signed 16-bit integer. This function is intended for registers containing
 *          two's complement data (such as Shunt Voltage and Current).
 *
 * @param[in]  hfault Pointer to the INA237 handle structure containing device
 *                    configuration and the I2C peripheral instance.
 * @param[in]  Reg    Target register address to read from (type ::INA237_Register_t).
 * @param[out] pData  Pointer to a signed 16-bit variable where the reconstructed
 *                    register value will be stored.
 *
 * @retval HAL_OK       Register read completed successfully over I2C.
 * @retval HAL_ERROR    An I2C communication failure occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read operation timed out.
 */
static HAL_StatusTypeDef INA237_ReadReg_Signed(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,int16_t *pData)
{
	HAL_StatusTypeDef status;
	uint8_t rx[2];

	status = HAL_I2C_Mem_Read(hfault->hi2c, hfault->Init.dev_i2c_addr, Reg, 1, rx, 2,HAL_MAX_DELAY);
	if(status!=HAL_OK)
	{
		return status;
	}
	*pData = (((int16_t)rx[0]<<8) | (int16_t)rx[1]);

	return status;
}
/**
 * @brief  Reads a 24-bit register from the INA237 device over I2C.
 * @details Validates the handle pointers, peripheral instance, device address,
 *          and output buffer pointer[cite: 1]. Transmits the 8-bit register address,
 *          reads 3 consecutive data bytes using the HAL memory API, and reconstructs
 *          them into an unsigned 32-bit integer[cite: 1].
 *
 * @param[in]  hfault Pointer to the INA237 handle structure containing device
 *                    and I2C peripheral configuration[cite: 1].
 * @param[in]  Reg    Target 24-bit register address to read from (type ::INA237_Register_t).
 * @param[out] pData  Pointer to a uint32_t variable where the reconstructed 24-bit
 *                    register value will be stored[cite: 1]. Must not be NULL[cite: 1].
 *
 * @retval HAL_OK       24-bit register read completed successfully over I2C[cite: 1].
 * @retval HAL_ERROR    A pointer argument is NULL, the target device address is 0,
 *                      or an I2C communication failure occurred[cite: 1].
 * @retval HAL_BUSY     The I2C peripheral is currently busy[cite: 1].
 * @retval HAL_TIMEOUT  The I2C read operation timed out[cite: 1].
 */
static HAL_StatusTypeDef INA237_ReadReg_24(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint32_t *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint8_t rx[3];

	status = HAL_I2C_Mem_Read(hfault->hi2c, hfault->Init.dev_i2c_addr, Reg, 1, rx, 3, HAL_MAX_DELAY);
	if(status!=HAL_OK)
	{
		return status;
	}
	*pData = ((uint32_t)rx[2]<<16|(uint32_t)rx[1]<<8|(uint32_t)rx[0]);
	return status;
}
/**
 * @brief  Calculates, programs, and caches the calibration settings for the INA237.
 * @details Validates the handle pointers, peripheral instance, device address, and user-defined
 *          shunt parameters (maximum expected current and shunt resistance). Reads the CONFIG
 *          register (00h) to determine the configured ADC range, calculates the appropriate
 *          15-bit SHUNT_CAL register value, and writes it over I2C. Upon a successful write,
 *          computes and caches the runtime `_current_lsb` and `_power_lsb` values within the
 *          handle structure for subsequent telemetry calculations.
 *
 * @param[in,out] hfault Pointer to the INA237 handle structure containing device configuration,
 *                       I2C instance, and internal state variables. Acts as an input for device
 *                       settings and an output for caching calculated LSB values.
 *
 * @retval HAL_OK       Calibration value successfully calculated, written, and cached.
 * @retval HAL_ERROR    A pointer argument is NULL, device address is 0, shunt/current parameters
 *                      are invalid (<= 0.0f), calculated calibration value is out of the valid
 *                      range (1 to 32767), or an I2C communication error occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read or write operation timed out.
 */
HAL_StatusTypeDef INA237_Set_Calib(INA237_Handle_TypeDef_t *hfault)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	if ((hfault->Init.max_cur_exp_A <= 0.0f) || (hfault->Init.shunt_res_Ohm <= 0.0f))
	{
		return HAL_ERROR;
	}
	float cal_f;
	HAL_StatusTypeDef status;
	uint16_t reg;
	status = INA237_ReadReg(hfault,INA237_REG_CONFIG,&reg);
	if(status != HAL_OK)
	{
		return status;
	}
	if(reg & __INA237_ADCRANGE_BIT_MASK)
	{
		cal_f = (float)(__INA237_CALIB_DIVIDEND_ADC1 / ((hfault->Init.max_cur_exp_A / __INA237_CURRENT_LSB_DIVISOR) * hfault->Init.shunt_res_Ohm));
	}
	else
	{
		cal_f = (float)(__INA237_CALIB_DIVIDEND_ADC0 / ((hfault->Init.max_cur_exp_A / __INA237_CURRENT_LSB_DIVISOR) * hfault->Init.shunt_res_Ohm));
	}
	if ((cal_f < 1.0f) || (cal_f > 32767.0f))
	{
		return HAL_ERROR;
	}
	uint16_t cal = ((uint16_t)(cal_f + 0.5f)>>1);

	status = INA237_WriteReg(hfault,INA237_REG_SHUNT_CAL,cal);
	if(status != HAL_OK)
	{
		return status;
	}

	hfault->_current_lsb = hfault->Init.max_cur_exp_A / __INA237_CURRENT_LSB_DIVISOR;
	hfault->_power_lsb = __INA237_POWER_LSB_MULTIPLIER * (hfault->Init.max_cur_exp_A / __INA237_CURRENT_LSB_DIVISOR);

	return status;
}
/**
 * @brief  Reads and calculates the shunt voltage in volts from the INA237.
 * @details Validates the device handle pointers, peripheral instance, device address,
 *          and destination data pointer. Reads the CONFIG register (00h) to determine
 *          the configured ADC range setting, retrieves the 16-bit signed raw shunt
 *          voltage using a signed read operation, and applies the corresponding
 *          LSB scaling factor based on the active ADC range to compute the voltage in Volts.
 *
 * @param[in]  hfault Pointer to the INA237 handle structure containing device configuration
 *                    and I2C peripheral instance.
 * @param[out] pData  Pointer to a float variable where the calculated shunt voltage
 *                    (in Volts) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Shunt voltage successfully read and calculated.
 * @retval HAL_ERROR    A pointer argument is NULL, the target device address is 0,
 *                      or an I2C communication error occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read operation timed out.
 */
HAL_StatusTypeDef INA237_Get_Shunt_Vltg_V(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint16_t reg;
	int16_t shnt_vlt_raw;
	HAL_StatusTypeDef status;
	status =  INA237_ReadReg(hfault,INA237_REG_CONFIG,&reg);
	if(status != HAL_OK)
	{
		return status;
	}
	status =  INA237_ReadReg_Signed(hfault,INA237_REG_SHUNT_VLTG,&shnt_vlt_raw);
	if(status != HAL_OK)
	{
		return status;
	}
	if(reg & __INA237_ADCRANGE_BIT_MASK)
	{
		*pData = (float)(shnt_vlt_raw * __INA237_SHUNTVLTG_LSB_ADC_1);
	}
	else
	{
		*pData = (float)(shnt_vlt_raw * __INA237_SHUNTVLTG_LSB_ADC_0);
	}
	return status;
}
/**
 * @brief  Reads and calculates the bus voltage in volts from the INA237.
 * @details Validates the handle pointers, peripheral instance, target device address,
 *          and destination output buffer[cite: 1]. Reads the 16-bit signed raw bus voltage
 *          from the Bus Voltage Register over I2C using a signed read routine and multiplies
 *          it by the fixed bus voltage LSB scaling factor (__INA237_BUSVLTG_LSB) to compute
 *          the voltage in Volts.
 *
 * @param[in]  hfault Pointer to the INA237 handle structure containing device configuration
 *                    and the I2C peripheral instance[cite: 1].
 * @param[out] pData  Pointer to a float variable where the calculated bus voltage
 *                    (in Volts) will be stored[cite: 1]. Must not be NULL[cite: 1].
 *
 * @retval HAL_OK       Bus voltage successfully read and calculated[cite: 1].
 * @retval HAL_ERROR    A pointer argument is NULL, the target device address is 0,
 *                      or an I2C communication failure occurred[cite: 1].
 * @retval HAL_BUSY     The I2C peripheral is currently busy[cite: 1].
 * @retval HAL_TIMEOUT  The I2C read operation timed out[cite: 1].
 */
HAL_StatusTypeDef INA237_Get_Bus_Vltg_V(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	int16_t bus_vlt_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg_Signed(hfault,INA237_REG_BUS_VLTG,&bus_vlt_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	*pData = (float)(bus_vlt_raw * __INA237_BUSVLTG_LSB);

	return status;
}
/**
 * @brief  Reads and calculates the internal die temperature in degrees Celsius from the INA237.
 * @details Validates the handle pointers, peripheral instance, target device address,
 *          and destination output buffer. Reads the 16-bit signed raw temperature value from
 *          the DIETEMP register (06h) over I2C using a signed read routine, shifts right to
 *          align the temperature bits, and multiplies by the temperature LSB factor
 *          (__INA237_TEMP_LSB) to calculate the temperature in degrees Celsius.
 *
 * @param[in]  hfault Pointer to the INA237 handle structure containing device configuration
 *                    and the I2C peripheral instance.
 * @param[out] pData  Pointer to a float variable where the calculated die temperature
 *                    (in °C) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Die temperature successfully read and calculated.
 * @retval HAL_ERROR    A pointer argument is NULL, the target device address is 0,
 *                      or an I2C communication failure occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read operation timed out.
 */
HAL_StatusTypeDef INA237_Get_Temp_C(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	int16_t die_temp_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg_Signed(hfault,INA237_REG_DIETEMP,&die_temp_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	*pData = (float)((die_temp_raw>>__INA237_DIETEMP_SHIFT) * __INA237_TEMP_LSB);

	return status;
}
/**
 * @brief  Reads and calculates the current in amperes from the INA237.
 * @details Validates the handle pointers, peripheral instance, target device address,
 *          and destination output buffer pointer. Reads the 16-bit signed raw current
 *          value from the Current Register (07h) over I2C using a signed read routine
 *          and multiplies it by the cached `_current_lsb` value to calculate the
 *          current in Amperes.
 *
 * @param[in]  hfault Pointer to the INA237 handle structure containing device
 *                    configuration, I2C instance, and cached conversion scaling.
 * @param[out] pData  Pointer to a float variable where the calculated current
 *                    (in Amperes) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Current successfully read and calculated.
 * @retval HAL_ERROR    A pointer argument is NULL, the target device address is 0,
 *                      or an I2C communication failure occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read operation timed out.
 */
HAL_StatusTypeDef INA237_Get_Current_A(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	int16_t cur_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg_Signed(hfault,INA237_REG_CURRENT,&cur_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	*pData = (float)(cur_raw * hfault->_current_lsb);

	return status;
}
/**
 * @brief  Reads and calculates the power in Watts from the INA237.
 * @details Validates the handle pointers, peripheral instance, target device address,
 *          and destination data pointer. Reads the 24-bit raw power value from the
 *          Power Register (08h) over I2C using a 24-bit read operation and multiplies
 *          it by the cached handle scaling factor (`_power_lsb`) to calculate the
 *          power in Watts.
 *
 * @param[in]  hfault Pointer to the INA237 handle structure containing device
 *                    configuration, I2C instance, and cached conversion scaling.
 * @param[out] pData  Pointer to a float variable where the calculated power
 *                    (in Watts) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Power successfully read and calculated.
 * @retval HAL_ERROR    A pointer argument is NULL, the target device address is 0,
 *                      or an I2C communication failure occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read operation timed out.
 */
HAL_StatusTypeDef INA237_Get_Power_W(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint32_t power_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg_24(hfault,INA237_REG_POWER,&power_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	*pData = (float)(power_raw * hfault->_power_lsb);

	return status;
}
/**
 * @brief  Sets a threshold limit value for a specified INA237 alert register over I2C.
 * @details Validates the handle pointer, peripheral instance, and device address.
 *          Validates the input limit value against the specific register's bit-width
 *          and sign constraints (e.g., 15-bit limits for SOVL, BOVL, BUVL; 16-bit
 *          for SUVL; 11-bit for TEMP_LIMIT), applies necessary bit-shifting for
 *          temperature alignment, and writes the value to the device over I2C.
 *
 * @param[in] hfault Pointer to the INA237 handle structure containing device configuration
 *                   and I2C peripheral instance.
 * @param[in] Reg    Target limit register to configure (type ::INA237_Register_t).
 *                   Must be one of the dedicated limit registers (SOVL, SUVL, BOVL,
 *                   BUVL, TEMP_LIMIT, or PWR_LIMIT).
 * @param[in] Val    16-bit raw threshold limit value to write into the register.
 *
 * @retval HAL_OK       Alert limit value successfully validated and written.
 * @retval HAL_ERROR    A pointer argument is NULL, the target device address is 0,
 *                      the limit value exceeds the register range, an invalid register
 *                      was selected, or an I2C communication error occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C write operation timed out.
 */
HAL_StatusTypeDef INA237_Set_Alert_Val(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t Val)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	switch(Reg)
	{
	case INA237_REG_SOVL:
		if(Val > 0x7FFFU)
		{
			return HAL_ERROR;
		}
		return INA237_WriteReg(hfault, INA237_REG_SOVL, Val);
		break;
	case INA237_REG_SUVL:
		if(Val > 0x8000U)
		{
			return HAL_ERROR;
		}
		return INA237_WriteReg(hfault, INA237_REG_SUVL, Val);
		break;
	case INA237_REG_BOVL:
		if(Val > 0x7FFFU)
		{
			return HAL_ERROR;
		}
		return INA237_WriteReg(hfault, INA237_REG_BOVL, Val);
		break;
	case INA237_REG_BUVL:
		if(Val > 0x7FFFU)
		{
			return HAL_ERROR;
		}
		return INA237_WriteReg(hfault, INA237_REG_BUVL, Val);
		break;
	case INA237_REG_TEMP_LIMIT:
		if(Val > 0x7FFU)
		{
			return HAL_ERROR;
		}
		uint16_t tx = Val << __INA237_TOL_POS;
		return INA237_WriteReg(hfault, INA237_REG_TEMP_LIMIT, tx);
		break;
	case INA237_REG_PWR_LIMIT:
		Val = Val * __INA237_POWER_LIMIT_MULTIPLIER;
		return INA237_WriteReg(hfault, INA237_REG_PWR_LIMIT, Val);
		break;
	default:
		return HAL_ERROR; // Wrong Register was used to set Limit Value
		/* Use Only the Limit Setting Registers*/
	}
}
/**
 * @brief  Configures or toggles individual alert/diagnostic bits in the INA237 DIAG_ALRT register.
 * @details Validates the handle pointer, peripheral instance, device address, bit position,
 *          and action state. Reads the current 16-bit DIAG_ALRT register (0Bh) over I2C,
 *          performs a read-modify-write operation to enable (set) or disable (clear) the
 *          specified alert/diagnostic bit, and writes the updated configuration back to the device.
 *
 * @param[in] hfault             Pointer to the INA237 handle structure containing device configuration
 *                               and the I2C peripheral instance.
 * @param[in] _Ina237_Diag_Alrt  Target configuration bit position to modify (type ::INA237_Diag_Alert_Config_t).
 *                               Must be between _INA237_DIAG_ALRT_APOL and _INA237_DIAG_ALRT_ALATCH.
 * @param[in] En_Di              Action to perform: ENABLE (1) to set the bit or DISABLE (0) to clear it.
 *
 * @retval HAL_OK       DIAG_ALRT register bit successfully updated over I2C.
 * @retval HAL_ERROR    A pointer argument is NULL, the target device address is 0,
 *                      the bit position is out of valid range, En_Di is invalid,
 *                      or an I2C communication error occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read or write operation timed out.
 */
HAL_StatusTypeDef INA237_Set_Diag_Alert_Config(INA237_Handle_TypeDef_t *hfault,INA237_Diag_Alert_Config_t _Ina237_Diag_Alrt,uint8_t En_Di)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 ||
			_Ina237_Diag_Alrt > _INA237_DIAG_ALRT_ALATCH || _Ina237_Diag_Alrt < _INA237_DIAG_ALRT_APOL)
	{
		return HAL_ERROR;
	}
	uint16_t tx;
	HAL_StatusTypeDef status;
	if(En_Di > ENABLE)
	{
		return HAL_ERROR;
	}
	status = INA237_ReadReg(hfault, INA237_REG_DIAG_ALRT,&tx);
	if(status != HAL_OK)
	{
		return status;
	}
	if(En_Di == DISABLE)
	{
		tx &= ~(1U << _Ina237_Diag_Alrt);
		status = INA237_WriteReg(hfault, INA237_REG_DIAG_ALRT, tx);
		if(status != HAL_OK)
		{
			return status;
		}
	}
	else
	{
		tx |= (1U << _Ina237_Diag_Alrt);
		status = INA237_WriteReg(hfault, INA237_REG_DIAG_ALRT, tx);
		if(status != HAL_OK)
		{
			return status;
		}
	}
	return status;
}
/**
 * @brief  Retrieves the status of a specific diagnostic or alert flag from the INA237.
 * @details Validates the handle pointer, I2C instance, device address, and flag parameter.
 *          Reads the 16-bit DIAG_ALRT register (0Bh) over I2C and evaluates whether the
 *          requested alert or diagnostic bit is set.
 *
 * @param[in] hfault             Pointer to the INA237 handle structure containing device
 *                               configuration and the I2C peripheral instance.
 * @param[in] Ina237_Diag_Alrt   Target diagnostic/alert flag to evaluate
 *                               (type ::INA237_Diag_Alert_t). Must not exceed
 *                               INA237_DIAG_ALRT_ALATCH.
 *
 * @retval 0x00       The specified flag is cleared (inactive).
 * @retval Non-zero   The bitmask corresponding to the asserted flag bit (active).
 * @retval HAL_ERROR  A pointer argument is NULL, the device address is 0, or the flag
 *                    parameter is out of range.
 * @retval 0x99       An I2C communication error occurred while reading the register.
 */
uint8_t INA237_Get_Diag_Alert_Flag(INA237_Handle_TypeDef_t *hfault,INA237_Diag_Alert_t Ina237_Diag_Alrt)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 ||
			Ina237_Diag_Alrt > INA237_DIAG_ALRT_ALATCH)
	{
		return HAL_ERROR;
	}
	uint16_t diag_alert;
	if(INA237_ReadReg(hfault, INA237_REG_DIAG_ALRT,&diag_alert) != HAL_OK)
	{
		return 0x99U;
	}
	return ((diag_alert & (1U << Ina237_Diag_Alrt)));
}
