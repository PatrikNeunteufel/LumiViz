% S203 KQ33 a: Lautheit je Ton an der TU-Berlin-Datenbank (reflexionsarm, 32 Mikrofone).
% Je SOFA-Datei: Ton aus dem einseitigen Spektrum (inverse FFT), auf 48 kHz umgetastet, K-Bewertung nach BS.1770 (wie
% `2026-11-15_S199_pruefstand_zupf.py`, momentary_max: Fenster 0,4 s, Schritt 0,1 s, -0,691 + 10 lg),
% (a) die Leistung als Mittel ueber die 32 Mikrofone -> Momentary-Maximum,
% (b) je Mikrofon das Momentary-Maximum -> Median und Streuung ueber die Mikrofone.
% Aufruf: octave-cli lautheit.m <ordner mit *.sofa (rekursiv)> <ausgabe.tsv>
pkg load netcdf
pkg load signal
arg = argv();
ordner = arg{1};
aus = arg{2};
b1 = [1.53512485958697, -2.69169618940638, 1.19839281085285]; a1 = [1.0, -1.69065929318241, 0.73248077421585];
b2 = [1.0, -2.0, 1.0];                                        a2 = [1.0, -1.99004745483398, 0.99007225036621];
fs = 48000; n = round(0.4 * fs); h = round(0.1 * fs);
liste = dir(fullfile(ordner, '**', '*.sofa'));
fid = fopen(aus, 'w');
fprintf(fid, 'datei\tmidi\tbeschreibung\tL_summe\tL_mik_median\tL_mik_std\tdauer_s\n');
for k = 1:numel(liste)
  f = fullfile(liste(k).folder, liste(k).name);
  re = ncread(f, 'Data.Real'); im = ncread(f, 'Data.Imag');
  X = squeeze(re + 1i * im);                    % Frequenz x 32
  if size(X, 1) < size(X, 2), X = X.'; end
  x = real(ifft([X; conj(X(end-1:-1:2, :))]));  % Zeit x 32, 44,1 kHz
  x = resample(x, 160, 147);
  y = filter(b2, a2, filter(b1, a1, x));
  p = y .^ 2;
  mom = @(q) -0.691 + 10 * log10(max((q(n+1:h:end) - q(1:h:end-n)) / n));
  c = [0; cumsum(mean(p, 2))];
  L = mom(c);
  Lm = zeros(1, size(p, 2));
  for r = 1:size(p, 2)
    cr = [0; cumsum(p(:, r))];
    Lm(r) = mom(cr);
  end
  midi = ncread(f, 'MIDINote');
  bes = ncreadatt(f, '/', 'Description');
  fprintf(fid, '%s\t%d\t%s\t%.3f\t%.3f\t%.3f\t%.3f\n', liste(k).name, midi, bes, L, median(Lm), std(Lm), size(x, 1) / fs);
  if mod(k, 20) == 0, printf('%d / %d\n', k, numel(liste)); fflush(stdout); end
end
fclose(fid);
printf('fertig: %d Dateien -> %s\n', numel(liste), aus);
