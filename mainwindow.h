#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "settingsdef.h"
#include "localedata.h"
#include "styles.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace Arc {
class RecentFiles;
}

class QTranslator;
class LocaleData;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void openProject(const QString& fileName);
	bool checkDataValid();
	void updateSelection();

	//generated
	void on_actionAbout_triggered();
	void on_actionAdd_triggered();
	void on_actionEdit_triggered();
	void on_actionRemove_triggered();
	void on_edit_filter_textChanged(const QString &text);
	void on_actionSettings_triggered();
	void on_actionFind_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionSave_triggered();
	void on_actionCompile_triggered();

protected:
	void closeEvent(QCloseEvent *event) override;
	void changeEvent(QEvent *event) override;
private:
	Ui::MainWindow *ui;
	class TextModel *model = nullptr;
	class QUndoStack *undoStack = nullptr;
	class QSettings *settings = nullptr;
	QAction *redoAction = nullptr;
	QAction *undoAction = nullptr;
	LocaleData *data = nullptr;
	class FindWidget *findWidget = nullptr;
	Settings settingsData;
	Arc::Style style;
	QTranslator *m_translator = nullptr;
	QTranslator *m_qtTranslator = nullptr;
	Arc::RecentFiles *recentFiles = nullptr;
	QString currentProject;

	static const QString PROGRAM_NAME;

	void updateUI(bool enabled);
	void readSettings();
	void writeSettings();
	void updateColumns();
	void applySettings(const Settings& settings);

	void showEditDialog(bool isNew);

	void changeLanguage(int lang);
	void changeLanguage(const QString& language);

	void saveProject(const QString& fileName);
};

#endif // MAINWINDOW_H
