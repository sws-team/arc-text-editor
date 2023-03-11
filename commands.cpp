#include "commands.h"

#include <QDebug>

AddTextCommand::AddTextCommand(const QList<LocaleData::Text> &texts, TextModel *model)
	: QUndoCommand(Q_NULLPTR)
	,isNew(false)
	,texts(texts)
	,model(model)
{

}

AddTextCommand::AddTextCommand(const LocaleData::Text &text, TextModel *model)
	: QUndoCommand(Q_NULLPTR)
	,isNew(true)
	,model(model)
{
	texts.push_back(text);
}

void AddTextCommand::undo()
{
	if (isNew)
		execCommand(false);
	else
		execCommand(true);
}

void AddTextCommand::redo()
{
	if (isNew)
		execCommand(true);
	else
		execCommand(false);
}

void AddTextCommand::execCommand(bool create)
{
	if (create) {
		foreach(const LocaleData::Text& text, texts) {
			model->addText(text);
		}
	}
	else {
		foreach(const LocaleData::Text& text, texts) {
			model->removeText(text);
		}
	}
}

EditTextCommand::EditTextCommand(TextModel *model,
								 const LocaleData::Text &oldText,
								 const LocaleData::Text &newText)
	: QUndoCommand(Q_NULLPTR)
	,model(model)
	,oldText(oldText)
	,newText(newText)
{

}

void EditTextCommand::undo()
{
	model->editText(newText, oldText);
}

void EditTextCommand::redo()
{
	model->editText(oldText, newText);
}
