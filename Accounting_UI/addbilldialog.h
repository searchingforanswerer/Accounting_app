#pragma once

#include <QDialog>
#include <QDateTime>

class QDoubleSpinBox;
class QDateEdit;
class QTimeEdit;
class QComboBox;
class QLineEdit;

class AddBillDialog : public QDialog
{
    Q_OBJECT

public:
    // ⭐ 传入现有类别列表
    explicit AddBillDialog(const QStringList &categories,
                               QWidget *parent = nullptr);

    double     amount()   const;
    QDateTime  dateTime() const;
    QString    typeText() const;   // "Income" or "Expense"
    QString    category() const;
    QString    note()     const;

private:
    QDoubleSpinBox *m_amountSpin;
    QDateEdit      *m_dateEdit;
    QTimeEdit      *m_timeEdit;
    QComboBox      *m_typeCombo;
    QComboBox      *m_categoryCombo;
    QLineEdit      *m_noteEdit;

    void setupUi(const QStringList &categories);
};
