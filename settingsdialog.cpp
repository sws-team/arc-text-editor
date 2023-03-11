#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "styles.h"

#include <QInputDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
	ui->sbx_fontSize->setMinimum(4);
	ui->sbx_fontSize->setMaximum(50);
	ui->cmb_style->addItems(Arc::Style::STYLE_NAMES);

	ui->cmb_lang->addItem("English");
	ui->cmb_lang->addItem("Русский");
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::setSettings(const Settings &settings)
{
	ui->sbx_fontSize->setValue(settings.font.pointSize());
	ui->cmb_style->setCurrentIndex(settings.styleType);
	ui->cmb_lang->setCurrentIndex(settings.language);
}

Settings SettingsDialog::settings() const
{
	Settings result;
	result.font = ui->fontComboBox->font();
	result.font.setPointSize(ui->sbx_fontSize->value());
	result.styleType = ui->cmb_style->currentIndex();
	result.language = ui->cmb_lang->currentIndex();
	return result;
}
