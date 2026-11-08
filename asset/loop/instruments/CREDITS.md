# Instruments — sources and credits

The built-in instruments under `instruments/` are LumiViz's own work. Some of their values are taken from published
measurements and models, or were set by measuring recordings through LumiViz's own model. The recordings themselves are
not shipped — only the values. Each value names its sources in the file (`sources`); the register of all sources is
`asset/loop/instrument_quellen.json` in the source tree.

<!-- Instrument sources: from here on written by make_instrument_credits.py -->

## Credits

| Source | License | Used in |
|---|---|---|
| Rigaud, David & Daudet (2011), eq. 5 | not checked (formula) | `grand_piano` (inharmonicity) |
| J. Chabassier, M. Duruflé, Physical parameters for piano modeling, INRIA RT-0425 (2012), hal-00688679 | not checked (values from tables) | `grand_piano` (longitudinalRatio, strikePosition, stringLength, stringMass) |
| Salamander Grand Piano V3 (Yamaha C5) by Alexander Holm, CC BY 3.0 (creativecommons.org/licenses/by/3.0/); used for measurement | CC BY 3.0 | `grand_piano` (bridgeAngle, decay, detune, highDecay, inharmonicity) |
| A. Stulov, D. Kartofelev (2009), eq. 9 (hammer felt, after Stulov 2005) | not checked (formulas and values) | `grand_piano` (hammerExponent, hammerHysteresis, hammerMass, hammerRelaxation, hammerStiffness) |

Also used, no credit required: LumiViz (own default or assumption) (own work).
