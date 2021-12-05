/*
 * @*************************************: 
 * @FilePath: \C\string.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-09-30 18:04:11
 * @LastEditors: dof
 * @LastEditTime: 2021-10-12 14:18:39
 * @Descripttion: 
 * @**************************************: 
 */
#include <iostream>
#include <string>

using namespace std;

//class abc
//{
//	public:
//
//	pr
//}

//#define VALUE(x) mdmkey##x
int main()
{
	//    string str1 = "Hello";
	//    string str2 = "World";
	//    string str3;
	//    int  len ;

	// ���� str1 �� str3
	////    str3 = str1;
	//    str3 = "ssssss";
	//    cout << "str3 : " << str3 << endl;
	//
	//	cout<<__func__<<__LINE__<<endl;
	//    // ���� str1 �� str2
	//    str3 = str1 + str2;
	//    cout << "str1 + str2 : " << str3 << endl;
	//
	//    // ���Ӻ�str3 ���ܳ���
	//    len = str3.size();
	//    cout << "str3.size() :  " << len << endl;

//	const char *mdmPath[] = {
//		"InternetGatewayDevice.ManagementServer.",
//		"InternetGatewayDevice.X_BROADCOM_COM_AppCfg.Tr69cCfg."};
//	const char *mdmKey[] = {
//		"PeriodicInformEnable",
//		"PeriodicInformInterval",
//		"URL",
//		"Username",
//		"Password",
//		"ConnectionRequestAuthentication",
//		"ConnectionRequestUsername",
//		"ConnectionRequestPassword"};
//
//	unsigned int pathSize = (sizeof(mdmPath) / sizeof(char)) / (sizeof(mdmPath[0]) / sizeof(char));
//	unsigned int keySize = (sizeof(mdmKey) / sizeof(char)) / (sizeof(mdmKey[0]) / sizeof(char));
//
	string strContent;
//
//	printf("zzzzz [%s: %d] %d %d\n", __func__, __LINE__, pathSize, keySize);
//	strContent = "{\n";
//	for (unsigned int i = 0; i < pathSize; i++)
//	{
//		//		CHgMdm mdmObj(mdmPath[i]);
//
//		printf("[%s: %d] %s\n\n\n\n", __func__, __LINE__, mdmPath[i]);
//		for (unsigned int j = 0; j < keySize; j++)
//		{
//			strContent += j?",\n":"";
//			printf("[%s: %d] fullpath: %s\n", __func__, __LINE__, mdmKey[j]);
//			strContent += "\"";
//			strContent += mdmKey[j];
//			strContent += "\":";
//			strContent += "\"";
//			strContent += "assssssssssssss";
//			strContent += "\"";
//		}
//		printf("for 2 success [%s: %d]\n\n\n\n", __func__, __LINE__);
//	}
	int i = 3; 
	strContent += "\b\b\b";
	strContent += to_string(300);
	strContent += "\n}\n\n";
	cout << "strContent  " << strContent << endl;

	//	cout<<(sizeof(mdmKey)/sizeof(char))/(sizeof(mdmKey[0])/sizeof(char))<<endl;
	//    printf("asdfgh\\n");
	//    printf("zzzzz %s\n", (char *)VALUE(0));
	return 0;
}
