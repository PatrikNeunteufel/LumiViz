/**
 * @file ZipArchiv.cpp
 * @brief Ein Zip schreiben und lesen ueber miniz (Datenablage_Konzept.md, A4)
 */

#include "core/ZipArchiv.hpp"

#include <miniz.h>

#include <QFile>
#include <QSaveFile>

namespace lumi::core
{

namespace
{

void setze(QString* fehler, const QString& text)
{
    if (fehler != nullptr) *fehler = text;
}

QString grund(mz_zip_archive& zip)
{
    return QString::fromLatin1(mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
}

}  // namespace

QByteArray alsZip(const std::vector<ZipEintrag>& eintraege, QString* fehler)
{
    setze(fehler, QString());
    mz_zip_archive zip;
    mz_zip_zero_struct(&zip);
    if (mz_zip_writer_init_heap(&zip, 0, 64 * 1024) == MZ_FALSE)
    {
        setze(fehler, QStringLiteral("Could not start the archive: %1").arg(grund(zip)));
        return {};
    }
    for (const ZipEintrag& eintrag : eintraege)
    {
        const QByteArray name = eintrag.name.toUtf8();
        MZ_TIME_T zeit = static_cast<MZ_TIME_T>(
            (eintrag.geaendert.isValid() ? eintrag.geaendert : QDateTime::currentDateTime()).toSecsSinceEpoch());
        // ein leerer Eintrag braucht trotzdem einen Zeiger
        const char* daten = eintrag.daten.isEmpty() ? "" : eintrag.daten.constData();
        if (mz_zip_writer_add_mem_ex_v2(&zip, name.constData(), daten, static_cast<size_t>(eintrag.daten.size()),
                                        nullptr, 0, static_cast<mz_uint>(MZ_DEFAULT_LEVEL), 0, 0, &zeit, nullptr, 0,
                                        nullptr, 0) == MZ_FALSE)
        {
            setze(fehler, QStringLiteral("Could not add '%1': %2").arg(eintrag.name, grund(zip)));
            mz_zip_writer_end(&zip);
            return {};
        }
    }
    void* puffer = nullptr;
    size_t groesse = 0;
    if (mz_zip_writer_finalize_heap_archive(&zip, &puffer, &groesse) == MZ_FALSE)
    {
        setze(fehler, QStringLiteral("Could not finish the archive: %1").arg(grund(zip)));
        mz_zip_writer_end(&zip);
        return {};
    }
    QByteArray aus;
    if (static_cast<qint64>(groesse) <= kZipMaxBytes)
        aus = QByteArray(static_cast<const char*>(puffer), static_cast<qsizetype>(groesse));
    else
        setze(fehler, QStringLiteral("The archive would be larger than %1 MB").arg(kZipMaxBytes / (1024 * 1024)));
    mz_free(puffer);
    mz_zip_writer_end(&zip);
    return aus;
}

bool ausZip(const QByteArray& zip, std::vector<ZipEintrag>& eintraege, QString* fehler)
{
    setze(fehler, QString());
    mz_zip_archive archiv;
    mz_zip_zero_struct(&archiv);
    if (mz_zip_reader_init_mem(&archiv, zip.constData(), static_cast<size_t>(zip.size()), 0) == MZ_FALSE)
    {
        setze(fehler, QStringLiteral("Not a readable zip archive: %1").arg(grund(archiv)));
        return false;
    }
    std::vector<ZipEintrag> gelesen;
    const mz_uint anzahl = mz_zip_reader_get_num_files(&archiv);
    gelesen.reserve(anzahl);
    qint64 summe = 0;
    for (mz_uint i = 0; i < anzahl; ++i)
    {
        mz_zip_archive_file_stat stand;
        if (mz_zip_reader_file_stat(&archiv, i, &stand) == MZ_FALSE)
        {
            setze(fehler, QStringLiteral("Could not read entry %1: %2").arg(i).arg(grund(archiv)));
            mz_zip_reader_end(&archiv);
            return false;
        }
        if (stand.m_is_directory != MZ_FALSE) continue;
        // die Groesse steht im Verzeichnis des Archivs — ihr nicht blind trauen
        summe += static_cast<qint64>(stand.m_uncomp_size);
        if (stand.m_uncomp_size > static_cast<mz_uint64>(kZipMaxBytes) || summe > kZipMaxBytes)
        {
            setze(fehler,
                  QStringLiteral("The archive unpacks to more than %1 MB").arg(kZipMaxBytes / (1024 * 1024)));
            mz_zip_reader_end(&archiv);
            return false;
        }
        ZipEintrag eintrag;
        eintrag.name = QString::fromUtf8(stand.m_filename);
        eintrag.geaendert = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(stand.m_time));
        eintrag.daten.resize(static_cast<qsizetype>(stand.m_uncomp_size));
        if (stand.m_uncomp_size > 0 &&
            mz_zip_reader_extract_to_mem(&archiv, i, eintrag.daten.data(), static_cast<size_t>(eintrag.daten.size()),
                                         0) == MZ_FALSE)
        {
            setze(fehler, QStringLiteral("Could not unpack '%1': %2").arg(eintrag.name, grund(archiv)));
            mz_zip_reader_end(&archiv);
            return false;
        }
        gelesen.push_back(std::move(eintrag));
    }
    mz_zip_reader_end(&archiv);
    eintraege = std::move(gelesen);
    return true;
}

bool schreibeZip(const QString& pfad, const std::vector<ZipEintrag>& eintraege, QString* fehler)
{
    const QByteArray zip = alsZip(eintraege, fehler);
    if (zip.isEmpty()) return false;
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly) || datei.write(zip) != zip.size() || !datei.commit())
    {
        setze(fehler, QStringLiteral("Could not write '%1': %2").arg(pfad, datei.errorString()));
        return false;
    }
    return true;
}

bool leseZip(const QString& pfad, std::vector<ZipEintrag>& eintraege, QString* fehler)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly))
    {
        setze(fehler, QStringLiteral("Could not open '%1': %2").arg(pfad, datei.errorString()));
        return false;
    }
    if (datei.size() > kZipMaxBytes)
    {
        setze(fehler, QStringLiteral("'%1' is larger than %2 MB").arg(pfad).arg(kZipMaxBytes / (1024 * 1024)));
        return false;
    }
    return ausZip(datei.readAll(), eintraege, fehler);
}

}  // namespace lumi::core
