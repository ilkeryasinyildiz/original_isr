#include "baseregister.h"
#include <QDebug>
#include <math.h>

BaseRegister::BaseRegister(const PosInt &stateSize)
{
    m_registerSize = stateSize;
    m_registerValue = BitArray(stateSize, 0);
}

void BaseRegister::loadRegisterValue(const BitArray &state)
{
    m_registerValue = state;
}

QVector<BitArray> BaseRegister::collectInvolvedStates(const BitIndexArray &involvedStateBits)
{
    PosInt rows = pow(2, involvedStateBits.size());
    PosInt cols = m_registerSize;
    QVector<BitArray> uniqueStates;

    //load first state
    BitArray reg;
    for( PosInt colIndex = 0; colIndex < m_registerSize; colIndex++ )
    {
        reg.append(0);
    }

    uniqueStates.append(reg);

    for( PosInt rowIndex = 1; rowIndex < rows; rowIndex++)
    {
        bool carry = true;

        for( PosInt colIndex = 0; colIndex < m_registerSize; colIndex++ )
        {
            bool isInvolvedBit = false;
            for( const BitIndex& bitIndex : involvedStateBits){
                if( bitIndex == colIndex){
                    isInvolvedBit = true;
                }
            }

            if (isInvolvedBit)
            {
                //is an involved bit
                //increment
                if (carry)
                {
                  if ( reg.at(colIndex) == 0 )
                  {
                    reg[colIndex] = 1;
                    carry = false;
                  }
                  else
                  {
                      reg[colIndex] = 0;
                      carry = true;
                  }
                }
            }
            else
            {
                //not involved set 0 to that column
                reg[colIndex] = 0;
            }

        }

        uniqueStates.append(reg);

    }

    return uniqueStates;
}

bool BaseRegister::fillWithString(const QString &strVal)
{
    if( strVal.length() != m_registerSize)
        return false;

    for( int i = 0; i < m_registerSize; i++)
    {
        m_registerValue[i] = QString(strVal.at(i)).toInt();
    }

    return true;
}

void BaseRegister::printRegister()
{
    QString registerContent;
    for( int i = 0; i < m_registerSize; i++)
    {
        registerContent += QString::number(m_registerValue[i]);
    }
    qDebug() << "Printing Register R(0," << "-" << m_registerSize-1 << "): " << registerContent;
}

const BitArray& BaseRegister::registerValue() const
{
    return m_registerValue;
}

void BaseRegister::clear()
{
    for( int i = 0; i < m_registerSize; i++)
    {
        m_registerValue[i] = 0;
    }
}



















