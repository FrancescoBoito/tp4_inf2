# Parte B — Registro de eventos con Arduino

Informática II — Francesco Boito Díaz

El programa registra detecciones de movimiento mediante dos sensores PIR conectados a un Arduino Uno. Cada evento se almacena en una lista enlazada con la zona de origen y su fecha y hora. Cada minuto, el sistema muestra los registros por el Monitor Serie y libera la memoria utilizada.

## Archivos del proyecto

- sketch.ino: código del programa.
- diagram.json: circuito y conexiones para Wokwi.
- VALIDACION.txt: detalle de las pruebas de lógica realizadas.

No se necesitan bibliotecas externas.

## Conexiones

| Sensor | Alimentación | Tierra | Salida |
| --- | --- | --- | --- |
| PIR de zona 1 | 5 V | GND | D2 |
| PIR de zona 2 | 5 V | GND | D3 |

Se eligieron D2 y D3 porque permiten utilizar interrupciones externas en el Arduino Uno R3. El código también puede utilizarse en un Nano clásico con ATmega328P, manteniendo las mismas conexiones.

## Funcionamiento

Al arrancar, el programa solicita la fecha y hora. Una vez ingresado un dato válido, habilita las interrupciones de los sensores. Se utiliza el modo RISING, por lo que cada transición de LOW a HIGH genera un evento. Si la salida del PIR permanece en HIGH, no se generan registros adicionales hasta que vuelva a LOW y ocurra otra detección.

Las rutinas de interrupción guardan únicamente la zona y el instante de detección en una cola circular auxiliar. La reserva de memoria y el manejo de la lista se realizan desde loop(), para mantener breves las interrupciones.

Cada evento se incorpora al final de una única lista enlazada mediante un nodo creado con malloc(). Los punteros primero y ultimo permiten conservar el orden de los registros y agregar nuevos nodos sin recorrer toda la lista.

Las variables compartidas con las interrupciones se declaran volatile. Además, las capturas se copian con las interrupciones deshabilitadas durante unas pocas instrucciones, para evitar lecturas incompletas.

El reloj avanza mediante millis() a partir de la fecha ingresada. La marca temporal corresponde al momento de detección, aunque el evento se procese o se imprima después.

Cada 60 segundos, el programa recorre la lista e imprime la zona, la fecha y la hora de cada evento. Luego libera los nodos con free() y deja ambos punteros en NULL, preparados para el siguiente período.

Para probar el punto B1 con un solo sensor, se puede cambiar CANTIDAD_ZONAS a 1. Con el valor 2 se ejecuta la versión completa del punto B2.

## Pruebas realizadas

En Wokwi se ingresó la fecha y hora 2026-10-24 17:00:00. Luego se activaron los sensores en el orden zona 1, zona 2 y zona 1. El informe mostró los siguientes registros:

| Evento | Zona | Hora de detección |
| --- | --- | --- |
| 1 | 1 | 17:00:06.661 |
| 2 | 2 | 17:00:10.889 |
| 3 | 1 | 17:00:15.815 |

No se informaron pérdidas por cola llena ni por falta de memoria. Durante el minuto siguiente no se activaron los sensores y el informe indicó cero eventos, sin repetir los registros anteriores.

También se realizaron pruebas automáticas de lógica sobre la validación de fechas, el cambio de mes, el orden de los eventos, el desbordamiento de millis() y el vaciado de la lista. Su alcance se detalla en VALIDACION.txt.

## Consideraciones

La memoria disponible es limitada. Si no puede crearse un nodo o se llena la cola auxiliar de 15 capturas pendientes, el programa contabiliza los eventos perdidos y los informa por Serie.

Durante la impresión, las interrupciones continúan capturando detecciones. Un evento producido cerca del cierre de un período puede aparecer en el informe siguiente, conservando su marca temporal original.

El sistema no utiliza un reloj RTC ni almacenamiento permanente. Al reiniciarlo, se pierden los registros pendientes y se debe ingresar nuevamente la fecha y hora. En un montaje físico también debe respetarse el tiempo de estabilización de los sensores PIR.


