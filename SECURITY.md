# Security Policy

ParallaxDTL es un CTF local. No use este codigo como motor financiero ni como
referencia productiva sin redisenar el modelo de autorizacion.

## Alcance del laboratorio

El laboratorio modela un bug intencional:

- el settlement vulnerable acepta un payer compatible pero no vinculado;
- la reconciliacion global pasa aunque el receipt haya sido pagado por una
  celda de terceros;
- el escenario `strict-exploit` muestra una validacion de binding esperada.

## Reporte de hallazgos

Para este repositorio local, documenta hallazgos con:

- escenario reproducible;
- salida JSON relevante;
- invariante violado;
- propuesta de fix;
- test de regresion.

## No objetivos

Este CTF no implementa criptografia real, firmas, red, persistencia, permisos
multiusuario ni integracion con cadenas. Los digests son deterministas para
tests y trazabilidad, no para seguridad criptografica.
