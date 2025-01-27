#include <furi.h>
#include <furi_hal.h>
#include <stm32wbxx_ll_adc.h>
#include <stm32wbxx_ll_pwr.h>

#define TAG "AdcTest"

typedef enum {
    FuriHalVref2048,
    FuriHalVref2500,
} FuriHalVref;

void furi_hal_adc_set_vref(FuriHalVref vref) {
    furi_assert(vref == FuriHalVref2048 || vref == FuriHalVref2500);
    uint32_t trim_value = 0;

    switch(vref) {
    case FuriHalVref2048:
        LL_VREFBUF_SetVoltageScaling(LL_VREFBUF_VOLTAGE_SCALE0);
        trim_value = LL_VREFBUF_SC0_GetCalibration() & 0x3FU;
        break;
    case FuriHalVref2500:
        LL_VREFBUF_SetVoltageScaling(LL_VREFBUF_VOLTAGE_SCALE1);
        trim_value = LL_VREFBUF_SC1_GetCalibration() & 0x3FU;
        break;
    }

    LL_VREFBUF_SetTrimming(trim_value);
}

void furi_hal_adc_init() {
    LL_VREFBUF_Enable();
    LL_VREFBUF_DisableHIZ();

    // FHCLK ≥ FADC / 4 if the resolution of all channels are 12-bit or 10-bit
    // STM32WB55xx ADC maximum frequency is 64MHz (corresponding to 4.27Msmp/s maximum)
    // TODO: calculate correct clock
    LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_CLOCK_SYNC_PCLK_DIV4);

    LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);
    LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT);
    LL_ADC_SetLowPowerMode(ADC1, LL_ADC_LP_MODE_NONE);

    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
    LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_DISABLE);
    LL_ADC_REG_SetSequencerDiscont(ADC1, LL_ADC_REG_SEQ_DISCONT_DISABLE);
    LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);
    LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_NONE);
    LL_ADC_REG_SetOverrun(ADC1, LL_ADC_REG_OVR_DATA_OVERWRITTEN);

    LL_ADC_SetOverSamplingScope(ADC1, LL_ADC_OVS_DISABLE);

    LL_ADC_DisableDeepPowerDown(ADC1);

    LL_ADC_EnableInternalRegulator(ADC1);
    furi_delay_us(LL_ADC_DELAY_INTERNAL_REGUL_STAB_US);

    LL_ADC_StartCalibration(ADC1, LL_ADC_SINGLE_ENDED);
    while(LL_ADC_IsCalibrationOnGoing(ADC1) != 0) {
        // TODO: add timeout
    }

    // TODO: calculate delay between ADC end of calibration and ADC enable
    furi_delay_us(100);
}

void furi_hal_adc_enable() {
    LL_ADC_Enable(ADC1);
    while(LL_ADC_IsActiveFlag_ADRDY(ADC1) == 0) {
        // TODO: add timeout
    }
}

void furi_hal_adc_disable() {
    LL_ADC_Disable(ADC1);
    while(LL_ADC_IsEnabled(ADC1)) {
        // TODO: add timeout
    }
}

void furi_hal_adc_deinit() {
    LL_ADC_DisableInternalRegulator(ADC1);
    LL_ADC_EnableDeepPowerDown(ADC1);
    LL_VREFBUF_EnableHIZ();
    LL_VREFBUF_Disable();
}

typedef enum {
    FuriHalAdcChannel0 = LL_ADC_CHANNEL_0,
    FuriHalAdcChannel1 = LL_ADC_CHANNEL_1,
    FuriHalAdcChannel2 = LL_ADC_CHANNEL_2,
    FuriHalAdcChannel3 = LL_ADC_CHANNEL_3,
    FuriHalAdcChannel4 = LL_ADC_CHANNEL_4,
    FuriHalAdcChannel5 = LL_ADC_CHANNEL_5,
    FuriHalAdcChannel6 = LL_ADC_CHANNEL_6,
    FuriHalAdcChannel7 = LL_ADC_CHANNEL_7,
    FuriHalAdcChannel8 = LL_ADC_CHANNEL_8,
    FuriHalAdcChannel9 = LL_ADC_CHANNEL_9,
    FuriHalAdcChannel10 = LL_ADC_CHANNEL_10,
    FuriHalAdcChannel11 = LL_ADC_CHANNEL_11,
    FuriHalAdcChannel12 = LL_ADC_CHANNEL_12,
    FuriHalAdcChannel13 = LL_ADC_CHANNEL_13,
    FuriHalAdcChannel14 = LL_ADC_CHANNEL_14,
    FuriHalAdcChannel15 = LL_ADC_CHANNEL_15,
    FuriHalAdcChannel16 = LL_ADC_CHANNEL_16,
    FuriHalAdcChannel17 = LL_ADC_CHANNEL_17,
    FuriHalAdcChannel18 = LL_ADC_CHANNEL_18,
    FuriHalAdcChannelVREFINT = LL_ADC_CHANNEL_VREFINT,
    FuriHalAdcChannelTEMPSENSOR = LL_ADC_CHANNEL_TEMPSENSOR,
    FuriHalAdcChannelVBAT = LL_ADC_CHANNEL_VBAT,
} FuriHalAdcChannel;

void furi_hal_adc_set_single_channel(FuriHalAdcChannel channel) {
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, channel);
    // TODO: calculate sampling time
    LL_ADC_SetChannelSamplingTime(ADC1, channel, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    LL_ADC_SetChannelSingleDiff(ADC1, channel, LL_ADC_SINGLE_ENDED);
}

uint32_t furi_hal_adc_read_sw() {
    LL_ADC_REG_StartConversion(ADC1);
    while(LL_ADC_IsActiveFlag_EOC(ADC1) == 0) {
    }
    return LL_ADC_REG_ReadConversionData12(ADC1);
}

int32_t connect_joystick() {
    furi_hal_gpio_init(&gpio_ext_pa7, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&gpio_ext_pa6, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    FURI_LOG_I(TAG, "gpio init success");
}

int32_t adc_example_app(void* p) {
    UNUSED(p);

    // PC3 is ADC1_IN4
    furi_hal_gpio_init(&gpio_ext_pc3, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    FURI_LOG_I(TAG, "Gpio Set OK");

    furi_hal_adc_init();
    FURI_LOG_I(TAG, "ADC Init OK");

    furi_hal_adc_set_vref(FuriHalVref2500);
    FURI_LOG_I(TAG, "Vref Set OK");

    furi_hal_adc_set_single_channel(FuriHalAdcChannel4);
    FURI_LOG_I(TAG, "ADC Set Channel OK");

    furi_hal_adc_enable();
    FURI_LOG_I(TAG, "ADC Enable OK");

    while(1) {
        uint32_t adc_value = furi_hal_adc_read_sw();
        float adc_voltage = 2.5f * (float)adc_value / 4096.0f;
        FURI_LOG_I(TAG, "ADC: %ld, %f V", adc_value, (double)adc_voltage);
        furi_delay_ms(10);
    }

    furi_hal_adc_disable();
    FURI_LOG_I(TAG, "ADC Disable OK");

    furi_hal_adc_deinit();
    FURI_LOG_I(TAG, "ADC Deinit OK");

    return 0;
}