//
// MIT license
//
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "QTRoomAcousticConfig.h"
#include <QtWidgets/QApplication>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
	QApplication application(argc, argv);
	QCommandLineParser parser;
	parser.addHelpOption();
	application.setWindowIcon(QIcon(":/images/Resources/RoomAcousticsNew.png"));
	std::cout << "Log started" << std::endl;
	Q_INIT_RESOURCE(roomaccousticnew);
	RoomAcousticQTConfig configWindow;
	if (application.arguments().count() >= 2) {
		QCommandLineOption configFile("configFile", QCoreApplication::translate("main", "Initialize config with <File>"),
			QCoreApplication::translate("main", "File"));
		QCommandLineOption outputFile("outputFile", QCoreApplication::translate("main", "Save simulation into <File>"),
			QCoreApplication::translate("main", "File"));
		QCommandLineOption metricsFile("metricsFile", QCoreApplication::translate("main", "Save all gathered metrics into <File>"),
			QCoreApplication::translate("main", "File"));
		parser.addOption(configFile);
		parser.addOption(outputFile);
		parser.addOption(metricsFile);
		parser.process(application);
		std::cout
			<< "Benchmarking mode initiated:" << std::endl
			<< "Config file: " << parser.value(configFile).toStdString() << std::endl
			<< "Output file: " << parser.value(outputFile).toStdString() << std::endl
			<< "Metrics file: " << parser.value(metricsFile).toStdString() << std::endl;
		configWindow.Init(true, parser.value(configFile).toStdString(), parser.value(outputFile).toStdString(), parser.value(metricsFile).toStdString());
	} else
		configWindow.Init(false, NULL, NULL, NULL);

	return application.exec();
}
