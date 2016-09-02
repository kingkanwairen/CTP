#ifndef caStatuskWidget_h
#define caStatusWidget_h

#include <QtGui/QWidget>
#include "CaiApi.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QGridLayout;
class QPushButton;
QT_END_NAMESPACE

class caStatusWidget : public QWidget
{
	Q_OBJECT

public:
	
	caStatusWidget(QWidget * parent = 0, Qt::WindowFlags flags = 0);

	~caStatusWidget();

private:
	
	QPushButton *createButton(const QString &text, const char *member);

public slots:
		void updateStatus();
		void doSetCaiApi(CaiApi* pCaiApi);
		void doDisconnectEngine();

private:
	
	CaiApi* mCaiApi;
	QLabel* mImageLabel;
	QGridLayout* mViewLayout;
	QPushButton* mUpdatehBtn;
	QPixmap mPixmap;
};

#endif