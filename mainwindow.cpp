#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutwidget.h"
#include "commands.h"
#include "version.h"
#include "localedata.h"
#include "editdialog.h"
#include "settingsdialog.h"
#include "findwidget.h"
#include "recentfiles.h"
#include "textmodel.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QSettings>
#include <QUndoStack>
#include <QDir>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QDesktopServices>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLabel>
#include <QInputDialog>
#include <QUndoStack>

const QString localesPath = "./locales.json";
const QString MainWindow::PROGRAM_NAME = MainWindow::tr("Arc Text Editor");
const QString fileExtension = "at-proj";

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	,ui(new Ui::MainWindow)
	,model(new TextModel(this))
	,undoStack(new QUndoStack(this))
	,data(new LocaleData)
	,findWidget(new FindWidget(this))
	,m_translator(new QTranslator(this))
	,m_qtTranslator(new QTranslator(this))
{
	ui->setupUi(this);
	this->setWindowTitle(PROGRAM_NAME);

	qApp->setApplicationName(PROGRAM_NAME);
	qApp->setApplicationVersion(APP_VERSION);
	qApp->setOrganizationName("SWS TEAM");

	settings = new QSettings("./settings.ini", QSettings::IniFormat, this);
	recentFiles = new Arc::RecentFiles(ui->menuRecent_files, this);

	model->setTexts(data);
	ui->table_texts->setModel(model);

	connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::closeAllWindows);
	connect(undoStack, &QUndoStack::cleanChanged, ui->actionSave, &QAction::setDisabled);
	connect(ui->table_texts, &QTableView::doubleClicked, this, &MainWindow::on_actionEdit_triggered);
	connect(ui->actionCheck_valid, &QAction::triggered, this, &MainWindow::checkDataValid);
	connect(recentFiles, &Arc::RecentFiles::openFileTriggered, this, &MainWindow::openProject);
	connect(ui->table_texts->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateSelection);

	ui->table_texts->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->table_texts->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui->table_texts->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->table_texts->setSortingEnabled(true);

	redoAction = undoStack->createRedoAction(this, tr("Redo"));
	undoAction = undoStack->createUndoAction(this, tr("Undo"));
	redoAction->setIcon(QIcon(":/icons/redo.png"));
	undoAction->setIcon(QIcon(":/icons/undo.png"));
	redoAction->setShortcut(QKeySequence("Ctrl+Y"));
	undoAction->setShortcut(QKeySequence("Ctrl+Z"));
	ui->menuEdit->addAction(redoAction);
	ui->menuEdit->addAction(undoAction);

//	findWidget->setTable(ui->table_texts);
	updateUI(false);
	if (!QFileInfo::exists(localesPath)) {
		QMessageBox::critical(this, PROGRAM_NAME,
							  tr("Locales file is missing!\nCheck %1").arg(localesPath));
	}
	else {
		data->init(localesPath);
	}
	model->resetModel();
	updateColumns();
	updateSelection();

	readSettings();
	applySettings(settingsData);
	changeLanguage(settingsData.language);

	if (settingsData.loadLast)
		QMetaObject::invokeMethod(this, "openProject",
								  Qt::QueuedConnection,
								  Q_ARG(const QString&, recentFiles->last()));
}

void MainWindow::openProject(const QString &fileName)
{
	if (fileName.isEmpty())
		return;
	if (!QFileInfo::exists(fileName)) {
		QMessageBox::critical(this, PROGRAM_NAME, tr("Project file is missing!"));
		return;
	}
	if (!data->load(fileName)) {
		QMessageBox::critical(this, PROGRAM_NAME, tr("Load project file error!"));
		return;
	}
	model->resetModel();
	currentProject = fileName;
	this->setWindowTitle(PROGRAM_NAME + " - " + currentProject);
	recentFiles->appendFileName(fileName);
	undoStack->clear();

	updateUI(true);
}

MainWindow::~MainWindow()
{
	writeSettings();
	delete data;
	delete ui;
}

void MainWindow::readSettings()
{
	settings->beginGroup(QLatin1String("MainWindow"));
	restoreGeometry(settings->value(QLatin1String("geometry")).toByteArray());
	restoreState(settings->value(QLatin1String("state"), QByteArray()).toByteArray());
	settings->endGroup();

	settings->beginGroup(QLatin1String("Settings"));
	settingsData.font = settings->value("font", qApp->font()).value<QFont>();
	settingsData.styleType = settings->value("style", Arc::Style::CLASSIC).toInt();
	settingsData.language = settings->value("language", 0).toInt();
	settingsData.loadLast = settings->value("loadLast", true).toBool();
	settings->endGroup();

	recentFiles->load(settings);
}

void MainWindow::writeSettings()
{
	settings->beginGroup(QLatin1String("MainWindow"));
	settings->setValue(QLatin1String("geometry"), saveGeometry());
	settings->setValue(QLatin1String("state"), saveState());
	settings->endGroup();

	settings->beginGroup(QLatin1String("Settings"));
	settings->setValue("font", QVariant::fromValue<QFont>(settingsData.font));
	settings->setValue("style", settingsData.styleType);
	settings->setValue("language", settingsData.language);
	settings->setValue("loadLast", settingsData.loadLast);
	settings->endGroup();

	recentFiles->save(settings);
}

void MainWindow::on_actionAbout_triggered()
{
	Arc::AboutWidget::showAboutWindow(PROGRAM_NAME, APP_VERSION, 2023, this);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (!undoStack->isClean()) {
		if (QMessageBox::question(this, PROGRAM_NAME, tr("Are you sure to exit?"),
								  QMessageBox::Yes | QMessageBox::No)
				!= QMessageBox::Yes) {
			event->ignore();
			return;
		}
	}
	QMainWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		ui->retranslateUi(this);
	}
	QMainWindow::changeEvent(event);
}

void MainWindow::updateUI(bool enabled)
{
	ui->actionSave->setEnabled(enabled);
}

void MainWindow::updateColumns()
{
	const int columnWidth = this->width() / (model->columnCount() + 1);
	for (int column = 0; column < model->columnCount(); ++column) {
		ui->table_texts->setColumnWidth(column, columnWidth);
	}
}

void MainWindow::applySettings(const Settings &settings)
{
	qApp->setFont(settings.font);
	if (settings.styleType != settingsData.styleType)
		style.setStyle(static_cast<Arc::Style::STYLE_TYPE>(settings.styleType));
	if (settings.language != settingsData.language)
		changeLanguage(settings.language);
	settingsData = settings;
}

void MainWindow::showEditDialog(bool isNew)
{
	LocaleData::Text oldText;
	EditDialog dlg(data, this);
	if (!isNew) {
		const QModelIndex index = ui->table_texts->currentIndex();
		if (index.isValid())
			oldText = index.sibling(index.row(), TextModel::COLUMN_TEXT_ID).data(TextModel::DATA_ROLE).value<LocaleData::Text>();
	}
	dlg.setText(oldText);
	if (dlg.exec() == QDialog::Accepted) {
		const LocaleData::Text newText = dlg.text();
		if (isNew) {
			AddTextCommand*command = new AddTextCommand(newText, model);
			undoStack->push(command);
		}
		else {
			EditTextCommand *command = new EditTextCommand(model, oldText, newText);
			undoStack->push(command);
		}
	}
}

void MainWindow::on_actionAdd_triggered()
{
	showEditDialog(true);
}

void MainWindow::on_actionEdit_triggered()
{
	showEditDialog(false);
}

void MainWindow::on_actionRemove_triggered()
{
	QList<LocaleData::Text> texts;
	const QModelIndexList indexes = ui->table_texts->selectionModel()->selectedRows(TextModel::COLUMN_TEXT_ID);
	foreach(const QModelIndex& index, indexes) {
		LocaleData::Text text;
		text = index.data(TextModel::DATA_ROLE).value<LocaleData::Text>();
		texts.push_back(text);
	}
	if (texts.isEmpty())
		return;
	undoStack->push(new AddTextCommand(texts, model));
}

void MainWindow::updateSelection()
{
	const QModelIndexList indexes = ui->table_texts->selectionModel()->selectedRows(TextModel::COLUMN_TEXT_ID);
	const bool enabled = !indexes.isEmpty();
	ui->actionEdit->setEnabled(enabled);
	ui->actionRemove->setEnabled(enabled);
}

void MainWindow::on_edit_filter_textChanged(const QString &text)
{
	for (int row = 0; row < model->rowCount(); ++row) {
		bool found = false;
		for (int column = 0; column < model->columnCount(); ++column) {
			if (found)
				break;
			const QModelIndex index = model->index(row, column);
			const QString textID = index.sibling(index.row(), TextModel::COLUMN_TEXT_ID).data(TextModel::DATA_ROLE).value<LocaleData::Text>().textID;
			const bool enabled = text.isEmpty()?true:textID.contains(text, Qt::CaseInsensitive);
			if (enabled)
				found = true;
		}
		ui->table_texts->setRowHidden(row, !found);
	}
}

void MainWindow::on_actionSettings_triggered()
{
	SettingsDialog dlg(this);
	dlg.setSettings(settingsData);
	if (dlg.exec() == QDialog::Accepted) {
		applySettings(dlg.settings());
	}
}

void MainWindow::on_actionFind_triggered()
{
	QPoint p = this->pos();
	p.setY(p.y() + this->height());
	findWidget->move(p);
	findWidget->resize(this->width(), findWidget->height());
	findWidget->show();
}

void MainWindow::changeLanguage(int lang)
{
	const QString langName = languages.value(lang);
	changeLanguage(langName);
}

void MainWindow::changeLanguage(const QString &language)
{
	const QString path = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
	if (m_qtTranslator->load("qt_" + language, path)) {
		qApp->installTranslator(m_qtTranslator);
	}
	if(m_translator->load(QString(":/tr_%1.qm").arg(language))) {
		qApp->installTranslator(m_translator);
	}
}

bool MainWindow::checkDataValid()
{
	const LocaleData::ValidErrorData result = data->checkValid();
	if (result.type == LocaleData::DATA_VALID)
		return true;
	QString errorString = LocaleData::printableError(result.type);
	if (!result.text.isEmpty()) {
		errorString += QString("(%1)").arg(result.text);
	}
	QMessageBox::critical(this, PROGRAM_NAME, errorString);
	return false;
}

void MainWindow::on_actionOpen_triggered()
{
	const QString fileName = QFileDialog::getOpenFileName(this, tr("Choose file"),
														  currentProject,
														  tr("Arc Text Projects(*.%1)").
														  arg(fileExtension));
	if (fileName.isEmpty())
		return;

	openProject(fileName);
}

void MainWindow::on_actionSave_as_triggered()
{
	const QString fileName = QFileDialog::getSaveFileName(this, tr("Choose file"),
														  currentProject,
														  tr("Arc Text Projects(*.%1)").
														  arg(fileExtension));
	if (fileName.isEmpty())
		return;

	saveProject(fileName);
}

void MainWindow::saveProject(const QString& fileName)
{
	data->save(fileName);
	openProject(fileName);
}

void MainWindow::on_actionSave_triggered()
{
	if (currentProject.isEmpty())
		on_actionSave_as_triggered();
	else
		saveProject(currentProject);
}

void MainWindow::on_actionCompile_triggered()
{
	const QString path = QFileDialog::getExistingDirectory(this, tr("Choose path"), QFileInfo(currentProject).path());
	if (path.isEmpty())
		return;
	if (!data->compile(path)) {
		QMessageBox::critical(this, PROGRAM_NAME, tr("Compile error!"));
	}
}

