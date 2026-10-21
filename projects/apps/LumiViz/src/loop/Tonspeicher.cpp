/**
 * @file Tonspeicher.cpp
 * @brief Loop: ein Speicher je Ton fuer das ganze Programm (Stufe 29b)
 */

#include "loop/Tonspeicher.hpp"

#include "loop/Sample.hpp"
#include "shape/Kontur.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>

#include <utility>

namespace lumi::loop
{

namespace
{

/// Ort, Groesse und Aenderungszeit einer Datei — wie `sampleDaten` sie vergleicht.
QByteArray dateiStand(const QString& datei)
{
    const QFileInfo info(datei);
    if (datei.isEmpty() || !info.isFile()) return QByteArrayLiteral("-");
    return QDir::cleanPath(info.absoluteFilePath()).toUtf8() + '|' + QByteArray::number(info.size()) + '|' +
           QByteArray::number(info.lastModified().toMSecsSinceEpoch());
}

}  // namespace

std::shared_ptr<const Klang> Tonspeicher::hole(const QString& schluessel)
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    const auto ort = m_orte.find(schluessel);
    if (ort == m_orte.end()) return nullptr;
    m_reihe.splice(m_reihe.begin(), m_reihe, ort->second);  // gerade gebraucht: nach vorn
    ++m_stand.treffer;
    return ort->second->klang;
}

void Tonspeicher::lege(const QString& schluessel, std::shared_ptr<const Klang> klang, double rechenzeitMs)
{
    if (klang == nullptr) return;
    const std::size_t bytes = klang->daten.size() * sizeof(float);
    const std::lock_guard<std::mutex> halt(m_sperre);
    if (const auto ort = m_orte.find(schluessel); ort != m_orte.end())
    {
        m_stand.bytes -= ort->second->bytes;
        m_reihe.erase(ort->second);
        m_orte.erase(ort);
    }
    m_reihe.push_front({schluessel, std::move(klang), bytes, rechenzeitMs});
    m_orte[schluessel] = m_reihe.begin();
    m_stand.bytes += bytes;
    ++m_stand.gelegt;
    haltGrenze();
}

std::optional<double> Tonspeicher::rechenzeitMs(const QString& schluessel) const
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    const auto ort = m_orte.find(schluessel);
    if (ort == m_orte.end()) return std::nullopt;
    return ort->second->rechenzeitMs;
}

void Tonspeicher::setzeGrenze(std::size_t bytes)
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    m_grenze = bytes;
    haltGrenze();
}

std::size_t Tonspeicher::grenze() const
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    return m_grenze;
}

TonspeicherStand Tonspeicher::stand() const
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    TonspeicherStand stand = m_stand;
    stand.toene = m_reihe.size();
    return stand;
}

void Tonspeicher::leere()
{
    const std::lock_guard<std::mutex> halt(m_sperre);
    m_reihe.clear();
    m_orte.clear();
    m_stand = {};
}

void Tonspeicher::haltGrenze()
{
    // hinten der am laengsten nicht gebrauchte; auch der eben gelegte geht, wenn er allein
    // ueber der Grenze liegt — wer ihn gelegt hat, haelt ihn noch
    while (m_stand.bytes > m_grenze && !m_reihe.empty())
    {
        const Eintrag& letzter = m_reihe.back();
        m_stand.bytes -= letzter.bytes;
        m_orte.erase(letzter.schluessel);
        m_reihe.pop_back();
        ++m_stand.verworfen;
    }
}

Tonspeicher& tonspeicher()
{
    static Tonspeicher eine;
    return eine;
}

QByteArray instrumentAbdruck(const Instrument& instrument)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(QJsonDocument(alsJson(instrument)).toJson(QJsonDocument::Compact));
    for (const Schicht& schicht : instrument.schichten)
    {
        if (schicht.schwingung == Schwingung::Sample)
        {
            hash.addData(QByteArrayView("|sample|"));
            hash.addData(dateiStand(sampleDatei(schicht.sample)));
        }
        else if (schicht.schwingung == Schwingung::Form)
        {
            hash.addData(QByteArrayView("|shape|"));
            shape::Zeichnung zeichnung;
            if (shape::holeZeichnung(schicht.form, zeichnung))
                hash.addData(QJsonDocument(shape::alsJson(zeichnung)).toJson(QJsonDocument::Compact));
        }
    }
    return hash.result().toHex();
}

QString tonSchluessel(const QString& tonName, int rate, const QByteArray& abdruck)
{
    return tonName + QLatin1Char('|') + QString::number(rate) + QLatin1Char('|') + QString::fromLatin1(abdruck);
}

QString klangSchluessel(const QString& klangName, const QString& datei, int rate)
{
    return klangName + QLatin1Char('|') + QString::number(rate) + QLatin1Char('|') + QString::fromUtf8(dateiStand(datei));
}

}  // namespace lumi::loop
