
#include <QtCore/QCoreApplication>
 
//
#include <QList>
#include <QMap>
#include <QDebug>
#include <iostream>
#include <QTextCodec>
 
//
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
 
	//QList容器测试
	qDebug() << QObject::tr("QList容器测试");	
	QList<int> list;
	for (int i = 0; i < 10; i++)
	{
		list.insert(list.end(), i);
	}
 
	QList<int>::iterator j;
	for (j = list.begin(); j!=list.end(); ++j)
	{
		qDebug() << (*j);
		*j = (*j) * 10;
	}
 
	QList<int>::const_iterator cj;
	for (cj = list.begin(); cj!=list.end(); ++cj)
	{
		qDebug() << (*cj);
	}
	
	
	//QMap容器测试
	qDebug() << QObject::tr("QMap容器测试");
	QMap<QString, QString> map;
	map.insert("beijing", "111");
	map.insert("shanghai", "021");
	map.insert("tianjin", "022");
 
	QMap<QString, QString>::const_iterator ck;
	for(ck = map.constBegin(); ck!=map.constEnd(); ck++)
	{
		qDebug() << ck.key() << " " << ck.value();
	}
 
	QMap<QString, QString>::iterator mk;
	mk = map.find("beijing");
	if (mk != map.end())
	{
		mk.value() = "010";
	}
	
	qDebug() << "";
 
	QMap<QString, QString>::const_iterator nck;
	for(nck = map.constBegin(); nck!=map.constEnd(); nck++)
	{
		qDebug() << nck.key() << " " << nck.value();
	}
 
	return a.exec();
}