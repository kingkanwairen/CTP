#ifndef USTP_MD_DIALOG_H
#define USTP_MD_DIALOG_H

#include <QtGui/QtGui>

QT_BEGIN_NAMESPACE
class QListWidgetItem;
class QListWidget;
class QLineEdit;
class QLabel;
QT_END_NAMESPACE

class USTPMarketWidget;

class USTPMdDialog : public QDialog
{
    Q_OBJECT

public:
	USTPMdDialog(USTPMarketWidget* pWidget, const QString& inss, QWidget* parent = 0);

	~USTPMdDialog();

signals:

	void onSubscribeMd(const QStringList& inss);

public slots:
	
	void doAddInstrument();

	void doUpInstrument();

	void doDownInstruemnt();

	void doDelInstrument(QListWidgetItem* item);

	void doSelInstrument(QListWidgetItem* item);

	void doDelAllInstruemnt();

	void doSubscribeMarket();

private:
	void initConnect(USTPMarketWidget* pWidget);

	void initList(const QString& list);

	bool saveToFile(const QString& list);

private:
	
	QLabel* mFirstInsLabel;
	QLabel* mSecondInsLabel;
	QLabel* mFirstInsFactorLabel;
	QLabel* mSecondInsFactorLabel;
	QLabel* mConstantValueLabel;
	QLineEdit* mFirstInsEdit;
	QLineEdit* mSecondInsEdit;
	QLineEdit* mFirstInsFactorEdit;
	QLineEdit* mSecondInsFactorEdit;
	QLineEdit* mConstantValueEdit;
    QListWidget * mListWidget;
	QPushButton* mAddBtn;
	QPushButton* mDelBtn;
	QPushButton* mUpBtn;
	QPushButton* mDownBtn;
	QPushButton* mConfirmBtn;
	QPushButton* mCancelBtn;
	QHBoxLayout* mHorizonLayout;
	QGridLayout* mGridLayout;
	QVBoxLayout* mViewLayout;
	int mSelIndex;
};

#endif
