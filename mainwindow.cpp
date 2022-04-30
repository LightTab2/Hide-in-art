#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->blumSpin    ->setValue(2783859748098954953ull);
    ui->seedSpin    ->setMaximum(std::numeric_limits<int>::max());

    //Zarządzanie plikami
    ui->actionLoad  ->setShortcut(QKeySequence(tr("Ctrl+O")));
    ui->actionSave  ->setShortcut(QKeySequence(tr("Ctrl+S")));
    ui->actionSaveAs->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));

    connect(ui->actionLoad,             &QAction::triggered,                this, &MainWindow::loadFile);
    connect(ui->actionSave,             &QAction::triggered,                this, &MainWindow::saveFile);
    connect(ui->actionSaveAs,           &QAction::triggered,                this, &MainWindow::saveAsFile);
    connect(ui->actionLoadSteganogram,  &QAction::triggered,                this, &MainWindow::loadFileSteganogram);
    connect(ui->actionSaveSteganogram,  &QAction::triggered,                this, &MainWindow::saveFileSteganogram);
    connect(ui->actionSaveAsSteganogram,&QAction::triggered,                this, &MainWindow::saveAsFileSteganogram);
    connect(ui->actionLoadMedium,       &QAction::triggered,                this, &MainWindow::loadFileMedium);
    connect(ui->blumGenerate,           &QPushButton::clicked,              this, &MainWindow::generateBlum);
    connect(ui->seedGenerate,           &QPushButton::clicked,              this, &MainWindow::generateSeed);
    connect(ui->blumSpin,               &BigSpin::valueChanged,             this, &MainWindow::changeBlum);
    connect(ui->seedSpin,               &QSpinBox::valueChanged,            this, [this](int value)     { this->seed = value; });
    connect(ui->hideButton,             &QPushButton::clicked,              this, &MainWindow::hideText);
}


void MainWindow::saveFile()
{
    if (savedFileName == "")
        return;

    QFile file(savedFileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Nie udało się otworzyć pliku"),
            file.errorString());

        return;
    }

    QDataStream out(&file);
    out.writeRawData(plainText, plainText.size());
}

void MainWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Zapisz plik jako"), "",
                                                    tr("Wszystkie pliki (*)"));
    if (fileName.isEmpty())
        return;

    savedFileName = fileName;
    saveFile();
}

void MainWindow::loadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Wczytaj plik"), "",
                                                    tr("Wszystkie pliki (*)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, tr("Nie udało się otworzyć pliku"),
            file.errorString());

        return;
    }

    plainText       = file.readAll();
    ui->plainText->setPlainText(plainText);
    savedFileName   = fileName;
}

void MainWindow::saveFileSteganogram()
{
    if (savedFileNameSteganogram == "")
        return;

    QImage img(reinterpret_cast<const uchar *>(steg.constData()), ui->stegImage->pixmap().width(), ui->stegImage->pixmap().height(), QImage::Format_ARGB32);
    img.save(savedFileNameSteganogram);
}

void MainWindow::saveAsFileSteganogram()
{
    if (!steg.size())
        return;
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Zapisz plik jako"), "",
                                                    //tr("Pliki bitmapowe (*.bmp)"),
                                                    tr("Pliki PNG (*.png)"));
    if (fileName.isEmpty())
            return;

    savedFileNameSteganogram = fileName;
    saveFileSteganogram();
}

void MainWindow::loadFileSteganogram()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Wczytaj plik"), "",
                                                    //tr("Pliki bitmapowe (*.bmp)"),
                                                    tr("Pliki PNG (*.png)"));
    if (fileName.isEmpty())
        return;

    QImage img;
    img.load(fileName, "PNG");
    steg.resize(img.sizeInBytes());
    memcpy(steg.data(), img.bits(), img.sizeInBytes());
    ui->stegImage   ->setPixmap(QPixmap::fromImage(img));
    ui->rImage      ->setPixmap(QPixmap());
    ui->gImage      ->setPixmap(QPixmap());
    ui->bImage      ->setPixmap(QPixmap());
    readHidden();
}


void MainWindow::loadFileMedium()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Wczytaj plik"), "",
                                                    //tr("Pliki bitmapowe (*.bmp)"),
                                                    tr("Pliki PNG (*.png)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, tr("Nie udało się otworzyć pliku"),
            file.errorString());

        return;
    }

    QPixmap pixmap(fileName, "PNG");
    ui->mediumImage->setPixmap(pixmap);
    QImage img = pixmap.toImage();
    medium.resize(img.sizeInBytes());
    memcpy(medium.data(), img.bits(), img.sizeInBytes());
}

void MainWindow::changeBlum()
{
    ui->pEdit->setText("?????");
    ui->qEdit->setText("?????");
    blum = ui->blumSpin->value();
}

MainWindow::~MainWindow()
{
    delete ui;
}
