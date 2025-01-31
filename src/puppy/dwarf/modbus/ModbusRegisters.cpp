#include <cstring>
#include <cstdlib>
#include <array>

#include "ModbusRegisters.hpp"

#include "timing.h"
#include "loadcell.hpp"
#include "PuppyConfig.hpp"
#include "otp.h"
#include "utility_extensions.hpp"

namespace dwarf::ModbusRegisters {

const uint32_t MIN_SystemDiscreteInput = (ftrstd::to_underlying(SystemDiscreteInput::is_picked));
const uint32_t MAX_SystemDiscreteInput = (ftrstd::to_underlying(SystemDiscreteInput::is_parked));

const uint32_t MIN_SystemCoil = (ftrstd::to_underlying(SystemCoil::tmc_enable));
const uint32_t MAX_SystemCoil = (ftrstd::to_underlying(SystemCoil::is_selected));

const uint32_t MIN_SystemInputRegister = (ftrstd::to_underlying(SystemInputRegister::hw_bom_id));
const uint32_t MAX_SystemInputRegister = (ftrstd::to_underlying(SystemInputRegister::marlin_error_message_end));

const uint32_t MIN_SystemHoldingRegister = (ftrstd::to_underlying(SystemHoldingRegister::nozzle_target_temperature));
const uint32_t MAX_SystemHoldingRegister = (ftrstd::to_underlying(SystemHoldingRegister::tmc_write_request_value_2));

static uint16_t s_SystemDiscreteInputs[MAX_SystemDiscreteInput - MIN_SystemDiscreteInput + 1];
static uint16_t s_SystemCoils[MAX_SystemCoil - MIN_SystemCoil + 1];
static uint16_t s_SystemInputRegisters[MAX_SystemInputRegister - MIN_SystemInputRegister + 1];
static uint16_t s_SystemHoldingRegisters[MAX_SystemHoldingRegister - MIN_SystemHoldingRegister + 1];

void Init() {
    //clear registers
    memset(s_SystemDiscreteInputs, 0, sizeof(s_SystemDiscreteInputs));
    memset(s_SystemCoils, 0, sizeof(s_SystemCoils));
    memset(s_SystemInputRegisters, 0, sizeof(s_SystemInputRegisters));
    memset(s_SystemHoldingRegisters, 0, sizeof(s_SystemHoldingRegisters));

    //add register blocks
    AddBlock(BlockType::DiscreteInput, s_SystemDiscreteInputs, MIN_SystemDiscreteInput, MAX_SystemDiscreteInput - MIN_SystemDiscreteInput + 1);
    AddBlock(BlockType::Coil, s_SystemCoils, MIN_SystemCoil, MAX_SystemCoil - MIN_SystemCoil + 1);
    AddBlock(BlockType::InputRegister, s_SystemInputRegisters, MIN_SystemInputRegister, MAX_SystemInputRegister - MIN_SystemInputRegister + 1);
    AddBlock(BlockType::HoldingRegister, s_SystemHoldingRegisters, MIN_SystemHoldingRegister, MAX_SystemHoldingRegister - MIN_SystemHoldingRegister + 1);

    //init registers from OTP
    serial_nr_t sn;     // Serial number = raw datamatrix
    uint32_t timestamp; // Unix timestamp, seconds since 1970
    uint8_t bom_id;
    timestamp = otp_get_timestamp();
    if ((otp_get_serial_nr(&sn) != sizeof(sn.txt)) || (otp_get_bom_id(&bom_id) == false)) {

        // Default to zero
        timestamp = 0;
        bom_id = 0;
        memset(sn.txt, 0, sizeof(sn.txt));
    }

    SetInputRegisterValue(ftrstd::to_underlying(SystemInputRegister::hw_bom_id), bom_id);
    SetInputRegisterValue(ftrstd::to_underlying(SystemInputRegister::hw_otp_timestamp_0), timestamp & 0xFFFF);
    SetInputRegisterValue(ftrstd::to_underlying(SystemInputRegister::hw_otp_timestamp_1), timestamp >> 16);

    static constexpr uint16_t raw_datamatrix_regsize = ftrstd::to_underlying(SystemInputRegister::hw_raw_datamatrix_last)
        - ftrstd::to_underlying(SystemInputRegister::hw_raw_datamatrix_first) + 1;
    // Check size of text -1 as the terminating \0 is not sent
    static_assert((raw_datamatrix_regsize * sizeof(uint16_t)) == (sizeof(sn.txt) - 1), "Size of raw datamatrix doesn't fit modbus registers");

    for (uint16_t i = 0; i < raw_datamatrix_regsize; i++) {
        uint16_t word = sn.txt[2 * i] | (sn.txt[2 * i + 1] << 8);
        SetInputRegisterValue(ftrstd::to_underlying(SystemInputRegister::hw_raw_datamatrix_first) + i, word);
    }
}

void SetBitValue(SystemDiscreteInput reg, bool value) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    SetDiscreteInputValue(regAddress, value);
}

void SetBitValue(SystemCoil reg, bool value) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    SetCoilValue(regAddress, value);
}

void SetRegValue(SystemInputRegister reg, uint16_t value) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    SetInputRegisterValue(regAddress, value);
}

void SetRegValue(SystemHoldingRegister reg, uint16_t value) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    SetHoldingRegisterValue(regAddress, value);
}

uint16_t GetRegValue(SystemInputRegister reg) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    uint16_t value = 0;
    GetInputRegisterValue(regAddress, &value);
    return value;
}

uint16_t GetRegValue(SystemHoldingRegister reg) {
    uint16_t regAddress = ftrstd::to_underlying(reg);
    uint16_t value = 0;
    GetHoldingRegisterValue(regAddress, &value);
    return value;
}

} //namespace
