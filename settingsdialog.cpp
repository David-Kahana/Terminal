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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QIntValidator>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <FileUtil.h>
#include <QMessageBox>
#include <QJsonObject>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(string settingsFile, QWidget *parent) :
	QDialog(parent),
	m_settingsFile(settingsFile),
    m_ui(new Ui::SettingsDialog),
    m_intValidator(new QIntValidator(0, 4000000, this))
{
    m_ui->setupUi(this);

    m_ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(m_ui->applyButton, &QPushButton::clicked,
            this, &SettingsDialog::apply);
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::showPortInfo);
    connect(m_ui->baudRateBox,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomBaudRatePolicy);
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomDevicePathPolicy);

	fillPortsParameters();
	fillPortsInfo();
	if (loadSettings() != OK)
	{
		updateSettings();
	}
}

SettingsDialog::~SettingsDialog()
{
    delete m_ui;
}

SettingsDialog::Settings SettingsDialog::settings() const
{
    return m_currentSettings;
}

void SettingsDialog::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    const QStringList list = m_ui->serialPortInfoListBox->itemData(idx).toStringList();
    m_ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    m_ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    m_ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    m_ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    m_ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    m_ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void SettingsDialog::apply()
{
    updateSettings();
	saveSettings();
    hide();
}

void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !m_ui->baudRateBox->itemData(idx).isValid();
    m_ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        m_ui->baudRateBox->clearEditText();
        QLineEdit *edit = m_ui->baudRateBox->lineEdit();
        edit->setValidator(m_intValidator);
    }
}

void SettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    const bool isCustomPath = !m_ui->serialPortInfoListBox->itemData(idx).isValid();
    m_ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        m_ui->serialPortInfoListBox->clearEditText();
}

void SettingsDialog::fillPortsParameters()
{
    m_ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    m_ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    m_ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    m_ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    m_ui->baudRateBox->addItem(tr("Custom"));

    m_ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    m_ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    m_ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    m_ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    m_ui->dataBitsBox->setCurrentIndex(3);

    m_ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    m_ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    m_ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    m_ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    m_ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    m_ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    m_ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    m_ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    m_ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    m_ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    m_ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

void SettingsDialog::fillPortsInfo()
{
    m_ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        m_ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    m_ui->serialPortInfoListBox->addItem(tr("Custom"));
}

void SettingsDialog::updateSettings()
{
    m_currentSettings.name = m_ui->serialPortInfoListBox->currentText();

    if (m_ui->baudRateBox->currentIndex() == 4) {
        m_currentSettings.baudRate = m_ui->baudRateBox->currentText().toInt();
    } else {
        m_currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    m_ui->baudRateBox->itemData(m_ui->baudRateBox->currentIndex()).toInt());
    }
    m_currentSettings.stringBaudRate = QString::number(m_currentSettings.baudRate);

    m_currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                m_ui->dataBitsBox->itemData(m_ui->dataBitsBox->currentIndex()).toInt());
    m_currentSettings.stringDataBits = m_ui->dataBitsBox->currentText();

    m_currentSettings.parity = static_cast<QSerialPort::Parity>(
                m_ui->parityBox->itemData(m_ui->parityBox->currentIndex()).toInt());
    m_currentSettings.stringParity = m_ui->parityBox->currentText();

    m_currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                m_ui->stopBitsBox->itemData(m_ui->stopBitsBox->currentIndex()).toInt());
    m_currentSettings.stringStopBits = m_ui->stopBitsBox->currentText();

    m_currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                m_ui->flowControlBox->itemData(m_ui->flowControlBox->currentIndex()).toInt());
    m_currentSettings.stringFlowControl = m_ui->flowControlBox->currentText();

    m_currentSettings.localEchoEnabled = m_ui->localEchoCheckBox->isChecked();
}

int32_t SettingsDialog::loadSettings()
{
	if (!CFileUtil::fileExists(m_settingsFile))
	{
		return -1; //file does not exist
	}
	char* readBuf = nullptr;
	uint32_t readSize = 0;
	if (CFileUtil::readFromFile(m_settingsFile, &readBuf, readSize) != OK)
	{
		return -2; //could not read file
	}
	printf_s("\nRead bytes: %d \n", readSize);
	//for (uint32_t i = 0; i < readSize; ++i)
	//{
		printf_s("\n %s \n", readBuf);
	//}
	QByteArray qba(readBuf, readSize-1);

	QJsonParseError jerr;
	m_settingsJson = QJsonDocument::fromJson(qba,&jerr);
	if (jerr.error != QJsonParseError::ParseError::NoError)
	{
		QMessageBox::warning(parentWidget(), QString("Problem parsing json settings"), 
			jerr.errorString(), QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
		delete[]readBuf;
		return -3; //Problem parsing json
	}
	if (json2settings() != OK)
	{
		return -4; //Problem parsing json settings
	}
	updateSettings();
	return OK;
}

int32_t SettingsDialog::saveSettings()
{
	settings2json();
	QByteArray qba = m_settingsJson.toJson();
	//qba[qba.size() - 1] = EOF;
	printf_s("\nWrite bytes: %d \n", qba.size());
	printf_s("\n %s \n", qba.data());
	if (CFileUtil::writeToFile(m_settingsFile, qba.data(), qba.size()) != OK)
	{
		return -1; //could not write file
	}
	return OK;
}

int32_t SettingsDialog::settings2json()
{
	QJsonObject main;
	main.insert("name", m_currentSettings.name);
	main.insert("baudRate", m_currentSettings.stringBaudRate);
	main.insert("dataBits", m_currentSettings.stringDataBits);
	main.insert("parity", m_currentSettings.stringParity);
	main.insert("stopBits", m_currentSettings.stringStopBits);
	main.insert("flowControl", m_currentSettings.stringFlowControl);
	main.insert("localEchoEnabled", m_currentSettings.localEchoEnabled);
	m_settingsJson = QJsonDocument();
	m_settingsJson.setObject(main);
	return OK;
}

int32_t SettingsDialog::json2settings() 
{
	if (!m_settingsJson.isObject())
	{
		return -1; //json document is not an object 
	}
	QJsonObject main = m_settingsJson.object();
	Settings newSettings;
	if (!checkJsonEntry(main,"name", QJsonValue::Type::String))
	{
		return -2; //json does not contain "name" or "name" is not a string
	}
	newSettings.name = main["name"].toString();
	
	if (!checkJsonEntry(main, "baudRate", QJsonValue::Type::String))
	{
		return -3; //json does not contain "baudRate" or "baudRate" is not a string
	}
	newSettings.stringBaudRate = main["baudRate"].toString();
	
	if (!checkJsonEntry(main, "dataBits", QJsonValue::Type::String))
	{
		return -4; //json does not contain "baudRate" or "baudRate" is not a string
	}
	newSettings.stringDataBits = main["dataBits"].toString();
	
	if (!checkJsonEntry(main, "parity", QJsonValue::Type::String))
	{
		return -5; //json does not contain "parity" or "parity" is not a string
	}
	newSettings.stringParity = main["parity"].toString();
	
	if (!checkJsonEntry(main, "stopBits", QJsonValue::Type::String))
	{
		return -6; //json does not contain "stopBits" or "stopBits" is not a string
	}
	newSettings.stringStopBits = main["stopBits"].toString();
	
	if (!checkJsonEntry(main, "flowControl", QJsonValue::Type::String))
	{
		return -7; //json does not contain "flowControl" or "flowControl" is not a string
	}
	newSettings.stringFlowControl = main["flowControl"].toString();

	if (!checkJsonEntry(main, "localEchoEnabled", QJsonValue::Type::Bool))
	{
		return -8; //json does not contain "localEchoEnabled" or "localEchoEnabled" is not a boolean
	}
	newSettings.localEchoEnabled = main["localEchoEnabled"].toBool();
	updateUiFromNewSettings(newSettings);
	return OK;
}

int32_t SettingsDialog::updateUiFromNewSettings(Settings newSettings)
{
	int idx = m_ui->serialPortInfoListBox->findText(newSettings.name);
	if (idx >= 0)
	{
		m_ui->serialPortInfoListBox->setCurrentIndex(idx);
	}
	idx = m_ui->baudRateBox->findText(newSettings.stringBaudRate);
	if (idx >= 0)
	{
		m_ui->baudRateBox->setCurrentIndex(idx);
	}
	idx = m_ui->dataBitsBox->findText(newSettings.stringDataBits);
	if (idx >= 0)
	{
		m_ui->dataBitsBox->setCurrentIndex(idx);
	}
	idx = m_ui->parityBox->findText(newSettings.stringParity);
	if (idx >= 0)
	{
		m_ui->parityBox->setCurrentIndex(idx);
	}
	idx = m_ui->stopBitsBox->findText(newSettings.stringStopBits);
	if (idx >= 0)
	{
		m_ui->stopBitsBox->setCurrentIndex(idx);
	}
	idx = m_ui->flowControlBox->findText(newSettings.stringFlowControl);
	if (idx >= 0)
	{
		m_ui->flowControlBox->setCurrentIndex(idx);
	}
	m_ui->localEchoCheckBox->setEnabled(newSettings.localEchoEnabled);
	return OK;
}

bool SettingsDialog::checkJsonEntry(QJsonObject obj, QString key, QJsonValue::Type t)
{
	return (obj.contains(key) && obj[key].type() == t);
}