#include "caEventDialog.h"
#include "caPublicDefine.h"
#include <QtGui/QtGui>
#include <vector>

using namespace  std;

caEventDialog::caEventDialog(CaiApi* pCaiApi, QWidget* parent)
{	
	m_pCaiApi = pCaiApi;
	mEventGroup = new QGroupBox(tr("Event"));
	mEventLabel = new QLabel(tr("Name:"));
	mEventLineEdit = new QLineEdit(this);
	mEventLabel->setBuddy(mEventLineEdit);

	mEventLayout = new QGridLayout;
	mEventLayout->addWidget(mEventLabel, 0, 0, 1, 1);
	mEventLayout->addWidget(mEventLineEdit, 0, 1, 1, 3);
	mEventGroup->setLayout(mEventLayout);

	mSendBtn = createButton(tr("·¢ËÍ"), SLOT(sendEvent()));
	mCancelBtn = createButton(tr("¹Ø±Õ"), SLOT(close()));
	mViewLayout = new QGridLayout;
	mViewLayout->addWidget(mEventGroup, 0, 0, 1, 5);
	mViewLayout->addWidget(mSendBtn, 1, 1, 1, 1);
	mViewLayout->addWidget(mCancelBtn, 1, 3, 1, 1);
	setLayout(mViewLayout);
	setWindowTitle(EVENT_WINDOW_TITLE);
}

caEventDialog::~caEventDialog()
{
	if (mEventLabel){
		delete mEventLabel;
		mEventLabel = NULL;
	}

	if (mEventLineEdit){
		delete mEventLineEdit;
		mEventLineEdit = NULL;
	}

	if (mSendBtn){
		delete mSendBtn;
		mSendBtn = NULL;
	}

	if (mCancelBtn){
		delete mCancelBtn;
		mCancelBtn = NULL;
	}

	if (mEventLayout){
		delete mEventLayout;
		mEventLayout = NULL;
	}

	if (mViewLayout){
		delete mViewLayout;
		mViewLayout = NULL;
	}
}

void caEventDialog::sendEvent()
{	
	QString name = mEventLineEdit->text();
	if (!name.isEmpty()){
		string stdEvent = name.toStdString();
		m_pCaiApi->SendEvent(const_cast<char*>(stdEvent.c_str()));
		mEventLineEdit->clear();
	}
}


QPushButton *caEventDialog::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

#include "moc_caEventDialog.cpp"