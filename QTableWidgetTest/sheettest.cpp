#include "sheettest.h"
#include "ui_sheettest.h"

#define ROWS 30

#define NO_Focus

///< 用于取消鼠标选中某单元格时，会出现虚线框的效果
#ifdef NO_Focus
#include <QStyledItemDelegate>
class NoFocusDelegate : public QStyledItemDelegate {
public:
    NoFocusDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {}

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem optionCopy(option);
        optionCopy.state &= ~QStyle::State_HasFocus; ///< 移除焦点状态
        QStyledItemDelegate::paint(painter, optionCopy, index);
    }
};
#endif

SheetTest::SheetTest(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SheetTest)
{
    ui->setupUi(this);
    initData();
    initUi();
    initSlots();
}

SheetTest::~SheetTest()
{
    ///< 经测试这个clear方式也会释放单元格中自定义的组件，所以下面的释放就不用了
    /// 但仍需注意，调用了clear后就最好别再使用单元格中的组件了，以免造成异常(可能指针未置空，我懒得验证了)
    qtw->clear();

    /// 释放申请存放led的内存
//    for (int i = 0; i < ROWS; i ++)
//    {
//        for (int j = 0; j < 3; j ++)
//        {
//            delete leds[i][j];
//            ///< 注意，下面两个的释放顺序，qws[i][j]相当于las[i][j]的容器，需要后释放，否则会出异常
//            delete las[i][j];
//            delete qws[i][j];
//        }
//    }

    delete ui;
}

void SheetTest::initData()
{
    QString comm_s("min-width:20px;min-height:20px;max-width:20px;max-height:20px;border-radius:13px;border:3px rgb(147, 147, 147);border-style:outset;");
    color.push_back(comm_s + QString("background-color:rgb(195, 195, 195);"));    ///< GREY
    color.push_back(comm_s + QString("background-color:rgb(255, 0, 0);"));        ///< RED
    color.push_back(comm_s + QString("background-color:rgb(0, 255, 0);"));        ///< GREEN
    color.push_back(comm_s + QString("background-color:rgb(255, 255, 0);"));      ///< YELLOW

    /// 创建灯和其存放灯的容器、布局，并存入容器，便于后面管理
    for (int i = 0; i < ROWS; i ++)
    {
        /// 创建每行的容器
        QVector<QLabel*> ledVec;
        QVector<QWidget*> widgetVec;
        QVector<QHBoxLayout*> layoutVec;

        /// 创建灯 并设置样式为熄灭状态 再将灯存入容器
        QLabel *led1 = new QLabel();
        QLabel *led2 = new QLabel();
        QLabel *led3 = new QLabel();
        led1->setStyleSheet(color[colorType::GREY]);
        led2->setStyleSheet(color[colorType::GREY]);
        led3->setStyleSheet(color[colorType::GREY]);
        ledVec.push_back(led1);
        ledVec.push_back(led2);
        ledVec.push_back(led3);

        /// 创建用于放置灯的widget和布局 并添加到容器
        QWidget* qw1 = new QWidget();
        QWidget* qw2 = new QWidget();
        QWidget* qw3 = new QWidget();
        QHBoxLayout* la1 = new QHBoxLayout();
        QHBoxLayout* la2 = new QHBoxLayout();
        QHBoxLayout* la3 = new QHBoxLayout();
        widgetVec.push_back(qw1);
        widgetVec.push_back(qw2);
        widgetVec.push_back(qw3);
        layoutVec.push_back(la1);
        layoutVec.push_back(la2);
        layoutVec.push_back(la3);

        /// 将灯添加到布局中 并设置居中
        for (int j = 0; j < 3; j ++)
        {
            layoutVec[j]->addWidget(ledVec[j]);
            layoutVec[j]->setMargin(0);
            layoutVec[j]->setAlignment(layoutVec[j], Qt::AlignCenter);
            widgetVec[j]->setLayout(layoutVec[j]);   ///< 再将布局添加到widget中
        }

        /// 最后将这一行的内容添加到总的容器
        leds.push_back(ledVec);
        qws.push_back(widgetVec);
        las.push_back(layoutVec);
    }

    selectOnOff = ui->selectOnOff;
    allOnOff = ui->allOnOff;

    cbModel = ui->cbModel;
    cbGrainSize = ui->cbGrainSize;
}

void SheetTest::initUi()
{
    /// 1 初始化单元格选择
    ///< 添加选中模式
    cbModel->addItem(u8"不可选中");
    cbModel->addItem(u8"只能选中一个项目(单元格/行/列)");
    cbModel->addItem(u8"可以选中多个项目(单元格/行/列)(无需按ctrl键)");
    cbModel->addItem(u8"配合ctrl可以选中多个项目(单元格/行/列)");
    cbModel->addItem(u8"配合ctrl实现区域选择");
    cbModel->setCurrentIndex(3);    ///< 默认是配合ctrl可以选中多个项目(单元格/行/列)
    ///< 添加最小选中粒度
    cbGrainSize->addItem(u8"按单元格选中");
    cbGrainSize->addItem(u8"按行选中");
    cbGrainSize->addItem(u8"按列选中");
    cbGrainSize->setCurrentIndex(0);    ///< 默认最小选中粒度是按单元格选中


    /// 2 给表格添加内容
    qtw = ui->tw;

    ///< 设置单元格样式
    QString cellStyle = R"(
        QTableView
        {
            font:16px "Consolas";
            color:rgb(150, 150, 150);
            selection-color:rgb(0, 0, 0);
            border:1px solid rgb(128, 128, 128);
            selection-background-color:rgb(240, 240, 240);
        }
    )";
    qtw->setStyleSheet(cellStyle);

#ifdef NO_Focus
    qtw->setItemDelegate(new NoFocusDelegate(qtw)); ///< 取消鼠标focus时出现的虚线框
#endif

    ///< 设置水平表头样式
    const QString horizontalHeaderStyle = R"(
        QHeaderView::section {
            color:rgb(10, 10, 10);
            font:16px "楷体";
            text-align:center;
            height:32px;
            background-color:rgb(230, 230, 230);
            border:1px solid rgb(200, 200, 200);
        }
    )";
    qtw->horizontalHeader()->setStyleSheet(horizontalHeaderStyle);

    ///< 设置垂直表头样式
    const QString verticalHeaderStyle = R"(
        QHeaderView::section {
            color:rgb(10, 10, 10);
            font:16px "Consolas";
            text-align:center;
            width:20px;
            background-color:rgb(230, 230, 230);
            border:1px solid rgb(200, 200, 200);
        }
    )";
    qtw->verticalHeader()->setStyleSheet(verticalHeaderStyle);
    qtw->verticalHeader()->setDefaultAlignment(Qt::AlignHCenter|Qt::AlignVCenter);  ///< 单独设置垂直表头文字居中，因为不知为啥样式中设置无效
//    qtw->verticalHeader()->setVisible(false);                  ///< 设置垂直表头不可见

    qtw->horizontalHeader()->setDefaultSectionSize(120);    ///< 设置列宽
    qtw->verticalHeader()->setDefaultSectionSize(35);       ///< 设置行高
    qtw->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);  ///< 固定列宽
    qtw->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);    ///< 固定行高
    qtw->horizontalHeader()->setHighlightSections(false);   ///< 选中时水平表头不变粗
    qtw->verticalHeader()->setHighlightSections(false);     ///< 选中时垂直表头不变粗

//    qtw->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); /// 自适应行列宽（设置此项时不能设置自动胀满）
//    qtw->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);   /// 自适应行高

    /// 设置水平表头内容
    QStringList header;
    header << u8"灯ID" << u8"红灯" << u8"绿灯" << u8"黄灯" << u8"        灯状态";
    qtw->setColumnCount(header.size());         ///< 设置表格列数
    qtw->setHorizontalHeaderLabels(header);     ///< 设置水平表头内容
    qtw->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);     ///< 设置水平表头的 灯状态 靠左

    qtw->setRowCount(ROWS);                     ///< 设置总行数
    qtw->setEditTriggers(QAbstractItemView::NoEditTriggers);   ///< 设置表结构默认不可编辑

    /// 下面是选中模式和粒度，详细搭配使用见README.md文档说明
//    qtw->setSelectionMode(QAbstractItemView::NoSelection);          ///< 无法选中
//    qtw->setSelectionMode(QAbstractItemView::SingleSelection);      ///< 只能选中一个项目(单元格/行/列)
//    qtw->setSelectionMode(QAbstractItemView::MultiSelection);       ///< 可以选中多个项目(单元格/行/列)(不用按ctrl键)
//    qtw->setSelectionMode(QAbstractItemView::ExtendedSelection);    ///< 配合ctrl可以选中多个项目(单元格/行/列)
//    qtw->setSelectionMode(QAbstractItemView::ContiguousSelection);  ///< 配合ctrl实现区域选择

//    qtw->setSelectionBehavior(QAbstractItemView::SelectItems);      ///< 设置最小粒度可以按单元格选中
//    qtw->setSelectionBehavior(QAbstractItemView::SelectRows);       ///< 设置最小粒度只能按行选中
//    qtw->setSelectionBehavior(QAbstractItemView::SelectColumns);    ///< 设置最小粒度只能按列选中

    /// 添加内容
    for (int i = 0; i < ROWS; i ++)
    {
        ///< 设置第i行第0列 为 灯ID
        qtw->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        qtw->item(i, 0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter); ///< 设置该单元格上下左右居中
        for (int j = 0; j < 3; j ++)
        {
            qtw->setCellWidget(i, j + 1, qws[i][j]);
        }        
        qtw->setItem(i, 4, new QTableWidgetItem(QString(u8"       已熄灭")));
        qtw->item(i, 4)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    }

    qtw->horizontalHeader()->setStretchLastSection(true);     /// 设置最后一列胀满表格
//    qtw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);    /// 设置所有列均匀胀满表格
}

void SheetTest::initSlots()
{
    QObject::connect(ui->selectOnOff, &QPushButton::clicked, this, &SheetTest::getSelectd);     ///< 点转换选中的灯
    QObject::connect(ui->allOnOff, &QPushButton::clicked, this, [=](){QSet<int> set;dealLeds(set, 1);});    ///< 点亮/熄灭所有灯

    /// 注意currentIndexChanged有重载，必须进行类型转换
    QObject::connect(ui->cbModel, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SheetTest::setModel);
    QObject::connect(ui->cbGrainSize, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SheetTest::setGrainSize);
}

void SheetTest::setModel(int id)
{
    qtw->clearSelection();
    switch (id)
    {
    case 0:
        qtw->setSelectionMode(QAbstractItemView::NoSelection);          ///< 无法选中
    break;
    case 1:
        qtw->setSelectionMode(QAbstractItemView::SingleSelection);      ///< 只能选中一个项目(单元格/行/列)
    break;
    case 2:
        qtw->setSelectionMode(QAbstractItemView::MultiSelection);       ///< 可以选中多个项目(单元格/行/列)(不用按ctrl键)
    break;
    case 3:
        qtw->setSelectionMode(QAbstractItemView::ExtendedSelection);    ///< 配合ctrl可以选中多个项目(单元格/行/列)
    break;
    case 4:
        qtw->setSelectionMode(QAbstractItemView::ContiguousSelection);  ///< 配合ctrl实现区域选择
    break;
    default:
    break;
    }
}

void SheetTest::setGrainSize(int id)
{
    qtw->clearSelection();
    switch (id)
    {
    case 0:
        qtw->setSelectionBehavior(QAbstractItemView::SelectItems);      ///< 设置最小粒度可以按单元格选中
    break;
    case 1:
        qtw->setSelectionBehavior(QAbstractItemView::SelectRows);       ///< 设置最小粒度只能按行选中
    break;
    case 2:
        qtw->setSelectionBehavior(QAbstractItemView::SelectColumns);    ///< 设置最小粒度只能按列选中
    break;
    default:
    break;
    }
}

void SheetTest::getSelectd()
{
    /**
    * 灵活处理，根据不同选中方式来获取选中行列或坐标
    * 此处处理灯，简单按照行来处理
    */

    /// 获取所有选择的行号和列号
    QSet<int> selectedRows, selectedColumns;
    for (const QModelIndex &index : qtw->selectionModel()->selectedIndexes()) {
        selectedRows.insert(index.row());
        selectedColumns.insert(index.column());
    }
    dealLeds(selectedRows, 0);
    return;

    /// 获取所有选中单元格的坐标
    QVector<QPair<int, int>>items;
    for (const QModelIndex &index : qtw->selectionModel()->selectedIndexes()) {
        items.push_back(qMakePair(index.row(), index.column()));
    }

    /// 获取所有选中区域的开始/结束行号 和 开始/结束列号（选择了几次，就有几个区域）
    QList<QTableWidgetSelectionRange> ranges = qtw->selectedRanges();
    int startRow, endRow, startCol, endCol;
    for (int i = 0; i < ranges.size(); i ++)
    {
        startRow    = ranges.at(i).topRow();
        endRow      = ranges.at(i).bottomRow();
        startCol    = ranges.at(i).leftColumn();
        endCol      = ranges.at(i).rightColumn();
    }
}

void SheetTest::dealLeds(QSet<int> set, int btnId)
{
    /// 点击转换选中的按钮
    if (btnId == 0) {
        if (ui->cbGrainSize->currentIndex() != 1) {  ///< 只处理按行选中粒度 的设置
            return;
        }

        for (auto row : set) {
            if (qtw->item(row, 4)->text() == u8"       已点亮") {
                leds[row][0]->setStyleSheet(color[colorType::GREY]);
                leds[row][1]->setStyleSheet(color[colorType::GREY]);
                leds[row][2]->setStyleSheet(color[colorType::GREY]);
                qtw->setItem(row, 4, new QTableWidgetItem(QString(u8"       已熄灭")));
            } else {
                leds[row][0]->setStyleSheet(color[colorType::RED]);
                leds[row][1]->setStyleSheet(color[colorType::GREEN]);
                leds[row][2]->setStyleSheet(color[colorType::YELLOW]);
                qtw->setItem(row, 4, new QTableWidgetItem(QString(u8"       已点亮")));
            }
        }
        return;
    }

    /// 点击全点亮/熄灭 的按钮
    if (btnId == 1) {
        if (ui->allOnOff->text() == u8"全点亮") {
            for (int i = 0; i < ROWS; i ++) {
                leds[i][0]->setStyleSheet(color[colorType::RED]);
                leds[i][1]->setStyleSheet(color[colorType::GREEN]);
                leds[i][2]->setStyleSheet(color[colorType::YELLOW]);
                qtw->setItem(i, 4, new QTableWidgetItem(QString(u8"       已点亮")));
            }
            ui->allOnOff->setText(u8"全熄灭");

        } else {
            for (int i = 0; i < ROWS; i ++) {
                leds[i][0]->setStyleSheet(color[colorType::GREY]);
                leds[i][1]->setStyleSheet(color[colorType::GREY]);
                leds[i][2]->setStyleSheet(color[colorType::GREY]);
                qtw->setItem(i, 4, new QTableWidgetItem(QString(u8"       已熄灭")));
            }
            ui->allOnOff->setText(u8"全点亮");
        }
    }
}
