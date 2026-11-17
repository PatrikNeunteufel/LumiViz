import hashlib, pathlib
d = pathlib.Path(__file__).parent / "abrufe"
for p in sorted(d.iterdir()):
    if p.is_file():
        b = p.read_bytes()
        print(f"| {p.name} | {len(b)} | {hashlib.sha256(b).hexdigest()} |")
