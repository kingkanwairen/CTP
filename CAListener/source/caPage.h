#ifndef caPage_h
#define caPage_h

#include <QtGui/QtGui>

class caNormalPage : public QWidget
{
	Q_OBJECT
public:
    caNormalPage(const QString& ip, const int& nPort, QWidget *parent = 0);

	~caNormalPage();

	int GetPort();
	QString GetIp();

private:

	QGroupBox* mLinkGroup;
	QLabel* mIpLabel;
	QLineEdit* mIpLineEdit;

	QLabel* mPortLabel;
	QLineEdit* mPortLineEdit;

	QGridLayout* mLinkLayout;
	QVBoxLayout *mViewLayout;
};

class caFilePage : public QWidget
{
	Q_OBJECT
public:
    caFilePage(const QString& path, const int& status, QWidget *parent = 0);

	~caFilePage();
	
	bool GetSaveStatus();
	QString GetPath();
private:

	QPushButton *createButton(const QString &text, const char *member);
	QComboBox *createComboBox(const QString &text = QString());

private slots:
	void browse();
private:
	QGroupBox* mPathGroup;
	QLabel* mPathLabel;
	QComboBox *mDirectoryComboBox;
	QPushButton *mBrowseButton;
	QGridLayout* mPathLayout;
	QVBoxLayout *mViewLayout;
	QLabel* mLinkLabel;

};


#endif
