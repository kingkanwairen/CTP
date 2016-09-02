#ifndef caConfigDialog_h
#define caConfigDialog_h
#define DEFAULT_PORT 15903

#include <QtGui/QDialog>
#include "caPage.h"

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
QT_END_NAMESPACE


class caConfigDialog : public QDialog
{
    Q_OBJECT

public:
	caConfigDialog(QString ip = QString(tr("127.0.0.1")), int iPort = DEFAULT_PORT, QString path = QDir::currentPath(), int status = 0, QWidget* parent = 0);

	~caConfigDialog();

private:

	void InitIconItem();

	void InitConnect();

signals:
		
	void onSetCofig(const QString& ip, const int& port);

public slots:

    void ChangePage(QListWidgetItem *currentItem, QListWidgetItem *previousItem);
	
	void ConfirmParam();

	void ApplyParam();

private:

    QListWidget * mListWidget;
    QStackedWidget * mStackWidget;
	QPushButton* mConfirmBtn;
	QPushButton* mCancelBtn;
	QPushButton* mApplicationBtn;
	QHBoxLayout* mHorizonLayout;
	QHBoxLayout* mBtnLayout;
	QVBoxLayout* mViewLayout;
	QListWidgetItem *mNormalItem;
	QListWidgetItem *mFileItem;
	caNormalPage* mNormalPage;
	caFilePage* mFilePage;
};

#endif
