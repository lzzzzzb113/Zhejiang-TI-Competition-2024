#include "SignalProcess.h"
#include "bsp_ADC.h"
#include "FFTInc.h"
#include "uartstdio.h"
#include "wind.h"
#include "LCD1602.h"

#define czk_VA_Switch 0 // 0是电流sA通道计算 1是电压sB通道计算

#define Show_Current_Channel 0
#define Show_Voltage_Channel 1
#define Show_ActivePower_Channel 2
#define Show_PowerFactor_Channel 3
#define Show_THD_Channel 4

extern uint16_t tem_THD;
extern uint8_t thdbuff_i;
extern uint16_t THDx;
extern uint32_t set_frq;

extern uint8_t ADC_Cnt;

float Show_filter(float Value, uint8_t Channel);

static float Current_K_Big = 0.0085f;
static float Current_B_Big = -0.0158f;

static float Current_K_Mid = 0.0084f;
static float Current_B_Mid = -0.0056f;

static float Current_K_Small = 0.0084f;  // todo 用的Mid的参数
static float Current_B_Small = -0.0056f; // todo 用的Mid的参数

static float Voltage_K_Small = 0.3872f;
static float Voltage_B_Small = -4.2811f;

static float Voltage_K_Big = 0.5317f;
static float Voltage_B_Big = -88.361f;

static float Power_K = 0.0032f;
static float Power_B = 2.5162f;

static float FFT_Compensate[10] = {
    1.000f * 0.0080971f,
    1.04661f * 0.0080971f,
    1.09292f * 0.0080971f,
    1.204878f * 0.0080971f,
    1.204878f * 0.0080971f,
    1.204878f * 0.0080971f,
    1.204878f * 0.0080971f,
    1.204878f * 0.0080971f,
    1.204878f * 0.0080971f,
    1.204878f * 0.0080971f};

/*  N值可根据实际情况调整
23.    排序采用冒泡法*/
#define Filter_N 4
uint16_t THDbuffer[Filter_N];   // 存放未排序的THD值
uint16_t THDbuffer_2[Filter_N]; // 存放排序后的THD值
uint16_t Mid_filter(void)
{
    uint8_t i, j, count;
    uint16_t temp, sum;

    memcpy(THDbuffer_2, THDbuffer, Filter_N * sizeof(uint16_t));
    for (i = 0; i < Filter_N - 1; i++)
        for (j = 0; j < Filter_N - 1 - i; j++)
            if (THDbuffer[j] > THDbuffer[j + 1])
            {
                temp = THDbuffer[j];
                THDbuffer[j] = THDbuffer[j + 1];
                THDbuffer[j + 1] = temp;
            }
    sum = 0;
    for (count = 1; count < Filter_N - 1; count++)
        sum += THDbuffer[count];
    return (uint16_t)(sum >> 1);
}

float Find_Max(uint16_t Index_L, uint16_t Index_H, uint16_t *Flag)
{
    float Max = 0;
    for (uint16_t i = Index_L; i <= Index_H; i++)
    {
#if czk_VA_Switch == 0
        if (sA[i].real > Max)
        {
            Max = sA[i].real;
            *Flag = i;
        }
#elif czk_VA_Switch == 1
        if (sB[i].real > Max)
        {
            Max = sB[i].real;
            *Flag = i;
        }
#endif
    }
    return Max;
}

// ptr_buf_in 输入数据指针
// len 输入数据长度
// ptr_THD 输出THD
// ptr_S_Frq 输出频率
uint16_t Index_H[Harmonic_Num_czk], Index_L[Harmonic_Num_czk], Index;

// 谐波次
float Tab_Harmonic[Harmonic_Num_czk][3];
void Calc_THD(float *ptr_buf_in, uint16_t len, uint16_t *ptr_THD, uint32_t *ptr_S_Frq)
{
    uint16_t i;
    // 谐波中心
    float tem_float;

    float THDx;
    int THD;

    for (i = 0; i < Harmonic_Num_czk; i++)
    {
        Tab_Harmonic[i][0] = 0;
        Tab_Harmonic[i][1] = 0;
        Tab_Harmonic[i][2] = 0;
    }

    for (uint8_t i = 0; i < Harmonic_Num_czk; i++)
    {
        float Linshi = (float)SampleRate / (NUM_SAMPLES);
        Index_H[i] = (float)(50 * (i + 1) + 10) / Linshi; // 70hz
        Index_L[i] = (float)(50 * (i + 1) - 10) / Linshi; // 30hz

        if (Find_Max(Index_L[i], Index_H[i], &Index) > 1.0f) // 50hz 为一个组，组里找最大的，最大值超过一定值就是基波
        {
            Tab_Harmonic[i][0] = (float)(Index);                     // 基波频率索引值
            Tab_Harmonic[i][1] = (float)Tab_Harmonic[i][0] * Linshi; // 基波真实频率
#if czk_VA_Switch == 0
            Tab_Harmonic[i][2] = sA[Index].real * WinCompensate; // 基波幅值
#elif czk_VA_Switch == 1
            Tab_Harmonic[i][2] = sB[Index].real * WinCompensate; // 基波幅值
#endif

            //            if(Tab_Harmonic[0][2] >1240 || Tab_Harmonic[0][2] <1220){
            //                __asm("BKPT 0");
            //            }
            uint16_t Freq = Index * Linshi;

            if (Freq < 1000)
            {
                if ((Freq % 10) >= 3 && (Freq % 10) <= 7)
                {
                    Tab_Harmonic[i][1] = (Freq / 10) * 10 + 5;
                }
                else if ((Freq % 10) <= 2)
                {
                    Tab_Harmonic[i][1] = (Freq / 10) * 10;
                }
                else if ((Freq % 10) >= 8)
                {
                    Tab_Harmonic[i][1] = (Freq / 10 + 1) * 10;
                }
            }
            else
            {
                Tab_Harmonic[i][1] = 1000;
            }
        }
    }

    for (i = 0; i < Harmonic_Num_czk; i++)
    {
        Tab_Harmonic[i][2] = Tab_Harmonic[i][2] * FFT_Compensate[i];
    }

    tem_float = 0;
    for (i = 1; i < Harmonic_Num_czk; i++)
    {
        tem_float = tem_float + Tab_Harmonic[i][2] * Tab_Harmonic[i][2];
    }

    THDx = sqrt(tem_float) / Tab_Harmonic[0][2];

    THD = THDx * 1000;

    *ptr_THD = THD;
    *ptr_S_Frq = (uint32_t)Tab_Harmonic[0][1];
}

// sA[i].real存储实部，sA[i].imag存储虚部
// 开始存储时域，经过FFT后存储频域
struct compx sA[NUM_SAMPLES];
struct compx sB[NUM_SAMPLES];
float FFT_Result[(NUM_SAMPLES >> 1)];

// 打印用
int32_t i32IPart[10];

// 视在功率，有功功率，功率因数
float ApparentPower = 0;
float ActivePower = 0;
float PowerFactor = 0;
float AverageVoltage = 0;
float AverageCurrent = 0;
float f32TempA = 0, f32TempB = 0, f32TempAll = 0;
float BufferA[NUM_SAMPLES], BufferB[NUM_SAMPLES];

extern void czk_GPIOPinWrite(uint32_t ui32Port, uint8_t ui8Pins, uint8_t ui8Val);
float TestCurrent = 0;
float TestVoltage = 0;

uint16_t Current_ZeroPoint_Index[2] = {0};
uint16_t Voltage_ZeroPoint_Index[2] = {0};
void Calc_Channl_FFTValue(void)
{
    uint32_t i;
    //    uint32_t MaxInputValue, MinInputValue;
    static uint8_t IO_A = 0;
    IO_A = 1 - IO_A;
    czk_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, IO_A);

        /* 计算平均值用于计算THD */
    f32TempA = f32TempB = 0;
    for (i = 0; i < NUM_SAMPLES; i++)
    {
        f32TempA += (float)dstBufferA[i];
        f32TempB += (float)dstBufferB[i];
    }
    if (ADC_Cnt < 3)
    {
        AverageCurrent = 1999;
        AverageVoltage = 2011;
    }


    /* 执行1024点浮点FFT运算 */
    for (i = 0; i < NUM_SAMPLES; i++)
    {
#if WinType_czk == 0
        sA[i].real = dstBufferA[i] * Vref / 4096;
        sA[i].imag = 0;
        sB[i].real = dstBufferB[i] * Vref / 4096;
        sB[i].imag = 0;
#elif WinType_czk == 1
        sA[i].real = (float)dstBufferA[i] * Vref / 4096 * flattop[i];
        sA[i].imag = 0;
        sB[i].real = (float)dstBufferB[i] * Vref / 4096 * flattop[i];
        sB[i].imag = 0;
#elif WinType_czk == 2
        sA[i].real = (float)(dstBufferA[i]-AverageCurrent) * Vref / 4096 * Hamming[i];
        sA[i].imag = 0;
        sB[i].real = (float)(dstBufferB[i]-AverageVoltage) * Vref / 4096 * Hamming[i];
        sB[i].imag = 0;
#endif
    }


    // 计算&存储有效值
    f32TempA = f32TempB = f32TempAll = 0;
    Current_ZeroPoint_Index[0] = Current_ZeroPoint_Index[1] = 0;
    for (i = 0; i < NUM_SAMPLES; i++)
    {
        BufferA[i] = (dstBufferA[i] - AverageCurrent);
        BufferB[i] = (dstBufferB[i] - AverageVoltage);

        f32TempA += (float)(BufferA[i] * BufferA[i]);
        f32TempB += (float)(BufferB[i] * BufferB[i]);
        f32TempAll += (float)(BufferA[i] * BufferB[i]);
    }

    f32TempA = (float)f32TempA / NUM_SAMPLES;
    f32TempB = (float)f32TempB / NUM_SAMPLES;
    czk_arm_sqrt_f32(f32TempA, &(f32TempA)); // 有效值
    czk_arm_sqrt_f32(f32TempB, &(f32TempB)); // 有效值

    /* 不限制点数实序列快速FFT, 浮点 */
    cfft(sA, NUM_SAMPLES);
    cfft(sB, NUM_SAMPLES);

    /* FFT 计算幅频 */
    for (i = 0; i < NUM_SAMPLES; i++)
    {
        czk_arm_sqrt_f32((float)(sA[i].real * sA[i].real + sA[i].imag * sA[i].imag), &sA[i].real);
        czk_arm_sqrt_f32((float)(sB[i].real * sB[i].real + sB[i].imag * sB[i].imag), &sB[i].real);
        sA[i].real = sA[i].real * WinCompensate;
        sB[i].real = sB[i].real * WinCompensate;
    }

    /* 计算THD */
    for (i = 0; i < (NUM_SAMPLES >> 1); i++)
    {
#if czk_VA_Switch == 0
        FFT_Result[i] = sA[i].real;
#elif czk_VA_Switch == 1
        FFT_Result[i] = sB[i].real;
#endif
    }

    Calc_THD(&(FFT_Result[0]), (NUM_SAMPLES >> 1), &tem_THD, &set_frq);

    // 滤波一下
    THDbuffer[thdbuff_i] = tem_THD;
    thdbuff_i++;

    if (thdbuff_i == Filter_N)
    {
        thdbuff_i = 0;
        THDx = Mid_filter();
    }

    // 电流校准
    if (f32TempA < 25) // 100mA
    {
        TestCurrent = (f32TempA)*Current_K_Small + Current_B_Small;
    }
    else if (f32TempA < 120) // 1A
    {
        TestCurrent = (f32TempA)*Current_K_Mid + Current_B_Mid;
    }
    else
    {
        TestCurrent = (f32TempA)*Current_K_Big + Current_B_Big;
    }
    TestCurrent = Show_filter(TestCurrent, Show_Current_Channel);

    // 电压校准
    if (f32TempB < 578) // 220V
    {
        TestVoltage = (f32TempB)*Voltage_K_Small + Voltage_B_Small;
    }
    else
    {
        TestVoltage = (f32TempB)*Voltage_K_Big + Voltage_B_Big;
    }
    TestVoltage = Show_filter(TestVoltage, Show_Voltage_Channel);

    ApparentPower = (float)(TestVoltage * TestCurrent); // 视在功率计算
    ActivePower = (float)f32TempAll / (NUM_SAMPLES);    // 有功功率计算
    ActivePower = Show_filter(ActivePower * Power_K + Power_B, Show_ActivePower_Channel);
    PowerFactor = (float)ActivePower / ApparentPower; // 相位计算
    if (PowerFactor > 1)
    {
        PowerFactor = 0.9999;
    }

#if START_czk_Log == 1

    for (i = 0; i < NUM_SAMPLES; i++)
    {
        i32IPart[0] = (int32_t)sA[i].real;
        i32IPart[1] = (int32_t)(sA[i].real * 1000.0f);
        i32IPart[1] = (int32_t)(i32IPart[1] - i32IPart[0] * 1000.0);
        UARTprintf("\r%3d.%03d", i32IPart[0], i32IPart[1]);
    }

    // // 电压互感器
    // i32IPart[0] = (int32_t)TestVoltage;
    // i32IPart[1] = (int32_t)(TestVoltage * 1000.0f);
    // i32IPart[1] = (int32_t)(i32IPart[1] - i32IPart[0] * 1000.0);

    // // 电流互感器
    // i32IPart[2] = (int32_t)TestCurrent;
    // i32IPart[3] = (int32_t)(TestCurrent * 1000.0f);
    // i32IPart[3] = (int32_t)(i32IPart[3] - i32IPart[2] * 1000.0);

    // // 有功功率
    // i32IPart[4] = (int32_t)ActivePower;
    // i32IPart[5] = (int32_t)(ActivePower * 1000.0f);
    // i32IPart[5] = (int32_t)(i32IPart[5] - i32IPart[4] * 1000.0);

    // // 功率因数
    // i32IPart[6] = (int32_t)PowerFactor;
    // i32IPart[7] = (int32_t)(PowerFactor * 1000.0f);
    // i32IPart[7] = (int32_t)(i32IPart[7] - i32IPart[6] * 1000.0);

    // // thd
    // i32IPart[8] = (int32_t)(THDx);
    // i32IPart[9] = (int32_t)(THDx * 1000.0f);
    // i32IPart[9] = (int32_t)(i32IPart[9] - i32IPart[8] * 1000.0);

    // // i32IPart[8] = (int32_t)Tab_Harmonic[0][2];
    // // i32IPart[9] = (int32_t)(Tab_Harmonic[0][2] * 1000.f);
    // // i32IPart[9] = (int32_t)(i32IPart[1] - i32IPart[0] * 1000);

    // UARTprintf("\r%3d.%03d,%3d.%03d,%3d.%03d,%3d.%03d,%3d.%02d", i32IPart[0], i32IPart[1], i32IPart[2], i32IPart[3], i32IPart[4], i32IPart[5], i32IPart[6], i32IPart[7], i32IPart[8], i32IPart[9]);

    // i32IPart[0] = (int32_t)Tab_Harmonic[0][2];
    // i32IPart[1] = (int32_t)(Tab_Harmonic[0][2] * 1000.f);
    // i32IPart[1] = (int32_t)(i32IPart[1] - i32IPart[0] * 1000);

    // i32IPart[2] = (int32_t)Tab_Harmonic[1][2];
    // i32IPart[3] = (int32_t)(Tab_Harmonic[1][2] * 1000.f);
    // i32IPart[3] = (int32_t)(i32IPart[3] - i32IPart[2] * 1000);

    // i32IPart[4] = (int32_t)Tab_Harmonic[2][2];
    // i32IPart[5] = (int32_t)(Tab_Harmonic[2][2] * 1000.f);
    // i32IPart[5] = (int32_t)(i32IPart[5] - i32IPart[4] * 1000);

    // i32IPart[6] = (int32_t)Tab_Harmonic[3][2];
    // i32IPart[7] = (int32_t)(Tab_Harmonic[0][2] * 1000.f);
    // i32IPart[7] = (int32_t)(i32IPart[7] - i32IPart[6] * 1000);

    // i32IPart[8] = (int32_t)(THDx);
    // i32IPart[9] = (int32_t)(THDx * 1000.0f);
    // i32IPart[9] = (int32_t)(i32IPart[9] - i32IPart[8] * 1000.0);

    // UARTprintf("\r%3d.%03d,%3d.%03d,%3d.%03d,%3d.%03d,%3d.%03d", i32IPart[0], i32IPart[1], i32IPart[2], i32IPart[3], i32IPart[4], i32IPart[5], i32IPart[6], i32IPart[7], i32IPart[8], i32IPart[9]);
#endif
    IO_A = 1 - IO_A;
    czk_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, IO_A);

    LCD_Epara.THD[0] = (uint16_t)(THDx);
    for (uint8_t i = 0; i < 10; i++)
    {
        LCD_Epara.THD[i + 1] = (uint16_t)(Tab_Harmonic[i][2] * 1000 / LCD_Epara.CT_Bn);
    }
    LCD_Epara.U = (uint16_t)(TestVoltage * 100);
    LCD_Epara.I = (uint16_t)(TestCurrent * 1000.0f / LCD_Epara.CT_Bn);
    LCD_Epara.P = (uint16_t)(ActivePower * 1000.0f / LCD_Epara.CT_Bn);
    LCD_Epara.PF = (uint16_t)(PowerFactor * 1000);
}

#define Filter_N 4
float ShowFilterBuffer[5][Filter_N] = {0};

float Show_filter(float Value, uint8_t Channel)
{
    static uint8_t i = 0;
    float Sum = 0;
    static float LastAve[5] = {0};
    if ((LastAve[Channel] * 0.9f) < Value && Value < (LastAve[Channel] * 1.1f))
    {
        LastAve[Channel] = Value;
    }
    else
    { // 输入值过大过小，直接覆盖
        for (uint8_t index = 0; index < Filter_N; index++)
        {
            ShowFilterBuffer[Channel][index] = Value;
        }
    }
    ShowFilterBuffer[Channel][i] = Value;
    i++;
    i = i % Filter_N;
    for (uint8_t index = 0; index < Filter_N; index++)
    {
        Sum += ShowFilterBuffer[Channel][index];
    }
    LastAve[Channel] = (float)(Sum / Filter_N);
    return LastAve[Channel];
}
