#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QMessageBox>
#include "qcustomplot.h"  // newly added QCustomPlot header

// Keep original C headers
extern "C" {
#include "wav.h"
#include "scrambler.h"
#include "stft.h"
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnOpenFile_clicked();
    void on_btnStartProcess_clicked();
    //newly added wave draw function
    void plotWaveform(QCustomPlot *plot, float *samples, int num_samples, int sample_rate);
    void onAlgoIndexChanged(int idx);

private:
    Ui::MainWindow *ui;
    QString currentFile;
    QElapsedTimer timer;
    WavFile *currentWav = nullptr;//save current data
    float *processedSamples = nullptr;//save produced vioce data

    // Get selected carrier frequency value
    int getSelectedCarrier();
    void clearPlotData(); //clear draw data and memory
};

#endif // MAINWINDOW_H
