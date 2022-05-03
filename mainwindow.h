#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QRegularExpressionValidator>
#include <QElapsedTimer>
#include <QtGlobal>
#include <QScrollArea>
#include <bitset>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void                generateData            ();
    void                generateBlum            ();
    void                generateSeed            ();
    void                changeBlum              ();
    void                generateBits            (quint32 size);

    void                saveFile                ();
    void                saveAsFile              ();
    void                loadFile                ();

    void                saveFileSteganogram     ();
    void                saveAsFileSteganogram   ();
    void                loadFileSteganogram     ();

    void                loadFileMedium          ();

    void                hideText                ();
    void                readHidden              ();

    QString             savedFileName,  savedFileNameSteganogram;

    quint64             seed = 1085421659u;
    quint64             blum = 2783859748098954953ull;
    quint16             sizeMarkFrequency = 100u;
    QByteArray          plainText, hiddenText, cypherBits, rngBits, medium, r, g, b, marks, steg;
};
