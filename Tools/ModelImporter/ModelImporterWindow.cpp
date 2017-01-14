/* NekoEngine - ModelImporter
 *
 * ModelImporterWindow.cpp
 * Author: Alexandru Naiman
 *
 * ModelImporterWindow implementation
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ModelImporterWindow.h"
#include "ui_ModelImporterWindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include "AboutDialog.h"

#include "AssimpConverter.h"

ModelImporterWindow::ModelImporterWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ModelImporterWindow)
{
	ui->setupUi(this);

	connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(ShowAboutDialog()));
	connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(ShowOpenFile()));

	connect(ui->browseInputFile, SIGNAL(clicked()), this, SLOT(ShowOpenFile()));
	connect(ui->browseOutputFile, SIGNAL(clicked()), this, SLOT(ShowSaveFile()));

	connect(ui->convertButton, SIGNAL(clicked()), this, SLOT(Convert()));
}

void ModelImporterWindow::ShowOpenFile()
{
	ui->inputFileEdit->setText(QFileDialog::getOpenFileName(this, "Select model"));
}

void ModelImporterWindow::ShowSaveFile()
{
	ui->outputFileEdit->setText(QFileDialog::getSaveFileName(this, "Select output file", "" , "*.nmesh"));
}

void ModelImporterWindow::ShowAboutDialog()
{
	AboutDialog *dlg = new AboutDialog(this);
	dlg->exec();
	delete dlg;
}

void ModelImporterWindow::Convert()
{
	AssimpConverter converter;

	if (converter.Convert(ui->inputFileEdit->text().toStdString().c_str(), ui->outputFileEdit->text().toStdString().c_str(), ui->forceSMChk->checkState() == Qt::Checked))
		QMessageBox::information(this, "Conversion complete", "The mesh has been converted");
	else
		QMessageBox::information(this, "Conversion failed", "The mesh has not been converted");
}

ModelImporterWindow::~ModelImporterWindow()
{
	delete ui;
}
