#ifndef SHEETTEST_H
#define SHEETTEST_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QComboBox>
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui { class SheetTest; }
QT_END_NAMESPACE

class SheetTest : public QWidget
{
    Q_OBJECT

public:
    SheetTest(QWidget *parent = nullptr);
    ~SheetTest();

private:
    void initUi();
    void initData();
    void initSlots();

private slots:
    void getSelectd();                  ///< 槽函数 获取当前选中的 行列号 或行列范围 或坐标
    void dealLeds(QSet<int>, int);      ///< 槽函数 处理当前选中的行的灯
    void setModel(int);                 ///< 槽函数 处理单元格选中模式
    void setGrainSize(int);             ///< 槽函数 处理单元格选中粒度

private:
    Ui::SheetTest *ui;
    QTableWidget* qtw;
    QPushButton* selectOnOff, *allOnOff;
    QComboBox* cbModel, *cbGrainSize;
    QVector<QString> color;             ///< 存放灯的样式字符串
    QVector<QVector<QLabel*>> leds;     ///< 用label来表示灯
    QVector<QVector<QWidget*>> qws;     ///< 用于放置灯的widget，因为如果直接把label灯放入单元格，无法设置居中显示
    QVector<QVector<QHBoxLayout*>> las; ///< 用于设置灯的布局
    enum colorType {GREY = 0, RED, GREEN, YELLOW};  ///< 灯颜色枚举
};
#endif // SHEETTEST_H
