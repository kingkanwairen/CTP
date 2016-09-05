#ifndef ___BASE64_H___   
#define ___BASE64_H___   

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <string>
using namespace std;

class USTPBase64
{
public:
	/*����
	DataByte
	[in]��������ݳ���,���ֽ�Ϊ��λ
	*/
	string Encode(const unsigned char* Data,int DataByte);
	/*����
	DataByte
	[in]��������ݳ���,���ֽ�Ϊ��λ
	OutByte
	[out]��������ݳ���,���ֽ�Ϊ��λ,�벻Ҫͨ������ֵ����
	������ݵĳ���
	*/
	string Decode(const char* Data,int DataByte,int& OutByte);


	bool decodeLicense(const QString& path);

	QString getUserId();

	QString getDateTime();

	bool getDateIsValid(QString& dateTime);

private:
	QStringList mDecodeLicense;

};

#endif // ___BASE64_H___  
