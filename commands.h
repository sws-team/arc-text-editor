#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include "textmodel.h"

class AddTextCommand : public QUndoCommand
{
public:

	explicit AddTextCommand(const QList<LocaleData::Text>& texts, TextModel *model);
	explicit AddTextCommand(const LocaleData::Text& text, TextModel *model);

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

	void execCommand(bool create);

private:
	bool isNew;
	QList<LocaleData::Text> texts;
	TextModel *model = nullptr;
};

class EditTextCommand : public QUndoCommand
{
public:
	explicit EditTextCommand(TextModel *model,
							 const LocaleData::Text& oldText,
							 const LocaleData::Text& newText);

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

private:
	TextModel *model = nullptr;
	LocaleData::Text oldText;
	LocaleData::Text newText;
};

#endif // COMMANDS_H
