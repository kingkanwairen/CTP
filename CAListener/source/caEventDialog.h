#ifndef caEventDialog_h
#define caEventDialog_h

#include <QtGui/QDialog>
#include "CaiApi.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QGridLayout;
QT_END_NAMESPACE

class caEventDialog : public QDialog
{
	Q_OBJECT

public:
	caEventDialog(CaiApi* pCaiApi, QWidget* parent = 0);

	~caEventDialog();

	public slots:
		void sendEvent();
private:
	QPushButton *createButton(const QString &text, const char *member);
private:
	CaiApi* m_pCaiApi;
	QLabel* mEventLabel;
	QLineEdit* mEventLineEdit;
	QGroupBox* mEventGroup;
	QPushButton* mSendBtn;
	QPushButton* mCancelBtn;
	QGridLayout* mEventLayout;
	QGridLayout* mViewLayout;
};

#endif