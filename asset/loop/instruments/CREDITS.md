# Instruments — sources and credits

The built-in instruments under `instruments/` are LumiViz's own work. Some of their values are taken from published
measurements and models, or were set by measuring recordings through LumiViz's own model. The recordings themselves are
not shipped — only the values. Each value names its sources in the file (`sources`); the register of all sources is
`asset/loop/instrument_quellen.json` in the source tree.

<!-- Instrument sources: from here on written by make_instrument_credits.py -->

## Credits

| Source | License | Used in |
|---|---|---|
| Scavone (CCRMA, Stanford University), pages on marimba, xylophone, vibraphone, glockenspiel, tubular bells, ccrma.stanford.edu | "(c)1998-2003 CCRMA, Stanford University. All Rights Reserved." (report only) - facts used, no text | `glockenspiel` (dampingExponent); `marimba` (ratio2, ratio3); `tubular_bells` (fundamentalRatio, strikePosition); `vibraphone` (ratio2) |
| Chapman, Investigating the sound of an African thumb piano (kalimba), Canadian Acoustics 34(3), 26-27, 2006 | not checked (no notice seen, report only) | `kalimba` (ratio2, t60, torsionT60) |
| Falaize, Helie, Rhodes-like instrument, passive method, DAFx-15; journal version JSV 390 (2017) 289-309 | not checked (no notice seen, report only) | `reed_electric_piano` (torsionT60); `tine_electric_piano` (mass, strikePosition, t60, torsionT60) |
| Pfeifle, Real-Time Physical Model of a Wurlitzer and Rhodes Electric Piano, DAFx-17, Edinburgh, 2017 | not checked (no notice seen, report only) | `reed_electric_piano` (partnerRatio, ratio2); `tine_electric_piano` (partnerRatio, ratio2) |
| Rigaud, David & Daudet (2011), eq. 5 | not checked (formula) | `grand_piano` (inharmonicity) |
| J. Chabassier, M. Duruflé, Physical parameters for piano modeling, INRIA RT-0425 (2012), hal-00688679 | not checked (values from tables) | `grand_piano` (longitudinalRatio, strikePosition, stringLength, stringMass) |
| Rucz, Ulveczki, A Coupled 3D Finite Element Model of the Sound Production by Mallet Percussion Instruments, ICSV27 (IIAV), 2021 | not checked (no notice seen, report only) | `celesta` (couplingPerCritical, dampingExponent, partnerDamping, partnerGain); `glockenspiel` (couplingPerCritical, partnerDamping, partnerRatio); `kalimba` (couplingPerCritical, partnerDamping, partnerRatio); `marimba` (couplingPerCritical, dampingExponent, mass, partnerDamping, partnerGain, partnerRatio, ratio2, ratio3, t60); `tubular_bells` (couplingPerCritical, dampingExponent, partnerDamping, partnerRatio); `vibraphone` (couplingPerCritical, dampingExponent, mass, partnerDamping, partnerGain, partnerRatio); `xylophone` (couplingPerCritical, dampingExponent, mass, partnerDamping, partnerGain, partnerRatio, ratio2, ratio3, t60) |
| Salamander Grand Piano V3 (Yamaha C5) by Alexander Holm, CC BY 3.0 (creativecommons.org/licenses/by/3.0/); used for measurement | CC BY 3.0 | `grand_piano` (bridgeAngle, decay, detune, highDecay, inharmonicity) |
| Soenderbo, Real-time physical model of the Rhodes Electric Piano, master's thesis, Aalborg University, 2024 | not checked: title page "The content of this report is freely available", "publication (with reference) may only be pursued due to agreement with the author" (report only) | `reed_electric_piano` (dampingExponent, torsionT60); `tine_electric_piano` (dampingExponent, t60, torsionT60) |
| A. Stulov, D. Kartofelev (2009), eq. 9 (hammer felt, after Stulov 2005) | not checked (formulas and values) | `grand_piano` (hammerExponent, hammerHysteresis, hammerMass, hammerRelaxation, hammerStiffness) |

Also used, no credit required: LumiViz (own default or assumption) (own work).
