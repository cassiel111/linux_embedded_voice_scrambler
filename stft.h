// 替换原来的4，改成8
#define NUM_BANDS 8
#define STFT_WINDOW 512
#define STFT_HOP 256

typedef struct {
    float re;
    float im;
} Complex;

// ② 更新 complex_shuffle 函数声明，加上第6个参数 invert
void stft_c(float *x, int n, Complex *out, int *frames_out, int *bins_out);
void istft_c(Complex *in, int frames, int bins, float *out);
void complex_shuffle(Complex *in, int F, int B, int nb, int *order, int invert);
