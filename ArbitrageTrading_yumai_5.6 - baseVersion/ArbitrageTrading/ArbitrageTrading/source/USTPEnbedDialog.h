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
	USTPEnbedDialog(const QString& firstIns, const QString& secIns, const double& orderPrice,  const int& direction, QWidget* parent = 0);

	~USTPEnbedDialog();

private:
	QGroupBox* createOrderGroup();
	QComboBox* createComboBox(const QStringList &itemList);
	QPushButton* createButton(const QString &text, const char *member);
	void initConnect(QWidget* widget);
	int getPrcision(const double& value);

signals:
	void onCreateEnbedOrder(const QString& firstIns, const QString& secIns, const QString& orderStyle, const QString& direction, const QString& offsetFlag, 
		const QString& hedgeFlag, const double& priceTick, const int& orderQty, bool isEnbed);
	protected slots:
		void createShortCutOrder();
		void createEnbedOrder();

private:

	QLabel* mFirstInsLabel;
	QLabel* mSecondInsLabel;
	QLabel* mStyleLabel;
	QLabel* mDirectionLabel;
	QLabel* mOffsetLabel;
	QLabel* mHedgeLabel;
	QLabel* mQtyLabel;
	QLabel* mPriceLabel;


	QLineEdit* mFirstInsEdit;
	QLineEdit* mSecondInsEdit;
	QComboBox* mStyleComboBox;
	QComboBox* mBSComboBox;
	QComboBox* mOffsetComboBox;
	QComboBox* mHedgeComboBox;

	QDoubleSpinBox* mPriceSpinBox;
	QSpinBox* mQtySpinBox;

	QPushButton* mConfirmBtn;
	QPushButton* mEnbedBtn;

	QGroupBox* mOrderGroup;

	QHBoxLayout* mBtnLayout;
	QVBoxLayout* mOrderLayout;
	QVBoxLayout* mViewLayout;

	QString mFirstInsName;
	QString mSecInsName;
	double mOrderPrice;
	int mDirection;

};

#endif