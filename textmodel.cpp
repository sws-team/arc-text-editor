#include "textmodel.h"

TextModel::TextModel(QObject *parent)
	: QAbstractTableModel(parent)
{

}

void TextModel::setTexts(LocaleData *data)
{
	beginResetModel();
	m_data = data;
	endResetModel();
}

void TextModel::resetModel()
{
	beginResetModel();
	endResetModel();
}

QVariant TextModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Orientation::Horizontal) {
		if (section == COLUMN_TEXT_ID) {
			return "TextID";
		}
		else {
			const QStringList headers = m_data->locales();
			return headers.value(section - 1);
		}
	}
	return QVariant();
}

int TextModel::rowCount(const QModelIndex &) const
{
	return m_data->data().size();
}

int TextModel::columnCount(const QModelIndex &) const
{
	return 1 + m_data->locales().size();
}

QVariant TextModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	const int row = index.row();
	const LocaleData::Text text = m_data->data().value(row);
	switch (role)
	{
	case DATA_ROLE:
		return QVariant::fromValue(text);
	case Qt::DisplayRole:
	{
		const int column = index.column();
		if (column == COLUMN_TEXT_ID) {
			return text.textID;
		}
		else {
			const QString locale = m_data->locales().value(column - 1);
			return text.texts.value(locale);
		}
	}
	default:
		break;
	}

	return QVariant();
}

void TextModel::addText(const LocaleData::Text &text)
{
	const int row = m_data->addText(text);
	beginInsertRows(QModelIndex(), row, row);
	endInsertRows();
}

void TextModel::editText(const LocaleData::Text &oldText, const LocaleData::Text &newText)
{
	m_data->editText(oldText, newText);
	emit dataChanged(QModelIndex(), QModelIndex());
}

void TextModel::removeText(const LocaleData::Text &text)
{
	const int row = m_data->removeText(text);
	beginRemoveRows(QModelIndex(), row, row);
	endRemoveRows();
}
