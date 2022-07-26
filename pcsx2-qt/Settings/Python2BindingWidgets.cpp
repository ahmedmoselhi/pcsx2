/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2022  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"

#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <algorithm>

#include "Settings/Python2BindingWidgets.h"
#include "Settings/ControllerSettingsDialog.h"
#include "Settings/ControllerSettingWidgetBinder.h"
#include "Settings/InputBindingWidget.h"
#include "Settings/SettingsDialog.h"
#include "EmuThread.h"
#include "QtUtils.h"
#include "SettingWidgetBinder.h"

#include "common/StringUtil.h"
#include "pcsx2/HostSettings.h"
#include "pcsx2/PAD/Host/PAD.h"

#include "pcsx2/USB/usb-python2/inputs/native/usb-python2-native.h"

Python2BindingWidget::Python2BindingWidget(QWidget* parent, ControllerSettingsDialog* dialog, u32 port)
	: QWidget(parent)
	, m_dialog(dialog)
{
	m_ui.setupUi(this);

	for (auto entry : usb_python2::native::buttonLabelList) {
		QListWidgetItem* item = new QListWidgetItem();
		item->setText(QString::fromStdString(entry));
		item->setData(Qt::UserRole, QString::fromStdString(entry));
		m_ui.inputList->addItem(item);
	}

	{
        m_ui.gameTypeFilter->addItem(QString::fromStdString("All"));
	}

	m_ui.keybindList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ui.keybindList->setAlternatingRowColors(true);
	m_ui.keybindList->setShowGrid(false);
	m_ui.keybindList->verticalHeader()->hide();
	m_ui.keybindList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui.keybindList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui.keybindList->horizontalHeader()->setStretchLastSection(true);

	for (int i = 0; i < 3; i++)
	{
		const int row = m_ui.keybindList->rowCount();
		m_ui.keybindList->insertRow(row);

		QTableWidgetItem* item = new QTableWidgetItem();
		item->setText(tr("Test %1").arg(i+1));
		item->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindList->setItem(row, 0, item);

		QCheckBox* cb = new QCheckBox(m_ui.keybindList);
		cb->setChecked(false);

		QWidget* cbw = new QWidget();
		QHBoxLayout* hbox = new QHBoxLayout(cbw);
		hbox->addWidget(cb);
		hbox->setAlignment(Qt::AlignCenter);
		hbox->setContentsMargins(0,0,0,0);

		m_ui.keybindList->setCellWidget(row, 1, cbw);

		QTableWidgetItem* item2 = new QTableWidgetItem();
		item2->setText(QString::fromStdString("Test Device"));
		item2->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindList->setItem(row, 2, item2);
	}


	m_ui.keybindListAnalogs->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ui.keybindListAnalogs->setAlternatingRowColors(true);
	m_ui.keybindListAnalogs->setShowGrid(false);
	m_ui.keybindListAnalogs->verticalHeader()->hide();
	m_ui.keybindListAnalogs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui.keybindListAnalogs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui.keybindListAnalogs->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	m_ui.keybindListAnalogs->horizontalHeader()->setStretchLastSection(true);

	for (int i = 0; i < 2; i++)
	{
		const int row = m_ui.keybindListAnalogs->rowCount();
		m_ui.keybindListAnalogs->insertRow(row);

		QTableWidgetItem* item = new QTableWidgetItem();
		item->setText(tr("Test %1").arg(i+1));
		item->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindListAnalogs->setItem(row, 0, item);

		QTableWidgetItem* item2 = new QTableWidgetItem();
		item2->setText(QString::fromStdString("0%"));
		item2->setData(Qt::UserRole, 100);
		item2->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindListAnalogs->setItem(row, 1, item2);

		QTableWidgetItem* item3 = new QTableWidgetItem();
		item3->setText(QString::fromStdString("100%"));
		item3->setData(Qt::UserRole, 100);
		item3->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindListAnalogs->setItem(row, 2, item3);

		QTableWidgetItem* item4 = new QTableWidgetItem();
		item4->setText(QString::fromStdString("Test Device"));
		item4->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindListAnalogs->setItem(row, 3, item4);
	}


	m_ui.keybindListOutputs->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ui.keybindListOutputs->setAlternatingRowColors(true);
	m_ui.keybindListOutputs->setShowGrid(false);
	m_ui.keybindListOutputs->verticalHeader()->hide();
	m_ui.keybindListOutputs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui.keybindListOutputs->horizontalHeader()->setStretchLastSection(true);

	for (int i = 0; i < 1; i++)
	{
		const int row = m_ui.keybindListOutputs->rowCount();
		m_ui.keybindListOutputs->insertRow(row);

		QTableWidgetItem* item = new QTableWidgetItem();
		item->setText(tr("Test %1").arg(i+1));
		item->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindListOutputs->setItem(row, 0, item);

		QTableWidgetItem* item4 = new QTableWidgetItem();
		item4->setText(QString::fromStdString("Test Device"));
		item4->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindListOutputs->setItem(row, 1, item4);
	}
}

Python2BindingWidget::~Python2BindingWidget() = default;

QIcon Python2BindingWidget::getIcon() const
{
	return QIcon::fromTheme("artboard-2-line");
}

void Python2BindingWidget::saveAndRefresh()
{
	QtHost::QueueSettingsSave();
	g_emu_thread->applySettings();
}
