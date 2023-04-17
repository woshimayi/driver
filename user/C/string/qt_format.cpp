/*
 * @*************************************:
 * @FilePath: /user/C/string/qt_format.cpp
 * @version:
 * @Author: dof
 * @Date: 2023-02-07 10:14:51
 * @LastEditors: dof
 * @LastEditTime: 2023-02-07 13:54:36
 * @Descripttion:
 * @**************************************:
 */

	// QSqlQuery类提供执行和操作的SQL语句的方法。
	// 可以用来执行DML（数据操作语言）语句，如SELECT、INSERT、UPDATE、DELETE，
	// 以及DDL（数据定义语言）语句，例如CREATE TABLE。
	// 也可以用来执行那些不是标准的SQL的数据库特定的命令。
	QSqlQuery q;

	QString create_sql = "create table student (id int primary key, name varchar(30), age int)";
	QString select_max_sql = "select max(id) from student";
	QString insert_sql = "insert into student values (?, ?, ?)";
	QString update_sql = "update student set name = :name where id = :id";
	QString select_sql = "select id, name from student";
	QString select_all_sql = "select * from student";
	QString delete_sql = "delete from student where id = ?";
	QString clear_sql = "delete from student";

	q.prepare(create_sql);
	if (!q.exec())
	{
		qDebug() << q.lastError();
	}
	else
	{
		qDebug() << "table created!";
	}

	// 查询最大id
	int max_id = 0;
	q.prepare(select_max_sql);
	if (!q.exec())
	{
		qDebug() << q.lastError();
	}
	else
	{
		while (q.next())
		{
			max_id = q.value(0).toInt();
			qDebug() << QString("max id:%1").arg(max_id);
		}
	}
	// 插入数据
	q.prepare(insert_sql);
	q.addBindValue(max_id + 1);
	q.addBindValue("name");
	q.addBindValue(25);
	if (!q.exec())
	{
		qDebug() << q.lastError();
	}
	else
	{
		qDebug() << "inserted!";
	}

	// 更新数据
	q.prepare(update_sql);
	q.bindValue(":name", "Qt");
	q.bindValue(":id", 1);
	if (!q.exec())
	{
		qDebug() << q.lastError();
	}
	else
	{
		qDebug() << "updated!";
	}

	// 查询部分数据
	if (!q.exec(select_sql))
	{
		qDebug() << q.lastError();
	}
	else
	{
		while (q.next())
		{
			int id = q.value("id").toInt();
			QString name = q.value("name").toString();

			qDebug() << QString("id:%1    name:%2").arg(id).arg(name);
		}
	}

	// 查询所有数据
	q.prepare(select_all_sql);
	if (!q.exec())
	{
		qDebug() << q.lastError();
	}
	else
	{
		while (q.next())
		{
			int id = q.value(0).toInt();
			QString name = q.value(1).toString();
			int age = q.value(2).toInt();

			qDebug() << QString("id:%1    name:%2    age:%3").arg(id).arg(name).arg(age);
		}
	}

	// 删除数据
	q.prepare(delete_sql);
	q.addBindValue(max_id);
	if (!q.exec())
	{
		qDebug() << q.lastError();
	}
	else
	{
		qDebug() << "deleted!";
	}

	// 清空表
	q.prepare(clear_sql);
	if (!q.exec())
	{
		qDebug() << q.lastError();
	}
	else
	{
		qDebug() << "cleared";
	}

// 关闭数据库
database.close();

// 删除数据库
QFile::remove("database.db");