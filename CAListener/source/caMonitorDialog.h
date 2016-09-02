#ifndef caMonitorDialog_h
#define caMonitorDialog_h

#include <QtGui/QDialog>
#include "CaiApi.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QGridLayout;
QT_END_NAMESPACE

class caMonitorDialog : public QDialog
{
	Q_OBJECT

public:
	caMonitorDialog(CaiApi* pCaiApi, QWidget* parent = 0);

	~caMonitorDialog();

	public slots:
		void insertMonitor();
		void browse();
private:
	QPushButton *createButton(const QString &text, const char *member);
private:
	CaiApi* m_pCaiApi;
	QLabel* mPathLabel;
	QLineEdit* mPathLineEdit;
	QGroupBox* mPathGroup;
	QPushButton* mFindBtn;
	QPushButton* mInsertBtn;
	QPushButton* mCancelBtn;
	QGridLayout* mPathLayout;
	QGridLayout* mViewLayout;
};

#endif