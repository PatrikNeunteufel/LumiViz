"""HAL-Suche (api.archives-ouvertes.fr) nach Arbeiten zu Tuba, Euphonium, Saxhorn, Horn; Rohantworten unter abrufe/."""
import json
import os
import urllib.parse
import urllib.request

ZIEL = os.path.join(os.path.dirname(os.path.abspath(__file__)), "abrufe")
ANFRAGEN = [
    'authFullName_s:"Rémi Mattéoli"',
    'title_t:tuba',
    'title_t:(saxhorn OR euphonium)',
    'title_t:(cor AND (perce OR impédance OR embouchure))',
    'abstract_t:(tuba AND (bore OR perce))',
    'title_t:("french horn" OR "cor d\'harmonie")',
]
for i, q in enumerate(ANFRAGEN):
    url = "https://api.archives-ouvertes.fr/search/?" + urllib.parse.urlencode(
        {"q": q, "rows": 40, "fl": "title_s,authFullName_s,producedDateY_i,docType_s,uri_s,fileMain_s"})
    with urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "research-script/1.0"}), timeout=60) as r:
        d = r.read()
    with open(os.path.join(ZIEL, f"hal_suche_{i:02d}.json"), "wb") as f:
        f.write(d)
    j = json.loads(d)["response"]
    print(f"=== {q} -> {j['numFound']}")
    for doc in j["docs"]:
        print("  -", doc.get("producedDateY_i"), doc.get("docType_s"), "|", (doc.get("title_s") or [""])[0][:110], "|",
              ", ".join((doc.get("authFullName_s") or [])[:3]), "|", doc.get("uri_s"), "|", doc.get("fileMain_s", "-"))
