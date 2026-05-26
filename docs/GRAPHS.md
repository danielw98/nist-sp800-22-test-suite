# GRAPHS - generarea figurilor

Toate figurile au o **singura sursa de adevar**: scriptul Python
`docs/make_figures.py`. Sursa de adevar este *calculul* (datele), nu fisierul
imagine; din acelasi calcul se emit trei forme.

## Ce emite `make_figures.py`

Pentru fiecare figura:

1. `docs/figures/<nume>.pdf` - vectorial, inclus de referatul LaTeX
   (`\includegraphics`). Calea PDF este nativa pentru pdflatex/xelatex, fara
   dependinte externe.
2. `docs/figures/<nume>.svg` - vectorial pentru web (afisare statica, fallback).
3. `docs/figure-data/<nume>.json` - tablourile brute, redate **interactiv** in
   aplicatia web cu Plotly.

Astfel referatul primeste imagini statice, iar web-ul primeste date pe care le
deseneaza interactiv - dintr-un singur calcul. (LaTeX poate include SVG doar prin
Inkscape/shell-escape, de aceea pastram PDF pentru referat si SVG/JSON pentru web.)

`make_figures.py` este, in plus, o implementare de referinta compacta in numpy a
testului spectral (pentru verificare incrucisata cu C++).

## Figurile

| Nume | Continut | Date JSON |
| --- | --- | --- |
| `spectrum_random_vs_periodic` | spectru aleator vs periodic + pragul | `{random,periodic:{mag,above,T,n}}` |
| `example_threshold_46_vs_48` | exemplul de 100 biti, controversa 46/48 | `{mag, T, n}` (modulele nesortate) |
| `pvalue_hist_datafiles` | uniformitatea p pentru pi si e | `{pi,e:{counts,streams,streamLength}}` |
| `montecarlo_pvalue_uniformity` | distributia N1 si p sub H0 (1M simulari PCG64) | `{n,trials,N0,meanN1,empSd,nistSd,ratio,varD,fracPlt01,n1Hist:{lo,counts},pHist}` |

## Cum se regenereaza

```bash
python docs/make_figures.py     # scrie figures/*.{pdf,svg} + figure-data/*.json
cd web && pnpm gen              # copiaza SVG + JSON in web/{public,data-static}
```

(Sursele LaTeX ale referatului se compileaza separat, local, cu `latexmk main.tex`
in `docs/`; ele nu sunt incluse in repo - vezi `.gitignore`.)

## Static (referat) vs interactiv (web)

- Referat: `docs/chapters/*.tex` includ `figures/<nume>.pdf`.
- Web: paginile importa `data-static/figures/<nume>.json` si deseneaza cu
  componentele Plotly (`web/components/BarChart.tsx`, `SpectrumChart.tsx`).
  Spectrele live din `/runner` si `/sequences` vin din binar (JSON `--spectrum`),
  nu din aceste figuri - acelea sunt graficele "din lucrare".

## Cum adaugi o figura

1. Scrie o functie in `docs/make_figures.py` care: calculeaza datele, apeleaza
   `save_fig(fig, "nume")` (PDF+SVG) si `dump_data("nume", obj)` (JSON).
2. Adaug-o in `main()`.
3. In referat: `\includegraphics{figures/nume.pdf}` in capitolul potrivit.
4. In web: importa `data-static/figures/nume.json` intr-o pagina si red-o cu Plotly.
5. Ruleaza `python docs/make_figures.py`, recompileaza referatul, `pnpm gen`.

## Date

Histogramele uniformitatii folosesc fisierele NIST din `data/` (pi, e). Simularea
Monte-Carlo genereaza secvente aleatoare (numpy, samanta fixa pentru
reproductibilitate). Numerele cheie (sigma empiric ~7.17 vs NIST ~6.97, Var(d)~1.056,
rata de respingere ~0.012) sunt tiparite de script si citate in referat.
