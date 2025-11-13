#include "addbilldialog.h"

#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDate>
#include <QTime>

AddBillDialog::AddBillDialog(const QStringList &categories,
                             QWidget *parent)
    : QDialog(parent),
      m_amountSpin(nullptr),
      m_dateEdit(nullptr),
      m_timeEdit(nullptr),
      m_typeCombo(nullptr),
      m_categoryCombo(nullptr),
      m_noteEdit(nullptr)
{
    setupUi(categories);
}

void AddBillDialog::setupUi(const QStringList &categories)
{
    setWindowTitle("Add Bill");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    // Amount
    m_amountSpin = new QDoubleSpinBox(this);
    m_amountSpin->setRange(0.01, 100000000.0);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setValue(0.01);
    formLayout->addRow("Amount:", m_amountSpin);

    // Date
    m_dateEdit = new QDateEdit(QDate::currentDate(), this);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_dateEdit->setCalendarPopup(true);
    formLayout->addRow("Date:", m_dateEdit);

    // Time
    m_timeEdit = new QTimeEdit(QTime::currentTime(), this);
    m_timeEdit->setDisplayFormat("HH:mm:ss");
    formLayout->addRow("Time:", m_timeEdit);

    // Type
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("Income");
    m_typeCombo->addItem("Expense");
    formLayout->addRow("Type:", m_typeCombo);

    // ⭐ Category：可编辑下拉框
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setEditable(true);      // 既能选，又能自己打字
    m_categoryCombo->addItems(categories);   // 加载现有类别列表
    formLayout->addRow("Category:", m_categoryCombo);

    // Note
    m_noteEdit = new QLineEdit(this);
    formLayout->addRow("Note:", m_noteEdit);

    mainLayout->addLayout(formLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        this
    );
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &AddBillDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &AddBillDialog::reject);

    mainLayout->addWidget(buttonBox);
}

// getters 不变，只有 category() 要改

double AddBillDialog::amount() const
{
    return m_amountSpin->value();
}

QDateTime AddBillDialog::dateTime() const
{
    return QDateTime(m_dateEdit->date(), m_timeEdit->time());
}

QString AddBillDialog::typeText() const
{
    return m_typeCombo->currentText();
}

QString AddBillDialog::category() const
{
    // ⭐ 当前选中/输入的类别
    return m_categoryCombo->currentText();
}

QString AddBillDialog::note() const
{
    return m_noteEdit->text();
}
