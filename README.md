演示视频 https://www.bilibili.com/video/BV1MfjB6NEuA/?share_source=copy_web&vd_source=876bf1103f08682be77bd620d4ba28d2
# Linux Embedded Voice Scrambler

## 项目简介

本项目基于 Ubuntu 18.04、Qt 4.8 和 S5PV210 嵌入式开发板实现了一套语音加扰/解扰系统，支持 WAV 音频文件的读取、处理、显示与保存。

系统提供图形化界面，可实现语音加扰、语音解扰、波形显示以及处理时延统计等功能，适用于嵌入式数字语音保密通信相关课程设计、毕业设计及音频信号处理研究。

---

## 开发环境

### 上位机环境

- Ubuntu 18.04
- Qt Creator
- Qt 4.8
- GCC

### 目标平台

- Samsung S5PV210
- ARM Cortex-A8
- Embedded Linux

---

## 功能特点

### 1. WAV文件处理

- 打开WAV音频文件
- 保存处理后的WAV文件
- 支持音频波形显示

### 2. 语音加扰

支持两种加扰算法：

#### Band Inversion（频谱倒置）

传统模拟语音保密算法。

特点：

- 实现简单
- 运算量低
- 实时性好
- 适合嵌入式平台

支持载波频率：

- 2632 Hz
- 2718 Hz
- 2868 Hz
- 3023 Hz
- 3196 Hz
- 3339 Hz
- 3495 Hz
- 3729 Hz

#### STFT Shuffle（频域乱序）

基于短时傅里叶变换（STFT）的频域加扰算法。

特点：

- 安全性高
- 可逆恢复
- 多子带频谱乱序
- 适用于数字语音保密通信

当前采用8子带乱序方案：

```
4 1 6 3 0 7 2 5
```

---

## 主要功能模块

```text
GUI界面(Qt4.8)
│
├── WAV文件读取
├── WAV文件保存
├── 原始波形显示
├── 处理后波形显示
├── Band Inversion加扰
├── Band Inversion解扰
├── STFT加扰
├── STFT解扰
└── 处理时延统计
```
```text
web网页
│
├── 板卡WAV文件同步
├── 原始波形显示
├── 处理后波形显示
├── 音频播放
└── 处理时延统计
```
---

## 工作模式

### Direct Pass

音频直通模式。

不对语音进行任何处理，仅完成文件复制和显示。

### Scramble

语音加扰模式。

支持：

- Band Inversion
- STFT Shuffle

### Descramble

语音解扰模式。

支持：

- Band Inversion Recovery
- STFT Recovery

---

## 图形界面功能

### 文件操作

- 打开WAV文件
- 保存处理结果

### 参数配置

- 工作模式选择
- 加扰算法选择
- 载波频率选择

### 波形显示

- 原始波形显示
- 处理后波形显示
- 波形缩放
- 波形拖动

### 性能监测

- 音频处理时延统计
- 实时显示处理耗时

---



## 编译方法

使用 Qt Creator 打开工程：

```bash
qmake
make
```

运行程序：

```bash
./VoiceUI
```

---

## 测试流程

1. 打开WAV语音文件
2. 选择工作模式
3. 选择加扰算法
4. 设置载波频率（Band Inversion）
5. 点击开始处理
6. 查看处理后波形
7. 保存输出文件
8. 网页联通单片机获取处理后的WAV文件
9. 查看时频图和播放音频

---



## 后续优化方向

- 实时麦克风采集
- ALSA音频驱动支持
- ARM NEON优化
- DSP加速
- 多级频域加扰
- 密钥动态配置
- 网络语音保密传输


---

## 作者

Cassiel111

---

## License

This project is intended for educational and research purposes only.
