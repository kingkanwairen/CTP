#ifndef caLogManager_h
#define caLogManager_h
#include <QtCore/QFile>

class caLogManager : public QObject
{
	Q_OBJECT
public:
	caLogManager();

	~caLogManager();
	/*!
	��ʼ��ȫ�ֶ���
	*/
	static bool initialize();

	/*!
	����ȫ�ֶ���
	*/
	static bool finalize();

	static void saveData(const QString& data);

	static void setFilePath(bool isSave, const QString& path);

private:
	static caLogManager *mThis;
	bool mIsSave;
	QFile mLogFile;
};

#endif 