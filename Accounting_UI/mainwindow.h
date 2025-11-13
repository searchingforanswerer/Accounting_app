#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>

class QLabel;
class QTableWidget;
class QPushButton;
class QTabWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddBillClicked();

private:
    Ui::MainWindow *ui;

    // widgets in Home tab
    QTabWidget   *m_tabWidget;
    QWidget      *m_homeTab;
    QLabel       *m_incomeLabel;
    QLabel       *m_expenseLabel;
    QTableWidget *m_table;
    QPushButton  *m_addBillButton;

    QStringList  m_categories;   // ⭐ 新增：全局的类别列表

    void setupHomeTab();
    void updateTotals();
};

#endif // MAINWINDOW_H
