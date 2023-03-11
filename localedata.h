#ifndef LOCALEDATA_H
#define LOCALEDATA_H

#include <QMap>
#include <QMetaType>

class LocaleData
{
public:
	LocaleData();

	struct Text {
		QString textID;
		QMap<QString, QString> texts;
		bool operator==(const Text& other){
			return this->textID == other.textID;
		}
	};

	void init(const QString& path);
	void init(const QStringList& locales);

	bool load(const QString& fileName);
	bool save(const QString& fileName);

	QStringList locales() const;
	QStringList textIDs() const;

	QString translate(const QString& localeName, const QString& textID) const;
	Text getText(const QString& textID) const;

	void editText(const Text& oldText, const Text& newText);
	int addText(const Text& text);
	int removeText(const Text& text);

	QList<Text> data() const;

	enum VALID_ERRORS {
		DATA_VALID,
		HAS_DUPLICATES,
		INCORRECT_ID,
		ID_EMPTY,
	};

	struct ValidErrorData {
		VALID_ERRORS type = DATA_VALID;
		QString text;
	};

	ValidErrorData checkValid() const;
	static QString printableError(LocaleData::VALID_ERRORS type);

	static const QRegularExpression VALID_SYMBOLS;

	struct StashData {
		enum STASH_TYPE {
			EDITED,
			ADDED,
			REMOVED,
		};
		STASH_TYPE type;
		Text text;
	};

	bool compile(const QString& path);

	static const QString LOCALE_PATH;
private:
	QStringList m_locales;//languages list
	QList<Text> m_data;
	int idCounter;

	static const QChar TEXT_SEPARATOR;
	static const QChar ENDLINE;
	static const QChar DELIMER;
};

Q_DECLARE_METATYPE(LocaleData::Text);

#endif // LOCALEDATA_H
