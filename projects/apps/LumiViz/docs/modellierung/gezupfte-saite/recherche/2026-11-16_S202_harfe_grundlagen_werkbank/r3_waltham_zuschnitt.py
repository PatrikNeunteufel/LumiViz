"""Schneidet Waltham 2008 Abb. 4 aus der 300-dpi-Seite (nur zum Ansehen) und misst die Grauwerte an den Saitenpunkten."""
from PIL import Image
d = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen\seiten"
im = Image.open(d + r"\waltham300-2.png").convert("L")
x0, y0 = 1338, 33
# Legende: im Zuschnitt x 870-960, y 245-590 -> vergroessert ablegen
leg = im.crop((x0 + 860, y0 + 240, x0 + 980, y0 + 600))
leg.resize((leg.width * 3, leg.height * 3)).save(d + r"\waltham_legende.png")
# Grauwerte der Legende: Mitte der Skala x ~ 890, je 10 px abwaerts
print("Legende (y im Zuschnitt, Grauwert):")
for y in range(250, 560, 10):
    print(y, im.getpixel((x0 + 890, y0 + y)))
