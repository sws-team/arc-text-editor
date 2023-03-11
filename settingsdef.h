#ifndef SETTINGSDEF_H
#define SETTINGSDEF_H

#include <QFont>

static const QStringList languages = {
	"en", "ru"
};

struct Settings {
	QFont font;
	int styleType = 0;
	int language = 0;
	bool loadLast = true;
};

#endif // SETTINGSDEF_H
