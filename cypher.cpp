#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::execute(bool bExecute)
{
    //Niektóre przyciski wywołują tą funkcję, gdy są wyłączane, a w ich miejsce włączane inne, które ponownie
    //wywołują tę funkcję (np. QRadioButton) i nie powinny tego robić drugi raz
    if (!bExecute)
        return;

    //Najpierw trzeba posprzątać po poprzednim uruchomieniu programu, o ile to się wydarzyło
    ui->secondText  ->clear();
    ui->visTable    ->setColumnCount(0u);
    ui->visTable    ->setRowCount(0u);

    if (rawData.isEmpty())
        return;

    unsigned    textSize    = rawData.size(),
                blockRem    = (textSize % 16u), //Liczba brakujących
                blockSize   = textSize + 16u - blockRem;

    //AES-128 jest zaimplementowany w ten sposób, że działa na wskaźnikach do quint8, więc musimy stworzyć taką strukturę o odpowiednim rozmiarze
    quint8* arrData = new quint8[blockSize];
    //Kopia danych wejściowych
    memcpy(arrData, rawData.toStdString().c_str(), blockSize);

    if (!bDecypher)
    {
        //Przy szyfrowaniu pozostawiamy znacznik końca szyfru oraz dopiełniamy blok do pełnego rozmiaru
        arrData[textSize] = '\1';
        for (unsigned i = blockRem + 1; i != 16u; ++i)
            arrData[textSize - blockRem + i] = '\0';
    }
    else
    {
        //Ogólnie rzecz biorąc, to nie powinno się wydarzyć, żebyśmy musieli wypełniać blok zerami, żeby deszyfrowanie było możliwe
        //Ale z racji, że użytkownik może "bawić" się programem i wprowadzać dziwne dane, po to jest ten zabieg
        if (blockRem != 0u)
            for (unsigned i = blockRem; i != 16u; ++i)
                arrData[textSize - blockRem + i] = '\0';
        //Przy deszyfrowaniu mie trzeba dodatkowego bloku, jezeli cały jest zapełniony
        else blockSize = textSize;
    }

    //Ustawiamy strukturę do wizualizacji
    unsigned    maxRows     = ceil(sqrt(blockSize / 2u)),
                maxColumns  = maxRows * 2u;
    QColor      blockColor(0u, 0u, 0u);
    unsigned    row     = 0u,   col     = 0u,
                pRow    = 0u,   pCol    = 0u;


    if (bVisualize)
    {
        ui->visTable->setRowCount(maxRows * 2u + 1u);
        ui->visTable->setColumnCount(maxColumns);
    }

    quint8      key[16u],
                cbcBuf[16u];

    std::string keyText = ui->keyText->text().toStdString();

    //Pobieramy klucz i zapisujemy w [key] z tych samych powodów, co stworzyliśmy [arrData]
    memset(key, 0u, 16u);
    memcpy(key, keyText.c_str(), qMin(keyText.length(), 16u));
    memcpy(cbcBuf, key, 16u);

    QElapsedTimer timer;
    timer.start();

    for (unsigned index = 0u; index != blockSize; index += 16u)
    {
        //Przechodzimy po danych blok po bloku, przesuwając wskaźnik co 16 pozycji
        quint8 *str = arrData + index;
        if (bVisualize)
        {
            unsigned    diffSum = 0u,
                        r, g, b;

            //Zapewniamy unikalny kolor sąsiednich bloków
            //Todo: stworzyć tablicę kolorów, którą najwyżej rozszerzamy, gdy jest taka potrzeba. Chodzi o to, żeby z każdym execute nie zmieniały się kolory, tylko pozostawały takie same
            while (diffSum < 100u)
            {
                diffSum = 0u;
                diffSum += qAbs(blockColor.red()     - (r = QRandomGenerator::global()->bounded(128u)));
                diffSum += qAbs(blockColor.green()   - (g = QRandomGenerator::global()->bounded(128u)));
                diffSum += qAbs(blockColor.blue()    - (b = QRandomGenerator::global()->bounded(128u)));
            }
            blockColor = QColor(127u + r, 127u + g, 127u + b);

            for (unsigned i = 0u; i != 16u; ++i)
            {
                QByteArray  newItemText;
                if (read == Hex)
                    newItemText = QByteArray::fromRawData(reinterpret_cast<const char*>(str) + i, 1u).toHex().toUpper();
                else
                    newItemText = QByteArray::fromRawData(reinterpret_cast<const char*>(str) + i, 1u);

                QTableWidgetItem *newItem = new QTableWidgetItem(newItemText);
                newItem->setBackground(QBrush(blockColor));

                ui->visTable->setItem(row, col, newItem);
                if (++col == maxColumns)
                {
                    col = 0u;
                    ++row;
                }
            }
        }

        //Wywołanie AES-128
        if (bDecypher)
            decrypt(str, key, block, cbcBuf);
        else
            encrypt(str, key, block, cbcBuf);

        if (bVisualize)
            for (unsigned i = 0u; i != 16u; ++i)
            {
                QByteArray  newItemText;
                if (read == Hex)
                    newItemText = QByteArray::fromRawData(reinterpret_cast<const char*>(str) + i, 1u).toHex().toUpper();
                else
                    newItemText = QByteArray::fromRawData(reinterpret_cast<const char*>(str) + i, 1u);

                QTableWidgetItem *newItem = new QTableWidgetItem(newItemText);
                newItem->setBackground(QBrush(blockColor));

                ui->visTable->setItem(maxRows + 1u + pRow, pCol, newItem);
                if (++pCol == maxColumns)
                {
                    pCol = 0u;
                    ++pRow;
                }
            }
    }

    unsigned    dataSize = 16u;
    if (bDecypher)
    {
        quint8 *str = arrData + (blockSize - 16u);
        //Jeżeli nie znajdziemy '\1', to oznacza, że deszyfrowanie nie powiodło się, a dane są corrupted
        //I tak je próbujemy odczytać, bo wyniki mogą być interesujące
        bool bFound = false;
        for (; dataSize != 0u; --dataSize)
            if (str[dataSize - 1u] == '\1')
            {
                bFound = true;
                break;
            }

        if (bFound)
            --dataSize;
        else
            dataSize = 16u;
    }
    ui->timeEdit->setText(QString::fromLatin1(std::to_string(timer.elapsed())));

    //Zapisujemy i wyświetlamy wynik
    processedRawData = QByteArray::fromRawData(reinterpret_cast<const char*>(arrData), blockSize + dataSize - 16u);
    if (read == Hex)
        ui->secondText->setPlainText(processedRawData.toHex().toUpper());
    else
        ui->secondText->setPlainText(processedRawData);
    delete[] arrData;
}
