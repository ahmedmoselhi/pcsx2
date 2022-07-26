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

struct ControllerBindingInfo
{
	const char* name;
	const char* display_name;
	PAD::ControllerBindingType type;
};

struct ControllerInfo
{
	const char* name;
	const ControllerBindingInfo* bindings;
	u32 num_bindings;
};

const ControllerBindingInfo s_system_binds[] = {
	{"Test", "Test", PAD::ControllerBindingType::Button},
	{"Service", "Service", PAD::ControllerBindingType::Button},
	{"Coin1", "Coin 1", PAD::ControllerBindingType::Button},
	{"Coin2", "Coin 2", PAD::ControllerBindingType::Button},
};

const ControllerBindingInfo s_guitarfreaks_binds[] = {
	{"GfP1Start", "P1 Start", PAD::ControllerBindingType::Button},
	{"GfP1Pick", "P1 Pick", PAD::ControllerBindingType::Button},
	{"GfP1Wail", "P1 Wail", PAD::ControllerBindingType::Button},
	{"GfP1EffectInc", "P1 Effect+", PAD::ControllerBindingType::Button},
	{"GfP1EffectDec", "P1 Effect-", PAD::ControllerBindingType::Button},
	{"GfP1NeckR", "P1 Neck R", PAD::ControllerBindingType::Button},
	{"GfP1NeckG", "P1 Neck G", PAD::ControllerBindingType::Button},
	{"GfP1NeckB", "P1 Neck B", PAD::ControllerBindingType::Button},

	{"GfP2Start", "P2 Start", PAD::ControllerBindingType::Button},
	{"GfP2Pick", "P2 Pick", PAD::ControllerBindingType::Button},
	{"GfP2Wail", "P2 Wail", PAD::ControllerBindingType::Button},
	{"GfP2EffectInc", "P2 Effect+", PAD::ControllerBindingType::Button},
	{"GfP2EffectDec", "P2 Effect-", PAD::ControllerBindingType::Button},
	{"GfP2NeckR", "P2 Neck R", PAD::ControllerBindingType::Button},
	{"GfP2NeckG", "P2 Neck G", PAD::ControllerBindingType::Button},
	{"GfP2NeckB", "P2 Neck B", PAD::ControllerBindingType::Button},
};

const ControllerBindingInfo s_drummania_binds[] = {
	{"DmStart", "Start", PAD::ControllerBindingType::Button},
	{"DmSelectL", "Select L", PAD::ControllerBindingType::Button},
	{"DmSelectR", "Select R", PAD::ControllerBindingType::Button},
	{"DmHihat", "Hihat", PAD::ControllerBindingType::Button},
	{"DmSnare", "Snare", PAD::ControllerBindingType::Button},
	{"DmHighTom", "High Tom", PAD::ControllerBindingType::Button},
	{"DmLowTom", "Low Tom", PAD::ControllerBindingType::Button},
	{"DmCymbal", "Cymbal", PAD::ControllerBindingType::Button},
	{"DmBassDrum", "Bass Drum", PAD::ControllerBindingType::Button},
};

const ControllerBindingInfo s_ddr_binds[] = {
	{"DdrP1Start", "P1 Start", PAD::ControllerBindingType::Button},
	{"DdrP1SelectL", "P1 Select L", PAD::ControllerBindingType::Button},
	{"DdrP1SelectR", "P1 Select R", PAD::ControllerBindingType::Button},
	{"DdrP1FootLeft", "P1 Left", PAD::ControllerBindingType::Button},
	{"DdrP1FootDown", "P1 Down", PAD::ControllerBindingType::Button},
	{"DdrP1FootUp", "P1 Up", PAD::ControllerBindingType::Button},
	{"DdrP1FootRight", "P1 Right", PAD::ControllerBindingType::Button},

	{"DdrP2Start", "P2 Start", PAD::ControllerBindingType::Button},
	{"DdrP2SelectL", "P2 Select L", PAD::ControllerBindingType::Button},
	{"DdrP2SelectR", "P2 Select R", PAD::ControllerBindingType::Button},
	{"DdrP2FootLeft", "P2 Left", PAD::ControllerBindingType::Button},
	{"DdrP2FootDown", "P2 Down", PAD::ControllerBindingType::Button},
	{"DdrP2FootUp", "P2 Up", PAD::ControllerBindingType::Button},
	{"DdrP2FootRight", "P2 Right", PAD::ControllerBindingType::Button},
};

const ControllerBindingInfo s_thrilldrive_binds[] = {
	{"ThrillDriveStart", "Start", PAD::ControllerBindingType::Button},
	{"ThrillDriveGearUp", "Gear Up", PAD::ControllerBindingType::Button},
	{"ThrillDriveGearDown", "Gear Down", PAD::ControllerBindingType::Button},
	{"ThrillDriveWheelAnalog", "Wheel", PAD::ControllerBindingType::Axis},
	{"ThrillDriveWheelLeft", "Wheel Left", PAD::ControllerBindingType::Button},
	{"ThrillDriveWheelRight", "Wheel Right", PAD::ControllerBindingType::Button},
	{"ThrillDriveAccelAnalog", "Acceleration", PAD::ControllerBindingType::HalfAxis},
	{"ThrillDriveAccel", "Acceleration", PAD::ControllerBindingType::Button},
	{"ThrillDriveBrake", "Brake", PAD::ControllerBindingType::Button},
	{"ThrillDriveBrakeAnalog", "Brake", PAD::ControllerBindingType::HalfAxis},
	{"ThrillDriveSeatbelt", "Seatbelt", PAD::ControllerBindingType::Button},
	{"ThrillDriveSeatbeltMotor", "Seatbelt", PAD::ControllerBindingType::Motor},
	{"ThrillDriveWheelMotor", "Wheel", PAD::ControllerBindingType::Motor},
};

const ControllerBindingInfo s_dance864_binds[] = {
	{"Dance864P1Start", "P1 Start", PAD::ControllerBindingType::Button},
	{"Dance864P1Left", "P1 Select L", PAD::ControllerBindingType::Button},
	{"Dance864P1Right", "P1 Select R", PAD::ControllerBindingType::Button},
	{"Dance864P1PadLeft", "P1 Left", PAD::ControllerBindingType::Button},
	{"Dance864P1PadCenter", "P1 Center", PAD::ControllerBindingType::Button},
	{"Dance864P1PadRight", "P1 Right", PAD::ControllerBindingType::Button},

	{"Dance864P2Start", "P2 Start", PAD::ControllerBindingType::Button},
	{"Dance864P2Left", "P2 Select L", PAD::ControllerBindingType::Button},
	{"Dance864P2Right", "P2 Select R", PAD::ControllerBindingType::Button},
	{"Dance864P2PadLeft", "P2 Left", PAD::ControllerBindingType::Button},
	{"Dance864P2PadCenter", "P2 Center", PAD::ControllerBindingType::Button},
	{"Dance864P2PadRight", "P2 Right", PAD::ControllerBindingType::Button},
};

const ControllerBindingInfo s_toysmarch_binds[] = {
	{"ToysMarchP1Start", "P1 Start", PAD::ControllerBindingType::Button},
	{"ToysMarchP1SelectL", "P1 Select L", PAD::ControllerBindingType::Button},
	{"ToysMarchP1SelectR", "P1 Select R", PAD::ControllerBindingType::Button},
	{"ToysMarchP1DrumL", "P1 Drum L", PAD::ControllerBindingType::Button},
	{"ToysMarchP1DrumR", "P1 Drum R", PAD::ControllerBindingType::Button},
	{"ToysMarchP1Cymbal", "P1 Cymbal", PAD::ControllerBindingType::Button},

	{"ToysMarchP2Start", "P2 Start", PAD::ControllerBindingType::Button},
	{"ToysMarchP2SelectL", "P2 Select L", PAD::ControllerBindingType::Button},
	{"ToysMarchP2SelectR", "P2 Select R", PAD::ControllerBindingType::Button},
	{"ToysMarchP2DrumL", "P2 Drum L", PAD::ControllerBindingType::Button},
	{"ToysMarchP2DrumR", "P2 Drum R", PAD::ControllerBindingType::Button},
	{"ToysMarchP2Cymbal", "P2 Cymbal", PAD::ControllerBindingType::Button},
};

const ControllerBindingInfo s_icca_binds[] = {
	{"KeypadP1_0", "P1 Keypad 0", PAD::ControllerBindingType::Button},
	{"KeypadP1_1", "P1 Keypad 1", PAD::ControllerBindingType::Button},
	{"KeypadP1_2", "P1 Keypad 2", PAD::ControllerBindingType::Button},
	{"KeypadP1_3", "P1 Keypad 3", PAD::ControllerBindingType::Button},
	{"KeypadP1_4", "P1 Keypad 4", PAD::ControllerBindingType::Button},
	{"KeypadP1_5", "P1 Keypad 5", PAD::ControllerBindingType::Button},
	{"KeypadP1_6", "P1 Keypad 6", PAD::ControllerBindingType::Button},
	{"KeypadP1_7", "P1 Keypad 7", PAD::ControllerBindingType::Button},
	{"KeypadP1_8", "P1 Keypad 8", PAD::ControllerBindingType::Button},
	{"KeypadP1_9", "P1 Keypad 9", PAD::ControllerBindingType::Button},
	{"KeypadP1_00", "P1 Keypad 00", PAD::ControllerBindingType::Button},
	{"KeypadP1InsertEject", "P1 Insert/Eject Card", PAD::ControllerBindingType::Button},

	{"KeypadP2_0", "P2 Keypad 0", PAD::ControllerBindingType::Button},
	{"KeypadP2_1", "P2 Keypad 1", PAD::ControllerBindingType::Button},
	{"KeypadP2_2", "P2 Keypad 2", PAD::ControllerBindingType::Button},
	{"KeypadP2_3", "P2 Keypad 3", PAD::ControllerBindingType::Button},
	{"KeypadP2_4", "P2 Keypad 4", PAD::ControllerBindingType::Button},
	{"KeypadP2_5", "P2 Keypad 5", PAD::ControllerBindingType::Button},
	{"KeypadP2_6", "P2 Keypad 6", PAD::ControllerBindingType::Button},
	{"KeypadP2_7", "P2 Keypad 7", PAD::ControllerBindingType::Button},
	{"KeypadP2_8", "P2 Keypad 8", PAD::ControllerBindingType::Button},
	{"KeypadP2_9", "P2 Keypad 9", PAD::ControllerBindingType::Button},
	{"KeypadP2_00", "P2 Keypad  00", PAD::ControllerBindingType::Button},
	{"KeypadP2InsertEject", "P2 Insert/Eject Card", PAD::ControllerBindingType::Button},
};

const ControllerInfo s_controller_info[] = {
	{"All", nullptr, 0},
	{"System", s_system_binds, std::size(s_system_binds)},
	{"Dance 86.4", s_dance864_binds, std::size(s_dance864_binds)},
	{"Dance Dance Revolution", s_ddr_binds, std::size(s_ddr_binds)},
	{"Drummania", s_drummania_binds, std::size(s_drummania_binds)},
	{"Guitar Freaks", s_guitarfreaks_binds, std::size(s_guitarfreaks_binds)},
	{"Thrill Drive", s_thrilldrive_binds, std::size(s_thrilldrive_binds)},
	{"Toy's March", s_toysmarch_binds, std::size(s_toysmarch_binds)},
	{"Card Reader", s_icca_binds, std::size(s_icca_binds)},
};

Python2BindingWidget::Python2BindingWidget(QWidget* parent, ControllerSettingsDialog* dialog, u32 port)
	: QWidget(parent)
	, m_dialog(dialog)
{
	m_ui.setupUi(this);

	// Inputs tab
	connect(m_ui.gameTypeFilter, &QComboBox::currentIndexChanged, this, [this](int index) {
		m_ui.inputList->clear();

		if (s_controller_info[index].bindings == nullptr) {
			for (auto controller_entry : s_controller_info) {
				if (controller_entry.bindings == nullptr)
					continue;

				for (u32 i = 0; i < controller_entry.num_bindings; i++) {
					auto entry = controller_entry.bindings[i];

					if (entry.type != PAD::ControllerBindingType::Button)
						continue;

					QListWidgetItem* item = new QListWidgetItem();
					item->setText(tr("%1 - %2").arg(QString::fromStdString(controller_entry.name)).arg(QString::fromStdString(entry.display_name)));
					item->setData(Qt::UserRole, QString::fromStdString(entry.name));
					m_ui.inputList->addItem(item);
				}
			}
		} else {
			for (u32 i = 0; i < s_controller_info[index].num_bindings; i++) {
				auto entry = s_controller_info[index].bindings[i];

				if (entry.type != PAD::ControllerBindingType::Button)
					continue;

				QListWidgetItem* item = new QListWidgetItem();
				item->setText(QString::fromStdString(entry.display_name));
				item->setText(tr("%1 - %2").arg(QString::fromStdString(s_controller_info[index].name)).arg(QString::fromStdString(entry.display_name)));
				item->setData(Qt::UserRole, QString::fromStdString(entry.name));
				m_ui.inputList->addItem(item);
			}
		}
	});

	for (std::size_t i = 0; i < std::size(s_controller_info); i++)
	{
		auto input_entry = s_controller_info[i];
		m_ui.gameTypeFilter->addItem(QString::fromStdString(input_entry.name), (int)i);
	}
	m_ui.gameTypeFilter->setCurrentIndex(0);

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

	// Analog Inputs tab
	for (auto controller_entry : s_controller_info) {
		if (controller_entry.bindings == nullptr)
			continue;

		for (u32 i = 0; i < controller_entry.num_bindings; i++) {
			auto entry = controller_entry.bindings[i];

			if (entry.type != PAD::ControllerBindingType::Axis && entry.type != PAD::ControllerBindingType::HalfAxis)
				continue;

			QListWidgetItem* item = new QListWidgetItem();
			item->setText(tr("%1 - %2").arg(QString::fromStdString(controller_entry.name)).arg(QString::fromStdString(entry.display_name)));
			item->setData(Qt::UserRole, QString::fromStdString(entry.name));
			m_ui.inputListAnalogs->addItem(item);
		}
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

	// Motors tab
	for (auto controller_entry : s_controller_info) {
		if (controller_entry.bindings == nullptr)
			continue;

		for (u32 i = 0; i < controller_entry.num_bindings; i++) {
			auto entry = controller_entry.bindings[i];

			if (entry.type != PAD::ControllerBindingType::Motor)
				continue;

			QListWidgetItem* item = new QListWidgetItem();
			item->setText(tr("%1 - %2").arg(QString::fromStdString(controller_entry.name)).arg(QString::fromStdString(entry.display_name)));
			item->setData(Qt::UserRole, QString::fromStdString(entry.name));
			m_ui.motorList->addItem(item);
		}
	}

	m_ui.keybindListMotors->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ui.keybindListMotors->setAlternatingRowColors(true);
	m_ui.keybindListMotors->setShowGrid(false);
	m_ui.keybindListMotors->verticalHeader()->hide();
	m_ui.keybindListMotors->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui.keybindListMotors->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui.keybindListMotors->horizontalHeader()->setStretchLastSection(true);

	for (int i = 0; i < 1; i++)
	{
		const int row = m_ui.keybindListMotors->rowCount();
		m_ui.keybindListMotors->insertRow(row);

		QTableWidgetItem* item = new QTableWidgetItem();
		item->setText(tr("Test %1").arg(i+1));
		item->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindListMotors->setItem(row, 0, item);

		QTableWidgetItem* item2 = new QTableWidgetItem();
		item2->setText(QString::fromStdString("100%"));
		item2->setData(Qt::UserRole, 100);
		item2->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindListMotors->setItem(row, 1, item2);

		QTableWidgetItem* item3 = new QTableWidgetItem();
		item3->setText(QString::fromStdString("Test Device"));
		item3->setFlags(item->flags() & ~(Qt::ItemIsEditable));
		m_ui.keybindListMotors->setItem(row, 2, item3);
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
