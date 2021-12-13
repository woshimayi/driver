#include <QtGui>

class Widget : public QWidget
{
	Q_OBJECT
  public: Widget(QWidget *parent = 0);
  protected:
	void paintEvent(QPaintEvent *event);
  private:
	QPoint m_CurrentPos;
	QPixmap m_Pixmap;
};

Widget::Widget(QWidget *parent)
	: QWidget(parent, Qt::FramelessWindowHint)
{
	m_Pixmap = QPixmap(200, 200);
	resize(200, 200);
	QPainter painter(&m_Pixmap);
	painter.setPen(Qt::red);
	painter.drawRect(50, 50, 100, 100);
	setMask(m_Pixmap);
}


void Widget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, m_Pixmap);
}

#include "main.moc"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	Widget *widget = new Widget;
	widget->show();
	return app.exec();
}
