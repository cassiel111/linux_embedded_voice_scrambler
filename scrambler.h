#ifndef SCRAMBLER_H
#define SCRAMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//法一:频带倒置

// 最多支持8个载波频段
#define MAX_BANDS 8

// 声明载波频率数组
extern const int standard_carriers[MAX_BANDS];

// 去除信号直流分量
void remove_dc(float *sig, int len);
// 生成正弦载波
void generate_carrier(float *carrier, int f_carrier, int fs, int len);
// 低通滤波
void cheby1_lp_filter(float *sig_out, float *sig_in, int len, int cutoff, int fs);
// 语音反转
void invert_band(float *out, float *sig, float *carrier, int len, int cutoff, int fs);

#endif
