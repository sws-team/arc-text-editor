#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "settingsdef.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget *parent = nullptr);
	~SettingsDialog();

	void setSettings(const Settings& settings);
	Settings settings() const;

private:
	Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
