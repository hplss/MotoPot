/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QMessageBox>
#include <QtSerialPort/QSerialPort>

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
//! [0]
    ui->setupUi(this);
    console = new Console;
    console->setEnabled(false);
    //setCentralWidget(console);
    ui->wConsole->setEnabled(true);
    console->setParent(ui->wConsole);
    //console->

//! [1]
    serial = new QSerialPort(this);
//! [1]
    settings = new SettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionClear->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    EnableCmdButtons( false );

    //Error Label stuff
    ui->ErrorLabel->setVisible( false );
    QPalette p = palette();
    p.setColor(QPalette::WindowText, Qt::red);
    ui->ErrorLabel->setPalette(p);
    //

    initActionsConnections();

    AutoAdjustEnabled = false;
    MatchArduino = true;
    ui->ResetButton->setEnabled( false );
    ui->commandButton->setEnabled( false );

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

//! [2]
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
//! [2]
    connect(console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));
//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}

//! [4]
void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(57600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    //serial->QSerialPort::DataTerminalReadySignal;
    //serial->setDataTerminalReady( true );
    //serial->s
    if (serial->open(QIODevice::ReadWrite)) {
            serial->setDataTerminalReady( true );
            console->setEnabled(true);
            console->setLocalEchoEnabled(p.localEchoEnabled);
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
            ui->actionConfigure->setEnabled(false);
            ui->actionClear->setEnabled(true);
            console->putData("Attempting Connection... \n");
            serial->write("10"); //This sends a "reset" signal to the arduino, just in case. 10 is an arbitrary number, made up out of thin air.
            ui->statusBar->showMessage(tr("Connected to %1 ")
                                       .arg(p.name));

            //Set other bools
            AutoAdjustEnabled = p.AutoAdjustEnabled;
            OverrideEnabled = p.OverrideEnabled;
            MatchArduino = p.MatchArduino;
            ui->ResetButton->setEnabled( true );

    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        ui->statusBar->showMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    serial->close();
    console->setEnabled(false);
    console->clear(); //Clear all console output when disconnecting.
    //Basic UI Buttons here
    ui->actionClear->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    //
    ui->statusBar->showMessage(tr("Disconnected"));

    EnableCmdButtons( false );//Command Buttons only here.
    ResetControls();
    ui->ResetButton->setEnabled( false );
    ui->commandButton->setEnabled( false );
    ui->ErrorLabel->setVisible( false );
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("Potentiometer Control "),
                       tr("This is a basic control panel application for the potentiometer controller. "
                          "This was modified from an existing QT project "
                          " -Andrew Ward"));
}
void MainWindow::EnableCmdButtons( bool bEnable, int motor ) //Yep
{
    if ( motor == -1 || motor == 0 )
    {
        ui->Motor0Slider->setEnabled( bEnable );
        ui->Motor0_Input->setEnabled( bEnable );
    }
    if ( motor == -1 || motor == 1 )
    {
        ui->Motor1Slider->setEnabled( bEnable );
        ui->Motor1_Input->setEnabled( bEnable );
    }
    if ( motor == -1 || motor == 2 )
    {
        ui->Motor2Slider->setEnabled( bEnable );
        ui->Motor2_Input->setEnabled( bEnable );
    }
    if ( motor == -1 || motor == 3 )
    {
        ui->Motor3Slider->setEnabled( bEnable );
        ui->Motor3_Input->setEnabled( bEnable );
    }
    if ( motor == -1 || motor == 4 )
    {
        ui->Motor4Slider->setEnabled( bEnable );
        ui->Motor4_Input->setEnabled( bEnable );
    }
}
void MainWindow::ResetControls( int controls )
{
    if ( controls == -1 || controls == 0)
    {
        ui->Motor0Slider->setSliderPosition(0);
        ui->Motor0_Input->setText("0");
        ui->Motor0_Label->setText("Pot 0: (Unavailable)");
    }
    if ( controls == -1 || controls == 1)
    {
        ui->Motor1Slider->setSliderPosition(0);
        ui->Motor1_Input->setText("0");
        ui->Motor1_Label->setText("Pot 1: (Unavailable)");
    }
    if ( controls == -1 || controls == 2)
    {
        ui->Motor2Slider->setSliderPosition(0);
        ui->Motor2_Input->setText("0");
        ui->Motor2_Label->setText("Pot 2: (Unavailable)");
    }
    if ( controls == -1 || controls == 3)
    {
        ui->Motor3Slider->setSliderPosition(0);
        ui->Motor3_Input->setText("0");
        ui->Motor3_Label->setText("Pot 3: (Unavailable)");
    }
    if ( controls == -1 || controls == 4)
    {
        ui->Motor4Slider->setSliderPosition(0);
        ui->Motor4_Input->setText("0");
        ui->Motor4_Label->setText("Pot 4: (Unavailable)");
    }
}

void MainWindow::clearconsole( void )
{
    console->clear();
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
   serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    QByteArray data = serial->readAll();

    if (data.size() == 0)
        return;

    int value = 0;
    if (data.contains("/r"))
    {
        console->putData("Device Ready.\n"); //Put the rest of our enable() shit below.
        if ( !AutoAdjustEnabled )
            ui->commandButton->setEnabled( true );

        if ( OverrideEnabled )
            EnableCmdButtons( true );
        return;
    }
    if (data.contains("Error"))
    {
        ui->ErrorLabel->setVisible( true );
        ui->ErrorLabel->setText( data );
    }
    else
        ui->ErrorLabel->setVisible( false );

    if ( data.contains("P0") )
    {
        EnableCmdButtons( true, 0 );

        value = QString(data).split(" ")[1].toInt();
        ui->Motor0_Label->setText(QString("Pot 0: %1 ohms").arg(value));

        if (MatchArduino)
            ui->Motor0Slider->setSliderPosition(value);
    }
    else if ( data.contains("P1") )
    {
        EnableCmdButtons( true, 1 );

        value = QString(data).split(" ")[1].toInt();
        ui->Motor1_Label->setText(QString("Pot 1: %1 ohms").arg(value));

        if (MatchArduino)
            ui->Motor1Slider->setSliderPosition(value);
    }
    else if ( data.contains("P2") ) //There might be a better way to do this, but fuck it.
    {
        EnableCmdButtons( true, 2 );

        value = QString(data).split(" ")[1].toInt();//convert the first part to Int
        ui->Motor2_Label->setText(QString("Pot 2: %1 ohms").arg(value));

        if (MatchArduino)
            ui->Motor2Slider->setSliderPosition(value);
    }
    else if ( data.contains("P3") )
    {
        EnableCmdButtons( true, 3 );

        value = QString(data).split(" ")[1].toInt();
        ui->Motor3_Label->setText(QString("Pot 3: %1 ohms").arg(value));

        if (MatchArduino)
            ui->Motor3Slider->setSliderPosition(value);
    }
    else if ( data.contains("P4") )
    {
        EnableCmdButtons( true, 4 );

        value = QString(data).split(" ")[1].toInt();
        ui->Motor4_Label->setText(QString("Pot 4: %1 ohms").arg(value));

        if (MatchArduino)
            ui->Motor4Slider->setSliderPosition(value);
    }

    console->putData( data );
}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConfigure, SIGNAL(triggered()), settings, SLOT(show()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(clearconsole())); //This connects an action button to a function. - Andrew
}
void MainWindow::ApplySettings( QString value )
{
    QByteArray settings = 0;
    settings.append( value );
    //settings.append("\n");
    serial->write(settings);

    if ( console->localEchoEnabled )
    {
        console->putData("Send Motor#");
        console->putData(settings);
        console->putData("\n");
    }
}
//Motor 0
void MainWindow::on_Motor0_Input_returnPressed()
{
    //ui->Motor0_Input->setText();
    int value = QString(ui->Motor0_Input->text()).toInt();
    if ( value < ui->Motor0Slider->minimum() || value >ui->Motor0Slider->maximum()  )
    {
        ui->Motor0_Input->setText("INV");
        return;
    }

    if ( AutoAdjustEnabled )
        ApplySettings( QString("0 %1").arg(value) );

    ui->Motor0Slider->setSliderPosition(value);

}

void MainWindow::on_Motor0Slider_sliderReleased()
{
    if ( AutoAdjustEnabled )
        ApplySettings( QString("0 %1").arg(ui->Motor0Slider->sliderPosition()) );
}

void MainWindow::on_Motor0Slider_valueChanged(int value)
{
    ui->Motor0_Input->setText(QString("%1").arg(value));
}

//

//Slider 1
void MainWindow::on_Motor1_Input_returnPressed()
{
    int value = QString(ui->Motor1_Input->text()).toInt();
    if ( value < ui->Motor1Slider->minimum() || value >ui->Motor1Slider->maximum()  )
    {
        ui->Motor1_Input->setText("INV");

        return;
    }

    if ( AutoAdjustEnabled )
        ApplySettings( QString("1 %1").arg(value) );

    ui->Motor1Slider->setSliderPosition(value);
}

void MainWindow::on_Motor1Slider_sliderReleased()
{
    if ( AutoAdjustEnabled )
        ApplySettings( QString("1 %1").arg(ui->Motor1Slider->sliderPosition()) );
}

void MainWindow::on_Motor1Slider_valueChanged(int value)
{
    ui->Motor1_Input->setText(QString("%1").arg(value));
}
//

//Slider 2
void MainWindow::on_Motor2_Input_returnPressed()
{
    //ui->Motor0_Input->setText();
    int value = QString(ui->Motor2_Input->text()).toInt();
    if ( value < ui->Motor2Slider->minimum() || value >ui->Motor2Slider->maximum()  )
    {
        ui->Motor2_Input->setText("INV");
        return;
    }

    if ( AutoAdjustEnabled )
         ApplySettings( QString("2 %1").arg(value) );

    ui->Motor2Slider->setSliderPosition(value);
}

void MainWindow::on_Motor2Slider_sliderReleased()
{
    if ( AutoAdjustEnabled )
        ApplySettings( QString("2 %1").arg(ui->Motor2Slider->sliderPosition()) );
}

void MainWindow::on_Motor2Slider_valueChanged(int value)
{
    ui->Motor2_Input->setText(QString("%1").arg(value));
}
//

//Slider 3
void MainWindow::on_Motor3_Input_returnPressed()
{
    //ui->Motor0_Input->setText();
    int value = QString(ui->Motor3_Input->text()).toInt();
    if ( value < ui->Motor3Slider->minimum() || value >ui->Motor3Slider->maximum()  )
    {
        ui->Motor3_Input->setText("INV");
        return;
    }
    if ( AutoAdjustEnabled )
        ApplySettings( QString("3 %1").arg(value) );

    ui->Motor3Slider->setSliderPosition(value);
}

void MainWindow::on_Motor3Slider_sliderReleased()
{
    if ( AutoAdjustEnabled )
        ApplySettings( QString("3 %1").arg(ui->Motor3Slider->sliderPosition()) );
}

void MainWindow::on_Motor3Slider_valueChanged(int value)
{
    ui->Motor3_Input->setText(QString("%1").arg(value));
}
//

//Slider 4
void MainWindow::on_Motor4_Input_returnPressed()
{
    //ui->Motor0_Input->setText();
    int value = QString(ui->Motor4_Input->text()).toInt();
    if ( value < ui->Motor4Slider->minimum() || value >ui->Motor4Slider->maximum()  )
    {
        ui->Motor4_Input->setText("INV");
        return;
    }
    if ( AutoAdjustEnabled )
        ApplySettings( QString("4 %1").arg(value) );

    ui->Motor4Slider->setSliderPosition(value);
}
void MainWindow::on_Motor4Slider_sliderReleased()
{
    if ( AutoAdjustEnabled )
        ApplySettings( QString("4 %1").arg(ui->Motor4Slider->sliderPosition()) );
}

void MainWindow::on_Motor4Slider_valueChanged(int value)
{
    ui->Motor4_Input->setText(QString("%1").arg(value));
}
//

void MainWindow::on_ResetButton_pressed()
{
    serial->write("10"); //And we're done.
}

void MainWindow::on_commandButton_pressed() //Gather values from sliders, then passit all along to ApplySettings()
{
    QString str = 0;
    if ( ui->Motor0Slider->isEnabled() )
    {
        str.append(QString(" 0 %1 ").arg(ui->Motor0Slider->sliderPosition()));
    }
    if ( ui->Motor1Slider->isEnabled() )
    {
        str.append(QString(" 1 %1 ").arg(ui->Motor1Slider->sliderPosition()));
    }
    if ( ui->Motor2Slider->isEnabled() )
    {
        str.append(QString(" 2 %1 ").arg(ui->Motor2Slider->sliderPosition()));
    }
    if ( ui->Motor3Slider->isEnabled() )
    {
        str.append(QString(" 3 %1 ").arg(ui->Motor3Slider->sliderPosition()));
    }
    if ( ui->Motor4Slider->isEnabled() )
    {
        str.append(QString(" 4 %1 ").arg(ui->Motor4Slider->sliderPosition()));
    }

    ApplySettings(str);
}
