"""R5: Aufbau der Koerperdatei concert_harp.lvinstbody lesen (nur lesen): Schluessel und die ersten Moden."""
import json
p = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbody"
k = json.load(open(p, encoding="utf-8"))
def zeige(o, tiefe=0, name="", max_tiefe=3):
    pre = "  " * tiefe
    if isinstance(o, dict):
        print(f"{pre}{name}: dict {list(o.keys())[:20]}")
        if tiefe < max_tiefe:
            for kk, v in o.items():
                zeige(v, tiefe + 1, kk, max_tiefe)
    elif isinstance(o, list):
        print(f"{pre}{name}: list[{len(o)}] erstes: {str(o[0])[:300] if o else ''}")
        if o and isinstance(o[0], dict) and tiefe < max_tiefe:
            zeige(o[0], tiefe + 1, name + "[0]", max_tiefe)
    else:
        print(f"{pre}{name}: {str(o)[:200]}")
zeige(k)
