% S207: wie 2026-11-17_S203_kq33_tu_lautheit.m, schreibt aber je Ton das Momentary-Maximum jedes der 32 Mikrofone.
% Aufruf: octave-cli tu_je_mikrofon.m <ordner mit *.sofa> <ausgabe.tsv>
pkg load netcdf
pkg load signal
arg = argv();
ordner = arg{1};
aus = arg{2};
b1 = [1.53512485958697, -2.69169618940638, 1.19839281085285]; a1 = [1.0, -1.69065929318241, 0.73248077421585];
b2 = [1.0, -2.0, 1.0];                                        a2 = [1.0, -1.99004745483398, 0.99007225036621];
fs = 48000; n = round(0.4 * fs); h = round(0.1 * fs);
liste = dir(fullfile(ordner, '*.sofa'));
fid = fopen(aus, 'w');
for k = 1:numel(liste)
  f = fullfile(liste(k).folder, liste(k).name);
  re = ncread(f, 'Data.Real'); im = ncread(f, 'Data.Imag');
  X = squeeze(re + 1i * im);
  if size(X, 1) < size(X, 2), X = X.'; end
  x = real(ifft([X; conj(X(end-1:-1:2, :))]));
  x = resample(x, 160, 147);
  y = filter(b2, a2, filter(b1, a1, x));
  p = y .^ 2;
  mom = @(q) -0.691 + 10 * log10(max((q(n+1:h:end) - q(1:h:end-n)) / n));
  L = mom([0; cumsum(mean(p, 2))]);
  fprintf(fid, '%d\t%.3f', ncread(f, 'MIDINote'), L);
  for r = 1:size(p, 2)
    fprintf(fid, '\t%.3f', mom([0; cumsum(p(:, r))]));
  end
  fprintf(fid, '\n');
  if mod(k, 10) == 0, printf('%d / %d\n', k, numel(liste)); fflush(stdout); end
end
fclose(fid);
if k == 1 || true
  try
    pos = ncread(f, 'ReceiverPosition');
    fp = fopen([aus '.pos'], 'w'); fprintf(fp, '%s\n', mat2str(squeeze(pos), 5)); fclose(fp);
  catch
  end
end
printf('fertig: %d Dateien -> %s\n', numel(liste), aus);
