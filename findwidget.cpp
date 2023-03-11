#include "findwidget.h"
#include "ui_findwidget.h"

#include <QTableWidget>
#include <QTableWidgetItem>

const QColor FindWidget::selectedColor = QColor(171,238,170, 255);

FindWidget::FindWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FindWidget)
	,table(nullptr)
{
	ui->setupUi(this);
	setWindowFlags(Qt::Popup);

	connect(ui->edit_text, &QLineEdit::returnPressed, this, &FindWidget::on_button_find_next_clicked);
}

FindWidget::~FindWidget()
{
	delete ui;
}

void FindWidget::setTable(QTableWidget *table)
{
	this->table = table;
	table->setStyleSheet("QTableWidget::item:selected{ background-color: rgba(255, 255, 0, 32) }");
}

void FindWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	ui->edit_text->setFocus();
}

void FindWidget::closeEvent(QCloseEvent *event)
{
	clearFind();
	QWidget::closeEvent(event);
}

void FindWidget::on_button_find_next_clicked()
{
	findText(ui->edit_text->text(), true);
}

void FindWidget::on_button_find_prev_clicked()
{
	findText(ui->edit_text->text(), false);
}

void FindWidget::findText(const QString &text, bool forward)
{
	if (table == nullptr)
		return;

	auto checkItem = [this, text](QTableWidgetItem *item) -> bool {
		const QString cellText = item->text();
		if (!text.isEmpty() && cellText.contains(text, Qt::CaseInsensitive)) {
			item->setBackground(QBrush(selectedColor));
			table->setCurrentItem(item);
			return true;
		}
		item->setBackground(QBrush(Qt::white));
		return false;
	};

	const QModelIndex index = table->currentIndex();
	int startRow = index.isValid()? index.row() : 0;
	int column = index.isValid()? index.column() : 0;
	if (forward) {
		//start index
		{
			column++;
			if (column == table->columnCount()) {
				column = 0;
				startRow++;
			}
		}
		//from current to end
		for (int row = startRow; row < table->rowCount(); ++row) {
			for (; column < table->columnCount(); ++column) {
				if (checkItem(table->item(row, column)))
					return;
			}
			column = 0;
		}
		//from start to current
		for (int row = 0; row > startRow; ++row) {
			for (int column = 0; column < table->columnCount(); ++column) {
				if (checkItem(table->item(row, column)))
					return;
			}
		}
	}
	else {
		//start index
		{
			column--;
			if (column == -1) {
				column = table->columnCount() - 1;
				startRow--;
			}
		}
		//from current to start
		for (int row = startRow; row >= 0; --row) {
			for (; column >= 0; --column) {
				if (checkItem(table->item(row, column)))
					return;
			}
			column = table->columnCount() - 1;
		}
		//from end to current
		for (int row = table->rowCount() - 1; row > startRow; --row) {
			for (column = table->columnCount() - 1; column >= 0; --column) {
				if (checkItem(table->item(row, column)))
					return;
			}
		}
	}
}

void FindWidget::clearFind()
{
	for (int row = 0; row < table->rowCount(); ++row) {
		for (int column = 0; column < table->columnCount(); ++column) {
			QTableWidgetItem *item = table->item(row, column);
			item->setBackground(QBrush(Qt::white));
		}
	}
}