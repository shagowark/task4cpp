#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QTextEdit>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <vector>
#include <string>
#include <set>
#include <utility>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadTextFile();
    void loadKeywordsFile();
    void saveOutputFile();
    void processFiles();

private:
    Ui::MainWindow *ui;
    QString textFilePath;
    QString keywordsFilePath;
    QString outputFilePath;
    QTextEdit *textEdit;
    QTextEdit *keywordsEdit;
    QTextEdit *outputEdit;
    QPushButton *loadTextButton;
    QPushButton *loadKeywordsButton;
    QPushButton *saveOutputButton;
    QPushButton *processButton;
    QLabel *textLabel;
    QLabel *keywordsLabel;
    QLabel *outputLabel;
    std::set<std::string> keywords;
    std::map<std::string, std::vector<std::pair<int, int>>> wordOccurrences;

    void logActivity(const QString &message, const QString &severity);
};

#endif // MAINWINDOW_H
