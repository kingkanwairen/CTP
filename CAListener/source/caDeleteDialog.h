#ifndef caDeleteDialog_h
#define caDeleteDialog_h

#include <QtGui/QDialog>
#include "CaiApi.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QGridLayout;
QT_END_NAMESPACE

class caDeleteDialog : public QDialog
{
	Q_OBJECT

public:
	caDeleteDialog(CaiApi* pCaiApi, QWidget* parent = 0);

	~caDeleteDialog();

	public slots:
		void deleteEvent();
private:
	QPushButton *createButton(const QString &text, const char *member);
private:
	CaiApi* m_pCaiApi;
	QLabel* mEventLabel;
	QLineEdit* mEventLineEdit;
	QGroupBox* mEventGroup;
	QPushButton* mDeleteBtn;
	QPushButton* mCancelBtn;
	QGridLayout* mEventLayout;
	QGridLayout* mViewLayout;
};

#endif