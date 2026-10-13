<#
.SYNOPSIS
    Traegt die Datei-Icons von LumiViz fuer den angemeldeten Benutzer in Windows ein
    (Explorer, Dateidialoge) - oder nimmt sie wieder heraus.

.DESCRIPTION
    Ohne Schalter aendert das Skript NICHTS: es zeigt nur, was es tun wuerde.

        .\register_file_icons.ps1                 nur anzeigen
        .\register_file_icons.ps1 -Apply          eintragen (eigene Endungen)
        .\register_file_icons.ps1 -Apply -Foreign eintragen, auch fremde Endungen
        .\register_file_icons.ps1 -Remove         die eigenen Eintraege entfernen

    Was eingetragen wird (nur HKCU\Software\Classes, keine Administratorrechte):

        <.endung>            (Standard) = LumiViz.<Art>
        LumiViz.<Art>        (Standard) = Beschreibung
        LumiViz.<Art>\DefaultIcon (Standard) = "<Ordner dieses Skripts>\<icon>.ico"

    Bewusst NICHT eingetragen: "Oeffnen mit LumiViz" (shell\open\command).
    LumiViz.exe nimmt heute keine Datei als Argument - ein Doppelklick im Explorer
    wuerde die App starten, ohne die Datei zu laden. Das Icon erscheint trotzdem.

    Regeln:
    - Eine Endung, die schon einem anderen Programm gehoert (ein anderer Eintrag in
      HKCU\Software\Classes\<.endung>), bleibt unberuehrt und wird gemeldet.
    - Hat der Benutzer fuer eine Endung im Explorer "Oeffnen mit ... immer" gewaehlt
      (UserChoice), gilt dessen Icon; das Skript meldet es und aendert daran nichts.
    - Audioformate werden nie eingetragen.
    - -Remove entfernt nur, was dieses Skript angelegt hat (Schluessel LumiViz.*
      und Endungen, die auf einen davon zeigen).
    - Die Icons werden mit ihrem Pfad eingetragen. Liegt das Repo oder die
      Installation spaeter woanders: -Remove, dann am neuen Ort -Apply.

    Die Liste der Endungen unten ist dieselbe wie in README.md (Tabelle oben);
    beide zusammen pflegen.
#>
[CmdletBinding()]
param(
    [switch]$Apply,
    [switch]$Remove,
    [switch]$Foreign
)

$ErrorActionPreference = 'Stop'
$iconDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$classes = 'HKCU:\Software\Classes'

# Endung -> ProgID, Beschreibung, Icon. Own = die Endung gehoert LumiViz.
$types = @(
    @{ Ext = '.lvfx';    ProgId = 'LumiViz.Preset';      Text = 'LumiViz Preset';            Icon = 'lumiviz.ico';         Own = $true  }
    @{ Ext = '.lvfx2';   ProgId = 'LumiViz.PresetStack'; Text = 'LumiViz Preset Stack';      Icon = 'lumiviz_lvfx2.ico';   Own = $true  }
    @{ Ext = '.lvkar';   ProgId = 'LumiViz.Karaoke';     Text = 'LumiViz Karaoke Preset';    Icon = 'lumiviz_karaoke.ico'; Own = $true  }
    @{ Ext = '.lvtrail'; ProgId = 'LumiViz.Trail';       Text = 'LumiViz Trail Preset';      Icon = 'lumiviz_trail.ico';   Own = $true  }
    @{ Ext = '.lvcomp';  ProgId = 'LumiViz.Composition'; Text = 'LumiViz Composition';       Icon = 'lumiviz_lvcomp.ico';  Own = $true  }
    @{ Ext = '.lvloop';  ProgId = 'LumiViz.Loop';        Text = 'LumiViz Loop';              Icon = 'lumiviz_lvloop.ico';  Own = $true  }
    @{ Ext = '.lvinst';  ProgId = 'LumiViz.Instrument';  Text = 'LumiViz Instrument';        Icon = 'lumiviz_lvinst.ico';  Own = $true  }
    @{ Ext = '.lvcontour'; ProgId = 'LumiViz.Contour';   Text = 'LumiViz Contour';           Icon = 'lumiviz_lvcontour.ico'; Own = $true }
    @{ Ext = '.lvfade';  ProgId = 'LumiViz.Fade';        Text = 'LumiViz Fade Preset';       Icon = 'lumiviz_lvfade.ico';  Own = $true  }
    @{ Ext = '.lvshape'; ProgId = 'LumiViz.CurvePiece';  Text = 'LumiViz Curve Piece Preset'; Icon = 'lumiviz_lvshape.ico'; Own = $true  }
    @{ Ext = '.lvmute';  ProgId = 'LumiViz.Mute';        Text = 'LumiViz Mute Profile';      Icon = 'lumiviz_lvmute.ico';  Own = $true  }
    @{ Ext = '.lvdata';  ProgId = 'LumiViz.Data';        Text = 'LumiViz Data Export';       Icon = 'lumiviz_lvdata.ico';  Own = $true  }
    @{ Ext = '.grad';    ProgId = 'LumiViz.Gradient';    Text = 'LumiViz Gradient';          Icon = 'lumiviz_grad.ico';    Own = $false }
    @{ Ext = '.avs';     ProgId = 'LumiViz.Avs';       Text = 'AVS Preset';                Icon = 'avs.ico';             Own = $false }
    @{ Ext = '.milk';    ProgId = 'LumiViz.MilkDrop';    Text = 'MilkDrop Preset';           Icon = 'milkdrop.ico';        Own = $false }
    @{ Ext = '.milk2';   ProgId = 'LumiViz.MilkDrop';    Text = 'MilkDrop Preset';           Icon = 'milkdrop.ico';        Own = $false }
    @{ Ext = '.lrc';     ProgId = 'LumiViz.Lyrics';      Text = 'Lyrics (LRC)';              Icon = 'lyrics_lrc.ico';      Own = $false }
)

function Get-DefaultValue([string]$path) {
    if (-not (Test-Path -LiteralPath $path)) { return $null }
    return (Get-Item -LiteralPath $path).GetValue('')
}

function Set-DefaultValue([string]$path, [string]$value) {
    if (-not (Test-Path -LiteralPath $path)) { New-Item -Path $path -Force | Out-Null }
    Set-Item -LiteralPath $path -Value $value
}

function Update-Explorer {
    # SHCNE_ASSOCCHANGED: der Explorer liest die Zuordnungen und Icons neu
    Add-Type -Namespace LumiViz -Name Shell -MemberDefinition @'
[System.Runtime.InteropServices.DllImport("shell32.dll")]
public static extern void SHChangeNotify(int eventId, uint flags, System.IntPtr item1, System.IntPtr item2);
'@ -ErrorAction SilentlyContinue
    [LumiViz.Shell]::SHChangeNotify(0x08000000, 0, [IntPtr]::Zero, [IntPtr]::Zero)
}

if ($Apply -and $Remove) { throw 'Entweder -Apply oder -Remove, nicht beides.' }
$mode = if ($Remove) { 'entfernen' } elseif ($Apply) { 'eintragen' } else { 'nur anzeigen (nichts wird geaendert)' }
Write-Host "LumiViz Datei-Icons: $mode"
Write-Host "Icon-Ordner: $iconDir"
Write-Host ''

$changed = $false
foreach ($t in $types) {
    $extKey = Join-Path $classes $t.Ext
    $progKey = Join-Path $classes $t.ProgId
    $iconPath = Join-Path $iconDir $t.Icon
    $current = Get-DefaultValue $extKey
    $userChoice = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$($t.Ext)\UserChoice"
    $hasChoice = Test-Path -LiteralPath $userChoice
    $ours = ($current -is [string]) -and $current.StartsWith('LumiViz.')

    if ($Remove) {
        if ($ours) {
            Write-Host ("{0,-10} entfernt (zeigte auf {1})" -f $t.Ext, $current)
            Remove-Item -LiteralPath $extKey -Recurse -Force
            $changed = $true
        } else {
            Write-Host ("{0,-10} nichts zu tun" -f $t.Ext)
        }
        if (Test-Path -LiteralPath $progKey) {
            Remove-Item -LiteralPath $progKey -Recurse -Force
            $changed = $true
        }
        continue
    }

    if (-not $t.Own -and -not $Foreign) {
        Write-Host ("{0,-10} uebersprungen: fremde Endung (mit -Foreign eintragen)" -f $t.Ext)
        continue
    }
    if (-not (Test-Path -LiteralPath $iconPath)) {
        Write-Host ("{0,-10} uebersprungen: {1} gibt es nicht" -f $t.Ext, $t.Icon)
        continue
    }
    if ($current -and -not $ours) {
        Write-Host ("{0,-10} uebersprungen: gehoert schon '{1}'" -f $t.Ext, $current)
        continue
    }
    $hint = if ($hasChoice) { '  (Achtung: im Explorer ist "Oeffnen mit" fest gewaehlt - dessen Icon gilt)' } else { '' }
    $state = if ($ours) { 'ist eingetragen' } else { 'noch nicht eingetragen' }
    Write-Host ("{0,-10} -> {1,-20} {2}  [{3}]{4}" -f $t.Ext, $t.ProgId, $t.Icon, $state, $hint)
    if ($Apply) {
        Set-DefaultValue $extKey $t.ProgId
        Set-DefaultValue $progKey $t.Text
        Set-DefaultValue (Join-Path $progKey 'DefaultIcon') ('"{0}"' -f $iconPath)
        $changed = $true
    }
}

Write-Host ''
if ($changed) {
    Update-Explorer
    Write-Host 'Fertig. Der Explorer wurde benachrichtigt; zeigt er noch alte Icons, einmal ab- und wieder anmelden.'
} elseif (-not $Apply -and -not $Remove) {
    Write-Host 'Nichts geaendert. Mit -Apply eintragen, mit -Remove wieder entfernen.'
} else {
    Write-Host 'Nichts zu aendern.'
}
