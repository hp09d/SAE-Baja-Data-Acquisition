#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART

unsigned char tx_buffer[5] = {0xBA,0xDA,0x55,0,0};
extern int uart0_filestream;
int rx_len = 0;
char rx_buf[256];

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    gotPreamble = false;

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()), this, SLOT(timerSlot()));
    timer->start(0);

    ui->SDStatus->setStyleSheet( "background-color: orange; border: 2px solid black;" );
    ui->SensorStatus->setStyleSheet( "background-color: orange; border: 2px solid black;" );

    ui->LogStatus->setStyleSheet( "QPushButton {background-color: yellow;}" );
    ui->PitStatus->setStyleSheet( "background-color: yellow; border: 2px solid black;" );
    ui->DriverStatus->setStyleSheet( "background-color: yellow; border: 2px solid black;" );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //er
    tx_buffer[3] = 'e';
    tx_buffer[4] = 'r';
    write(uart0_filestream, &tx_buffer[0], 5);
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->SDStatus->setStyleSheet( "background-color: orange;" );
    ui->SensorStatus->setStyleSheet( "background-color: orange;" );
    ui->LogStatus->setStyleSheet( "QPushButton {background-color: orange;}" );
}

void MainWindow::timerSlot(){
    static int bytectr = 0;
    static char buf[3] = {0,0,0};

    if( bytectr >= 3 ){
        ui->lcdNumber->display(buf[2]);
        ui->progressBar->setValue(buf[1]);
        ui->lcdNumber->update();
        ui->progressBar->update();

        if( (buf[0] & 0x80) == 0x80 ){
            ui->SDStatus->setStyleSheet( "background-color: red;" );
            ui->LogStatus->setStyleSheet( "QPushButton {background-color: red;}" );
        }else{
            ui->SDStatus->setStyleSheet( "background-color: green;" );

            if( (buf[0] & 0x40) == 0x40 ){
                ui->LogStatus->setStyleSheet( "QPushButton {background-color: green;}" );
                //ui->LogStatus->update();
            }else{
                ui->LogStatus->setStyleSheet( "QPushButton {background-color: yellow;}" );
                //ui->LogStatus->update();
            }
        }

        if( (buf[0] & 0x20) == 0x20 ){
            ui->SensorStatus->setStyleSheet( "background-color: red;" );
        }else{
            ui->SensorStatus->setStyleSheet( "background-color: green;" );
        }

        if( (buf[0] & 0x08) == 0x08 ){
            ui->PitStatus->setStyleSheet( "background-color: red;" );
        }else{
            ui->PitStatus->setStyleSheet( "background-color: yellow;" );
        }

        if( (buf[0] & 0x10) == 0x10 ){
            ui->DriverStatus->setStyleSheet( "background-color: red;" );
        }else{
            ui->DriverStatus->setStyleSheet( "background-color: yellow;" );
        }

        bytectr = 0;
        gotPreamble = false;
    }

    rx_len = read(uart0_filestream, (void*)rx_buf, 1);

    if(rx_len > 0){
        if( gotPreamble == false ){
            gotPreamble = getPreamble(rx_buf[0]);
        }else{
            buf[bytectr] = rx_buf[0];
            bytectr++;
        }
    }
}

bool MainWindow::getPreamble(char val){
    static int bytectr = 0;

    if( val == tx_buffer[bytectr] ){
        bytectr++;
    }else{
        bytectr = 0;
    }

    if(bytectr == 3){
        bytectr = 0;
        return 1;
    }else{
        return 0;
    }
}

void MainWindow::on_quitButton_clicked()
{
    close();
}

void MainWindow::on_LogStatus_clicked()
{
    //lo
    tx_buffer[3] = 'l';
    tx_buffer[4] = 'o';
    write(uart0_filestream, &tx_buffer[0], 5);
}
