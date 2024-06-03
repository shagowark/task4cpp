#include "mainwindow.h"
#include <QRegularExpression>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <fstream>
#include <vector>
#include <string>
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), textFilePath(""), keywordsFilePath(""), outputFilePath("") {

    ui->setupUi(this);

    textEdit = new QTextEdit(this);
    keywordsEdit = new QTextEdit(this);
    outputEdit = new QTextEdit(this);
    outputEdit->setReadOnly(true);

    loadTextButton = new QPushButton("Load Text File", this);
    loadKeywordsButton = new QPushButton("Load Keywords File", this);
    saveOutputButton = new QPushButton("Save Output File", this);
    processButton = new QPushButton("Process Files", this);

    textLabel = new QLabel("Your text", this);
    keywordsLabel = new QLabel("Keywords", this);
    outputLabel = new QLabel("Result", this);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->addWidget(textLabel);
    textLayout->addWidget(textEdit);
    textLayout->addWidget(loadTextButton);

    QVBoxLayout *keywordsLayout = new QVBoxLayout();
    keywordsLayout->addWidget(keywordsLabel);
    keywordsLayout->addWidget(keywordsEdit);
    keywordsLayout->addWidget(loadKeywordsButton);

    QVBoxLayout *outputLayout = new QVBoxLayout();
    outputLayout->addWidget(outputLabel);
    outputLayout->addWidget(outputEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(textLayout);
    mainLayout->addLayout(keywordsLayout);
    mainLayout->addLayout(outputLayout);
    mainLayout->addWidget(processButton);
    mainLayout->addWidget(saveOutputButton);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(loadTextButton, &QPushButton::clicked, this, &MainWindow::loadTextFile);
    connect(loadKeywordsButton, &QPushButton::clicked, this, &MainWindow::loadKeywordsFile);
    connect(saveOutputButton, &QPushButton::clicked, this, &MainWindow::saveOutputFile);
    connect(processButton, &QPushButton::clicked, this, &MainWindow::processFiles);
}

MainWindow::~MainWindow() {}

void MainWindow::loadTextFile() {
    textFilePath = QFileDialog::getOpenFileName(this, "Open Text File", "", "Text Files (*.txt);;All Files (*)");
    if (!textFilePath.isEmpty()) {
        QFile file(textFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Cannot open file: " + textFilePath);
            logActivity("Failed to open text file: " + textFilePath, "ERROR");
            return;
        }

        QTextStream in(&file);
        QString text;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            text += line + "\n";
        }
        textEdit->setPlainText(text);
        file.close();
        logActivity("Loaded text file: " + textFilePath, "INFO");
    }
}

void MainWindow::loadKeywordsFile() {
    keywordsFilePath = QFileDialog::getOpenFileName(this, "Open Keywords File", "", "Text Files (*.txt);;All Files (*)");
    if (!keywordsFilePath.isEmpty()) {
        QFile file(keywordsFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Cannot open file: " + keywordsFilePath);
            logActivity("Failed to open text file: " + keywordsFilePath, "ERROR");
            return;
        }

        QTextStream in(&file);
        QString text;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            QStringList words = line.split(QRegularExpression(" "), Qt::SkipEmptyParts);
            for (const QString& word : words) {
                keywords.insert(word.toStdString());
            }
            text += line + "\n";
        }
        keywordsEdit->setPlainText(text);
        file.close();
        logActivity("Loaded keywords file: " + keywordsFilePath, "INFO");
    }
}

void MainWindow::saveOutputFile() {
    outputFilePath = QFileDialog::getSaveFileName(this, "Save Output File", "", "Text Files (*.txt);;All Files (*)");
    if (!outputFilePath.isEmpty()) {
        std::ofstream outputFileStream(outputFilePath.toStdString());
        if (!outputFileStream.is_open()) {
            QMessageBox::warning(this, "Error", "Cannot open file: " + outputFilePath);
            logActivity("Failed to open output file: " + outputFilePath, "ERROR");
            return;
        }

        outputFileStream << outputEdit->toPlainText().toStdString();
        outputFileStream.close();
        logActivity("Saved output file: " + outputFilePath, "INFO");
    }
}

void MainWindow::processFiles() {
    if (textFilePath.isEmpty() || keywordsFilePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please load both text and keywords files.");
        logActivity("Text or keywords file not loaded", "ERROR");
        return;
    }

    std::ifstream textFileStream(textFilePath.toStdString());
    if (!textFileStream.is_open()) {
        QMessageBox::warning(this, "Error", "Cannot open text file: " + textFilePath);
        logActivity("Failed to open text file for processing: " + textFilePath, "ERROR");
        return;
    }

    wordOccurrences.clear();
    int lineNumber = 0;
    std::string line;

    while (std::getline(textFileStream, line)) {
        lineNumber++;
        std::vector<std::string> words;
        size_t start = 0;
        size_t end = line.find_first_of(" \t\n\r\f\v", start);
        while (end != std::string::npos) {
            words.push_back(line.substr(start, end - start));
            start = end + 1;
            end = line.find_first_of(" \t\n\r\f\v", start);
        }
        words.push_back(line.substr(start));
        int columnNumber = 0;
        for (const std::string &word : words) {
            columnNumber++;
            if (keywords.find(word) == keywords.end()) {
                wordOccurrences[word].push_back(std::make_pair(lineNumber, columnNumber));
            }
        }
    }
    textFileStream.close();

    QString result;
    for (auto it = wordOccurrences.cbegin(); it != wordOccurrences.cend(); ++it) {
        result += QString::fromStdString(it->first) + " ";
        for (const auto &pos : it->second) {
            result += "(" + QString::number(pos.first) + "," + QString::number(pos.second) + ") ";
        }
        result += "\n";
    }
    outputEdit->setPlainText(result);

    logActivity("Processed files", "INFO");
}


void MainWindow::logActivity(const QString &message, const QString &severity) {
    QFile logFile("activity.log");
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Cannot open log file";
        return;
    }

    QTextStream out(&logFile);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    out << timestamp << " [" << severity << "] " << message << "\n";
    logFile.close();
}
