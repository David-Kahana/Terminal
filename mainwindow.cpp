/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "FileUtil.h"
#include "Protocol.h"
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_console(new Console),
    m_settings(new SettingsDialog),
    m_serial(new QSerialPort(this))
{
	m_filesToSend.clear();
    m_ui->setupUi(this);
    m_console->setEnabled(false);
    //setCentralWidget(m_console);
	m_ui->verticalLayout_3->addWidget(m_console);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);

    m_ui->statusBar->addWidget(m_status);

    initActionsConnections();

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    connect(m_console, &Console::getData, this, &MainWindow::writeData);
}

MainWindow::~MainWindow()
{
    delete m_settings;
    delete m_ui;
}

void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) 
	{
        m_console->setEnabled(true);
        m_console->setLocalEchoEnabled(p.localEchoEnabled);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } 
	else 
	{
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        showStatusMessage(tr("Open error"));
    }
}

void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    m_console->setEnabled(false);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void MainWindow::readData()
{
    const QByteArray data = m_serial->readAll();
	for (auto b : data)
	{
		inBuf[inBufInd] = (uint8_t)b;
		inBufInd++;
	}
	if (inBufInd >= 7) //search for replay message
	{
		bool msgFound = false;
		replay = (uint8_t)Replays::NO_REPLAY;
		uint32_t ind = 0;
		while (!msgFound && ind < inBufInd + 7)
		{
			MessageReplay* rep = (MessageReplay*)(&(inBuf[ind]));
			if (rep->start == START_MSG && rep->end == END_MSG && rep->size == 8 && rep->msgType == MessageTypes::REPLAY)
			{
				msgFound = true;
				replay = (uint8_t)(rep->replay);
				inBufInd = 0; //fix it!!!
			}
			ind++;
		}
	}
	inBufInd = inBufInd % 128;
    m_console->putData(data);
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(m_ui->actionClear, &QAction::triggered, m_console, &Console::clear);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
	connect(m_ui->actionSend_FIle, &QAction::triggered, this, &MainWindow::sendFiles);
	connect(m_ui->actionSendFolder, &QAction::triggered, this, &MainWindow::sendFolder);
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::sendFiles()
{
	m_filesToSend.clear();
	m_filesToSend = QFileDialog::getOpenFileNames(this,	"Select one or more files to send",	".","Bitmap (*.bmp);;text (*.txt *.csv);;binary (*.bin)");
	if (m_filesToSend.size() > 0)
	{
		for (int i = 0; i < m_filesToSend.size(); ++i)
		{
			QFileInfo tempInfo(m_filesToSend[i]);
			printf_s("%d) %s, ", i + 1, m_filesToSend[i].toStdString().c_str());
			printf_s("%d Bytes", (int)tempInfo.size());
			if (tempInfo.suffix().compare("bmp", Qt::CaseInsensitive) == 0)
			{
				QImage image;
				image.load(m_filesToSend[i]);
				int w = image.width();
				int h = image.height();
				printf_s(", width: %d, height: %d", w, h);
				int offset = 4;
				uint8_t* sendBuf = new uint8_t[w * h * 3 + offset];
				uint16_t* tmp = (uint16_t *)sendBuf;
				*tmp = (uint16_t)w;
				*(tmp+1) = (uint16_t)h;
				for (int r = 0; r < h; ++r)
				{
					for (int c = 0; c < w; ++c)
					{
						QRgb pix = image.pixel(c, r);
						sendBuf[r * w * 3 + c * 3 + offset] = (uint8_t)qRed(pix);
						sendBuf[r * w * 3 + c * 3 + offset + 1] = (uint8_t)qGreen(pix);
						sendBuf[r * w * 3 + c * 3 + offset + 2] = (uint8_t)qBlue(pix);
					}
				}
				m_serial->write((char*)sendBuf, (qint64)(w * h * 3 + offset));
				//printf_s("\n %x, %x, %x, %x", sendBuf[0], sendBuf[1], sendBuf[2], sendBuf[3]);
			}
			printf_s("\n");
		}
	}
}

void MainWindow::sendFolder()
{
	QString dirName = "";
	QString startDir = "D:/Work/David/Images";
	//dirName = QFileDialog::getExistingDirectory(this, "Select directory", startDir, QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
	dirName = "D:/Work/David/Images/vid"; //shortcut for debugging
	vector<string> fileList;
	int status = CFileUtil::getFilesInDir(dirName.toStdString(), fileList);
	if (status != OK)
	{
		printf_s("\n!!! Error in CFileUtil::getFilesInDir(...) \n");
		return;
	}
	//for (auto & fileName : fileList)
	//{
	//	printf_s("%s\n", fileName.c_str());
	//}
	sort(fileList.begin(), fileList.end());
	//printf_s("\n*** SORTED ***\n");
	//for (auto & fileName : fileList)
	//{
	//	printf_s("%s\n", fileName.c_str());
	//}
	
	QImage inImage;
	vector<QImage> images;
	vector<uint8_t*> txBufs; //buffers sent to serial
	QString fileToLoad;
	int width,height, pitch, totalSize;
	QImage::Format imFormat;
	for (auto fileName : fileList)
	{
		fileToLoad = dirName + "/" + QString::fromStdString(fileName);
		inImage.load(fileToLoad);
		images.push_back(inImage.convertToFormat(QImage::Format_RGB888));
		width = images.back().width();
		height = images.back().height();
		imFormat = images.back().format();
		pitch = images.back().bytesPerLine();
		totalSize = images.back().sizeInBytes();
		printf_s("%03d) %s, width: %d, height: %d, pitch: %d, bytes: %d\n", (int)images.size(), fileName.c_str(), width, height, pitch, totalSize);
		uint32_t offset = 7;
		uint32_t sz = height * pitch + offset;
		uint8_t* buf = new uint8_t[sz];
		*buf = START_MSG;                          //MessageFrame.start
		uint32_t* tmp32 = (uint32_t *)(buf + 1);
	    *tmp32 = (uint32_t)(sz);                   //MessageFrame.size
		*(buf + 5) = (uint8_t)MessageTypes::DATA;  //MessageFrame.msgType 
		const uint8_t* pixels = images.back().constBits();
		memcpy(buf + offset - 1, pixels, height * pitch); //MessageFrame.data 
		*(buf + sz - 1) = END_MSG;                 //MessageFrame.end
		txBufs.push_back(buf);
	}
	//config TX
	MessageConfigPixle msgConf;
	msgConf.start = START_MSG;
	msgConf.size = 19;
	msgConf.msgType = MessageTypes::CONFIG;
	msgConf.cfgType = ConfigTypes::PIXEL;
	msgConf.bpp = 24;
	msgConf.width = 16;
	msgConf.height = 8;
	msgConf.pitch = 48;
	msgConf.frameSize = 384;
	msgConf.end = END_MSG;


	//
	qint64 ret = m_serial->write((char*)(&msgConf), (qint64)(msgConf.size));
	printf_s("\nTotal of %d bytes were sent CONF\n", (int)ret);
	CFileUtil::Sleep(200);
	//wait for replay
	Replays* rep = (Replays *)(&replay);
	while (*rep == Replays::NO_REPLAY)
	{
		CFileUtil::Sleep(10);
	}
	if (*rep != Replays::ACK)
	{
		printf_s("did not get ACK, got %d\n", (int)(*rep));
		return;
	}




	//MessageFrame frameMsg;
	//frameMsg.start = START_MSG;
	//frameMsg.size = msgConf.frameSize + 7;
	//frameMsg.msgType = MessageTypes::DATA;
	//frameMsg.data = txBufs[1];
	//frameMsg.end = END_MSG;



	//char confTx[11];



	//uint8_t* b = txBufs[128];
	//printf_s("128) w: %d, h: %d, p: %d, 127R: %d, 127G: %d, 127B: %d,\n", 
	//	*((uint16_t*)b), *((uint16_t*)(b + 2)), *((uint16_t*)(b + 4)),
	//	*(b + 6 + 127*3), *(b + 6 + 127 * 3 + 1), *(b + 6 + 127 * 3 + 2));
	//uint8_t* tmp = images[0].bits();
	//printf_s("R: %d, G: %d, B: %d\n", (int)(*tmp), (int)(*(tmp+1)), (int)(*(tmp + 2)));
	//uint8_t* b = txBufs[128];
	//int w = (int)(*((uint16_t*)b));
	//int h = (int)(*((uint16_t*)(b + 2)));
	//int p = (int)(*((uint16_t*)(b + 4)));
	//qint64 ret = m_serial->write((char*)b, (qint64)(w * p + 6));

	//qint64 ret = m_serial->write((char*)b, (qint64)(images[128].sizeInBytes()));
	//printf_s("\nTotal of %d bytes were sent\n", (int)ret);
	
	//m_filesToSend.clear();
	//if (m_filesToSend.size() > 0)
	//{
	//	for (int i = 0; i < m_filesToSend.size(); ++i)
	//	{
	//		QFileInfo tempInfo(m_filesToSend[i]);
	//		printf_s("%d) %s, ", i + 1, m_filesToSend[i].toStdString().c_str());
	//		printf_s("%d Bytes", (int)tempInfo.size());
	//		if (tempInfo.suffix().compare("bmp", Qt::CaseInsensitive) == 0)
	//		{
	//			QImage image;
	//			image.load(m_filesToSend[i]);
	//			int w = image.width();
	//			int h = image.height();
	//			printf_s(", width: %d, height: %d", w, h);
	//			int offset = 4;
	//			uint8_t* sendBuf = new uint8_t[w * h * 3 + offset];
	//			uint16_t* tmp = (uint16_t *)sendBuf;
	//			*tmp = (uint16_t)w;
	//			*(tmp + 1) = (uint16_t)h;
	//			for (int r = 0; r < h; ++r)
	//			{
	//				for (int c = 0; c < w; ++c)
	//				{
	//					QRgb pix = image.pixel(c, r);
	//					sendBuf[r * w * 3 + c * 3 + offset] = (uint8_t)qRed(pix);
	//					sendBuf[r * w * 3 + c * 3 + offset + 1] = (uint8_t)qGreen(pix);
	//					sendBuf[r * w * 3 + c * 3 + offset + 2] = (uint8_t)qBlue(pix);
	//				}
	//			}
	//			m_serial->write((char*)sendBuf, (qint64)(w * h * 3 + offset));
	//			//printf_s("\n %x, %x, %x, %x", sendBuf[0], sendBuf[1], sendBuf[2], sendBuf[3]);
	//		}
	//		printf_s("\n");
	//	}
	//}
}
