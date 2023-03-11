#ifndef TEXTMODEL_H
#define TEXTMODEL_H

#include <QAbstractTableModel>
#include "localedata.h"

class TextModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit TextModel(QObject *parent = nullptr);

	void setTexts(LocaleData *data);
	void resetModel();

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	void addText(const LocaleData::Text& text);
	void editText(const LocaleData::Text& oldText, const LocaleData::Text& newText);
	void removeText(const LocaleData::Text& text);

	static const Qt::ItemDataRole DATA_ROLE = Qt::UserRole;
	constexpr static int COLUMN_TEXT_ID = 0;
private:
	LocaleData *m_data = nullptr;
};


#endif // TEXTMODEL_H
