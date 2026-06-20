#include "wav.h"

// 打开并读取一个.wav文件
int wav_read(const char *filename, WavFile *wav) {
    //以二进制只读模式打开文件，失败返回错误
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;

    //阅读头文件，知道音频采样率、声道等信息
    fread(&wav->header, 1, sizeof(WavHeader), f);
    int data_size = wav->header.data_size;
    wav->num_samples = data_size / (wav->header.bits_per_sample / 8);//总共有多少个声音点

    short *buf = malloc(data_size);
    fread(buf, 1, data_size, f);//读取音频到buf

    //申请 float 类型数组，用于语音加扰算法
    wav->samples = malloc(wav->num_samples * sizeof(float));
    for (int i = 0; i < wav->num_samples; i++) {
        wav->samples[i] = buf[i] / 32768.0f;
    }

    free(buf);
    fclose(f);
    return 0;
}

//把处理好的音频写回新的.wav文件
int wav_write(const char *filename, WavFile *wav, float *samples_out) {
    // 二进制写模式
    FILE *f = fopen(filename, "wb");
    if (!f) return -1;

    //先写文件头
    fwrite(&wav->header, 1, sizeof(WavHeader), f);
    int data_size = wav->header.data_size;
    short *buf = malloc(data_size);

    for (int i = 0; i < wav->num_samples; i++) {
        float s = samples_out[i];
        if (s > 1) s = 1;
        if (s < -1) s = -1;
        buf[i] = s * 32767;
    }

    fwrite(buf, 1, data_size, f);
    free(buf);
    fclose(f);
    return 0;
}

// 释放内存
void wav_free(WavFile *wav) {
    free(wav->samples);
}