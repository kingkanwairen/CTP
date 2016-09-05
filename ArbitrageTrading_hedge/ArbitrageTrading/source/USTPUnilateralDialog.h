#ifndef USTP_UNILATERAL_DIALOG_H
#define USTP_UNILATERAL_DIALOG_H

#include <QtGui/QtGui>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
class QLabel;
class QSpinBox;
class QCheckBox;
class QGroupBox;
QT_END_NAMESPACE

class USTPUnilateralWidget;

class USTPUnilateralDialog : public QDialog
{
	Q_OBJECT

public:
	USTPUnilateralDialog(USTPUnilateralWidget* pUnilateralWidget, QWidget* parent = 0);

	~USTPUnilateralDialog();

private:
	QGroupBox* createOrderGroup();
	QComboBox* createComboBox(const QStringList &itemList);
	QPushButton* createButton(const QString &text, const char *member);
	void initConnect(USTPUnilateralWidget* pUnilateralWidget);

signals:
	void onCreateUnilateralOrder(const QString& instrumentId, const QString& direction, const QString& offsetFlag, 
		const double& orderPrice, const int& orderQty, const int& cyclePoint, const int& cycleNum);
	protected slots:
		void createOrder();

private:

	QLabel* mInsLabel;
	QLabel* mDirectionLabel;
	QLabel* mOffsetLabel;
	QLabel* mOrderPriceLabel;
	QLabel* mOrderQtyLabel;
	QLabel* mCyclePointLabel;
	QLabel* mCycleNumLabel;

	QLineEdit* mInsEdit;
	QComboBox* mBSComboBox;
	QComboBox* mOffsetComboBox;
	QDoubleSpinBox* mOrderPriceSpinBox;
	QSpinBox* mOrderQtySpinBox;
	QSpinBox* mCyclePointSpinBox;	
	QSpinBox* mCycleNumSpinBox;

	QPushButton* mConfirmBtn;
	QPushButton* mCancelBtn;

	QGroupBox* mOrderGroup;

	QHBoxLayout* mBtnLayout;
	QVBoxLayout* mViewLayout;
};

#endif