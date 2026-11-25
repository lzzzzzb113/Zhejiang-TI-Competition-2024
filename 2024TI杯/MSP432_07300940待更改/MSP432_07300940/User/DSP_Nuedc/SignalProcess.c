#include "SignalProcess.h"
#include "bsp_ADC.h"
#include "FFTInc.h"
#include "uartstdio.h"
#include "wind.h"

extern uint16_t tem_THD;
extern uint8_t thdbuff_i;
extern uint16_t THDbuffer[11];
extern uint16_t THDx;
extern uint32_t set_frq;

/*  N值可根据实际情况调整
23.    排序采用冒泡法*/
#define Filter_N 6

uint16_t Mid_filter(void)
{
    uint8_t i, j, count;
    uint16_t temp, sum;

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
    return (uint16_t)(sum / (Filter_N - 2));
}

float Find_Max(uint16_t Index_L, uint16_t Index_H, uint16_t *Flag)
{
    float Max = 0;
    for (uint16_t i = Index_L; i <= Index_H; i++)
    {
        if (sA[i].real > Max)
        {
            Max = sA[i].real;
            *Flag = i;
        }
    }
    return Max;
}

// ptr_buf_in 输入数据指针
// len 输入数据长度
// ptr_THD 输出THD
// ptr_S_Frq 输出频率
void Calc_THD(float *ptr_buf_in, uint16_t len, uint16_t *ptr_THD, uint32_t *ptr_S_Frq)
{
    float Tab_Harmonic[16][3];
    uint16_t scan_frq_cnt = 2;

    uint16_t setFFTmaxFreqIndex;
    float setFFTmaxValue;
    uint16_t i;
    // 谐波中心
    float tem_frq;
    uint16_t index_start, index_end;
    uint16_t rcd_har_pos;
    float tem_float;
    uint16_t tem_max;

    float THDx;
    int THD;

    // 获得最大的幅值和所在的位置
    setFFTmaxFreqIndex = 0;
    setFFTmaxValue = 0;
    // 找到最大值
    for (i = 2; i < NUM_SAMPLES / 2; i++) // 前几个是直流量 且只有前一半的数据是有效的
    {
        if (setFFTmaxValue < ptr_buf_in[i])
        {
            setFFTmaxValue = ptr_buf_in[i];
            setFFTmaxFreqIndex = i;
        }
    }
    //
    for (i = 0; i < 16; i++)
    {
        Tab_Harmonic[i][0] = 0;
        Tab_Harmonic[i][1] = 0;
        Tab_Harmonic[i][2] = 0;
    }

    // // 查找基波和谐波点
    // Tab_Harmonic[0][0] = (float)setFFTmaxFreqIndex;                            // 基波频率索引值
    // Tab_Harmonic[0][1] = (float)setFFTmaxFreqIndex * SampleRate / NUM_SAMPLES; // 基波真实频率
    // Tab_Harmonic[0][2] = ptr_buf_in[setFFTmaxFreqIndex];                       // 基波幅值
    // rcd_har_pos = 1;
    // scan_frq_cnt = 2;

    for (uint8_t i = 0; i < 16; i++)
    {
        uint16_t Index_H, Index_L, Index;
        float Linshi = (float)SampleRate / (NUM_SAMPLES);
        Index_H = (float)(50 * (i + 1) + 20) / Linshi; // 70hz
        Index_L = (float)(50 * (i + 1) - 20) / Linshi; // 30hz

        if (Find_Max(Index_L, Index_H, &Index) > 1) // 50hz 为一个组，组里找最大的，最大值超过一定值就是基波
        {
            Tab_Harmonic[i][0] = (float)(Index);                     // 基波频率索引值
            Tab_Harmonic[i][1] = (float)Tab_Harmonic[i][0] * Linshi; // 基波真实频率
            Tab_Harmonic[i][2] = sA[Index].real;                     // 基波幅值

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

    // while (1)
    // {
    //     tem_frq = scan_frq_cnt * Tab_Harmonic[0][0];
    //     scan_frq_cnt++;

    //     if (tem_frq > NUM_SAMPLES / 2)
    //     {
    //         break;
    //     }

    //     // 向前找一部分
    //     if (tem_frq > (setFFTmaxFreqIndex / 4.0))
    //     {
    //         index_start = (uint16_t)(tem_frq - (setFFTmaxFreqIndex / 4.0));
    //     }
    //     else
    //     {
    //         index_start = 2;
    //     }

    //     // 向后找一部分
    //     if ((tem_frq + (setFFTmaxFreqIndex / 4.0)) > 512)
    //     {
    //         index_end = 512;
    //     }
    //     else
    //     {
    //         index_end = (uint16_t)(tem_frq + (setFFTmaxFreqIndex / 4));
    //     }

    //     // find max
    //     tem_float = 0;
    //     for (i = index_start; i < index_end; i++)
    //     {
    //         if (ptr_buf_in[i] > tem_float)
    //         {
    //             tem_float = ptr_buf_in[i];
    //             tem_max = i;
    //         }
    //     }

    //     Tab_Harmonic[rcd_har_pos][0] = (float)(tem_max);
    //     Tab_Harmonic[rcd_har_pos][1] = (float)Tab_Harmonic[rcd_har_pos][0] * SampleRate / NUM_SAMPLES;
    //     Tab_Harmonic[rcd_har_pos][2] = ptr_buf_in[tem_max];
    //     rcd_har_pos++;

    //     if (rcd_har_pos > 16)
    //     {
    //         break;
    //     }
    // }

    tem_float = 0;
    for (i = 1; i < 10; i++)
    {
        tem_float = tem_float + Tab_Harmonic[i][2] * Tab_Harmonic[i][2];
    }

    THDx = sqrt(tem_float) / Tab_Harmonic[0][2];

    THD = THDx * 1000;

    *ptr_THD = THD;
    *ptr_S_Frq = (uint32_t)Tab_Harmonic[0][1];

    // todo
    //    JiBoVal = Tab_Harmonic[1][2] / Tab_Harmonic[0][2];
    //    XieBoVal = Tab_Harmonic[2][2] / Tab_Harmonic[0][2];
    //    XieBoVal3 = Tab_Harmonic[3][2] / Tab_Harmonic[0][2];
    //    XieBoVal4 = Tab_Harmonic[4][2] / Tab_Harmonic[0][2];
    //    XieBoVal5 = Tab_Harmonic[5][2] / Tab_Harmonic[0][2];
}

// sA[i].real存储实部，sA[i].imag存储虚部
// 开始存储时域，经过FFT后存储频域
struct compx sA[NUM_SAMPLES];
struct compx sB[NUM_SAMPLES];
float FFT_Result[(NUM_SAMPLES >> 1)];

// 打印用
int32_t i32IPart[6];

// 三种功率
float ApparentPower = 0;
float ActivePower = 0;
float PowerFactor = 0;
float f32TempA = 0, f32TempB = 0, f32TempAll = 0;

extern void czk_GPIOPinWrite(uint32_t ui32Port, uint8_t ui8Pins, uint8_t ui8Val);

void Calc_Channl_FFTValue(void)
{
    uint32_t i;
    uint32_t setFFTmaxValue;
    uint32_t setFFTmaxFreqIndex;
    uint32_t MaxInputValue, MinInputValue;
    static uint8_t IO_A = 0;
    IO_A = 1 - IO_A;
    czk_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, IO_A);
    /* 执行1024点浮点FFT运算 */
    for (i = 0; i < NUM_SAMPLES; i++)
    {
        sA[i].real = dstBufferA[i] * Vref / 4096 * flattop[i];
        sA[i].imag = 0;
        sB[i].real = dstBufferB[i] * Vref / 4096 * flattop[i];
        sB[i].imag = 0;
        // sA[i].real = dstBufferA[i] * Vref / 4096;
        // sA[i].imag = 0;
        // sB[i].real = dstBufferB[i] * Vref / 4096;
        // sB[i].imag = 0;
    }

    /* 找到输入数据的最大值与最小值 */
    MaxInputValue = dstBufferA[0];
    MinInputValue = dstBufferA[0];

    for (i = 1; i < NUM_SAMPLES; i++)
    {
        if (dstBufferA[i] > MaxInputValue)
        {
            MaxInputValue = dstBufferA[i];
        }
        else if (dstBufferA[i] < MinInputValue)
        {
            MinInputValue = dstBufferA[i];
        }
    }
    // 计算&存储有效值
    f32TempA = f32TempB = f32TempAll = 0;
    for (i = 0; i < NUM_SAMPLES; i++)
    {
        f32TempA += (float)(sA[i].real * sA[i].real);
        f32TempB += (float)(sB[i].real * sB[i].real);
        f32TempAll += (float)(sA[i].real * sB[i].real);
    }
    f32TempA = (float)f32TempA / NUM_SAMPLES;
    f32TempB = (float)f32TempB / NUM_SAMPLES;
    czk_arm_sqrt_f32(f32TempA, &(f32TempA));          // 有效值
    czk_arm_sqrt_f32(f32TempB, &(f32TempB));          // 有效值
    ApparentPower = (float)(f32TempA * f32TempB);     // 视在功率计算
    ActivePower = (float)f32TempAll / NUM_SAMPLES;    // 有功功率计算
    PowerFactor = (float)ActivePower / ApparentPower; // 相位计算

#if START_czk_Log == 1
    // i32IPart[0] = (int32_t)ApparentPower;
    // i32IPart[1] = (int32_t)(ApparentPower * 1000.0f);
    // i32IPart[1] = (int32_t)(i32IPart[1] - i32IPart[0] * 1000.0);

    // i32IPart[2] = (int32_t)ActivePower;
    // i32IPart[3] = (int32_t)(ActivePower * 1000.0f);
    // i32IPart[3] = (int32_t)(i32IPart[3] - i32IPart[2] * 1000.0);

    // i32IPart[4] = (int32_t)PowerFactor;
    // i32IPart[5] = (int32_t)(PowerFactor * 1000.0f);
    // i32IPart[5] = (int32_t)(i32IPart[5] - i32IPart[4] * 1000.0);
    // UARTprintf("\r%3d.%03d, %3d.%03d, %3d.%03d", i32IPart[0], i32IPart[1], i32IPart[2], i32IPart[3], i32IPart[4], i32IPart[5]);
#endif

    /* 不限制点数实序列快速FFT, 浮点 */
    cfft(sA, NUM_SAMPLES);
    cfft(sB, NUM_SAMPLES);

    /* FFT 计算幅频 */
    for (i = 0; i < NUM_SAMPLES; i++)
    {
        // sA[i].real = sA[i].real*4.64f; //平顶窗恢复系数
        // sB[i].real = sB[i].real*4.64f; //平顶窗恢复系数
        czk_arm_sqrt_f32((float)(sA[i].real * sA[i].real + sA[i].imag * sA[i].imag), &sA[i].real);
        czk_arm_sqrt_f32((float)(sB[i].real * sB[i].real + sB[i].imag * sB[i].imag), &sB[i].real);
    }

    /* 串口打印求解的幅频 */
#if START_czk_Log == 1
    // for (i = 0; i < NUM_SAMPLES; i++)
    // {
    //     i32IPart[0] = (int32_t)sA[i].real;
    //     i32IPart[1] = (int32_t)(sA[i].real * 1000.0f);
    //     i32IPart[1] = (int32_t)(i32IPart[1] - i32IPart[0] * 1000.0);
    //     UARTprintf("\r%3d.%03d", i32IPart[0], i32IPart[1]);
    // }

    // for (i = 0; i < NUM_SAMPLES; i++)
    // {
    //     i32IPart[0] = (int32_t)sB[i].real;
    //     i32IPart[1] = (int32_t)(sB[i].real * 1000.0f);
    //     i32IPart[1] = (int32_t)(i32IPart[1] - i32IPart[0] * 1000.0);
    //     UARTprintf("\r%3d.%03d", i32IPart[0], i32IPart[1]);
    // }
#endif

    /* 获得最大的幅值和所在的位置 */
    setFFTmaxFreqIndex = 0;
    setFFTmaxValue = 0;
    for (i = 1; i < (NUM_SAMPLES >> 1); i++)
    {
        if (setFFTmaxValue < sA[i].real)
        {
            setFFTmaxValue = sA[i].real;
            setFFTmaxFreqIndex = i;
        }
        // 这里顺便从结构体变成数组
        FFT_Result[i] = sA[i].real;
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
#if START_czk_Log == 1
    i32IPart[0] = (int32_t)THDx;
    UARTprintf("\r%3d", i32IPart[0]);
#endif
    // /* 估算频率 */
    // uiFreq = (float)setFFTmaxFreqIndex * set_frq / NUM_SAMPLES;

    // DispPeriod = set_frq / uiFreq;

    // // Draw_Wave(&dstBufferA[10], DispPeriod);

    // LastDispPeriod = DispPeriod;

    // setFFTmaxValue = 0;
    IO_A = 1 - IO_A;
    czk_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, IO_A);
}
