#ifndef USTP_PARAM_DIALOG_H
#define USTP_PARAM_DIALOG_H

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

class USTParamDialog : public QDialog
{
	Q_OBJECT

public:
	USTParamDialog(USTPStrategyWidget* pStrategyWidget, QWidget* parent = 0);

	~USTParamDialog();

private:
	QGroupBox* createOrderGroup();
	QGroupBox* createSpotGroup();
	QComboBox* createComboBox(const QStringList &itemList);
	QPushButton* createButton(const QString &text, const char *member);
	void initConnect(USTPStrategyWidget* pStrategyWidget);

signals:
	void onCreateNewOrder(const QString& firstIns, const QString& secIns, const QString& orderStyle, const QString& direction, const QString& offsetFlag, 
		const QString& hedgeFlag, const double& priceTick, const int& orderQty, const int& firstElapseTime, const int& secElapseTime, const int& openFirstPrice,
		const int& openSecondPrice, const int& referTickNum, const int& superPriceStall, const int& cyclePrice, const int& actionReferTickNum, const int& actionSuperSilpNum, 
		bool isDefineOrder, bool isOpponentPriceType, bool isCancelFirstOrder, bool isCancelSecOrder, bool isCylce, bool isActionReferTick);
	protected slots:
		void createOrder();
		void doEnableBox(int selIndex);

private:

	QLabel* mFirstInsLabel;
	QLabel* mSecondInsLabel;
	QLabel* mStyleLabel;
	QLabel* mDirectionLabel;
	QLabel* mOffsetLabel;
	QLabel* mHedgeLabel;
	QLabel* mQtyLabel;
	QLabel* mPriceLabel;
	QLabel* mFirstTimeLabel;
	QLabel* mSecondTimeLabel;
	QLabel* mReferTickLabel;
	QLabel* mSuperPriceLabel;
	QLabel* mCycleLabel;
	QLabel* mActionReferTickLabel;
	QLabel* mActionSuperSlipLabel;

	QLabel* mFirstOpenLabel;
	QLabel* mSecondOpenLabel;

	QLineEdit* mFirstInsEdit;
	QLineEdit* mSecondInsEdit;
	QComboBox* mStyleComboBox;
	QComboBox* mBSComboBox;
	QComboBox* mOffsetComboBox;
	QComboBox* mHedgeComboBox;

	QDoubleSpinBox* mPriceSpinBox;
	QSpinBox* mQtySpinBox;
	QSpinBox* mFirstTimeSpinBox;
	QSpinBox* mSecondTimeSpinBox;
	QSpinBox* mReferTickSpinBox;
	QSpinBox* mSuperPriceSpinBox;
	QSpinBox* mCycleSpinBox;
	QSpinBox* mActionReferTickSpinBox;
	QSpinBox* mActionSlipSpinBox;

	QSpinBox* mFirstPriceSpinBox;
	QSpinBox* mSecondPriceSpinBox;

	QCheckBox* mDefineOrderCheckBox;
	QCheckBox* mFirstTimeCheckBox;
	QCheckBox* mSecondTimeCheckBox;
	QCheckBox* mOppoentPriceCheckBox;
	QCheckBox* mCycleCheckBox;
	QCheckBox* mActionReferTickCheckBox;

	QPushButton* mConfirmBtn;
	QPushButton* mCancelBtn;

	QGroupBox* mOrderGroup;
	QGroupBox* mSpotGroup;

	QHBoxLayout* mBtnLayout;
	QVBoxLayout* mOrderLayout;
	QVBoxLayout* mViewLayout;
};

#endif