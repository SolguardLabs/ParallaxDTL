# ParallaxDTL

![banner](./assets/banner.png)

ParallaxDTL es un laboratorio CTF autocontenido escrito en C con tests en
JavaScript. Modela un sistema de settlement entre celdas de liquidez donde cada
celda representa un asset, una ruta operativa y una politica de salida.

El bug central esta en la reconciliacion final: el sistema comprueba que el
total global de reservas cuadre, pero no exige que la celda que paga un receipt
sea la misma celda que genero ese receipt. Un atacante puede generar una
obligacion desde su celda, pagarla con una celda compatible que contiene fondos
de terceros y liberar el surplus de la celda original para retirarlo.

## Componentes

- `src/pdtl.h`: tipos de dominio, limites del laboratorio y API publica.
- `src/pdtl_ledger.c`: mutaciones contables, settlement vulnerable e invariante
  estricto de binding.
- `src/pdtl_scenarios.c`: escenarios reproducibles expuestos por CLI.
- `src/pdtl_hash.c`: digests deterministas para celdas, receipts, rutas y
  estado.
- `src/pdtl_json.c`: serializacion JSON sin dependencias externas.
- `tests/node`: tests `node:test` que ejecutan el binario y validan salidas.
- `scripts/build.mjs`: build portable con `gcc`, `clang`, `cc`, `cl` o WSL
  `gcc` en Windows.

## Escenarios CLI

```bash
node scripts/build.mjs
./build/parallaxdtl flow
./build/parallaxdtl snapshot
./build/parallaxdtl exploit
./build/parallaxdtl strict-exploit
./build/parallaxdtl cancel
./build/parallaxdtl fee-rotation
```

En Windows sin compilador C nativo, el script usa WSL si encuentra `gcc` dentro
de WSL. Los tests usan el mismo runner generado en `build/runner.json`.

### `snapshot`

Inicializa la superficie:

- celda de fees del protocolo;
- celda origen de Alice;
- celda auxiliar de Alice;
- celda sponsor con fondos de terceros;
- celda merchant beneficiaria.

No emite receipts. Sirve como baseline para revisar `initial_supply`,
`cell_reserves`, celdas y digests.

### `flow`

Ejecuta el camino sano:

1. mueve liquidez entre celdas de Alice;
2. consolida la celda auxiliar en la celda origen;
3. emite un receipt desde la celda origen;
4. liquida el receipt desde la misma celda origen;
5. retira surplus permitido;
6. cierra la ruta.

El reporte debe tener `conservation_ok: true` y `binding_ok: true`.

### `exploit`

Ejecuta la vulnerabilidad:

1. la celda `origin` genera un receipt de `700`;
2. la celda `sponsor` paga ese receipt aunque no lo genero;
3. el ledger libera `reserved_out` de `origin`;
4. `origin` retira `980`, cuando antes del pago externo solo podia retirar
   `280`;
5. la ruta se cierra y la reconciliacion global sigue pasando.

El reporte muestra:

- `conservation_ok: true`;
- `binding_ok: false`;
- `ctf_vulnerability_triggered: true`;
- `exploit.excess_withdrawal: 700`;
- `receipts[0].paid_by_cell != receipts[0].origin_cell`.

### `strict-exploit`

Ejecuta el mismo intento contra la funcion estricta. El settlement queda
rechazado con `strict_rejection: true`, el receipt sigue `pending` y el sponsor
mantiene sus fondos. Este escenario documenta la condicion que deberia existir
en el camino vulnerable.

### `cancel`

Emite un receipt pendiente, lo cancela por timeout operativo, libera la reserva
de la celda origen, permite una retirada parcial y cierra la ruta. Cubre el
camino normal de unwind de obligaciones que nunca llegaron a settlement.

### `fee-rotation`

Actualiza la tarifa de `blue-route` de `200` a `350` bps antes de emitir un
receipt nuevo. El receipt usa la tarifa vigente, liquida con binding estricto y
acredita la fee a la tesoreria. Este escenario cubre mantenimiento de rutas sin
mezclarlo con la vulnerabilidad.

## Metricas de Ruta

El JSON incluye `routes`, una vista agregada por ruta con:

- reservas totales y disponibles;
- `reserved_out` y gross pendiente;
- receipts pendientes, liquidados y cancelados;
- presupuesto maximo de salida derivado de la politica;
- utilizacion y health score operacional;
- digest de ruta.

Estas metricas ayudan a detectar deuda pendiente, saturacion de salida y cambios
de tarifa sin tener que recomputar la superficie desde las celdas individuales.

## Comandos

```bash
npm run build
npm test
npm run test:ctf
npm run loc
npm run ci
```

No hay dependencias npm. Los tests usan solo modulos built-in de Node.

## Objetivo del CTF

El objetivo defensivo es corregir el camino vulnerable sin romper los escenarios
legitimos:

- `flow` debe seguir liquidando correctamente;
- `snapshot` debe conservar la superficie inicial;
- `strict-exploit` muestra el comportamiento esperado ante un payer externo;
- el exploit no debe poder producir `binding_ok: false` con
  `conservation_ok: true`.

La correccion minima es impedir que `pdtl_settle_receipt_vulnerable` aplique un
settlement cuando `receipt->origin_cell != paying_cell`. Una correccion mas rica
podria permitir sponsored settlement, pero el receipt tendria que vincular de
forma explicita el sponsor autorizado y la politica de salida.

## Modelo Contable

La reconciliacion global compara:

```text
sum(cell.reserve) + external_withdrawals + burned_after_genesis
==
initial_supply + minted_after_genesis
```

Ese invariante detecta perdidas o creacion de suministro, pero no detecta que
una obligacion haya sido pagada por una celda ajena. Para este laboratorio, el
invariante faltante es:

```text
for receipt in settled_receipts:
    receipt.paid_by_cell == receipt.origin_cell
```

La diferencia entre ambos invariantes es el punto de aprendizaje del CTF.

## Guia de Investigacion

Una revision defensiva util empieza por comparar `flow` y `exploit`.
Ambos llegan a `conservation_ok: true`, ambos cierran la ruta y ambos dejan el
receipt como `settled`. La diferencia importante no aparece en el total global,
sino en la relacion entre tres campos:

- `receipts[0].origin_cell`;
- `receipts[0].paid_by_cell`;
- `cells.origin.reserved_out`.

En `flow`, `paid_by_cell` coincide con `origin_cell`, por lo que liberar
`reserved_out` en la celda origen es correcto. En `exploit`, `paid_by_cell`
apunta a `sponsor`, pero el motor libera igualmente el bloqueo de `origin`.

Tambien conviene mirar los importes del objeto `exploit`:

- `legitimate_withdrawable_before`: saldo que Alice podia retirar mientras el
  receipt seguia pendiente;
- `available_after_foreign_payment`: saldo que pasa a estar disponible cuando
  el sponsor paga;
- `excess_withdrawal`: diferencia liberada por el pago externo.

El challenge no depende de carreras, overflow, parsing ambiguo ni corrupcion de
memoria. Es una vulnerabilidad de modelo: la autorizacion de quien asume la
obligacion no esta en el mismo dominio que la reconciliacion de reservas.

## Senales de Fix Correcto

Un fix minimo deberia hacer que `exploit` deje de reportar
`ctf_vulnerability_triggered: true`. Si se conserva el escenario como prueba
negativa, el resultado esperado seria parecido a `strict-exploit`: receipt
pendiente, sponsor intacto y `origin.reserved_out` todavia bloqueado.

Una implementacion con sponsors autorizados no deberia usar simplemente
`paid_by_cell != origin_cell` como rechazo universal. En ese caso, el receipt
tendria que contener el sponsor autorizado y el digest del receipt deberia
cubrir ese campo. Los tests se podrian ampliar para distinguir:

- payer origen directo;
- sponsor autorizado;
- sponsor no autorizado;
- sponsor autorizado pero con politica de salida incompatible;
- sponsor autorizado que intenta pagar despues del cierre de ruta.

## Notas Windows

PowerShell puede bloquear `npm.ps1` por politica de ejecucion. En ese caso se
pueden usar los comandos equivalentes:

```powershell
npm.cmd run build
npm.cmd test
node scripts/build.mjs
node --test tests/node/*.test.js
```

El script de build deja un `build/runner.json` para que los tests sepan si deben
ejecutar un binario nativo o un binario Linux compilado mediante WSL.
