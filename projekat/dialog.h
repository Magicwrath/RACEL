#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>
#include <QString>
#include <QMessageBox>
#include <cmath>
#include <iostream>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <time.h>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

//**********hardware macros**********
//load resistance in kOhm
//the potentiometer default is 5k
#define RL_VALUE	5

//Value of the factor Rs/R0 in the datasheet graph
//at 1000ppm of H2 in the fresh air
//we must calibrate this
#define CLEAN_AIR_FACTOR	9.83

//**********sampling related macros**********
#define NUM_OF_CALIBRATION_SAMPLES	100
#define CALIBRATION_SAMPLE_INTERVAL	50 //in ms

#define READ_INTERVAL	100 //in ms
#define NUM_OF_READ_SAMPLES 10

#define ECHO_PIN 26
#define TRIG_PIN 27
#define LED_PIN 28


namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();


private slots:
    void on_pushButton_clicked();
    void calibrate_sensor();
    void sample_sensor();
    void sample_distance();

    void on_lpg_radioButton_clicked(bool checked);
    void on_ethanol_radioButton_clicked(bool checked);
    void on_co_radioButton_clicked(bool checked);
    void on_alarm_control_button_clicked(bool checked);

private:
    Ui::Dialog *ui;

    //sensor
    double r0_calibrated;

    //i2c
    const char PCF8591 = 0x48; //i2c address of the ADC
    int fd; //i2c device handle
    int read_sensor_adc_val();

    //calibration
    int num_of_calibrations = 0;
    double calibration_value = 0; //sum of adc values during calibration
    QTimer *calibration_timer;

    //sampling
    int num_of_samples = 0;
    double rs_value = 0;
    QTimer *sample_timer;

    //gas concentration
    double lpg_concentration = 0; //in ppm
    double ethanol_concentration = 0; // in ppm
    double co_concentration = 0; //in ppm
    const int dangerous_gas_thresh[3] = {1900, 1000, 15};

    //alarm
    bool alarm_active = false;
    bool alarm_turned_on = false;
    void alarm_check(int threshold, double concentration);

    //chart
    QLineSeries *chart_series;
    QChart *chart;
    QChartView *chart_view;
    int x = 0;

    //ultrasonic sensor
    QTimer *sample_distance_timer;
};

#endif // DIALOG_H
