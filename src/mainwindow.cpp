#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <cstring>
#include <QVector>
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    currentFile = "";

    // ===================== 模式选项 =====================
    ui->cboWorkMode->addItem("Direct Pass");
    ui->cboWorkMode->addItem("Scramble");
    ui->cboWorkMode->addItem("Descramble");
    ui->cboWorkMode->setCurrentIndex(0);

    ui->cboAlgorithm->addItem("Band Inversion");
    ui->cboAlgorithm->addItem("STFT Shuffle");
    ui->cboAlgorithm->setCurrentIndex(0);

    // 载波频率
    ui->cboCarrier->addItem("2632");
    ui->cboCarrier->addItem("2718");
    ui->cboCarrier->addItem("2868");
    ui->cboCarrier->addItem("3023");
    ui->cboCarrier->addItem("3196");
    ui->cboCarrier->addItem("3339");
    ui->cboCarrier->addItem("3495");
    ui->cboCarrier->addItem("3729");
    ui->cboCarrier->setCurrentIndex(0);

    // ===================== 关键：STFT 不需要载波 =====================

    connect(ui->cboAlgorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlgoIndexChanged(int)));

    ui->labelFileName->setText("File: None");
    ui->labelDelay->setText("Delay: 0 ms");

    // 绘图初始化
    ui->plotOriginal->xAxis->setLabel("Time (s)");
    ui->plotOriginal->yAxis->setLabel("Amplitude");
    ui->plotOriginal->yAxis->setRange(-1.1, 1.1);

    ui->plotProcessed->xAxis->setLabel("Time (s)");
    ui->plotProcessed->yAxis->setLabel("Amplitude");
    ui->plotProcessed->yAxis->setRange(-1.1, 1.1);

    ui->plotOriginal->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->plotProcessed->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

MainWindow::~MainWindow()
{
    clearPlotData();
    delete ui;
}

void MainWindow::onAlgoIndexChanged(int idx)
{
    if (idx == 1)
    {
        ui->cboCarrier->setEnabled(false);
        ui->cboCarrier->setToolTip("STFT mode does not need carrier");
    }
    else
    {
        ui->cboCarrier->setEnabled(true);
        ui->cboCarrier->setToolTip("");
    }
}


void MainWindow::clearPlotData()
{
    if (currentWav) {
        wav_free(currentWav);
        free(currentWav);
        currentWav = nullptr;
    }
    if (processedSamples) {
        free(processedSamples);
        processedSamples = nullptr;
    }
    ui->plotOriginal->clearPlottables();
    ui->plotProcessed->clearPlottables();
    ui->plotOriginal->replot();
    ui->plotProcessed->replot();
}

int MainWindow::getSelectedCarrier()
{
    int idx = ui->cboCarrier->currentIndex();
    static int carriers[] = {2632,2718,2868,3023,3196,3339,3495,3729};
    if (idx >=0 && idx < 8)
        return carriers[idx];
    return carriers[0];
}

void MainWindow::plotWaveform(QCustomPlot *plot, float *samples, int num_samples, int sample_rate)
{
    plot->clearPlottables();
    QVector<double> x, y;
    const int MAX_DRAW_POINTS = 1200;
    int step = num_samples > MAX_DRAW_POINTS ? num_samples / MAX_DRAW_POINTS : 1;

    double t = 1.0 / sample_rate;
    for (int i = 0; i < num_samples; i += step) {
        x.append(i * t);
        y.append(samples[i]);
    }

    QCPGraph *g = plot->addGraph();
    g->setData(x, y);
    g->setPen(QPen(Qt::blue, 1));
    plot->xAxis->setRange(x.first(), x.last());
    plot->yAxis->setRange(-1.1, 1.1);
    plot->replot();
}

void MainWindow::on_btnOpenFile_clicked()
{
    clearPlotData();
    QFileDialog dlg(this, "Select WAV", "/root", "*.wav");
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    if (dlg.exec()) {
        currentFile = dlg.selectedFiles().first();
        ui->labelFileName->setText("File: " + currentFile);

        currentWav = (WavFile*)malloc(sizeof(WavFile));
        if (!currentWav || wav_read(currentFile.toUtf8(), currentWav) != 0) {
            QMessageBox::critical(this, "Error", "Read WAV failed");
            clearPlotData();
            return;
        }
        plotWaveform(ui->plotOriginal, currentWav->samples, currentWav->num_samples, currentWav->header.sample_rate);
    }
}

// ===================== 核心：完全对齐你原版 main 算法 =====================
void MainWindow::on_btnStartProcess_clicked()
{
    if (!currentWav || currentFile.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Select a WAV first");
        return;
    }

    if (processedSamples) { free(processedSamples); processedSamples = nullptr; }
    processedSamples = (float*)malloc(currentWav->num_samples * sizeof(float));
    memcpy(processedSamples, currentWav->samples, currentWav->num_samples * sizeof(float));

    int workMode  = ui->cboWorkMode->currentIndex();
    int algoMode  = ui->cboAlgorithm->currentIndex();
    int fc        = getSelectedCarrier();
    int fs        = currentWav->header.sample_rate;
    int len       = currentWav->num_samples;

    QString outFile;
    timer.start();

    // -----------------------------
    // 直通
    // -----------------------------
    if (workMode == 0) {
        outFile = "/root/out_direct.wav";
        QFile::copy(currentFile, outFile);
        plotWaveform(ui->plotProcessed, processedSamples, len, fs);
    }
    // -----------------------------
    // 加扰
    // -----------------------------
    else if (workMode == 1) {
        if (algoMode == 0) {
            // Band inversion 加扰
            float *carrier = (float*)malloc(len * sizeof(float));
            generate_carrier(carrier, fc, fs, len);
            float *sig = (float*)malloc(len * sizeof(float));
            memcpy(sig, currentWav->samples, len * sizeof(float));
            remove_dc(sig, len);
            // 严格原版：第5个参数是 fc，不是 4000！
            invert_band(processedSamples, sig, carrier, len, fc, fs);
            float gain = 4.0f;
            for(int i = 0; i < len; i++){
                processedSamples[i] *= gain;
            }
            free(sig);
            free(carrier);
            outFile = QString("/root/sc_invert_%1.wav").arg(fc);
        } else {

            // STFT 加扰
            Complex *stftBuf = (Complex*)malloc(len * sizeof(Complex)*2);
            int F, B;
            stft_c(currentWav->samples, len, stftBuf, &F, &B);
            QApplication::processEvents();

            // 8子带加强乱序

            int shuffle_order[NUM_BANDS] = {4,1,6,3,0,7,2,5};
            // 加扰 invert=0
            complex_shuffle(stftBuf, F, B, NUM_BANDS, shuffle_order, 0);

            istft_c(stftBuf, F, B, processedSamples);
            free(stftBuf);
            outFile = "/root/sc_stft_8.wav";
        }
        wav_write(outFile.toUtf8(), currentWav, processedSamples);
        plotWaveform(ui->plotProcessed, processedSamples, len, fs);
    }
    // -----------------------------
    // 解扰
    // -----------------------------
    else if (workMode == 2) {
        if (algoMode == 0) {
            // Band inversion 解扰：和加扰完全相同调用 → 100% 还原
            float *carrier = (float*)malloc(len * sizeof(float));
            generate_carrier(carrier, fc, fs, len);
            float *sig = (float*)malloc(len * sizeof(float));
            memcpy(sig, currentWav->samples, len * sizeof(float));
            invert_band(processedSamples, sig, carrier, len, fc, fs);
            free(sig);
            free(carrier);
            outFile = QString("/root/de_invert_%1.wav").arg(fc);
        } else {

            // STFT 解扰
            Complex *stftBuf = (Complex*)malloc(len * sizeof(Complex)*2);
            int F, B;
            stft_c(currentWav->samples, len, stftBuf, &F, &B);
            QApplication::processEvents();


            // 解扰用和加扰同样的表，因为内部做了两次倒置 = 还原
            int deshuffle_order[NUM_BANDS] = {4,1,6,3,0,7,2,5};
            // 解扰 invert=1
            complex_shuffle(stftBuf, F, B, NUM_BANDS, deshuffle_order, 1);

            istft_c(stftBuf, F, B, processedSamples);
            free(stftBuf);
            outFile = "/root/de_stft_8.wav";
        }
        wav_write(outFile.toUtf8(), currentWav, processedSamples);
        plotWaveform(ui->plotProcessed, processedSamples, len, fs);
    }

    int ms = timer.elapsed();
    ui->labelDelay->setText(QString("Delay: %1 ms").arg(ms));
    QMessageBox::information(this, "OK", "Finished:\n" + outFile);
}
