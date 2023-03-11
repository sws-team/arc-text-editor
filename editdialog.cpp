#include "editdialog.h"
#include "ui_editdialog.h"

#include <QMessageBox>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QRegularExpressionValidator>

EditDialog::EditDialog(LocaleData *data, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EditDialog)
	,localeData(data)
	,validator(new QRegularExpressionValidator(this))
{
	ui->setupUi(this);
	validator->setRegularExpression(LocaleData::VALID_SYMBOLS);
	ui->edit_id->setValidator(validator);

	foreach(const QString& locale, data->locales()) {
		TextWidget *txt_wgt = new TextWidget(locale, this);
		ui->content->addWidget(txt_wgt);
		textWidgets.push_back(txt_wgt);
	}
}

EditDialog::~EditDialog()
{
	delete ui;
}

void EditDialog::accept()
{
	if (ui->edit_id->text().isEmpty()) {
		QMessageBox::critical(this, this->windowTitle(), tr("ID is empty!"));
		return;
	}

	foreach(const TextWidget* widget, textWidgets) {
		if (widget->text().isEmpty()) {
			QMessageBox::critical(this, this->windowTitle(), tr("Translation in (%1) is empty!").arg(widget->locale()));
			return;
		}
	}

	//check valid
	const QStringList ids = localeData->textIDs();
	if (ids.count(ui->edit_id->text()) > 1) {
		QMessageBox::critical(this, this->windowTitle(), tr("Text ID already exists!"));
		return;
	}

	QDialog::accept();
}

void EditDialog::setText(const LocaleData::Text &text)
{
	ui->edit_id->setText(text.textID);
	QMapIterator<QString, QString> it(text.texts);
	while(it.hasNext()) {
		it.next();
		foreach(TextWidget* widget, textWidgets) {
			if (it.key() == widget->locale()) {
				widget->setText(it.value());
				break;
			}
		}
	}
}

LocaleData::Text EditDialog::text() const
{
	LocaleData::Text text;
	text.textID = ui->edit_id->text();
	foreach(const TextWidget* widget, textWidgets) {
		text.texts.insert(widget->locale(), widget->text());
	}
	return text;
}

TextWidget::TextWidget(const QString &locale, QWidget *parent)
	: QWidget(parent)
	,m_locale(locale)
{
	QHBoxLayout *layout =  new QHBoxLayout(this);
	QGroupBox *gb_locale = new QGroupBox(this);
	gb_locale->setTitle(locale);
	QHBoxLayout *layout_locale = new QHBoxLayout(gb_locale);
	edit_text = new QPlainTextEdit(gb_locale);
	layout_locale->addWidget(edit_text);
	gb_locale->setLayout(layout_locale);
	layout->addWidget(gb_locale);
	setLayout(layout);
}

TextWidget::~TextWidget()
{

}

void TextWidget::setText(const QString &text)
{
	edit_text->setPlainText(text);
}

QString TextWidget::text() const
{
	return edit_text->toPlainText();
}

QString TextWidget::locale() const
{
	return m_locale;
}

