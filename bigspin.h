#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QLineEdit>

class BigSpinPrivate;
class BigSpin : public QAbstractSpinBox
{
    Q_OBJECT
public:
    BigSpin(QWidget *parent = 0)
    {
        connect(this->lineEdit(), &QLineEdit::textEdited, this, &BigSpin::onEditFinished);
    };
    ~BigSpin() {};

    quint64 value() const
    {
        return longValue;
    };

    quint64 minimum() const
    {
        return minValue;
    };

    void setMinimum(quint64 min)
    {
        minValue = min;
        if (longValue < minValue)
            longValue = minValue;

        this->lineEdit()->setText(textFromValue(longValue));
    }

    quint64 maximum() const
    {
        return maxValue;
    };

    void setMaximum(quint64 max)
    {
        maxValue = max;
        if (longValue > maxValue)
            longValue = maxValue;

        this->lineEdit()->setText(textFromValue(longValue));
    }

    void setRange(quint64 min, quint64 max)
    {
        setMinimum(min);
        setMaximum(max);
    }

    virtual void stepBy(int steps)
    {
        auto newValue = longValue;
        if (steps < 0 && ((newValue + steps) > newValue))
            newValue = 21;
        else if (steps > 0 && ((newValue + steps) < newValue))
            newValue = std::numeric_limits<quint64>::max();
        else
            newValue += steps;
        if ((newValue < minValue) || (newValue >maxValue))
            return;

        lineEdit()->setText(textFromValue(newValue));
        setValue(newValue);
    }

protected:
    virtual QValidator::State validate(QString &input, int &pos) const
    {
        bool bValid;
        quint64 value = input.toULongLong(&bValid);
        if (!bValid)
            return QValidator::Invalid;

        if (value < minValue || value > maxValue)
            return QValidator::Invalid;

        return QValidator::Acceptable;
    }

    virtual quint64 valueFromText(const QString &text) const
    {
        return text.toLongLong();
    }

    virtual QString textFromValue(quint64 val) const
    {
        return QString::number(val);
    }

    virtual QAbstractSpinBox::StepEnabled stepEnabled() const
    {
            return StepUpEnabled | StepDownEnabled;
    }
public Q_SLOTS:
    void setValue(quint64 value)
    {
        longValue = value;
        lineEdit()->setText(textFromValue(value));
        valueChanged(value);
    }

    void onEditFinished(QString input)
    {
        int pos = 0;
        if (QValidator::Acceptable == validate(input, pos))
            setValue(valueFromText(input));
        else
        {
            if (longValue < minValue)
                longValue = minValue;
            else if (longValue > maxValue)
                longValue = maxValue;
        }
        setValue(longValue);
    }
Q_SIGNALS:
    void valueChanged(quint64 value);

private:
    quint64 minValue = std::numeric_limits<quint64>::min();
    quint64 maxValue = std::numeric_limits<quint64>::max();
    quint64 longValue = 0;

    Q_PROPERTY(quint64 minimum  READ minimum    WRITE setMinimum)
    Q_PROPERTY(quint64 maximum  READ maximum    WRITE setMaximum)
    Q_PROPERTY(quint64 value    READ value      WRITE setValue NOTIFY valueChanged USER true)
    Q_DECLARE_PRIVATE(BigSpin)
};
