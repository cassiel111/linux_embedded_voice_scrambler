#include "stft.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.141592653589793f
#endif

static float hann(int n, int N) {
    return 0.42f - 0.5f*cosf(2*M_PI*n/(N-1)) + 0.08f*cosf(4*M_PI*n/(N-1));
}

// 快速傅里叶变换 FFT
static void fft(Complex *data, int n, int invert) {
    // 位反转置换
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;

        if (i < j) {
            Complex tmp = data[i];
            data[i] = data[j];
            data[j] = tmp;
        }
    }

    // 蝴蝶运算
    for (int len = 2; len <= n; len <<= 1) {
        float ang = 2 * M_PI / len * (invert ? -1 : 1);
        Complex wlen;
        wlen.re = cosf(ang);
        wlen.im = sinf(ang);

        for (int i = 0; i < n; i += len) {
            Complex w;
            w.re = 1;
            w.im = 0;

            for (int j = 0; j < len/2; j++) {
                Complex u = data[i+j];
                Complex v;
                v.re = data[i+j+len/2].re * w.re - data[i+j+len/2].im * w.im;
                v.im = data[i+j+len/2].re * w.im + data[i+j+len/2].im * w.re;

                data[i+j].re = u.re + v.re;
                data[i+j].im = u.im + v.im;
                data[i+j+len/2].re = u.re - v.re;
                data[i+j+len/2].im = u.im - v.im;

                Complex nw;
                nw.re = w.re * wlen.re - w.im * wlen.im;
                nw.im = w.re * wlen.im + w.im * wlen.re;
                w = nw;
            }
        }
    }

    // 逆变换归一化
    if (invert) {
        for (int i = 0; i < n; i++) {
            data[i].re /= n;
            data[i].im /= n;
        }
    }
}

// ================= stft_c 不变 =================
void stft_c(float *x, int n, Complex *out, int *frames_out, int *bins_out) {
    int N = STFT_WINDOW;
    int H = STFT_HOP;
    int F = (n - N)/H + 1;
    int B = N/2 + 1;

    *frames_out = F;
    *bins_out = B;

    Complex *fftBuf = malloc(N * sizeof(Complex));
    float *w = malloc(N * sizeof(float));

    for (int f = 0; f < F; f++) {
        for (int i = 0; i < N; i++) {
            w[i] = x[f*H + i] * hann(i, N);
        }
        for (int i = 0; i < N; i++) {
            fftBuf[i].re = w[i];
            fftBuf[i].im = 0;
        }
        fft(fftBuf, N, 0);
        for (int k = 0; k < B; k++) {
            out[f*B + k].re = fftBuf[k].re;
            out[f*B + k].im = fftBuf[k].im;
        }
    }

    free(w);
    free(fftBuf);
}

// ================= istft_c 不变 =================
void istft_c(Complex *in, int frames, int bins, float *out) {
    int N = STFT_WINDOW;
    int H = STFT_HOP;
    int n = (frames-1)*H + N;
    memset(out, 0, n * sizeof(float));

    Complex *fftBuf = malloc(N * sizeof(Complex));
    float *buf = malloc(N * sizeof(float));

    for (int f = 0; f < frames; f++) {
        memset(fftBuf, 0, N * sizeof(Complex));
        for (int k = 0; k < bins; k++) {
            fftBuf[k].re = in[f*bins + k].re;
            fftBuf[k].im = in[f*bins + k].im;
        }
        for (int k = 1; k < N/2; k++) {
            fftBuf[N - k].re = fftBuf[k].re;
            fftBuf[N - k].im = -fftBuf[k].im;
        }
        fft(fftBuf, N, 1);
        for (int i = 0; i < N; i++) {
            buf[i] = fftBuf[i].re;
        }
        for (int i = 0; i < N; i++) {
            out[f*H + i] += buf[i] * hann(i, N);
        }
    }

    free(buf);
    free(fftBuf);
}

// ================= 加强版：置换 + 子带频谱倒置 =================
void complex_shuffle(Complex *in, int F, int B, int nb, int *order, int invert) {
    int bs = B / nb;
    Complex *tmp = (Complex*)malloc(F * B * sizeof(Complex));
    memcpy(tmp, in, F * B * sizeof(Complex));

    for (int b = 0; b < nb; b++) {
        int src_band = order[b];
        for (int f = 0; f < F; f++) {
            for (int k = 0; k < bs; k++) {
                int target_pos = f * B + b * bs + k;
                // 子带内部频谱倒置，加强混淆
                int k_rev = bs - 1 - k;
                int src_pos  = f * B + src_band * bs + k_rev;

                in[target_pos] = tmp[src_pos];
            }
        }
    }

    free(tmp);
}
