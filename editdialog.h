#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>
#include "localedata.h"

namespace Ui {
class EditDialog;
}

class TextWidget;
class EditDialog : public QDialog
{
	Q_OBJECT

public:
	explicit EditDialog(LocaleData *data, QWidget *parent = nullptr);
	~EditDialog();

	void accept() override;

	void setText(const LocaleData::Text& text);
	LocaleData::Text text() const;

private:
	Ui::EditDialog *ui;

	QList<TextWidget*> textWidgets;
	LocaleData *localeData;
	class QRegularExpressionValidator* validator;
};

class TextWidget : public QWidget
{
	Q_OBJECT
public:
	TextWidget(const QString& locale, QWidget* parent);
	virtual ~TextWidget();

	void setText(const QString& text);

	QString text() const;
	QString locale() const;

private:
	class QPlainTextEdit *edit_text;
	QString m_locale;
};

#endif // EDITDIALOG_H
