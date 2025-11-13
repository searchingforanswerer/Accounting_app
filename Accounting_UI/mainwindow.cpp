#include "mainwindow.h"
#include "ui_mainwindow.h"     // 由 mainwindow.ui 自动生成
#include "addbilldialog.h"

#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_tabWidget(nullptr),
      m_homeTab(nullptr),
      m_incomeLabel(nullptr),
      m_expenseLabel(nullptr),
      m_table(nullptr),
      m_addBillButton(nullptr)
{
    ui->setupUi(this);

    // ⭐ 默认类别，可以随便改成你想要的
        m_categories << "Food"
                     << "Transport"
                     << "Shopping"
                     << "Salary"
                     << "Entertainment"
                     << "Other";

    // centralWidget 是 ui 里默认生成的，我们在上面重新布置布局
    QWidget *central = this->centralWidget();
    if (!central) {
        central = new QWidget(this);
        setCentralWidget(central);
    }

    auto *mainLayout = new QVBoxLayout(central);
    central->setLayout(mainLayout);

    // 底部 TabWidget
    m_tabWidget = new QTabWidget(central);
    m_tabWidget->setTabPosition(QTabWidget::South);  // tab 在底部
    mainLayout->addWidget(m_tabWidget);

    // Home tab
    setupHomeTab();

    // 其它几个 tab 先放空白页面
    QWidget *searchTab   = new QWidget;
    QWidget *analysisTab = new QWidget;
    QWidget *settingsTab = new QWidget;

    m_tabWidget->addTab(m_homeTab,   "Home");
    m_tabWidget->addTab(searchTab,   "Search");
    m_tabWidget->addTab(analysisTab, "Analysis");
    m_tabWidget->addTab(settingsTab, "Settings");

    setWindowTitle("Accounting Demo UI");
    resize(900, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupHomeTab()
{
    m_homeTab = new QWidget(m_tabWidget);
    auto *homeLayout = new QVBoxLayout(m_homeTab);

    // ===== 顶部：今日收入 / 支出 =====
    auto *topLayout = new QHBoxLayout();

    m_incomeLabel = new QLabel(m_homeTab);
    m_expenseLabel = new QLabel(m_homeTab);

    // 初始文本：两行
    m_incomeLabel->setText("Today's Income\n￥0.00");
    m_expenseLabel->setText("Today's Expense\n￥0.00");

    // 字体稍微放大 & 加粗
    QFont f = m_incomeLabel->font();
    f.setPointSize(f.pointSize() + 4);
    f.setBold(true);
    m_incomeLabel->setFont(f);
    m_expenseLabel->setFont(f);

    // 颜色区分：收入绿色，支出红色
    m_incomeLabel->setStyleSheet("color: #008000;");  // 深绿
    m_expenseLabel->setStyleSheet("color: #C00000;"); // 深红

    // 居中 & 允许自动换行（两行更好看）
    m_incomeLabel->setAlignment(Qt::AlignCenter);
    m_expenseLabel->setAlignment(Qt::AlignCenter);
    m_incomeLabel->setWordWrap(true);
    m_expenseLabel->setWordWrap(true);

    topLayout->addWidget(m_incomeLabel);
    topLayout->addWidget(m_expenseLabel);

    homeLayout->addLayout(topLayout);

    // ===== 中间：表格，显示今日账单 =====
    m_table = new QTableWidget(m_homeTab);
    m_table->setColumnCount(5);

    QStringList headers;
    headers << "Time"      // HH:mm
            << "Amount"    // ￥xx.xx
            << "Type"      // Income / Expense
            << "Category"
            << "Note";
    m_table->setHorizontalHeaderLabels(headers);

    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    homeLayout->addWidget(m_table, 1);

    // ===== 底部：Add Bill 按钮 =====
    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    m_addBillButton = new QPushButton("Add Bill", m_homeTab);
    bottomLayout->addWidget(m_addBillButton);

    bottomLayout->addStretch();
    homeLayout->addLayout(bottomLayout);

    connect(m_addBillButton, &QPushButton::clicked,
            this, &MainWindow::onAddBillClicked);
}

void MainWindow::onAddBillClicked()
{
    // ⭐ 把当前已有的类别传进去
    AddBillDialog dlg(m_categories, this);

    if (dlg.exec() == QDialog::Accepted) {
        QDateTime dt   = dlg.dateTime();
        double    amt  = dlg.amount();
        QString   type = dlg.typeText();
        QString   cat  = dlg.category();
        QString   note = dlg.note();

        // ⭐ 如果是新类别，加入全局列表，供下次使用
        if (!cat.isEmpty() && !m_categories.contains(cat)) {
            m_categories.append(cat);
        }

        // 表格插入一行（你的原逻辑）
        m_table->insertRow(0);

        QString timeStr   = dt.time().toString("HH:mm");
        QString amountStr = QString("￥%1").arg(amt, 0, 'f', 2);

        m_table->setItem(0, 0, new QTableWidgetItem(timeStr));
        m_table->setItem(0, 1, new QTableWidgetItem(amountStr));
        m_table->setItem(0, 2, new QTableWidgetItem(type));
        m_table->setItem(0, 3, new QTableWidgetItem(cat));
        m_table->setItem(0, 4, new QTableWidgetItem(note));

        updateTotals();
    }
}


void MainWindow::updateTotals()
{
    double income = 0.0;
    double expense = 0.0;

    for (int row = 0; row < m_table->rowCount(); ++row) {
        QTableWidgetItem *amountItem = m_table->item(row, 1);
        QTableWidgetItem *typeItem   = m_table->item(row, 2);
        if (!amountItem || !typeItem) continue;

        QString amtStr = amountItem->text();   // "￥xx.xx"
        if (amtStr.startsWith(QStringLiteral("￥"))) {
            amtStr = amtStr.mid(1);
        }

        bool ok = false;
        double value = amtStr.toDouble(&ok);
        if (!ok) continue;

        QString type = typeItem->text();
        if (type == "Income") {
            income += value;
        } else {
            expense += value;
        }
    }

    m_incomeLabel->setText(
        QString("Today's Income\n￥%1").arg(income, 0, 'f', 2));
    m_expenseLabel->setText(
        QString("Today's Expense\n￥%1").arg(expense, 0, 'f', 2));

}
