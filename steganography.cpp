#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::hideText()
{
    plainText = ui->plainText->toPlainText().toUtf8();
    quint32 size = plainText.size() * 8;
    if (!size)
        return;
    if (size > medium.size())
    {
        QMessageBox::critical(this, tr("Błąd"), tr("Do poprawnego działania należy wczytać obraz o rozmiarze co najmniej %1 (wysokość * szerokość)").arg(QString::number((2 + size)/3)));
        return;
    }
    //Jeszcze null terminator
    size += 8;
    generateBits(medium.size());

    cypherBits      .clear();
    cypherBits      .reserve(size);
    quint32 i = 0u;
    for (; i != plainText.size(); ++i)
        for (quint32 j = 0u; j != 8; ++j)
            cypherBits.append((plainText[i] >> j) & 1);

    for (quint32 j = 0u; j != 8; ++j)
        cypherBits.append('\0');

    steg.resize(medium.size());
    r.resize(medium.size());
    g.resize(medium.size());
    b.resize(medium.size());
    memset(r.data(), 0, medium.size());
    memset(g.data(), 0, medium.size());
    memset(b.data(), 0, medium.size());
    memcpy(steg.data(), medium.constData(), medium.size());

    quint32 cypherSize = cypherBits.size();
    for (quint32 i = 0u, j = 0u; i != steg.size(); ++i)
    {
        //alpha
        if ((i % 4u) == 3u)
        {
            g[i]    = static_cast<uchar>(255u);
            b[i]    = static_cast<uchar>(255u);
            r[i]    = static_cast<uchar>(255u);
            continue;
        }

        char oldStegBit = steg[i];
        steg[i] = (steg[i] & ~(1)) + (cypherBits[j % cypherSize] ^ rngBits[j]);
        ++j;
        if (oldStegBit != steg[i])
            switch (i % 4)
            {
                case 1:
                    g[i]    = static_cast<uchar>(255u);
                break;
                case 2:
                    b[i]    = static_cast<uchar>(255u);
                break;
                default:
                    r[i]    = static_cast<uchar>(255u);
                    break;
            }
    }

    QImage image(reinterpret_cast<const unsigned char*>(r.constData()), ui->mediumImage->pixmap().width(), ui->mediumImage->pixmap().height(), QImage::Format_ARGB32);
    QPixmap pixie = QPixmap::fromImage(image);
    ui->rImage->setPixmap(pixie);

    image = QImage(reinterpret_cast<const unsigned char*>(g.constData()), ui->mediumImage->pixmap().width(), ui->mediumImage->pixmap().height(), QImage::Format_ARGB32);
    pixie = QPixmap::fromImage(image);
    ui->gImage->setPixmap(pixie);

    image = QImage(reinterpret_cast<const unsigned char*>(b.constData()), ui->mediumImage->pixmap().width(), ui->mediumImage->pixmap().height(), QImage::Format_ARGB32);
    pixie = QPixmap::fromImage(image);
    ui->bImage->setPixmap(pixie);

    image = QImage(reinterpret_cast<const unsigned char*>(steg.constData()), ui->mediumImage->pixmap().width(), ui->mediumImage->pixmap().height(), QImage::Format_ARGB32);
    pixie = QPixmap::fromImage(image);
    ui->stegImage->setPixmap(pixie);

    readHidden();
}

void MainWindow::readHidden()
{
    hiddenText.clear();
    QByteArray charArray;

    charArray.resize(8);
    generateBits(steg.size());
    for (quint32 i = 0u, j = 0u; i != steg.size() - 8u; ++i)
    {
        if ((i % 4u) == 3u)
            continue;

        charArray[7u - (j % 8u)] = '0' + ((steg[i] & 1) ^ rngBits[j]);
        if ((++j % 8u) == 0u)
        {
            uchar c = static_cast<char>(QString(charArray).toUInt(nullptr, 2));
            if (c == '\0')
                break;

            hiddenText.append(c);
        }
    }
    ui->hiddenText->setPlainText(QString::fromUtf8(hiddenText));
}
