#include "localedata.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

const QRegularExpression LocaleData::VALID_SYMBOLS = QRegularExpression("[A-Za-z_.0-9]*");
const QChar LocaleData::TEXT_SEPARATOR = ':';
const QChar LocaleData::ENDLINE = '\n';
const QChar LocaleData::DELIMER = '$';

LocaleData::LocaleData()
{

}

void LocaleData::init(const QString &path)
{
	m_locales.clear();
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	const QByteArray data = file.readAll();
	file.close();
	const QJsonArray json = QJsonDocument::fromJson(data).array();
	foreach(const QJsonValue& jsonLocale, json) {
		const QString locale = jsonLocale.toString();
		m_locales.push_back(locale);
	}
}

void LocaleData::init(const QStringList &locales)
{
	m_locales = locales;
}

bool LocaleData::load(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}
	const QByteArray bytes = file.readAll();
	QJsonParseError error;
	const QJsonArray json = QJsonDocument::fromJson(bytes, &error).array();
	if (error.error != QJsonParseError::ParseError::NoError) {
		return false;
	}
	m_data.clear();
	for (int i = 0; i < json.size(); ++i) {
		const QJsonObject jsonText = json.at(i).toObject();
		LocaleData::Text text;
		text.textID = jsonText["ID"].toString();

		const QJsonObject jsonTr = jsonText["tr"].toObject();
		for(const QString& locale : m_locales) {
			text.texts.insert(locale, jsonTr[locale].toString());
		}
		m_data.push_back(text);
	}
	return true;
}

bool LocaleData::save(const QString &fileName)
{
	QJsonArray json;
	for(const LocaleData::Text& text : m_data) {
		QJsonObject jsonText;
		jsonText["ID"] = text.textID;

		QJsonObject jsonTr;
		for(const QString& locale : m_locales) {
			jsonTr[locale] = text.texts.value(locale);
		}
		jsonText["tr"] = jsonTr;
		json.append(jsonText);
	}
	const QByteArray bytes = QJsonDocument(json).toJson();
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly))
		return false;
	const quint64 written = file.write(bytes);
	if (written == 0)
		return false;
	return true;
}

QStringList LocaleData::locales() const
{
	return m_locales;
}

QStringList LocaleData::textIDs() const
{
	QStringList result;
	foreach(const Text& text, m_data) {
		result.push_back(text.textID);
	}
	std::sort(result.begin(), result.end());
	return result;
}

QString LocaleData::translate(const QString &localeName, const QString &textID) const
{
	foreach(const Text& text, m_data) {
		if (text.textID == textID) {
			return text.texts.value(localeName);
		}
	}
	return QString();
}

LocaleData::Text LocaleData::getText(const QString &textID) const
{
	foreach(const Text& text, m_data) {
		if (text.textID == textID) {
			return text;
		}
	}
	return Text();
}

void LocaleData::editText(const LocaleData::Text &oldText, const Text &newText)
{
	for(LocaleData::Text &t : m_data) {
		if (t.textID == oldText.textID) {
			t = newText;
			break;
		}
	}
}

int LocaleData::addText(const LocaleData::Text &text)
{
	const int row = m_data.size();
	m_data.push_back(text);
	return row;
}

int LocaleData::removeText(const Text &text)
{
	for (int i = 0; i < m_data.size(); ++i) {
		if (m_data.value(i).textID == text.textID) {
			m_data.removeAt(i);
			return i;
		}
	}
	return -1;
}

QList<LocaleData::Text> LocaleData::data() const
{
	return m_data;
}

LocaleData::ValidErrorData LocaleData::checkValid() const
{
	ValidErrorData errorData;
	QSet<QString> textIDs;
	foreach(const Text& text, m_data) {
		if (text.textID.isEmpty()) {
			errorData.type = ID_EMPTY;
			break;
		}
		if (!VALID_SYMBOLS.match(text.textID).hasMatch()) {
			errorData.type = INCORRECT_ID;
			errorData.text = text.textID;
			break;
		}
		const QString lowCaseTextID = text.textID.toLower();
		if (textIDs.contains(lowCaseTextID)) {
			errorData.type = HAS_DUPLICATES;
			errorData.text = text.textID;
			break;
		}
		textIDs.insert(lowCaseTextID);
	}
	return errorData;
}

QString LocaleData::printableError(VALID_ERRORS type)
{
	switch (type)
	{
	case LocaleData::HAS_DUPLICATES:
		return QObject::tr("Data has duplicates!");
	case LocaleData::ID_EMPTY:
		return QObject::tr("Some id(s) is empty!");
	case LocaleData::INCORRECT_ID:
		return QObject::tr("Some id(s) is incorrect!");
	default:
		break;
	}
	return QString();
}

bool LocaleData::compile(const QString &path)
{
	QString result;
	result += "#include \"gamemanagers.h\"\n";
	result += "#include \"engine.h\"\n";
	result += "\n";
	result += "#define TR_PAIR(X,Y) std::pair<int, sf::String>(X, Y)\n";
	result += "\n";
	result += "void GameManagers::loadTranslations()\n";
	result += "{\n";

	foreach(const QString& locale, m_locales) {
		result += QString("\tstd::map<int, sf::String> %1Translation;\n").arg(locale);
	}
	result += "\n";

//	englishTranslation.insert(TR_PAIR(TranslationsManager::LANGUAGE_ID, L"English"));

	foreach (const Text& text, m_data) {
		QMapIterator<QString, QString> it(text.texts);
		while(it.hasNext()) {
			it.next();
			result += QString("\t%1Translation.insert(TR_PAIR(%2, L\"%3\"));\n")
					.arg(it.key()).arg(text.textID).arg(it.value());
		}
		result += "\n";
	}

	foreach(const QString& locale, m_locales) {
		result += QString("\tEngine::Instance().translationsManager()->addTranslation(std::string(\"%1\"), %1Translation);\n").arg(locale);
	}
	result += "}\n";

	const QString fileName = path + "/gametranslations.cpp";
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly)) {
		return false;
	}
	file.write(result.toUtf8());
	file.close();
	return true;
}

