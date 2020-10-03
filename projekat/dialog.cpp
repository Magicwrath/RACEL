#include "dialog.h"
#include "ui_dialog.h"


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    //set up wiring pi
    if(wiringPiSetup() == -1)
        exit(1);

    //set up I2C communication with ADC
    fd = wiringPiI2CSetup(PCF8591);

    //create timer object connect timeout signal to slot
    calibration_timer = new QTimer(this);
    sample_timer = new QTimer(this);
    sample_distance_timer = new QTimer(this);
    connect(calibration_timer, SIGNAL(timeout()), this, SLOT(calibrate_sensor()));
    connect(sample_timer, SIGNAL(timeout()), this, SLOT(sample_sensor()));
    connect(sample_distance_timer, SIGNAL(timeout()), this, SLOT(sample_distance()));
    calibrate_sensor();

    QString output_str = QString::number(dangerous_gas_thresh[0]);
    ui->alarm_lineEdit->setText(output_str);

    //set pin 28 in WPi to output mode, to control the LED
    pinMode(LED_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(TRIG_PIN, OUTPUT);

    //set up the chart
    chart_series = new QLineSeries();
    chart = new QChart();
    *chart_series << QPointF(0, 0);

    chart->legend()->hide();
    chart->addSeries(chart_series);
    chart->createDefaultAxes();
    chart->axisX()->setTitleText("samples");
    chart->axisY()->setTitleText("ppm");
    chart->axisX()->setRange(0, 5);
    chart->axisY()->setRange(0, 3000);
    chart->setTitle("Gas concentration");

    chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
    chart_view->setBackgroundBrush(Qt::white);

    ui->chartLayout->addWidget(chart_view);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked() {
    //reset the chart
    chart_series->clear();
    x = 0;
    chart_series->append(QPointF(0, 0));

    //calibrate the sensor
    calibrate_sensor();
    sample_timer->stop();
}

int Dialog::read_sensor_adc_val() {
    int adcVal;

    //sensor output is connected to ADC channel 1
    wiringPiI2CReadReg8(fd, 0x01); //dummy read
    adcVal = wiringPiI2CReadReg8(fd, 0x01); //read the sensor output

    return adcVal;
}

//this function calibrates the sensor R0 value by the following principle:
//The sensor resistance (Rs) and load resistance (Rl) form a voltage divider
//The sensor output (V0) is the voltage accros Rl
//By measuring V0 (we already know the value of Rl), we can calculate Rs,
//and by using the clear air factor (Rs/R0) we can calculate R0
void Dialog::calibrate_sensor() {
    QString status_string = "Calibrating...";
    calibration_timer->start(CALIBRATION_SAMPLE_INTERVAL);
    ui->value_lineEdit->setText(status_string);
    //read the adc value NUM_OF_CALIBRATION_SAMPLES times
    //with CALIBRATION_SAMPLE_INTERVAL intervals
    if(num_of_calibrations < NUM_OF_CALIBRATION_SAMPLES) {
        double adcVal = read_sensor_adc_val();
        calibration_value += RL_VALUE * ((255 - adcVal) / adcVal); //result is in kOhm
        ++num_of_calibrations;
    } else {
        //calculate the calibrated R0 value and stop the timer
        //start the sample timer
        calibration_timer->stop();
        num_of_calibrations = 0;

        calibration_value /= NUM_OF_CALIBRATION_SAMPLES;
        //now we divide the average with the clean air factor
        calibration_value /= CLEAN_AIR_FACTOR;

        r0_calibrated = calibration_value;
        //std::cout << "r0_calibrated = " << r0_calibrated << std::endl;
        calibration_value = 0;
        ui->value_lineEdit->clear();
        sample_timer->start(READ_INTERVAL);
    }
}

//same principle as the calibrate_sensor() function, except that we just
//calculate Rs based on V0 and Rl, and we use the calibrated R0 and Rs,
//to calculate the concetration of the specific gas in ppm
void Dialog::sample_sensor() {
    if(num_of_samples < NUM_OF_READ_SAMPLES) {
        double adcVal = read_sensor_adc_val();
        //std::cout << "adcVal = " << adcVal << std::endl;
        rs_value += RL_VALUE * ((255 - adcVal) / adcVal); //result is in kOhm
        ++num_of_samples;
    } else {
        //sample_timer->stop();
        num_of_samples = 0;

        rs_value /= NUM_OF_READ_SAMPLES;

        //chart x axis shifting
        if(x < 5) {
            ++x;
        } else {
            x = 0;
            chart_series->clear();
        }

        //calculate the gas concentration based on the approximation
        //of the datasheet graph
        QString output_str;
        if(ui->lpg_radioButton->isChecked()) {
            lpg_concentration = pow(10, (2.3 - ((0.21 - log10(rs_value / r0_calibrated)) / (-0.47))));
            output_str = QString::number(lpg_concentration);
            //output_str.append(" ppm");
            ui->value_lineEdit->setText(output_str);
            output_str.clear();

            alarm_check(dangerous_gas_thresh[0], lpg_concentration);
            chart_series->append(QPointF(x, lpg_concentration));
        }

        if(ui->ethanol_radioButton->isChecked()) {
            ethanol_concentration = pow(10, (2.3 - ((0.462 - log10(rs_value / r0_calibrated)) / (-0.378))));
            output_str = QString::number(ethanol_concentration);
            //output_str.append(" ppm");
            ui->value_lineEdit->setText(output_str);
            output_str.clear();

            alarm_check(dangerous_gas_thresh[1], ethanol_concentration);
            chart_series->append(QPointF(x, ethanol_concentration));
        }

        if(ui->co_radioButton->isChecked()) {
            co_concentration = pow(10, (2.3 - ((0.53 - log10(rs_value / r0_calibrated)) / (-0.34))));
            output_str = QString::number(co_concentration);
            //output_str.append(" ppm");
            ui->value_lineEdit->setText(output_str);
            output_str.clear();

            alarm_check(dangerous_gas_thresh[2], co_concentration);
            chart_series->append(QPointF(x, co_concentration));
        }

        rs_value = 0;

        //sample_timer->start(READ_INTERVAL);
    }
}


void Dialog::on_lpg_radioButton_clicked(bool checked)
{
    QString output_str;
    if(checked) {
        //set alarm value
        output_str = QString::number(dangerous_gas_thresh[0]);
        ui->alarm_lineEdit->setText(output_str);
        output_str.clear();

        //reset the chart
        chart_series->clear();
        x = 0;
        chart_series->append(QPointF(0, 0));
    }
}

void Dialog::on_ethanol_radioButton_clicked(bool checked)
{
    QString output_str;
    if(checked) {
        //set alarm value
        output_str = QString::number(dangerous_gas_thresh[1]);
        ui->alarm_lineEdit->setText(output_str);
        output_str.clear();

        //reset the chart
        chart_series->clear();
        x = 0;
        chart_series->append(QPointF(0, 0));
    }
}

void Dialog::on_co_radioButton_clicked(bool checked)
{
    QString output_str;
    if(checked) {
        //set alarm value
        output_str = QString::number(dangerous_gas_thresh[2]);
        ui->alarm_lineEdit->setText(output_str);
        output_str.clear();

        //reset the chart
        chart_series->clear();
        x = 0;
        chart_series->append(QPointF(0, 0));
    }
}

void Dialog::alarm_check(int threshold, double concentration) {
    if(concentration >= threshold && alarm_turned_on) {
        digitalWrite(LED_PIN, HIGH);
        QMessageBox::warning(this, "Alarm", "Dangerous gas levels!");
    }
}

void Dialog::on_alarm_control_button_clicked(bool checked)
{
    if(!checked) {
        alarm_turned_on = false;
        ui->alarm_control_button->setText("Off");
        sample_distance_timer->stop();
    } else {
        alarm_turned_on = true;
        ui->alarm_control_button->setText("On");
        sample_distance_timer->start(300);
    }
}

void Dialog::sample_distance() {
    struct timespec req, rem;
    //sleep timer for 10000ns = 10us = 0.01ms
    req.tv_sec = 0;
    req.tv_nsec = 10000;

    //write high on trigger pin
    digitalWrite(TRIG_PIN, HIGH);
    //sleep for 0.01ms
    if(nanosleep(&req, &rem) < 0) {
        exit(1);
    }
    //std::cout << "YOOO" << std::endl;
    digitalWrite(TRIG_PIN, LOW);

    int num_of_zero_cycles = 0;
    int num_of_one_cycles = 0;

    while(digitalRead(ECHO_PIN) == 0)
        ++num_of_zero_cycles;

    while(digitalRead(ECHO_PIN) == 1)
        ++num_of_one_cycles;

    //std::cout << num_of_zero_cycles << std::endl;
    //std::cout << num_of_one_cycles << std::endl;

    if(num_of_zero_cycles > num_of_one_cycles)
        digitalWrite(LED_PIN, LOW);
}
