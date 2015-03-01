#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART

extern int uart0_filestream;

unsigned char tx_buffer[5] = {0xBA,0xDA,0x55,0,0};
int rx_len = 0;
char rx_buf[10];

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    gotPreamble = false;

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()), this, SLOT(timerSlot()));
    timer->start(0);
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
    //cl
    tx_buffer[0] = 'c';
    tx_buffer[1] = 'l';
    write(uart0_filestream, &tx_buffer[0], 5);
}

void MainWindow::timerSlot(){
    static int bytectr = 0;
    static char buf[2] = {0,0};

    if( bytectr >= 2 ){
        ui->lcdNumber->display(buf[1]);
        ui->progressBar->setValue(buf[0]);
        ui->lcdNumber->update();
        ui->progressBar->update();
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
