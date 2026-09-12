/*
 * INA237.c
 *
 *  Created on: Sep 4, 2026
 *      Author: SRIVATHSJAYAN
 */
#include "INA237.h"

static HAL_StatusTypeDef INA237_WriteReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t Data);
static HAL_StatusTypeDef INA237_ReadReg_24(INA237_Handle_TypeDef_t *hfault,INA237_Register_t reg,uint8_t *pData);

/**
 * @brief  Resets the INA237 device.
 * @details Sets the reset bit in the INA237 CONFIG register, causing the
 *          device to reset and restore its registers to their default values.
 *
 * @param[in] hfault INA237 handle structure.
 *
 * @retval HAL_OK    Reset command transmitted successfully.
 * @retval HAL_ERROR I2C communication failure.
 */
HAL_StatusTypeDef INA237_Reset(INA237_Handle_TypeDef_t *hfault)
{
	uint16_t reset_bit = 0x01 << __INA237_RST_POS;

	return INA237_WriteReg(hfault, INA237_REG_CONFIG,reset_bit);
}
/**
 * @brief  Writes a 16-bit value to an INA237 register.
 * @details Sends the target 8-bit register address followed by the 16-bit
 *          register data (MSB first) over the I2C interface.
 *
 * @param[in] hfault Handle structure containing I2C configuration and peripheral instance.
 * @param[in] Reg    The 8-bit INA237 register address/command (type ::INA237_Register_t).
 * @param[in] Data   16-bit data value to be written to the register.
 *
 * @retval HAL_OK       Register write completed successfully.
 * @retval HAL_ERROR    I2C communication failure or hardware error.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
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
	if(hfault->Init.op_mode == INA237_OP_MODE_PWR_DWN_0)
	{
		return HAL_ERROR;
	}
	if(hfault->Init.op_mode == INA237_OP_MODE_PWR_DWN_8)
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
 * @brief  Writes a 16-bit value to an INA237 register.
 * @details Sends the target 8-bit register address followed by the 16-bit 
 *          register data (MSB first) over the I2C interface.
 *
 * @param[in] hfault Handle structure containing I2C configuration and peripheral instance.
 * @param[in] Reg    The 8-bit INA237 register address/command (type ::INA237_Register_t).
 * @param[in] Data   16-bit data value to be written to the register.
 * @retval HAL_OK       Register write completed successfully.
 * @retval HAL_ERROR    I2C communication failure or hardware error.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
static HAL_StatusTypeDef INA237_WriteReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t Data)
{
	HAL_StatusTypeDef status;
	uint8_t tx[3];
	tx[0] = (uint8_t)Reg;
	tx[1] = (uint8_t)(Data>>8);
	tx[2] = (uint8_t)Data;
	status =  HAL_I2C_Master_Transmit(hfault->hi2c,hfault->Init.dev_i2c_addr,tx, 3, HAL_MAX_DELAY);
	return status;
}
/**
 * @brief  Reads a 16-bit (2-byte) register from an INA237 device.
 *
 * @details Sends the target 8-bit register address, waits for 5 ms,
 *          and receives 2 bytes of register data into the provided buffer.
 *
 * @param[in]  hfault Handle structure containing I2C settings and instance.
 * @param[in]  reg    The 8-bit INA237 register address/command (type ::INA237_Register_t).
 * @param[out] pData  Pointer to a buffer of at least 2 bytes where the
 *                    received data will be written (MSB first). Must not be NULL.
 *
 * @retval HAL_OK       Register read completed successfully.
 * @retval HAL_ERROR    pData is NULL or an I2C communication failure occurred.
 * @retval HAL_BUSY     I2C peripheral is busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA237_ReadReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint8_t *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t reg_addr = (uint8_t)Reg;
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Transmit(hfault->hi2c,hfault->Init.dev_i2c_addr, &reg_addr, 1, HAL_MAX_DELAY);
	HAL_Delay(5);
	if(status != HAL_OK)
	{
		return status;
	}
	status = HAL_I2C_Master_Receive(hfault->hi2c,hfault->Init.dev_i2c_addr, pData, 2, HAL_MAX_DELAY);

	return status;
}
/**
 * @brief  Reads a 24-bit register from the INA237 sensor over I2C.
 * @details Transmits the 8-bit target register address, waits for a brief
 *          stabilization delay, and then reads 3 consecutive bytes (24 bits)
 *          into the provided buffer.
 *
 * @param[in]  hfault Handle structure containing the I2C configuration and peripheral instance.
 * @param[in]  reg    The 8-bit register address/command to read from (type ::INA237_Register_t).
 * @param[out] pData  Pointer to a 3-byte array where the received 24-bit raw register
 *                    data will be stored (MSB first). Must not be NULL.
 *
 * @retval HAL_OK       Register read completed successfully.
 * @retval HAL_ERROR    pData pointer is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
static HAL_StatusTypeDef INA237_ReadReg_24(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint8_t *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t reg_addr = (uint8_t)Reg;
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Transmit(hfault->hi2c,hfault->Init.dev_i2c_addr, &reg_addr, 1, HAL_MAX_DELAY);
	HAL_Delay(5);
	if(status != HAL_OK)
	{
		return status;
	}
	status = HAL_I2C_Master_Receive(hfault->hi2c,hfault->Init.dev_i2c_addr, pData, 3, HAL_MAX_DELAY);

	return status;
}
/**
 * @brief  Calculates and writes the calibration register value for the INA237.
 * @details Computes the 16-bit SHUNT_CAL value using the user-provided maximum expected
 *          current and shunt resistance, then writes it to the INA237_REG_SHUNT_CAL register.
 *
 * @param[in] hfault Handle structure containing INA237 settings and peripheral configuration.
 *
 * @retval HAL_OK       Calibration register successfully calculated and written.
 * @retval HAL_ERROR    Invalid shunt parameters (<= 0) or calculated calibration
 *                      value is out of valid range (1 to 32767).
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
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
	cal_f = (float)((819.2f * 1000000) / ((hfault->Init.max_cur_exp_A / 32768.0f) * hfault->Init.shunt_res_Ohm));

	if ((cal_f < 1.0f) || (cal_f > 32767.0f))
	{
		return HAL_ERROR;
	}
	uint16_t cal = (uint16_t)(cal_f + 0.5f);

	status = INA237_WriteReg(hfault,INA237_REG_SHUNT_CAL,cal);
	if(status != HAL_OK)
	{
		return status;
	}

	hfault->_current_lsb = hfault->Init.max_cur_exp_A / 32768.0f;
	hfault->_power_lsb = 0.2f * (hfault->Init.max_cur_exp_A / 32768.0f);

	return status;
}
/**
 * @brief  Reads and calculates the shunt voltage in volts from the INA237 sensor.
 * @details Checks the configured ADC voltage range in the ADC_CONFIG register,
 *          reads the raw 16-bit shunt voltage register, and applies the corresponding
 *          conversion factor (1.25 uV/LSB or 5 uV/LSB) to calculate the voltage.
 *
 * @param[in]  hfault INA237 handle structure containing the I2C configuration.
 * @param[out] pData  Pointer to a float variable where the calculated shunt voltage
 *                    (in Volts) will be written. Must not be NULL.
 *
 * @retval HAL_OK       Shunt voltage successfully read and calculated.
 * @retval HAL_ERROR    pData pointer is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA237_Get_Shunt_Vltg_V(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t rx[2];
	int16_t shnt_vlt_raw;
	double shnt_cnv_fact = 0.0;
	HAL_StatusTypeDef status;
	status =  INA237_ReadReg(hfault,INA237_REG_ADC_CONFIG,rx);
	if(status != HAL_OK)
	{
		return status;
	}

	if((rx[1U] & (1U <<__INA237_ADCRANGE_POS)) == 0)
	{
		shnt_cnv_fact = 5e-6f;
	}
	else
	{
		shnt_cnv_fact = 1.25e-6f;
	}
	status =  INA237_ReadReg(hfault,INA237_REG_SHUNT_VLTG,rx);
	if(status != HAL_OK)
	{
		return status;
	}
	shnt_vlt_raw = ((uint16_t)rx[0] << 8) | rx[1];

	*pData = (shnt_vlt_raw * shnt_cnv_fact);

	return status;
}
/**
 * @brief  Reads and calculates the bus voltage in volts from the INA237 sensor.
 * @details Reads the raw 16-bit bus voltage register over I2C and converts it
 *          to volts using the fixed 3.125 mV/LSB scaling factor.
 *
 * @param[in]  hfault Handle structure containing I2C configuration and peripheral instance.
 * @param[out] pData  Pointer to a float variable where the calculated bus voltage
 *                    (in Volts) will be written. Must not be NULL.
 *
 * @retval HAL_OK       Bus voltage successfully read and calculated.
 * @retval HAL_ERROR    pData pointer is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA237_Get_Bus_Vltg_V(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t rx[2];
	uint16_t bus_vlt_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg(hfault,INA237_REG_BUS_VLTG,rx);
	if(status != HAL_OK)
	{
		return status;
	}

	bus_vlt_raw = ((uint16_t)rx[0] << 8) | rx[1];

	*pData = (bus_vlt_raw * 3.125e-3f);

	return status;
}
/**
 * @brief  Reads and calculates the die temperature in degrees Celsius from the INA237 sensor.
 * @details Reads the raw 16-bit die temperature register over I2C and converts
 *          it to degrees Celsius using the fixed 125 m°C/LSB (0.125 °C/LSB)
 *          scaling factor.
 *
 * @param[in]  hfault Handle structure containing I2C configuration and peripheral instance.
 * @param[out] pData  Pointer to a float variable where the calculated die
 *                    temperature (in °C) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Die temperature successfully read and calculated.
 * @retval HAL_ERROR    pData pointer is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA237_Get_Temp_C(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t rx[2];
	uint16_t die_temp_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg(hfault,INA237_REG_DIETEMP,rx);
	if(status != HAL_OK)
	{
		return status;
	}

	die_temp_raw = (uint16_t)rx[0] << 8 | rx[1];

	float die_temp = (die_temp_raw * 125e-3f);

	*pData = die_temp;

	return status;
}
/**
 * @brief  Reads and calculates the current in Amperes from the INA237 sensor.
 * @details Reads the raw 16-bit signed current register over I2C and multiplies
 *          it by the calculated Current_LSB (max_cur_exp_A / 32768.0) to obtain
 *          the value in Amperes.
 *
 * @param[in]  hfault Handle structure containing INA237 configuration and I2C instance.
 * @param[out] pData  Pointer to a float variable where the calculated current
 *                    (in Amperes) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Current value successfully read and calculated.
 * @retval HAL_ERROR    pData pointer is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA237_Get_Current_A(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t rx[2];
	int16_t cur_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg(hfault,INA237_REG_CURRENT,rx);
	if(status != HAL_OK)
	{
		return status;
	}
	cur_raw = ((uint16_t)rx[0] << 8) | rx[1];

	*pData = (float)cur_raw * hfault->_current_lsb;

	return status;
}
/**
 * @brief  Reads and calculates the power in Watts from the INA237 sensor.
 * @details Reads the 24-bit raw power register over I2C and calculates the
 *          actual power value using the configured maximum expected current
 *          (Power LSB = 0.2 * Current LSB).
 *
 * @param[in]  hfault INA237 handle structure containing configuration settings.
 * @param[out] pData  Pointer to a float variable where the calculated power
 *                    (in Watts) will be stored. Must not be NULL.
 *
 * @retval HAL_OK       Power value successfully read and calculated.
 * @retval HAL_ERROR    pData pointer is NULL or an I2C communication error occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
 */
HAL_StatusTypeDef INA237_Get_Power_W(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint8_t rx[3];
	uint32_t power_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg_24(hfault,INA237_REG_POWER,rx);
	if(status != HAL_OK)
	{
		return status;
	}
	power_raw = ((uint32_t)rx[0] << 16) | ((uint32_t)rx[1] << 8) | (uint32_t)rx[2];

	float power = (float)power_raw * hfault->_power_lsb;

	*pData = power;

	return status;
}
/**
 * @brief  Sets a threshold limit value for an INA237 alert register.
 * @details Validates the input limit value based on the specified register's
 *          bit-width and range constraints, applies required bit-shifting,
 *          and writes the value to the target INA237 alert register.
 *
 * @param[in] hfault INA237 handle structure containing I2C configuration.
 * @param[in] Reg    The alert/limit register to update (type ::INA237_Register_t).
 *                   Must be a valid limit register (e.g., SOVL, SUVL, BOVL,
 *                   BUVL, TEMP_LIMIT, or PWR_LIMIT).
 * @param[in] Val    16-bit raw limit value to be written.
 *
 * @retval HAL_OK       Alert limit value successfully validated and written.
 * @retval HAL_ERROR    Invalid limit value for the specified register, an invalid
 *                      register was provided, or an I2C communication failure occurred.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
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
		return INA237_WriteReg(hfault, INA237_REG_PWR_LIMIT, Val);
		break;
	default:
		return HAL_ERROR; // Wrong Register was used to set Limit Value
		/* Use Only the Limit Setting Registers*/
	}
}
/**
 * @brief  Configures or toggles individual bits in the INA237 DIAG_ALRT register.
 * @details Reads the current 16-bit DIAG_ALRT register, validates the bit position
 *          and state, modifies the specified bit (ENABLE or DISABLE), and writes
 *          the updated configuration back over I2C.
 *
 * @param[in] hfault  Handle structure containing INA237 configuration and I2C instance.
 * @param[in] Bit_Pos Target bit position/mask to modify (type ::INA237_Diag_Alert_Config_t).
 *                    Must be within the valid range (12 to 16).
 * @param[in] En_Di   Action to perform: ENABLE (1) to set the bit or DISABLE (0) to clear it.
 *
 * @retval HAL_OK       Diagnostic/Alert register bit successfully modified and written.
 * @retval HAL_ERROR    Invalid bit position, invalid En_Di value, or an I2C communication failure.
 * @retval HAL_BUSY     I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  I2C transfer timed out.
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
	status = INA237_ReadReg(hfault, INA237_REG_DIAG_ALRT, (uint8_t*)&tx);
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
 * @details Reads the 16-bit DIAG_ALRT register over I2C and checks whether the
 *          specified flag bit is set.
 *
 * @param[in] hfault Handle structure containing INA237 configuration and I2C instance.
 * @param[in] Flag   Target diagnostic/alert flag to evaluate (type ::INA237_Diag_Alert_t).
 *
 * @retval 1      The specified alert/diagnostic flag is set (active).
 * @retval 0      The specified alert/diagnostic flag is cleared (inactive).
 * @retval 0x99   An I2C communication error occurred while reading the register.
 */
uint8_t INA237_Get_Diag_Alert_Flag(INA237_Handle_TypeDef_t *hfault,INA237_Diag_Alert_t Ina237_Diag_Alrt)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 ||
			Ina237_Diag_Alrt > INA237_DIAG_ALRT_ALATCH)
	{
		return HAL_ERROR;
	}
	uint16_t diag_alert;
	if(INA237_ReadReg(hfault, INA237_REG_DIAG_ALRT, (uint8_t*)&diag_alert) != HAL_OK)
	{
		return 0x99U;
	}
	return ((diag_alert & (1U << Ina237_Diag_Alrt)) == 1U);
}
