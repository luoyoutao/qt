
### QTableWidget::setSelectionMode与QTableWidget::setSelectionBehavior配套使用规则
#### 其中QTableWidget::setSelectionMode可以取如下值
QAbstractItemView::NoSelection            ///< 无法选中
QAbstractItemView::SingleSelection        ///< 只能选中一个项目(单元格/行/列)
QAbstractItemView::MultiSelection         ///< 可以选中多个项目(单元格/行/列)(不用按ctrl键)
QAbstractItemView::ExtendedSelection      ///< 配合ctrl可以选中多个项目(单元格/行/列)
QAbstractItemView::ContiguousSelection    ///< 配合ctrl实现区域选择

#### 其中QTableWidget::setSelectionBehavior可以取如下值
QAbstractItemView::SelectItems    ///< 设置最小粒度可以按单元格选中
QAbstractItemView::SelectRows     ///< 设置最小粒度只能按行选中
QAbstractItemView::SelectColumns  ///< 设置最小粒度只能按列选中
**需注意，当setSelectionMode取值SingleSelection时，这里不代表最小粒度，而是只能按照setSelectionBehavior设置的固定模式选中**
#### 1 当setSelectionMode取NoSelection时，任何单元格无法选中
#### 2 当setSelectionMode取SingleSelection时
2.1 若setSelectionBehavior取SelectItems，只能选中某一个单元格，按住ctrl点击选中的单元格则取消选中
2.2 若setSelectionBehavior取SelectRows，只能选中某一行单元格，按住ctrl点击选中的行则取消选中
2.3 若setSelectionBehavior取SelectColumns，只能选中某一列单元格，按住ctrl点击选中的列则取消选中
#### 3 当setSelectionMode取MultiSelection时（选中多个项目无需按ctrl键，再次点击选中项目则取消选中）
3.1 若setSelectionBehavior取SelectItems，表示最小粒度可以选择到某一个单元格，当然点击表头可以选中某行某列
3.2 若setSelectionBehavior取SelectRows，表示最小粒度只能按照行来选中
3.3 若setSelectionBehavior取SelectColumns，表示最小粒度只能按照列来选中
#### 4 当setSelectionMode取ExtendedSelection时（选中多个项目需按ctrl键，松开ctrl键再点击某一项，则之前选中的项目自动取消选中）
#### 5 当setSelectionMode取ContiguousSelection时（配合ctrl键可以实现区域选择）

### QTableWidget的内存释放简单说明
#### 1 QTableWidget->setItem(row, col, new QTableWidgetItem(QString("xxx")));
如果是第二次调用，qt会自动释放之前的匿名对象；
#### 2 QTableWidget->clear()
clear()方法会清空单元格内容，并释放自定义的组件和内置的组件
