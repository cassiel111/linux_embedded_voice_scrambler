#ifndef WAV_H
#define WAV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// WAV 文件的固定头部格式（44字节）
typedef struct 
{
    char riff[4];
    int file_size; 
    char wave[4];
    char fmt[4];
    int fmt_len;
    short audio_fmt; //音频格式
    short channels;// 声道数
    int sample_rate;
    int byte_rate;
    short block_align;//数据对齐
    short bits_per_sample;//位深，一般为16位
    char data[4];
    int data_size;//音频数据长度
} WavHeader;

// 打包文件头和音频数据
typedef struct 
{
    WavHeader header;
    float *samples;//音频数据（浮点数格式）
    int num_samples;
} WavFile;

int wav_read(const char *filename, WavFile *wav);

int wav_write(const char *filename, WavFile *wav, float *samples_out);

void wav_free(WavFile *wav);//释放内存

#endif