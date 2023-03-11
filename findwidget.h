#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>

namespace Ui {
class FindWidget;
}

class QTableWidget;

class FindWidget : public QWidget
{
	Q_OBJECT

public:
	explicit FindWidget(QWidget *parent = nullptr);
	~FindWidget();

	void setTable(QTableWidget *table);

protected:
	void showEvent(QShowEvent *event) override;
	void closeEvent(QCloseEvent *event) override;

private slots:
	void on_button_find_next_clicked();

	void on_button_find_prev_clicked();

private:
	Ui::FindWidget *ui;
	QTableWidget *table;

	void findText(const QString& text, bool forward);
	void clearFind();
	static const QColor selectedColor;
};

#endif // FINDWIDGET_H
