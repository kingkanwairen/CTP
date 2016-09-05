#ifndef USTP_ENBED_DIALOG_H
#define USTP_ENBED_DIALOG_H

#include <QtGui/QtGui>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
class QLabel;
class QSpinBox;
class QCheckBox;
class QGroupBox;
QT_END_NAMESPACE

class USTPStrategyWidget;

class USTPEnbedDialog : public QDialog
{
	Q_OBJECT

public:
	USTPEnbedDialog(const int& row, const QString& selIns, const double& orderPrice,  const int& direction, QWidget* parent = 0);

	~USTPEnbedDialog();

private:
	QGroupBox* createOrderGroup();
	QComboBox* createComboBox(const QStringList &itemList);
	QPushButton* createButton(const QString &text, const char *member);
	void initConnect(QWidget* widget);
	int getPrcision(const double& value);
signals:
	void onCreateEnbedOrder(const QString& selIns, const QString& direction, const QString& offsetFlag, const double& priceTick, const int& orderQty,
		const int& openSuperPrice, const int& stopProfitNum, const int& stopLossNum, const int& row);
	protected slots:
		void createShortCutOrder();

private:

	QLabel* mFirstInsLabel;
	QLabel* mDirectionLabel;
	QLabel* mOffsetLabel;
	QLabel* mQtyLabel;
	QLabel* mPriceLabel;
	QLabel* mOpenSuperLabel;
	QLabel* mStopProfitLabel;
	QLabel* mStopLossLabel;

	QLineEdit* mFirstInsEdit;
	QComboBox* mBSComboBox;
	QComboBox* mOffsetComboBox;
	
	QSpinBox* mQtySpinBox;
	QSpinBox* mOpenSuperBox;
	QSpinBox* mStopProfitBox;
	QSpinBox* mStopLossBox;
	QDoubleSpinBox* mPriceSpinBox;
	

	QPushButton* mConfirmBtn;
	QPushButton* mCancelBtn;

	QGroupBox* mOrderGroup;

	QHBoxLayout* mBtnLayout;
	QVBoxLayout* mOrderLayout;
	QVBoxLayout* mViewLayout;

	QString mSelInsName;
	double mOrderPrice;
	int mDirection;
	int mRow;

};

#endif