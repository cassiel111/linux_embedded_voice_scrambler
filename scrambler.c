#define M_PI 3.14159265358979323846f
#include "scrambler.h"
#include <stdio.h>
#include <stdlib.h>
#include "wav.h"
#include "stft.h"



//法一:频带倒置

// 8个固定载波频率
const int standard_carriers[MAX_BANDS] = {
    2632, 2718, 2868, 3023, 3196, 3339, 3495, 3729
};


// 去直流偏移
void remove_dc(float *sig, int len) {
    // 先求平均值
    float mean = 0;
    for (int i = 0; i < len; i++) mean += sig[i];
    mean /= len;
    // 再让每个点减取平均值
    for (int i = 0; i < len; i++) sig[i] -= mean;
}

// 生成载波
void generate_carrier(float *carrier, int f_carrier, int fs, int len) {
    float t_step = 1.0f / fs;
    for (int i = 0; i < len; i++) {
        float t = i * t_step;
        carrier[i] = sinf(2 * M_PI * f_carrier * t);
    }
}

// 低通滤波(目前没有任何滤波)
// void cheby1_lp_filter(float *out, float *in, int len, int cutoff, int fs) {
//     for (int i = 0; i < len; i++) out[i] = in[i];
// }

// 一阶低通滤波
void cheby1_lp_filter(float *out, float *in, int len, int cutoff, int fs) {
    // 滤波参数
    float Ts = 1.0f / fs;           // 采样周期
    float pi = 3.14159f;
    float fc = cutoff;              // 截止频率
    float b = 2.0f * pi * fc * Ts;
    float alpha = b / (b + 1.0f);

    // 上一次输出
    float out_last = in[0];         // 初始化为第一个样本

    // 逐点滤波
    for (int i = 0; i < len; i++) {
        out[i] = out_last + alpha * (in[i] - out_last);
        out_last = out[i];
    }
}


// 语音反转
void invert_band(float *out, float *sig, float *carrier, int len, int cutoff, int fs) {
    float *filtered = malloc(len * sizeof(float));
    // 滤波
    cheby1_lp_filter(filtered, sig, len, cutoff, fs);

    // 语音*载波=反转
    for (int i = 0; i < len; i++) {
        out[i] = filtered[i] * carrier[i];
    }

    // 再次滤波
    cheby1_lp_filter(filtered, out, len, cutoff, fs);
    memcpy(out, filtered, len * sizeof(float));

    // 释放内存
    free(filtered);
}

