"""Sucht in Zenodo und GitHub nach Datensaetzen mit Bohrungen von Tuba, Euphonium, Horn.
Legt jede Rohantwort unter abrufe/ ab (Regel: was roh gelesen wird, bleibt liegen) und druckt Titel + URL."""
import json
import os
import urllib.parse
import urllib.request

ZIEL = os.path.join(os.path.dirname(os.path.abspath(__file__)), "abrufe")
os.makedirs(ZIEL, exist_ok=True)


def hole(url, name):
    req = urllib.request.Request(url, headers={"User-Agent": "research-script/1.0", "Accept": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=60) as r:
            daten = r.read()
            code = r.status
    except urllib.error.HTTPError as e:
        daten = e.read()
        code = e.code
    with open(os.path.join(ZIEL, name), "wb") as f:
        f.write(daten)
    return code, daten


ZENODO = ["tuba", "euphonium", "horn bore", "brass instrument bore", "brass input impedance",
          "french horn impedance", "saxhorn", "wagner tuba"]
GITHUB = ["tuba", "euphonium", "brass bore profile", "french horn impedance", "brass instrument geometry",
          "openwind instrument"]

for i, q in enumerate(ZENODO):
    url = "https://zenodo.org/api/records?" + urllib.parse.urlencode({"q": q, "size": 25})
    code, d = hole(url, f"zenodo_suche_{i:02d}.json")
    print(f"=== Zenodo '{q}' -> {code}")
    if code == 200:
        j = json.loads(d)
        print("   gesamt:", j.get("hits", {}).get("total"))
        for h in j.get("hits", {}).get("hits", []):
            m = h.get("metadata", {})
            print("   -", m.get("publication_date"), "|", m.get("resource_type", {}).get("title"), "|",
                  m.get("title", "")[:120], "|", h.get("links", {}).get("self_html"))

for i, q in enumerate(GITHUB):
    url = "https://api.github.com/search/repositories?" + urllib.parse.urlencode({"q": q, "per_page": 20})
    code, d = hole(url, f"github_suche_{i:02d}.json")
    print(f"=== GitHub '{q}' -> {code}")
    if code == 200:
        j = json.loads(d)
        print("   gesamt:", j.get("total_count"))
        for h in j.get("items", []):
            print("   -", h.get("full_name"), "|", (h.get("description") or "")[:100], "|", h.get("html_url"))
