# ==============================================================================
# TestingVerknuepfung.ps1 - legt neben LumiViz.exe die Verknuepfung
# "LumiViz.exe - Testing.lnk" an: Ziel die Exe, Argument --testing,
# Arbeitsordner der Ordner der Exe.
#
# Gerufen nach jedem Bau von LumiViz.exe (CMakeLists.txt im Repo-Root, Block
# [Sichttest]; docs/ui/Composer_Konzept.md Paragraph 12.2, Naht N25).
# Von Hand:
#   powershell -NoProfile -ExecutionPolicy Bypass -File TestingVerknuepfung.ps1 -Exe <Pfad zu LumiViz.exe>
#
# Scheitert das Anlegen, endet das Skript mit 0 und einer Warnung - eine
# fehlende Verknuepfung soll keinen Bau abbrechen.
# Nur ASCII in dieser Datei: Windows PowerShell 5.1 liest sie sonst als ANSI.
# ==============================================================================
param(
    [Parameter(Mandatory = $true)][string]$Exe
)

try {
    $exePfad = [System.IO.Path]::GetFullPath($Exe)
    if (-not (Test-Path -LiteralPath $exePfad)) {
        Write-Host "[Sichttest] Warnung: $exePfad fehlt - keine Verknuepfung angelegt"
        exit 0
    }
    $ordner = Split-Path -Parent $exePfad
    $name = (Split-Path -Leaf $exePfad) + ' - Testing.lnk'
    $ziel = Join-Path $ordner $name

    $shell = New-Object -ComObject WScript.Shell
    $lnk = $shell.CreateShortcut($ziel)
    $lnk.TargetPath = $exePfad
    $lnk.Arguments = '--testing'
    $lnk.WorkingDirectory = $ordner
    $lnk.Description = 'LumiViz mit der Sichttest-Steuerung starten (--testing)'
    $lnk.Save()
    Write-Host "[Sichttest] $name"
}
catch {
    Write-Host "[Sichttest] Warnung: Verknuepfung nicht angelegt - $($_.Exception.Message)"
}
exit 0
