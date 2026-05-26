# API public - suita NIST SP 800-22

API JSON pentru testele de aleatorism NIST SP 800-22. Endpoint-urile ruleaza
binarele C++ reale (`nist_test` / `nist_assess`) pe server.

- Baza productie: `https://prng-nist-tests.student-dev.ro/api`
- Baza local: `http://localhost:3000/api`
- Tip continut: `application/json`

Toate testele folosesc acelasi contract: alegi testul prin slug-ul din cale
(`/api/run/{test}`), trimiti bitii (+ parametri optionali) si primesti acelasi
plic de raspuns. Lista testelor + starea lor: `GET /api/health`.

## Limite de rata

Toate endpoint-urile (cu exceptia `/health`) sunt limitate, in memorie (se reseteaza
la repornirea containerului), configurabile prin variabile de mediu. Limitele sunt
generoase (bursturile sunt permise):

| Limita | Valoare implicita | Variabila |
| --- | --- | --- |
| cereri / minut / IP | 60 | `RL_PER_MIN` |
| cereri / saptamana / IP | 2000 | `RL_PER_IP_WEEK` |
| cereri / saptamana global | 100000 | `RL_GLOBAL_WEEK` |

**Scutire (allowlist):** IP-urile din `RL_ALLOWLIST` si cele locale ocolesc complet
limitele. IP-ul clientului se ia din `X-Forwarded-For`. Fiecare raspuns include
`X-RateLimit-Limit/Remaining/Reset`; la depasire se intoarce `429` cu `Retry-After`.

## POST /api/run/{test}

Ruleaza un test pe o secventa de biti. `{test}` este slug-ul (ex. `monobit`,
`dft`, `runs`).

Corp cerere:

```json
{
  "bits": "1100100100...",   // doar caracterele 0/1 sunt pastrate; max 2.000.000 biti
  "alpha": 0.01,             // 0.0001 .. 0.5 (implicit 0.01)
  "block": 10,               // M/m pentru testele pe blocuri (optional)
  "method": "auto"           // "auto" | "fft" | "direct" (doar dft)
}
```

Raspuns `200` (plic uniform):

```json
{
  "test": "monobit",
  "n": 100,
  "p_value": 0.109599,
  "passed": true,
  "statistic": 1.6,
  "critical": 2.575829,
  "distribution": "half-normal (upper)",
  "stats": { "S_n": -16.0, "s_obs": 1.6 }
}
```

Verdictul (`passed`) se da prin **valoarea critica**: `statistic` se compara cu
`critical` (`z_{1-alpha/2}` pentru `distribution` normal/half-normal, `chi^2_{alpha,df}`
- cu `dof` - pentru chi-patrat); `p_value` ramane informatie secundara. `cusum` nu are
o valoare critica tabelata, deci ramane pe baza de p (fara campurile de mai sus).
`stats` contine cantitatile intermediare specifice testului. Testul `dft` adauga si
`"spectrum": [ {"k":0,"mag":16.0,"above":false}, ... ]` (esantionat la <= 4000 puncte).
Testele cu mai multe valori p (random excursions + varianta) adauga
`"p_values": [ {"label":"x=-4","p":0.57}, ... ]` (verdictul foloseste minimul).

Toate cele 15 teste sunt rulabile. Parametri per test:

| test | parametri | stats |
| --- | --- | --- |
| `monobit` | alpha | S_n, s_obs |
| `block-frequency` | alpha, block (M) | M, N, chi_sq |
| `runs` | alpha | V_obs, pi |
| `longest-run` | alpha | M, N, chi_sq |
| `rank` | alpha | matrices, F_32, F_31, F_rem, chi_sq |
| `dft` | alpha, method | threshold, N0, N1, d (+ spectrum) |
| `non-overlapping` | alpha, block (m) | m, M, N, mu, chi_sq |
| `overlapping` | alpha | M, N, m, chi_sq |
| `universal` | alpha | L, Q, K, f_n, expected |
| `linear-complexity` | alpha, block (M) | M, N, chi_sq |
| `serial` | alpha, block (m) | m, psi_sq_m, del_psi, del2_psi, p_value2 |
| `approx-entropy` | alpha, block (m) | m, ap_en, chi_sq |
| `cusum` | alpha | z_forward, z_reverse, p_reverse |
| `random-excursions` | alpha | J, enough_cycles (+ p_values per stare) |
| `random-excursions-variant` | alpha | J (+ p_values per stare) |

Coduri: `400` (`invalid_json`, `invalid_request`, `no_bits`, `too_many_bits`),
`404` (`unknown_test`), `501` (`not_implemented` - rezervat; momentan toate cele 15
teste sunt implementate), `429` (`rate_limited`), `500` (`binary_failed`, `run_error`),
`504` (`timeout`, peste 30 s).

Exemplu:

```bash
curl -s https://prng-nist-tests.student-dev.ro/api/run/monobit \
  -H 'Content-Type: application/json' \
  -d '{"bits":"1100100100001111110110101010001000100001011010001100001000110100110001001100011001100010100010111000"}'
```

## POST /api/assess/{test}

Analiza pe multe fluxuri (proportie de treceri + uniformitatea valorilor p) peste
fisierele NIST, pentru testul `{test}`.

Corp cerere:

```json
{
  "files": ["data.pi", "data.e"],  // doar fisiere din lista permisa
  "length": 10000,                 // biti / flux (1000 .. 1.000.000; implicit 10000)
  "maxStreams": 0,                 // 0 = toate (implicit 0)
  "alpha": 0.01,
  "block": 10                      // optional, ca la /run
}
```

Raspuns `200`: `{ "test": "...", "files": [ { "file", "bitsAvailable", "streamLength",
"streamsUsed", "bins", "uniformity", "uniformityPassed", "passCount", "sampleCount",
"proportion", "proportionMin", "proportionMax", "proportionPassed", "verdict" } ] }`.

Fisiere permise: `data.e`, `data.pi`, `data.sqrt2`, `data.sqrt3`, `data.sha1`,
`bits_nist_example.txt`. Timeout 60 s.

## GET /api/sequence?file=&offset=&len=

O felie marginita dintr-un fisier NIST (max 65536 biti), pentru previzualizare.
Raspuns: `{ "file", "totalBits", "offset", "len", "bits" }`.

## GET /api/health

Stare + capabilitati: binare prezente, configuratia limitelor, fisierele permise si
**catalogul de teste** (`{id, name, status}` pentru toate cele 15, cu `status`
`implemented` sau `planned`). Fara limita de rata.
