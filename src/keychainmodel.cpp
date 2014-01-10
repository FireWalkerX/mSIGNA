///////////////////////////////////////////////////////////////////////////////
//
// CoinVault
//
// keychainmodel.cpp
//
// Copyright (c) 2013 Eric Lombrozo
//
// All Rights Reserved.

#include "keychainmodel.h"

#include <QStandardItemModel>

using namespace CoinQ::Vault;
using namespace std;

KeychainModel::KeychainModel()
    : vault(NULL)
{
    QStringList columns;
    columns << tr("Key Chain Name") << tr("Type") << tr("Keys") << tr("Hash");
    setHorizontalHeaderLabels(columns);
}

void KeychainModel::setVault(CoinQ::Vault::Vault* vault)
{
    this->vault = vault;
    update();
}

void KeychainModel::update()
{
    removeRows(0, rowCount());

    if (!vault) return;

    std::vector<KeychainInfo> keychains = vault->getKeychains();
    for (auto& keychain: keychains) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(QString::fromStdString(keychain.name())));

        bool isPrivate = (keychain.type() == Keychain::PRIVATE);
        QStandardItem* typeItem = new QStandardItem(
            isPrivate  ? tr("Private") : tr("Public"));
        typeItem->setData(isPrivate, Qt::UserRole);
        row.append(typeItem);
        
        row.append(new QStandardItem(QString::number(keychain.numkeys())));
        row.append(new QStandardItem(QString::fromStdString(uchar_vector(keychain.hash()).getHex())));
        appendRow(row);
    }    
}

void KeychainModel::exportKeychain(const QString& keychainName, const QString& fileName, bool exportPrivate) const
{
    if (!vault) {
        throw std::runtime_error("No vault is loaded.");
    }

    vault->exportKeychain(keychainName.toStdString(), fileName.toStdString(), exportPrivate);
}

void KeychainModel::importKeychain(const QString& keychainName, const QString& fileName, bool& importPrivate)
{
    if (!vault) {
        throw std::runtime_error("No vault is loaded.");
    }

    vault->importKeychain(keychainName.toStdString(), fileName.toStdString(), importPrivate);
    update();
}

bool KeychainModel::exists(const QString& keychainName) const
{
    QList<QStandardItem*> items = findItems(keychainName, Qt::MatchExactly, 0);
    return !items.isEmpty();
}

QVariant KeychainModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole && index.column() == 1) {
        // Right-align numeric fields
        return Qt::AlignRight;
    }

    return QStandardItemModel::data(index, role);
}

bool KeychainModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::EditRole) {
        if (index.column() == 0) {
            // Keychain name edited.
            if (!vault) return false;

            try {
                //vault->renameKeychain(index.data().toString().toStdString(), value.toString().toStdString());
                return false;
                setItem(index.row(), index.column(), new QStandardItem(value.toString()));
                return true;
            }
            catch (const std::exception& e) {
                emit error(QString::fromStdString(e.what()));
            }
        }
        return false;
    }

    return true;
}

Qt::ItemFlags KeychainModel::flags(const QModelIndex& index) const
{
    if (index.column() == 0) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
